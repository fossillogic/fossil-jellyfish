/*
 * -----------------------------------------------------------------------------
 * Project: Fossil Logic
 *
 * This file is part of the Fossil Logic project, which aims to develop high-
 * performance, cross-platform applications and libraries. The code contained
 * herein is subject to the terms and conditions defined in the project license.
 *
 * Author: Michael Gene Brockus (Dreamer)
 *
 * Copyright (C) 2024 Fossil Logic. All rights reserved.
 * -----------------------------------------------------------------------------
 */
#include "fossil/ai/jellyfish.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

void fossil_jellyfish_hash(const char *input, const char *output, uint8_t *hash_out) {
    const uint32_t FNV_PRIME = 0x01000193;
    uint32_t hash = 0x811c9dc5;
    size_t in_len = strlen(input);
    size_t out_len = strlen(output);

    // Mix in lengths
    hash ^= in_len;
    hash *= FNV_PRIME;
    hash ^= out_len;
    hash *= FNV_PRIME;

    // Mix input
    for (size_t i = 0; i < in_len; ++i) {
        hash ^= (uint8_t)input[i];
        hash *= FNV_PRIME;
        hash ^= (hash >> 5);
    }

    // Mix output
    for (size_t i = 0; i < out_len; ++i) {
        hash ^= (uint8_t)output[i];
        hash *= FNV_PRIME;
        hash ^= (hash >> 5);
    }

    // Final mix
    hash ^= (hash << 7);
    hash ^= (hash >> 3);

    // Spread into output
    uint32_t h = hash;
    for (size_t i = 0; i < FOSSIL_JELLYFISH_HASH_SIZE; ++i) {
        h ^= (h >> 13);
        h *= FNV_PRIME;
        h ^= (h << 11);
        hash_out[i] = (uint8_t)((h >> (8 * (i % 4))) & 0xFF);
    }
}

void fossil_jellyfish_init(fossil_jellyfish_chain *chain) {
    chain->count = 0;
    memset(chain->memory, 0, sizeof(chain->memory));
}

void fossil_jellyfish_learn(fossil_jellyfish_chain *chain, const char *input, const char *output) {
    // Step 1: Check if input-output pair already exists
    for (size_t i = 0; i < FOSSIL_JELLYFISH_MAX_MEM; ++i) {
        fossil_jellyfish_block *block = &chain->memory[i];
        if (!block->valid) continue;
        if (strncmp(block->input, input, FOSSIL_JELLYFISH_INPUT_SIZE) == 0 &&
            strncmp(block->output, output, FOSSIL_JELLYFISH_OUTPUT_SIZE) == 0) {
            // Already learned: reinforce instead
            block->confidence += 0.1f;
            if (block->confidence > 1.0f) block->confidence = 1.0f;
            block->usage_count += 1;
            block->timestamp = (uint64_t)time(NULL);
            return;
        }
    }

    // Step 2: Try to find an unused (invalid) slot to reuse
    for (size_t i = 0; i < FOSSIL_JELLYFISH_MAX_MEM; ++i) {
        fossil_jellyfish_block *block = &chain->memory[i];
        if (!block->valid) {
            // Reuse this slot
            strncpy(block->input, input, FOSSIL_JELLYFISH_INPUT_SIZE - 1);
            block->input[FOSSIL_JELLYFISH_INPUT_SIZE - 1] = '\0';

            strncpy(block->output, output, FOSSIL_JELLYFISH_OUTPUT_SIZE - 1);
            block->output[FOSSIL_JELLYFISH_OUTPUT_SIZE - 1] = '\0';

            block->timestamp = (uint64_t)time(NULL);
            block->valid = 1;
            block->confidence = 1.0f;
            block->usage_count = 0;

            fossil_jellyfish_hash(input, output, block->hash);

            chain->count += 1;
            return;
        }
    }

    // Step 3: All slots full, run cleanup
    fossil_jellyfish_cleanup(chain);

    // Step 4: Try again to find a reusable slot after cleanup
    for (size_t i = 0; i < FOSSIL_JELLYFISH_MAX_MEM; ++i) {
        fossil_jellyfish_block *block = &chain->memory[i];
        if (!block->valid) {
            strncpy(block->input, input, FOSSIL_JELLYFISH_INPUT_SIZE - 1);
            block->input[FOSSIL_JELLYFISH_INPUT_SIZE - 1] = '\0';

            strncpy(block->output, output, FOSSIL_JELLYFISH_OUTPUT_SIZE - 1);
            block->output[FOSSIL_JELLYFISH_OUTPUT_SIZE - 1] = '\0';

            block->timestamp = (uint64_t)time(NULL);
            block->valid = 1;
            block->confidence = 1.0f;
            block->usage_count = 0;

            fossil_jellyfish_hash(input, output, block->hash);

            chain->count += 1;
            return;
        }
    }
}

