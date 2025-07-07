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
    if (chain->count >= FOSSIL_JELLYFISH_MAX_MEM) {
        fossil_jellyfish_cleanup(chain);
    }

    fossil_jellyfish_block *block = &chain->memory[chain->count++];
    strncpy(block->input, input, FOSSIL_JELLYFISH_INPUT_SIZE - 1);
    strncpy(block->output, output, FOSSIL_JELLYFISH_OUTPUT_SIZE - 1);
    block->timestamp = (uint64_t)time(NULL);
    block->valid = 1;
    block->confidence = 1.0f;
    block->usage_count = 0;

    fossil_jellyfish_hash(input, output, block->hash);
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
    for (size_t i = 0; i < chain->count; ++i) {
        if (chain->memory[i].valid && chain->memory[i].confidence >= 0.05f) {
            chain->memory[new_count++] = chain->memory[i];
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

int fossil_jellyfish_export_fish(const fossil_jellyfish_chain *chain, const char *filepath) {
    FILE *fp = fopen(filepath, "w");
    if (!fp) return 0;

    for (size_t i = 0; i < chain->count; ++i) {
        const fossil_jellyfish_block *b = &chain->memory[i];  // ✅ FIXED
        if (!b->valid) continue;

        fprintf(fp, "[FISH_BLOCK]\n");
        fprintf(fp, "INPUT: %s\n", b->input);
        fprintf(fp, "OUTPUT: %s\n", b->output);
        fprintf(fp, "CONFIDENCE: %.3f\n", b->confidence);
        fprintf(fp, "USAGE: %u\n", b->usage_count);
        fprintf(fp, "TIME: %llu\n", (unsigned long long)b->timestamp);
        fprintf(fp, "HASH: ");
        for (int j = 0; j < FOSSIL_JELLYFISH_HASH_SIZE; ++j)
            fprintf(fp, "%02x", b->hash[j]);
        fprintf(fp, "\n[/FISH_BLOCK]\n\n");
    }

    fclose(fp);
    return 1;
}

static void strip_newline(char *s) {
    size_t len = strlen(s);
    if (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r'))
        s[len - 1] = '\0';
}

int fossil_jellyfish_import_fish(fossil_jellyfish_chain *chain, const char *filepath) {
    FILE *fp = fopen(filepath, "r");
    if (!fp) return 0;

    char line[256];
    fossil_jellyfish_block temp = {0};
    int in_block = 0;

    while (fgets(line, sizeof(line), fp)) {
        strip_newline(line);

        if (strcmp(line, "[FISH_BLOCK]") == 0) {
            memset(&temp, 0, sizeof(temp));
            in_block = 1;
        } else if (strcmp(line, "[/FISH_BLOCK]") == 0 && in_block) {
            if (chain->count < FOSSIL_JELLYFISH_MAX_MEM) {
                chain->memory[chain->count++] = temp;
            }
            in_block = 0;
        } else if (in_block) {
            if (strncmp(line, "INPUT: ", 7) == 0)
                strncpy(temp.input, line + 7, FOSSIL_JELLYFISH_INPUT_SIZE - 1);
            else if (strncmp(line, "OUTPUT: ", 8) == 0)
                strncpy(temp.output, line + 8, FOSSIL_JELLYFISH_OUTPUT_SIZE - 1);
            else if (strncmp(line, "CONFIDENCE: ", 12) == 0)
                temp.confidence = (float)atof(line + 12);
            else if (strncmp(line, "USAGE: ", 7) == 0)
                temp.usage_count = (uint32_t)atoi(line + 7);
            else if (strncmp(line, "TIME: ", 6) == 0)
                temp.timestamp = (uint64_t)atoll(line + 6);
            else if (strncmp(line, "HASH: ", 6) == 0) {
                const char *h = line + 6;
                for (int i = 0; i < FOSSIL_JELLYFISH_HASH_SIZE && h[i * 2]; ++i) {
                    unsigned int byte;
                    sscanf(h + i * 2, "%02x", &byte);
                    temp.hash[i] = (uint8_t)byte;
                }
            }
            temp.valid = 1;
        }
    }

    fclose(fp);
    return 1;
}

