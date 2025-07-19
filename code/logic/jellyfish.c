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
#include <inttypes.h>
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

static void skip_whitespace(const char **ptr) {
    while (isspace(**ptr)) (*ptr)++;
}

static bool match_key(const char **ptr, const char *key) {
    skip_whitespace(ptr);
    size_t len = strlen(key);
    if (strncmp(*ptr, key, len) == 0) {
        *ptr += len;
        return true;
    }
    return false;
}

static bool parse_string(const char **ptr, char *out, size_t maxlen) {
    skip_whitespace(ptr);
    if (**ptr != '"') return false;
    (*ptr)++;
    size_t i = 0;
    while (**ptr && **ptr != '"' && i < maxlen - 1) {
        if (**ptr == '\\') (*ptr)++; // Skip escape char
        out[i++] = *(*ptr)++;
    }
    if (**ptr != '"') return false;
    (*ptr)++;
    out[i] = '\0';
    return true;
}

static bool parse_hash(const char **ptr, uint8_t *hash_out) {
    char hexstr[FOSSIL_JELLYFISH_HASH_SIZE * 2 + 1] = {0};
    if (!parse_string(ptr, hexstr, sizeof(hexstr))) return false;

    if (strlen(hexstr) != FOSSIL_JELLYFISH_HASH_SIZE * 2) return false;

    for (size_t i = 0; i < FOSSIL_JELLYFISH_HASH_SIZE; ++i) {
        char byte[3] = { hexstr[i * 2], hexstr[i * 2 + 1], 0 };
        hash_out[i] = (uint8_t)strtoul(byte, NULL, 16);
    }
    return true;
}

static bool parse_number(const char **ptr, double *out_d, uint64_t *out_u64, int *out_i, uint32_t *out_u32) {
    skip_whitespace(ptr);
    char buffer[64];
    size_t i = 0;
    while ((isdigit(**ptr) || **ptr == '.' || **ptr == '-') && i < sizeof(buffer) - 1)
        buffer[i++] = *(*ptr)++;
    buffer[i] = '\0';

    if (out_d) *out_d = atof(buffer);
    if (out_u64) *out_u64 = strtoull(buffer, NULL, 10);
    if (out_i) *out_i = atoi(buffer);
    if (out_u32) *out_u32 = (uint32_t)strtoul(buffer, NULL, 10);

    return i > 0;
}

int fossil_jellyfish_load(fossil_jellyfish_chain *chain, const char *filepath) {
    FILE *fp = fopen(filepath, "rb");
    if (!fp) return 0;

    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *data = malloc(fsize + 1);
    if (!data) {
        fclose(fp);
        return 0;
    }

    fread(data, 1, fsize, fp);
    data[fsize] = '\0';
    fclose(fp);

    const char *ptr = data;
    skip_whitespace(&ptr);

    if (!match_key(&ptr, "{\"signature\":")) { free(data); return 0; }

    char sig[8];
    if (!parse_string(&ptr, sig, sizeof(sig)) || strcmp(sig, "JFS1") != 0) {
        free(data);
        return 0;
    }

    if (!match_key(&ptr, ",\"blocks\":[")) { free(data); return 0; }

    size_t count = 0;
    bool ok = true;

    while (*ptr && *ptr != ']') {
        if (count >= FOSSIL_JELLYFISH_MAX_MEM) break;

        fossil_jellyfish_block *b = &chain->memory[count];
        memset(b, 0, sizeof(*b));

        if (!match_key(&ptr, "{")) { ok = false; break; }

        if (!match_key(&ptr, "\"input\":")) { ok = false; break; }
        if (!parse_string(&ptr, b->input, sizeof(b->input))) { ok = false; break; }

        if (!match_key(&ptr, ",\"output\":")) { ok = false; break; }
        if (!parse_string(&ptr, b->output, sizeof(b->output))) { ok = false; break; }

        if (!match_key(&ptr, ",\"hash\":")) { ok = false; break; }
        if (!parse_hash(&ptr, b->hash)) { ok = false; break; }

        if (!match_key(&ptr, ",\"timestamp\":")) { ok = false; break; }
        if (!parse_number(&ptr, NULL, &b->timestamp, NULL, NULL)) { ok = false; break; }

        if (!match_key(&ptr, ",\"valid\":")) { ok = false; break; }
        if (!parse_number(&ptr, NULL, NULL, &b->valid, NULL)) { ok = false; break; }

        if (!match_key(&ptr, ",\"confidence\":")) { ok = false; break; }
        if (!parse_number(&ptr, &b->confidence, NULL, NULL, NULL)) { ok = false; break; }

        if (!match_key(&ptr, ",\"usage_count\":")) { ok = false; break; }
        if (!parse_number(&ptr, NULL, NULL, NULL, &b->usage_count)) { ok = false; break; }

        if (!match_key(&ptr, "}")) { ok = false; break; }

        count++;
        skip_whitespace(&ptr);
        if (*ptr == ',') ptr++;
    }

    free(data);
    chain->count = ok ? count : 0;
    return ok ? 1 : 0;
}