const char* fossil_jellyfish_reason(fossil_jellyfish_chain *chain, const char *input) {
    for (size_t i = 0; i < chain->count; ++i) {
        if (chain->memory[i].valid && strncmp(chain->memory[i].input, input, FOSSIL_JELLYFISH_INPUT_SIZE) == 0) {
            chain->memory[i].usage_count++;
            if (chain->memory[i].confidence < 1.0f)
                chain->memory[i].confidence += 0.05f;
            return chain->memory[i].output;
        }
    }
    return "Unknown";
}

// Removes blocks that are older and marked invalid (or just garbage collect oldest)
void fossil_jellyfish_cleanup(fossil_jellyfish_chain *chain) {
    size_t new_count = 0;
    for (size_t i = 0; i < FOSSIL_JELLYFISH_MAX_MEM; ++i) {
        if (!chain->memory[i].valid) continue;
        if (chain->memory[i].confidence < 0.05f) {
            chain->memory[i].valid = 0;
        } else {
            new_count++;
        }
    }
    chain->count = new_count;
}

void fossil_jellyfish_dump(const fossil_jellyfish_chain *chain) {
    for (size_t i = 0; i < chain->count; ++i) {
        printf("Block %llu:\n", (unsigned long long)i);
        printf("  Input: %s\n", chain->memory[i].input);
        printf("  Output: %s\n", chain->memory[i].output);
        printf("  Time: %llu\n", (unsigned long long)chain->memory[i].timestamp);
        printf("  Hash: ");
        for (int j = 0; j < FOSSIL_JELLYFISH_HASH_SIZE; ++j) {
            printf("%02x", chain->memory[i].hash[j]);
        }
        printf("\n");
    }
}

int fossil_jellyfish_save(const fossil_jellyfish_chain *chain, const char *filepath) {
    FILE *fp = fopen(filepath, "wb");
    if (!fp) return 0;

    fwrite(&chain->count, sizeof(size_t), 1, fp);
    fwrite(chain->memory, sizeof(fossil_jellyfish_block), chain->count, fp);
    fclose(fp);
    return 1;
}

int fossil_jellyfish_load(fossil_jellyfish_chain *chain, const char *filepath) {
    FILE *fp = fopen(filepath, "rb");
    if (!fp) return 0;

    fread(&chain->count, sizeof(size_t), 1, fp);
    if (chain->count > FOSSIL_JELLYFISH_MAX_MEM) chain->count = FOSSIL_JELLYFISH_MAX_MEM;

    fread(chain->memory, sizeof(fossil_jellyfish_block), chain->count, fp);
    fclose(fp);
    return 1;
}

static int fossil_jellyfish_similarity(const char *a, const char *b) {
    int cost = 0;
    size_t i = 0, j = 0;

    while (a[i] && b[j]) {
        char ac = a[i];
        char bc = b[j];

        // case-insensitive match
        if (ac >= 'A' && ac <= 'Z') ac += 32;
        if (bc >= 'A' && bc <= 'Z') bc += 32;

        if (ac != bc) {
            cost++;
        }
        i++;
        j++;
    }

    // Penalty for remaining characters
    while (a[i++]) cost++;
    while (b[j++]) cost++;

    return cost;
}

const char* fossil_jellyfish_reason_fuzzy(fossil_jellyfish_chain *chain, const char *input) {
    int best_score = 1000;
    const char *best_output = "Unknown";

    for (size_t i = 0; i < chain->count; ++i) {
        if (!chain->memory[i].valid) continue;

        int score = fossil_jellyfish_similarity(input, chain->memory[i].input);
        if (score == 0) return chain->memory[i].output; // Exact match
        if (score < best_score) {
            best_score = score;
            best_output = chain->memory[i].output;
        }
    }

    // impose a fuzzy threshold
    if (best_score > (int)(strlen(input) / 2)) {
        return "Unknown";
    }

    return best_output;
}

void fossil_jellyfish_decay_confidence(fossil_jellyfish_chain *chain, float decay_rate) {
    const float MIN_CONFIDENCE = 0.05f;

    for (size_t i = 0; i < chain->count; ++i) {
        if (!chain->memory[i].valid) continue;

        // Apply exponential decay
        chain->memory[i].confidence *= (1.0f - decay_rate);

        // Clamp to zero
        if (chain->memory[i].confidence < 0.0f) {
            chain->memory[i].confidence = 0.0f;
        }

        // Invalidate if confidence too low
        if (chain->memory[i].confidence < MIN_CONFIDENCE) {
            chain->memory[i].valid = 0;
        }
    }
}

