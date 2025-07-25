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

// HASH Algorithm magic

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
uint64_t get_time_microseconds(void) {
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    uint64_t t = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    return t / 10; // 100-nanosecond intervals to microseconds
}
#else
#include <sys/time.h>
uint64_t get_time_microseconds(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000ULL + tv.tv_usec;
}
#endif

static uint64_t get_device_salt(void) {
    // FNV-1a 64-bit base offset
    uint64_t hash = 0xcbf29ce484222325ULL;

    // Cross-platform user and home detection
#if defined(_WIN32) || defined(_WIN64)
    const char *vars[] = {
        getenv("USERNAME"),
        getenv("USERPROFILE"),
        getenv("COMPUTERNAME")
    };
#else
    const char *vars[] = {
        getenv("USER"),
        getenv("HOME"),
        getenv("SHELL"),
        getenv("HOSTNAME")
    };
#endif

    // Mix in each variable if it exists
    for (size_t v = 0; v < sizeof(vars) / sizeof(vars[0]); ++v) {
        const char *val = vars[v];
        if (val) {
            for (size_t i = 0; val[i]; ++i) {
                hash ^= (uint8_t)val[i];
                hash *= 0x100000001b3ULL;
            }
        }
    }

    return hash;
}

void fossil_jellyfish_hash(const char *input, const char *output, uint8_t *hash_out) {
    const uint64_t PRIME = 0x100000001b3ULL;
    static uint64_t SALT = 0;
    if (SALT == 0) SALT = get_device_salt();  // Initialize salt once

    uint64_t state1 = 0xcbf29ce484222325ULL ^ SALT;
    uint64_t state2 = 0x84222325cbf29ce4ULL ^ ~SALT;

    size_t in_len = strlen(input);
    size_t out_len = strlen(output);

    uint64_t nonce = get_time_microseconds();  // Microsecond resolution

    for (size_t i = 0; i < in_len; ++i) {
        state1 ^= (uint8_t)input[i];
        state1 *= PRIME;
        state1 ^= (state1 >> 27);
        state1 ^= (state1 << 33);
    }

    for (size_t i = 0; i < out_len; ++i) {
        state2 ^= (uint8_t)output[i];
        state2 *= PRIME;
        state2 ^= (state2 >> 29);
        state2 ^= (state2 << 31);
    }

    // Nonce and length entropy
    state1 ^= nonce ^ ((uint64_t)in_len << 32);
    state2 ^= ~nonce ^ ((uint64_t)out_len << 16);

    // Mixing rounds
    for (int i = 0; i < 6; ++i) {
        state1 += (state2 ^ (state1 >> 17));
        state2 += (state1 ^ (state2 >> 13));
        state1 ^= (state1 << 41);
        state2 ^= (state2 << 37);
        state1 *= PRIME;
        state2 *= PRIME;
    }

    for (size_t i = 0; i < FOSSIL_JELLYFISH_HASH_SIZE; ++i) {
        uint64_t mixed = (i % 2 == 0) ? state1 : state2;
        mixed ^= (mixed >> ((i % 7) + 13));
        mixed *= PRIME;
        mixed ^= SALT;
        hash_out[i] = (uint8_t)((mixed >> (8 * (i % 8))) & 0xFF);
    }
}

// Parses a string like: "key": "value"
static bool match_key_value(const char **ptr, const char *key, const char *value) {
    const char *p = *ptr;
    while (isspace(*p)) p++;
    size_t klen = strlen(key);
    if (strncmp(p, "\"", 1) != 0 || strncmp(p + 1, key, klen) != 0 || strncmp(p + 1 + klen, "\":", 2) != 0)
        return false;
    p += klen + 3;
    while (isspace(*p)) p++;
    size_t vlen = strlen(value);
    if (strncmp(p, "\"", 1) != 0 || strncmp(p + 1, value, vlen) != 0 || strncmp(p + 1 + vlen, "\"", 1) != 0)
        return false;
    *ptr = p + vlen + 2;
    return true;
}