static void hex_encode(const uint8_t *hash, size_t len, char *out, size_t out_size) {
    const char *hex = "0123456789abcdef";
    if (out_size < len * 2 + 1) return;
    for (size_t i = 0; i < len; ++i) {
        out[i * 2]     = hex[(hash[i] >> 4) & 0xF];
        out[i * 2 + 1] = hex[hash[i] & 0xF];
    }
    out[len * 2] = '\0';
}

int fossil_jellyfish_save(const fossil_jellyfish_chain *chain, const char *filepath) {
    FILE *fp = fopen(filepath, "wb");
    if (!fp) return 0;

    fprintf(fp, "{\n  \"signature\": \"JFS1\",\n  \"blocks\": [\n");

    for (size_t i = 0; i < chain->count; ++i) {
        const fossil_jellyfish_block *b = &chain->memory[i];

        // Escape input/output strings (basic)
        char input_escaped[FOSSIL_JELLYFISH_INPUT_SIZE * 2] = {0};
        char output_escaped[FOSSIL_JELLYFISH_OUTPUT_SIZE * 2] = {0};

        char *dst = input_escaped;
        // Escape input
        char *dst = input_escaped;
        for (const char *src = b->input; *src && (size_t)(dst - input_escaped) < sizeof(input_escaped) - 2; ++src) {
            if (*src == '"' || *src == '\\') *dst++ = '\\';
            *dst++ = *src;
        }
        *dst = '\0';
        
        // Escape output
        dst = output_escaped;
        for (const char *src = b->output; *src && (size_t)(dst - output_escaped) < sizeof(output_escaped) - 2; ++src) {
            if (*src == '"' || *src == '\\') *dst++ = '\\';
            *dst++ = *src;
        }
        *dst = '\0';

        char hash_hex[FOSSIL_JELLYFISH_HASH_SIZE * 2 + 1];
        hex_encode(b->hash, FOSSIL_JELLYFISH_HASH_SIZE, hash_hex, sizeof(hash_hex));

        fprintf(fp,
            "    {\n"
            "      \"input\": \"%s\",\n"
            "      \"output\": \"%s\",\n"
            "      \"hash\": \"%s\",\n"
            "      \"timestamp\": %" PRIu64 ",\n"
            "      \"valid\": %d,\n"
            "      \"confidence\": %.6f,\n"
            "      \"usage_count\": %u\n"
            "    }%s\n",
            input_escaped, output_escaped, hash_hex, b->timestamp, b->valid, b->confidence, b->usage_count,
            (i < chain->count - 1) ? "," : "");
    }

    fprintf(fp, "  ]\n}\n");
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
    printf("Total Memories: %llu\n", (unsigned long long)chain->count);
    printf("Valid Memories: %llu\n", (unsigned long long)(fossil_jellyfish_knowledge_coverage(chain) * chain->count));

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

static void strip_quotes(char *str) {
    size_t len = strlen(str);
    if (len >= 2 && (str[0] == '\'' || str[0] == '"') && str[len - 1] == str[0]) {
        memmove(str, str + 1, len - 2);
        str[len - 2] = '\0';
    }
}

int fossil_jellyfish_parse_jellyfish_file(const char *filepath, fossil_jellyfish_mindset *out_mindsets, int max_mindsets) {
    FILE *fp = fopen(filepath, "r");
    if (!fp) return 0;

    char line[512];
    fossil_jellyfish_mindset *current = NULL;
    int mindset_count = 0;

    while (fgets(line, sizeof(line), fp)) {
        char *trim = line;
        while (*trim && isspace(*trim)) trim++;

        // Skip blank lines
        if (*trim == '\0' || *trim == '\n') continue;

        // Handle comments (lines starting with #, excluding logic tags)
        if (*trim == '#' && trim[1] != ':') continue;

        // Handle logic tags like #:taint-check or #:bootstrap
        if (strncmp(trim, "#:", 2) == 0 && current) {
            char tag[64] = {0};
            sscanf(trim + 2, "%63s", tag);  // Skip "#:"
            if (strlen(tag) > 0 && current->tag_count < FOSSIL_JELLYFISH_MAX_TAGS) {
                strncpy(current->tags[current->tag_count++], tag, 63);
            }
            continue;
        }

        if (strncmp(trim, "mindset(", 8) == 0) {
            if (mindset_count >= max_mindsets) break;
            current = &out_mindsets[mindset_count++];
            memset(current, 0, sizeof(fossil_jellyfish_mindset));

            char *start = strchr(trim, '(') + 1;
            char *end = strchr(start, ')');
            if (start && end) {
                *end = '\0';
                strncpy(current->name, start, sizeof(current->name) - 1);
                strip_quotes(current->name);
            }
        }
        else if (current && strstr(trim, "description:")) {
            sscanf(trim, "description: %[^\n]", current->description);
            strip_quotes(current->description);
        }
        else if (current && strstr(trim, "priority:")) {
            sscanf(trim, "priority: %d", &current->priority);
        }
        else if (current && strstr(trim, "confidence_threshold:")) {
            sscanf(trim, "confidence_threshold: %f", &current->confidence_threshold);
        }
        else if (current && strstr(trim, "activation_condition:")) {
            sscanf(trim, "activation_condition: %[^\n]", current->activation_condition);
            strip_quotes(current->activation_condition);
        }
        else if (current && strstr(trim, "models: [")) {
            char *p = strchr(trim, '[') + 1;
            while (p && *p && *p != ']') {
                char model[256] = {0};
                sscanf(p, "%[^,\n]", model);
                strip_quotes(model);
                if (strlen(model) > 0 && current->model_count < FOSSIL_JELLYFISH_MAX_MODEL_FILES) {
                    strncpy(current->model_files[current->model_count++], model, 255);
                }
                p = strchr(p, ',');
                if (p) p++;
            }
        }
        else if (current && strstr(trim, "tags: [")) {
            char *p = strchr(trim, '[') + 1;
            while (p && *p && *p != ']') {
                char tag[64] = {0};
                sscanf(p, "%[^,\n]", tag);
                strip_quotes(tag);
                if (strlen(tag) > 0 && current->tag_count < FOSSIL_JELLYFISH_MAX_TAGS) {
                    strncpy(current->tags[current->tag_count++], tag, 63);
                }
                p = strchr(p, ',');
                if (p) p++;
            }
        }
    }

    fclose(fp);
    return mindset_count;
}

int fossil_jellyfish_validate_mindset(const fossil_jellyfish_mindset *mindset) {
    if (strlen(mindset->name) == 0) return 0;
    if (mindset->model_count == 0) return 0;
    for (int i = 0; i < mindset->model_count; ++i) {
        if (strstr(mindset->model_files[i], ".fish") == NULL) return 0;
    }
    return 1;
}

int fossil_jellyfish_load_mindset_file(const char *filepath, fossil_jellyfish_mind *mind) {
    fossil_jellyfish_mindset sets[16];
    int count = fossil_jellyfish_parse_jellyfish_file(filepath, sets, 16);
    if (count <= 0) return 0;

    for (int i = 0; i < count; ++i) {
        if (!fossil_jellyfish_validate_mindset(&sets[i])) continue;
        for (int j = 0; j < sets[i].model_count; ++j) {
            if (mind->model_count >= FOSSIL_JELLYFISH_MAX_MODELS) break;
            if (fossil_jellyfish_load(&mind->models[mind->model_count], sets[i].model_files[j]) == 0) {
                strncpy(mind->model_names[mind->model_count], sets[i].model_files[j], 63);
                mind->model_count++;
            }
        }
    }

    return 1;
}