const char* fossil_jellyfish_reason_chain(fossil_jellyfish_chain *chain, const char *input, int depth) {
    const char *current = input;
    const char *last_valid = input;

    for (int i = 0; i < depth; ++i) {
        const char *next = fossil_jellyfish_reason_fuzzy(chain, current);

        if (strcmp(next, "Unknown") == 0 || strcmp(next, current) == 0) {
            break;
        }

        last_valid = next;
        current = next;
    }

    return last_valid;
}

int fossil_jellyfish_mind_load_model(fossil_jellyfish_mind *mind, const char *filepath, const char *name) {
    if (mind->model_count >= FOSSIL_JELLYFISH_MAX_MODELS) return 0;

    fossil_jellyfish_chain *target = &mind->models[mind->model_count];
    if (!fossil_jellyfish_load(target, filepath)) return 0;

    strncpy(mind->model_names[mind->model_count], name, 63);
    mind->model_names[mind->model_count][63] = '\0';
    mind->model_count++;
    return 1;
}

const char* fossil_jellyfish_mind_reason(fossil_jellyfish_mind *mind, const char *input) {
    const char *best_output = "Unknown";
    float best_confidence = 0.0f;

    for (size_t i = 0; i < mind->model_count; ++i) {
        fossil_jellyfish_chain *model = &mind->models[i];
        for (size_t j = 0; j < model->count; ++j) {
            fossil_jellyfish_block *b = &model->memory[j];
            if (!b->valid) continue;
            if (strncmp(b->input, input, FOSSIL_JELLYFISH_INPUT_SIZE) == 0) {
                if (b->confidence > best_confidence) {
                    best_output = b->output;
                    best_confidence = b->confidence;
                }
            }
        }
    }
    return best_output;
}

size_t fossil_jellyfish_tokenize(const char *input, char tokens[][FOSSIL_JELLYFISH_TOKEN_SIZE], size_t max_tokens) {
    size_t token_count = 0;
    size_t len = strlen(input);
    size_t i = 0;

    while (i < len && token_count < max_tokens) {
        // Skip leading non-alphanum
        while (i < len && !isalnum(input[i])) i++;
        if (i >= len) break;

        // Extract token
        size_t t = 0;
        while (i < len && isalnum(input[i]) && t < FOSSIL_JELLYFISH_TOKEN_SIZE - 1) {
            tokens[token_count][t++] = tolower(input[i++]);
        }
        tokens[token_count][t] = '\0';
        token_count++;
    }

    return token_count;
}

const fossil_jellyfish_block *fossil_jellyfish_best_memory(const fossil_jellyfish_chain *chain) {
    const fossil_jellyfish_block *best = NULL;
    float best_score = 0.0f;

    for (size_t i = 0; i < chain->count; ++i) {
        const fossil_jellyfish_block *b = &chain->memory[i];
        if (!b->valid) continue;
        if (b->confidence > best_score) {
            best_score = b->confidence;
            best = b;
        }
    }

    return best;
}

float fossil_jellyfish_knowledge_coverage(const fossil_jellyfish_chain *chain) {
    if (chain->count == 0) return 0.0f;

    size_t valid = 0;
    for (size_t i = 0; i < chain->count; ++i) {
        if (chain->memory[i].valid) valid++;
    }

    return (float)valid / (float)chain->count;
}

int fossil_jellyfish_detect_conflict(const fossil_jellyfish_chain *chain, const char *input, const char *output) {
    for (size_t i = 0; i < chain->count; ++i) {
        const fossil_jellyfish_block *b = &chain->memory[i];
        if (!b->valid) continue;
        if (strncmp(b->input, input, FOSSIL_JELLYFISH_INPUT_SIZE) == 0) {
            if (strncmp(b->output, output, FOSSIL_JELLYFISH_OUTPUT_SIZE) != 0) {
                return 1; // conflicting output
            }
        }
    }
    return 0;
}

void fossil_jellyfish_reflect(const fossil_jellyfish_chain *chain) {
    printf("== Jellyfish Self-Reflection ==\n");
    printf("Total Memories: %zu\n", chain->count);
    printf("Valid Memories: %zu\n", (size_t)(fossil_jellyfish_knowledge_coverage(chain) * chain->count));

    const fossil_jellyfish_block *best = fossil_jellyfish_best_memory(chain);
    if (best) {
        printf("Strongest Memory:\n");
        printf("  Input : %s\n", best->input);
        printf("  Output: %s\n", best->output);
        printf("  Confidence: %.2f\n", best->confidence);
        printf("  Usage Count: %u\n", best->usage_count);
    } else {
        printf("No confident memories yet.\n");
    }
    printf("================================\n");
}