static bool skip_key(const char **ptr, const char *key) {
    const char *p = *ptr;
    while (isspace(*p)) p++;
    size_t klen = strlen(key);
    if (strncmp(p, "\"", 1) != 0 || strncmp(p + 1, key, klen) != 0 || strncmp(p + 1 + klen, "\":", 2) != 0)
        return false;
    *ptr = p + klen + 3;
    return true;
}

static bool skip_symbol(const char **ptr, char symbol) {
    const char *p = *ptr;
    while (isspace(*p)) p++;
    if (*p != symbol)
        return false;
    *ptr = p + 1;
    return true;
}

static bool skip_comma(const char **ptr) {
    const char *p = *ptr;
    while (isspace(*p)) p++;
    if (*p == ',') {
        *ptr = p + 1;
        return true;
    }
    return false;
}

static bool parse_string_field(const char **ptr, const char *key, char *out, size_t max) {
    if (!skip_key(ptr, key)) return false;
    if (!skip_symbol(ptr, '"')) return false;

    const char *p = *ptr;
    size_t i = 0;

    while (*p && *p != '"' && i < max - 1) {
        if (*p == '\\' && *(p + 1)) p++; // skip escape
        out[i++] = *p++;
    }

    if (*p != '"') return false;
    out[i] = '\0';
    *ptr = p + 1;
    return true;
}

static bool parse_number_field(const char **ptr, const char *key, double *out_d, uint64_t *out_u64, int *out_i, uint32_t *out_u32) {
    if (!skip_key(ptr, key)) return false;

    char *end;
    const char *p = *ptr;

    while (isspace(*p)) p++;

    if (out_d) {
        *out_d = strtod(p, &end);
    } else if (out_u64) {
        *out_u64 = strtoull(p, &end, 10);
    } else if (out_i) {
        *out_i = strtol(p, &end, 10);
    } else if (out_u32) {
        *out_u32 = strtoul(p, &end, 10);
    } else {
        return false;
    }

    if (end == p) return false;
    *ptr = end;
    return true;
}

void fossil_jellyfish_init(fossil_jellyfish_chain *chain) {
    if (!chain) return;
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

            // Calculate delta_ms and duration_ms
            uint64_t prev_ts = 0;
            for (ssize_t j = (ssize_t)i - 1; j >= 0; --j) {
                if (chain->memory[j].valid) {
                    prev_ts = chain->memory[j].timestamp;
                    break;
                }
            }
            block->delta_ms = prev_ts ? (uint32_t)((block->timestamp - prev_ts) * 1000) : 0;
            block->duration_ms = 0; // Set to 0, or measure if possible

            block->valid = 1;
            block->confidence = 1.0f;
            block->usage_count = 0;

            memset(block->device_id, 0, FOSSIL_DEVICE_ID_SIZE);
            memset(block->signature, 0, FOSSIL_SIGNATURE_SIZE);

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

            // Calculate delta_ms and duration_ms
            uint64_t prev_ts = 0;
            for (ssize_t j = (ssize_t)i - 1; j >= 0; --j) {
                if (chain->memory[j].valid) {
                    prev_ts = chain->memory[j].timestamp;
                    break;
                }
            }
            block->delta_ms = prev_ts ? (uint32_t)((block->timestamp - prev_ts) * 1000) : 0;
            block->duration_ms = 0; // Set to 0, or measure if possible

            block->valid = 1;
            block->confidence = 1.0f;
            block->usage_count = 0;

            memset(block->device_id, 0, FOSSIL_DEVICE_ID_SIZE);
            memset(block->signature, 0, FOSSIL_SIGNATURE_SIZE);

            fossil_jellyfish_hash(input, output, block->hash);

            chain->count += 1;
            return;
        }
    }
}

// Removes blocks that are older and marked invalid (or just garbage collect oldest)
void fossil_jellyfish_cleanup(fossil_jellyfish_chain *chain) {
    // Remove invalid or low-confidence blocks and compact the memory array.
    size_t dst = 0;
    for (size_t src = 0; src < FOSSIL_JELLYFISH_MAX_MEM; ++src) {
        fossil_jellyfish_block *block = &chain->memory[src];
        if (block->valid && block->confidence >= 0.05f) {
            if (dst != src) {
                chain->memory[dst] = *block;
            }
            dst++;
        } else {
            // Optionally clear the block for security/cleanliness
            memset(block, 0, sizeof(fossil_jellyfish_block));
        }
    }
    chain->count = dst;
}

void fossil_jellyfish_dump(const fossil_jellyfish_chain *chain) {
    for (size_t i = 0; i < chain->count; ++i) {
        const fossil_jellyfish_block *b = &chain->memory[i];
        printf("Block %llu:\n", (unsigned long long)i);
        printf("  Input      : %s\n", b->input);
        printf("  Output     : %s\n", b->output);
        printf("  Timestamp  : %llu\n", (unsigned long long)b->timestamp);
        printf("  Delta ms   : %u\n", b->delta_ms);
        printf("  Duration ms: %u\n", b->duration_ms);
        printf("  Confidence : %.2f\n", b->confidence);
        printf("  Usage Count: %u\n", b->usage_count);
        printf("  Valid      : %d\n", b->valid);

        printf("  Device ID  : ");
        for (size_t j = 0; j < FOSSIL_DEVICE_ID_SIZE; ++j) {
            printf("%02x", b->device_id[j]);
        }
        printf("\n");

        printf("  Signature  : ");
        for (size_t j = 0; j < FOSSIL_SIGNATURE_SIZE; ++j) {
            printf("%02x", b->signature[j]);
        }
        printf("\n");

        printf("  Hash       : ");
        for (size_t j = 0; j < FOSSIL_JELLYFISH_HASH_SIZE; ++j) {
            printf("%02x", b->hash[j]);
        }
        printf("\n");
    }
}

int fossil_jellyfish_load(fossil_jellyfish_chain *chain, const char *filepath) {
    FILE *fp = fopen(filepath, "rb");
    if (!fp) return 0;

    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (length <= 0 || (size_t)length > 1024 * 1024) {
        fclose(fp);
        return 0;
    }

    char *buffer = malloc(length + 1);
    if (!buffer) {
        fclose(fp);
        return 0;
    }

    if (fread(buffer, 1, length, fp) != (size_t)length) {
        free(buffer);
        fclose(fp);
        return 0;
    }
    buffer[length] = '\0';
    fclose(fp);

    const char *ptr = buffer;
    if (!match_key_value(&ptr, "signature", "JFS1")) {
        free(buffer);
        return 0;
    }

    if (!skip_key(&ptr, "blocks") || !skip_symbol(&ptr, '[')) {
        free(buffer);
        return 0;
    }

    size_t count = 0;
    while (*ptr && *ptr != ']' && count < FOSSIL_JELLYFISH_MAX_MEM) {
        fossil_jellyfish_block *b = &chain->memory[count];
        memset(b, 0, sizeof(*b));

        if (!skip_symbol(&ptr, '{')) break;
        if (!parse_string_field(&ptr, "input", b->input, sizeof(b->input))) break;
        if (!parse_string_field(&ptr, "output", b->output, sizeof(b->output))) break;

        // Parse hash as hex string
        if (!skip_key(&ptr, "hash") || !skip_symbol(&ptr, '"')) break;
        for (size_t i = 0; i < FOSSIL_JELLYFISH_HASH_SIZE; ++i) {
            unsigned int val = 0;
            if (sscanf(ptr, "%2x", &val) != 1) break;
            b->hash[i] = (uint8_t)val;
            ptr += 2;
        }
        if (!skip_symbol(&ptr, '"')) break;

        if (!parse_number_field(&ptr, "timestamp", NULL, &b->timestamp, NULL, NULL)) break;
        if (!parse_number_field(&ptr, "delta_ms", NULL, NULL, NULL, &b->delta_ms)) break;
        if (!parse_number_field(&ptr, "duration_ms", NULL, NULL, NULL, &b->duration_ms)) break;
        if (!parse_number_field(&ptr, "valid", NULL, NULL, &b->valid, NULL)) break;

        double temp_conf = 0;
        if (!parse_number_field(&ptr, "confidence", &temp_conf, NULL, NULL, NULL)) break;
        b->confidence = (float)temp_conf;

        if (!parse_number_field(&ptr, "usage_count", NULL, NULL, NULL, &b->usage_count)) break;

        // Parse device_id as hex string
        if (!skip_key(&ptr, "device_id") || !skip_symbol(&ptr, '"')) break;
        for (size_t i = 0; i < FOSSIL_DEVICE_ID_SIZE; ++i) {
            unsigned int val = 0;
            if (sscanf(ptr, "%2x", &val) != 1) break;
            b->device_id[i] = (uint8_t)val;
            ptr += 2;
        }
        if (!skip_symbol(&ptr, '"')) break;

        // Parse signature as hex string
        if (!skip_key(&ptr, "signature") || !skip_symbol(&ptr, '"')) break;
        for (size_t i = 0; i < FOSSIL_SIGNATURE_SIZE; ++i) {
            unsigned int val = 0;
            if (sscanf(ptr, "%2x", &val) != 1) break;
            b->signature[i] = (uint8_t)val;
            ptr += 2;
        }
        if (!skip_symbol(&ptr, '"')) break;

        if (!skip_symbol(&ptr, '}')) break;

        skip_comma(&ptr);
        ++count;
    }

    if (!skip_symbol(&ptr, ']') || !skip_symbol(&ptr, '}')) {
        free(buffer);
        return 0;
    }

    chain->count = count;
    free(buffer);
    return 1;
}

int fossil_jellyfish_save(const fossil_jellyfish_chain *chain, const char *filepath) {
    FILE *fp = fopen(filepath, "wb");
    if (!fp) return 0;

    fprintf(fp, "{\n");
    fprintf(fp, "  \"signature\": \"JFS1\",\n");
    fprintf(fp, "  \"blocks\": [\n");

    for (size_t i = 0; i < chain->count; ++i) {
        const fossil_jellyfish_block *b = &chain->memory[i];

        char input_escaped[2 * FOSSIL_JELLYFISH_INPUT_SIZE] = {0};
        char output_escaped[2 * FOSSIL_JELLYFISH_OUTPUT_SIZE] = {0};

        char *dst = input_escaped;
        for (const char *src = b->input; *src && (size_t)(dst - input_escaped) < sizeof(input_escaped) - 2; ++src) {
            if (*src == '"' || *src == '\\') *dst++ = '\\';
            *dst++ = *src;
        }
        *dst = '\0';

        dst = output_escaped;
        for (const char *src = b->output; *src && (size_t)(dst - output_escaped) < sizeof(output_escaped) - 2; ++src) {
            if (*src == '"' || *src == '\\') *dst++ = '\\';
            *dst++ = *src;
        }
        *dst = '\0';

        fprintf(fp, "    {\n");
        fprintf(fp, "      \"input\": \"%s\",\n", input_escaped);
        fprintf(fp, "      \"output\": \"%s\",\n", output_escaped);
        fprintf(fp, "      \"hash\": \"");
        for (size_t j = 0; j < FOSSIL_JELLYFISH_HASH_SIZE; ++j) {
            fprintf(fp, "%02x", b->hash[j]);
        }
        fprintf(fp, "\",\n");
        fprintf(fp, "      \"timestamp\": %" PRIu64 ",\n", b->timestamp);
        fprintf(fp, "      \"delta_ms\": %" PRIu32 ",\n", b->delta_ms);
        fprintf(fp, "      \"duration_ms\": %" PRIu32 ",\n", b->duration_ms);
        fprintf(fp, "      \"valid\": %d,\n", b->valid);
        fprintf(fp, "      \"confidence\": %.6f,\n", b->confidence);
        fprintf(fp, "      \"usage_count\": %" PRIu32 ",\n", b->usage_count);

        // Write device_id as hex string
        fprintf(fp, "      \"device_id\": \"");
        for (size_t j = 0; j < FOSSIL_DEVICE_ID_SIZE; ++j) {
            fprintf(fp, "%02x", b->device_id[j]);
        }
        fprintf(fp, "\",\n");

        // Write signature as hex string
        fprintf(fp, "      \"signature\": \"");
        for (size_t j = 0; j < FOSSIL_SIGNATURE_SIZE; ++j) {
            fprintf(fp, "%02x", b->signature[j]);
        }
        fprintf(fp, "\"\n");

        fprintf(fp, "    }%s\n", (i < chain->count - 1) ? "," : "");
    }

    fprintf(fp, "  ]\n");
    fprintf(fp, "}\n");

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

const char* fossil_jellyfish_reason(fossil_jellyfish_chain *chain, const char *input) {
    // Try exact match first
    for (size_t i = 0; i < chain->count; ++i) {
        if (chain->memory[i].valid && strncmp(chain->memory[i].input, input, FOSSIL_JELLYFISH_INPUT_SIZE) == 0) {
            chain->memory[i].usage_count++;
            if (chain->memory[i].confidence < 1.0f)
                chain->memory[i].confidence += 0.05f;
            return chain->memory[i].output;
        }
    }

    // Fuzzy match fallback
    int best_score = 1000;
    const char *best_output = "Unknown";
    for (size_t i = 0; i < chain->count; ++i) {
        if (!chain->memory[i].valid) continue;
        int score = fossil_jellyfish_similarity(input, chain->memory[i].input);
        if (score == 0) return chain->memory[i].output; // Exact match (shouldn't happen here)
        if (score < best_score) {
            best_score = score;
            best_output = chain->memory[i].output;
        }
    }
    // Impose a fuzzy threshold
    if (best_score > (int)(strlen(input) / 2)) {
        return "Unknown";
    }
    return best_output;
}

void fossil_jellyfish_decay_confidence(fossil_jellyfish_chain *chain, float decay_rate) {
    const float MIN_CONFIDENCE = 0.05f;

    for (size_t i = 0; i < chain->count; ++i) {
        fossil_jellyfish_block *block = &chain->memory[i];
        if (!block->valid) continue;

        // Apply exponential decay
        block->confidence *= (1.0f - decay_rate);

        // Clamp to zero
        if (block->confidence < 0.0f) {
            block->confidence = 0.0f;
        }

        // Invalidate if confidence too low
        if (block->confidence < MIN_CONFIDENCE) {
            block->valid = 0;
        }
    }
}

size_t fossil_jellyfish_tokenize(const char *input, char tokens[][FOSSIL_JELLYFISH_TOKEN_SIZE], size_t max_tokens) {
    size_t token_count = 0;
    size_t len = strlen(input);
    size_t i = 0;

    while (i < len && token_count < max_tokens) {
        // Skip leading non-alphanum
        while (i < len && !isalnum((unsigned char)input[i])) i++;
        if (i >= len) break;

        // Extract token
        size_t t = 0;
        while (i < len && isalnum((unsigned char)input[i]) && t < FOSSIL_JELLYFISH_TOKEN_SIZE - 1) {
            tokens[token_count][t++] = (char)tolower((unsigned char)input[i++]);
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
    if (!chain || chain->count == 0) return 0.0f;

    size_t valid = 0;
    for (size_t i = 0; i < chain->count; ++i) {
        const fossil_jellyfish_block *b = &chain->memory[i];
        // - valid flag is set
        if (!b->valid) continue;
        // - input and output are non-empty
        if (b->input[0] == '\0' || b->output[0] == '\0') continue;
        // - hash is not all zero
        int hash_nonzero = 0;
        for (size_t j = 0; j < FOSSIL_JELLYFISH_HASH_SIZE; ++j)
            if (b->hash[j]) { hash_nonzero = 1; break; }
        if (!hash_nonzero) continue;
        // - device_id and signature are not all zero
        int dev_nonzero = 0, sig_nonzero = 0;
        for (size_t j = 0; j < FOSSIL_DEVICE_ID_SIZE; ++j)
            if (b->device_id[j]) { dev_nonzero = 1; break; }
        for (size_t j = 0; j < FOSSIL_SIGNATURE_SIZE; ++j)
            if (b->signature[j]) { sig_nonzero = 1; break; }
        if (!dev_nonzero || !sig_nonzero) continue;
        // Optionally, check timestamp for sanity
        if (b->timestamp == 0) continue;

        valid++;
    }

    return (float)valid / (float)chain->count;
}

int fossil_jellyfish_detect_conflict(const fossil_jellyfish_chain *chain, const char *input, const char *output) {
    for (size_t i = 0; i < chain->count; ++i) {
        const fossil_jellyfish_block *b = &chain->memory[i];
        if (!b->valid) continue;
        if (strncmp(b->input, input, FOSSIL_JELLYFISH_INPUT_SIZE) == 0) {
            // Check for conflicting output
            if (strncmp(b->output, output, FOSSIL_JELLYFISH_OUTPUT_SIZE) != 0) {
                // Optionally, could also check device_id, signature, or other fields for more nuanced conflict detection
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
        printf("  Input      : %s\n", best->input);
        printf("  Output     : %s\n", best->output);
        printf("  Confidence : %.2f\n", best->confidence);
        printf("  Usage Count: %u\n", best->usage_count);
        printf("  Timestamp  : %" PRIu64 "\n", best->timestamp);
        printf("  Delta ms   : %u\n", best->delta_ms);
        printf("  Duration ms: %u\n", best->duration_ms);

        printf("  Device ID  : ");
        for (size_t i = 0; i < FOSSIL_DEVICE_ID_SIZE; ++i) {
            printf("%02x", best->device_id[i]);
        }
        printf("\n");

        printf("  Signature  : ");
        for (size_t i = 0; i < FOSSIL_SIGNATURE_SIZE; ++i) {
            printf("%02x", best->signature[i]);
        }
        printf("\n");

        printf("  Hash       : ");
        for (int j = 0; j < FOSSIL_JELLYFISH_HASH_SIZE; ++j) {
            printf("%02x", best->hash[j]);
        }
        printf("\n");
    } else {
        printf("No confident memories yet.\n");
    }
    printf("================================\n");
}

bool fossil_jellyfish_verify_block(const fossil_jellyfish_block* block) {
    if (!block) return false;

    // Check input and output validity
    if (strlen(block->input) == 0 || strlen(block->output) == 0) return false;

    // Check hash validity (simple example: all zeros is invalid)
    for (size_t i = 0; i < FOSSIL_JELLYFISH_HASH_SIZE; i++) {
        if (block->hash[i] != 0) break;
        if (i == FOSSIL_JELLYFISH_HASH_SIZE - 1) return false;
    }

    return true;
}

bool fossil_jellyfish_verify_chain(const fossil_jellyfish_chain* chain) {
    if (!chain) return false;

    // Check each block in the chain
    for (size_t i = 0; i < chain->count; i++) {
        if (!fossil_jellyfish_verify_block(&chain->memory[i])) return false;
    }

    return true;
}

/**
 * Parses a .jellyfish (JellyDSL) file with Meson-like syntax and extracts models.
 *
 * @param filepath     Path to the .jellyfish file.
 * @param out_models   Array to store parsed models.
 * @param max_models   Maximum number of models to store.
 * @return             Number of models parsed, or 0 on failure.
 */
int fossil_jellyfish_parse_jellyfish_file(const char *filepath, fossil_jellyfish_jellydsl *out, int max_models) {
    if (!filepath || !out || max_models <= 0) return 0;

    FILE *fp = fopen(filepath, "r");
    if (!fp) return 0;

    char line[1024];
    int model_count = 0;
    fossil_jellyfish_jellydsl *current = NULL;
    int in_model = 0;

    while (fgets(line, sizeof(line), fp)) {
        char *trim = line;
        while (isspace(*trim)) ++trim;
        size_t len = strlen(trim);
        while (len > 0 && (trim[len - 1] == '\n' || trim[len - 1] == '\r')) trim[--len] = 0;

        // Start of a new model block
        if (strncmp(trim, "model(", 6) == 0) {
            if (model_count >= max_models) break;
            current = &out[model_count++];
            memset(current, 0, sizeof(*current));

            char *start = strchr(trim, '\'');
            char *end = start ? strchr(start + 1, '\'') : NULL;
            if (start && end && (size_t)(end - start - 1) < sizeof(current->name)) {
                strncpy(current->name, start + 1, end - start - 1);
                current->name[end - start - 1] = '\0';
            }

            in_model = 1;
            continue;
        }

        if (!in_model || !current) continue;

        // End of model block
        if (strchr(trim, '}')) {
            in_model = 0;
            continue;
        }

        // Handle key:value lines
        char *colon = strchr(trim, ':');
        if (!colon) continue;

        *colon = '\0';
        char *key = trim;
        char *value = colon + 1;
        while (isspace(*value)) value++;

        // Trim quotes from value
        if (*value == '\'' || *value == '"') value++;
        char *quote_end = strrchr(value, '\'');
        if (!quote_end) quote_end = strrchr(value, '"');
        if (quote_end) *quote_end = '\0';

        // Parse specific fields
        if (strcmp(key, "description") == 0) {
            strncpy(current->description, value, sizeof(current->description) - 1);
        } else if (strcmp(key, "activation_condition") == 0) {
            strncpy(current->activation_condition, value, sizeof(current->activation_condition) - 1);
        } else if (strcmp(key, "source_uri") == 0) {
            strncpy(current->source_uri, value, sizeof(current->source_uri) - 1);
        } else if (strcmp(key, "origin_device_id") == 0) {
            strncpy(current->origin_device_id, value, sizeof(current->origin_device_id) - 1);
        } else if (strcmp(key, "version") == 0) {
            strncpy(current->version, value, sizeof(current->version) - 1);
        } else if (strcmp(key, "content_hash") == 0) {
            strncpy(current->content_hash, value, sizeof(current->content_hash) - 1);
        } else if (strcmp(key, "state_machine") == 0) {
            strncpy(current->state_machine, value, sizeof(current->state_machine) - 1);
        } else if (strcmp(key, "created_at") == 0) {
            current->created_at = (uint64_t)atoll(value);
        } else if (strcmp(key, "updated_at") == 0) {
            current->updated_at = (uint64_t)atoll(value);
        } else if (strcmp(key, "trust_score") == 0) {
            current->trust_score = strtof(value, NULL);
        } else if (strcmp(key, "immutable") == 0) {
            current->immutable = atoi(value);
        } else if (strcmp(key, "priority") == 0) {
            current->priority = atoi(value);
        } else if (strcmp(key, "confidence_threshold") == 0) {
            current->confidence_threshold = strtof(value, NULL);
        } else if (strcmp(key, "tags") == 0) {
            char *tag = strchr(line, '[');
            if (tag) {
                tag++;
                char *tok = strtok(tag, ",]");
                while (tok && current->tag_count < FOSSIL_JELLYFISH_MAX_TAGS) {
                    while (*tok == ' ' || *tok == '\'' || *tok == '"') tok++;
                    char *end = tok + strlen(tok) - 1;
                    while (end > tok && (*end == '\'' || *end == '"' || *end == ' ')) *end-- = '\0';
                    strncpy(current->tags[current->tag_count++], tok, 31);
                    tok = strtok(NULL, ",]");
                }
            }
        } else if (strcmp(key, "models") == 0) {
            char *mod = strchr(line, '[');
            if (mod) {
                mod++;
                char *tok = strtok(mod, ",]");
                while (tok && current->model_count < FOSSIL_JELLYFISH_MAX_MODELS) {
                    while (*tok == ' ' || *tok == '\'' || *tok == '"') tok++;
                    char *end = tok + strlen(tok) - 1;
                    while (end > tok && (*end == '\'' || *end == '"' || *end == ' ')) *end-- = '\0';
                    strncpy(current->models[current->model_count++], tok, 31);
                    tok = strtok(NULL, ",]");
                }
            }
        }
    }

    fclose(fp);
    return model_count;
}
