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

// Main Source Implementation

void fossil_jellyfish_init(fossil_jellyfish_chain_t *chain) {
    if (!chain) return;

    chain->count = 0;
    memset(chain->device_id, 0, sizeof(chain->device_id));
    chain->created_at = (uint64_t)time(NULL);
    chain->updated_at = chain->created_at;

    // Initialize reserved blocks for each block type (if within memory range)
    for (int t = JELLY_BLOCK_BASIC; t <= JELLY_BLOCK_VERIFIED && t < FOSSIL_JELLYFISH_MAX_MEM; ++t) {
        fossil_jellyfish_block_t *block = &chain->memory[t];
        memset(block, 0, sizeof(fossil_jellyfish_block_t)); // Clean slate
        block->block_type = t;
        block->valid = 0;
        block->confidence = 0.0f;
        block->immutable = (t == JELLY_BLOCK_VERIFIED) ? 1 : 0;
        block->imagined = (t == JELLY_BLOCK_IMAGINED || t == JELLY_BLOCK_DERIVED || t == JELLY_BLOCK_EXPERIMENTAL) ? 1 : 0;
    }
}

void fossil_jellyfish_learn(fossil_jellyfish_chain_t *chain, const char *input, const char *output) {
    if (!chain || !input || !output) return;

    // Step 1: Check if input-output pair already exists for any block type
    for (size_t i = 0; i < FOSSIL_JELLYFISH_MAX_MEM; ++i) {
        fossil_jellyfish_block_t *block = &chain->memory[i];
        if (!block->valid) continue;
        if (strncmp(block->input, input, FOSSIL_JELLYFISH_INPUT_SIZE) == 0 &&
            strncmp(block->output, output, FOSSIL_JELLYFISH_OUTPUT_SIZE) == 0) {
            // Already learned: reinforce based on block type
            switch (block->block_type) {
                case JELLY_BLOCK_BASIC:
                    block->confidence += 0.1f;
                    break;
                case JELLY_BLOCK_IMAGINED:
                    block->confidence += 0.05f;
                    break;
                case JELLY_BLOCK_DERIVED:
                    block->confidence += 0.07f;
                    break;
                case JELLY_BLOCK_EXPERIMENTAL:
                    block->confidence += 0.03f;
                    break;
                case JELLY_BLOCK_VERIFIED:
                    block->confidence += 0.02f;
                    break;
                default:
                    block->confidence += 0.1f;
                    break;
            }
            if (block->confidence > 1.0f) block->confidence = 1.0f;
            block->usage_count += 1;
            block->timestamp = (uint64_t)time(NULL);
            return;
        }
    }

    // Step 2: Try to find an unused (invalid) slot to reuse
    for (size_t i = 0; i < FOSSIL_JELLYFISH_MAX_MEM; ++i) {
        fossil_jellyfish_block_t *block = &chain->memory[i];
        if (!block->valid) {
            // Choose block type based on heuristics (simple: always BASIC, can be improved)
            int block_type = JELLY_BLOCK_BASIC;
            int imagined = 0;
            uint32_t imagined_from_index = 0;
            char imagination_reason[128] = {0};

            // Example: If input contains "imagine" or "hypothesize", mark as IMAGINED
            if (strstr(input, "imagine") || strstr(input, "hypothesize")) {
                block_type = JELLY_BLOCK_IMAGINED;
                imagined = 1;
                strncpy(imagination_reason, "Marked as imagined due to input pattern", sizeof(imagination_reason) - 1);
            }

            // Fill block fields
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
            block->duration_ms = 0;

            block->valid = 1;
            block->confidence = 1.0f;
            block->usage_count = 0;

            memset(block->device_id, 0, FOSSIL_DEVICE_ID_SIZE);
            memset(block->signature, 0, FOSSIL_SIGNATURE_SIZE);

            block->block_type = block_type;
            block->imagined = imagined;
            block->imagined_from_index = imagined_from_index;
            strncpy(block->imagination_reason, imagination_reason, sizeof(block->imagination_reason) - 1);

            block->immutable = (block_type == JELLY_BLOCK_VERIFIED) ? 1 : 0;

            fossil_jellyfish_hash(input, output, block->hash);

            chain->count += 1;
            return;
        }
    }

    // Step 3: All slots full, run cleanup
    fossil_jellyfish_cleanup(chain);

    // Step 4: Try again to find a reusable slot after cleanup
    for (size_t i = 0; i < FOSSIL_JELLYFISH_MAX_MEM; ++i) {
        fossil_jellyfish_block_t *block = &chain->memory[i];
        if (!block->valid) {
            int block_type = JELLY_BLOCK_BASIC;
            int imagined = 0;
            uint32_t imagined_from_index = 0;
            char imagination_reason[128] = {0};

            if (strstr(input, "imagine") || strstr(input, "hypothesize")) {
                block_type = JELLY_BLOCK_IMAGINED;
                imagined = 1;
                strncpy(imagination_reason, "Marked as imagined due to input pattern", sizeof(imagination_reason) - 1);
            }

            strncpy(block->input, input, FOSSIL_JELLYFISH_INPUT_SIZE - 1);
            block->input[FOSSIL_JELLYFISH_INPUT_SIZE - 1] = '\0';

            strncpy(block->output, output, FOSSIL_JELLYFISH_OUTPUT_SIZE - 1);
            block->output[FOSSIL_JELLYFISH_OUTPUT_SIZE - 1] = '\0';

            block->timestamp = (uint64_t)time(NULL);

            uint64_t prev_ts = 0;
            for (ssize_t j = (ssize_t)i - 1; j >= 0; --j) {
                if (chain->memory[j].valid) {
                    prev_ts = chain->memory[j].timestamp;
                    break;
                }
            }
            block->delta_ms = prev_ts ? (uint32_t)((block->timestamp - prev_ts) * 1000) : 0;
            block->duration_ms = 0;

            block->valid = 1;
            block->confidence = 1.0f;
            block->usage_count = 0;

            memset(block->device_id, 0, FOSSIL_DEVICE_ID_SIZE);
            memset(block->signature, 0, FOSSIL_SIGNATURE_SIZE);

            block->block_type = block_type;
            block->imagined = imagined;
            block->imagined_from_index = imagined_from_index;
            strncpy(block->imagination_reason, imagination_reason, sizeof(block->imagination_reason) - 1);

            block->immutable = (block_type == JELLY_BLOCK_VERIFIED) ? 1 : 0;

            fossil_jellyfish_hash(input, output, block->hash);

            chain->count += 1;
            return;
        }
    }
}

// Removes blocks that are older and marked invalid (or just garbage collect oldest)
void fossil_jellyfish_cleanup(fossil_jellyfish_chain_t *chain) {
    if (!chain) return;
    size_t dst = 0;
    for (size_t src = 0; src < FOSSIL_JELLYFISH_MAX_MEM; ++src) {
        fossil_jellyfish_block_t *block = &chain->memory[src];
        int keep = 0;

        if (block->valid) {
            int type = block->block_type;

            // Defensive: restrict type to known enum range
            if (type < JELLY_BLOCK_BASIC || type > JELLY_BLOCK_VERIFIED)
                type = JELLY_BLOCK_BASIC;

            switch (type) {
                case JELLY_BLOCK_BASIC:
                    keep = (block->confidence >= 0.05f);
                    break;
                case JELLY_BLOCK_IMAGINED:
                    keep = (block->confidence >= 0.10f &&
                            block->imagination_reason[0] != '\0');
                    break;
                case JELLY_BLOCK_DERIVED:
                    keep = (block->confidence >= 0.10f &&
                            block->imagined_from_index > 0);
                    break;
                case JELLY_BLOCK_EXPERIMENTAL:
                    keep = (block->confidence >= 0.20f);
                    break;
                case JELLY_BLOCK_VERIFIED:
                    keep = 1;
                    break;
            }
        }

        if (keep) {
            if (dst != src) {
                chain->memory[dst] = *block;
            }
            dst++;
        } else {
            memset(block, 0, sizeof(fossil_jellyfish_block_t));
        }
    }
    chain->count = dst;
}

int fossil_jellyfish_audit(const fossil_jellyfish_chain_t *chain) {
    if (!chain) return -1;

    int issues = 0;

    for (size_t i = 0; i < chain->count; ++i) {
        const fossil_jellyfish_block_t *block = &chain->memory[i];
        if (!block->valid) continue;

        switch (block->block_type) {
            case JELLY_BLOCK_BASIC:
                // BASIC: Should not be imagined, should have input/output, confidence in [0,1]
                if (block->imagined) {
                    printf("[AUDIT] Block %llu: BASIC block marked as imagined.\n", (unsigned long long)i);
                    issues++;
                }
                if (strlen(block->input) == 0 || strlen(block->output) == 0) {
                    printf("[AUDIT] Block %llu: BASIC block missing input or output.\n", (unsigned long long)i);
                    issues++;
                }
                if (block->confidence < 0.0f || block->confidence > 1.0f) {
                    printf("[AUDIT] Block %llu: BASIC block has invalid confidence %.2f.\n", (unsigned long long)i, block->confidence);
                    issues++;
                }
                break;
            case JELLY_BLOCK_IMAGINED:
                // IMAGINED: Must be imagined, must have a reason
                if (!block->imagined) {
                    printf("[AUDIT] Block %llu: IMAGINED block not marked as imagined.\n", (unsigned long long)i);
                    issues++;
                }
                if (strlen(block->imagination_reason) == 0) {
                    printf("[AUDIT] Block %llu: IMAGINED block missing imagination reason.\n", (unsigned long long)i);
                    issues++;
                }
                break;
            case JELLY_BLOCK_DERIVED:
                // DERIVED: Should have a source index if imagined, and a reason
                if (block->imagined && block->imagined_from_index == 0) {
                    printf("[AUDIT] Block %llu: DERIVED block imagined but missing source index.\n", (unsigned long long)i);
                    issues++;
                }
                if (block->imagined && strlen(block->imagination_reason) == 0) {
                    printf("[AUDIT] Block %llu: DERIVED block imagined but missing reason.\n", (unsigned long long)i);
                    issues++;
                }
                break;
            case JELLY_BLOCK_EXPERIMENTAL:
                // EXPERIMENTAL: Should not have high confidence
                if (block->confidence > 0.9f) {
                    printf("[AUDIT] Block %llu: EXPERIMENTAL block has suspiciously high confidence.\n", (unsigned long long)i);
                    issues++;
                }
                break;
            case JELLY_BLOCK_VERIFIED:
                // VERIFIED: Should be immutable, signature must verify
                if (!block->immutable) {
                    printf("[AUDIT] Block %llu: VERIFIED block not marked immutable.\n", (unsigned long long)i);
                    issues++;
                }
                if (!fossil_jellyfish_block_verify_signature(block, block->device_id)) {
                    printf("[AUDIT] Block %llu: VERIFIED block signature verification failed.\n", (unsigned long long)i);
                    issues++;
                }
                break;
            default:
                printf("[AUDIT] Block %llu: Unknown block type %d.\n", (unsigned long long)i, block->block_type);
                issues++;
                break;
        }
    }

    return issues;
}

int fossil_jellyfish_prune(fossil_jellyfish_chain_t *chain, float min_confidence) {
    if (!chain) return 0;
    int pruned = 0;

    for (size_t i = 0; i < chain->count; ++i) {
        fossil_jellyfish_block_t *block = &chain->memory[i];

        if (!block->valid || block->immutable) continue;

        // Prune based on block type and confidence
        switch (block->block_type) {
            case JELLY_BLOCK_BASIC:
            case JELLY_BLOCK_IMAGINED:
            case JELLY_BLOCK_DERIVED:
            case JELLY_BLOCK_EXPERIMENTAL:
            case JELLY_BLOCK_VERIFIED:
                if (block->confidence < min_confidence) {
                    block->valid = 0;
                    pruned++;
                }
                break;
            default:
                // Unknown type: treat as prunable if confidence is low
                if (block->confidence < min_confidence) {
                    block->valid = 0;
                    pruned++;
                }
                break;
        }
    }
    return pruned;
}

void fossil_jellyfish_dump(const fossil_jellyfish_chain_t *chain) {
    static const char *block_type_names[] = {
        "BASIC", "IMAGINED", "DERIVED", "EXPERIMENTAL", "VERIFIED"
    };

    for (size_t i = 0; i < chain->count; ++i) {
        const fossil_jellyfish_block_t *b = &chain->memory[i];
        printf("Block %llu:\n", (unsigned long long)i);

        printf("  Type       : %s\n",
            (b->block_type >= 0 && b->block_type <= 4) ? block_type_names[b->block_type] : "UNKNOWN");

        printf("  Input      : %s\n", b->input);
        printf("  Output     : %s\n", b->output);
        printf("  Timestamp  : %llu\n", (unsigned long long)b->timestamp);
        printf("  Delta ms   : %u\n", b->delta_ms);
        printf("  Duration ms: %u\n", b->duration_ms);
        printf("  Confidence : %.2f\n", b->confidence);
        printf("  Usage Count: %u\n", b->usage_count);
        printf("  Valid      : %d\n", b->valid);
        printf("  Immutable  : %d\n", b->immutable);

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

        // Show imagination-related fields for IMAGINED, DERIVED, EXPERIMENTAL
        if (b->block_type == JELLY_BLOCK_IMAGINED ||
            b->block_type == JELLY_BLOCK_DERIVED ||
            b->block_type == JELLY_BLOCK_EXPERIMENTAL) {
            printf("  Imagined   : %d\n", b->imagined);
            printf("  Imagined From Index: %u\n", b->imagined_from_index);
            printf("  Imagination Reason : %s\n", b->imagination_reason);
        }
    }
}

static int fossil_jellyfish_similarity(const char *a, const char *b) {
    if (!a || !b) return -1; // invalid input

    size_t i = 0, j = 0;
    int cost = 0;

    while (a[i] && b[j]) {
        char ac = tolower((unsigned char)a[i]);
        char bc = tolower((unsigned char)b[j]);

        if (ac != bc) cost++;
        i++;
        j++;
    }

    // Add cost for leftover characters
    cost += (int)strlen(a + i);
    cost += (int)strlen(b + j);

    return cost;
}

const char* fossil_jellyfish_reason(fossil_jellyfish_chain_t *chain, const char *input) {
    if (!chain || !input) return "Unknown";

    // Track best match for each block type
    fossil_jellyfish_block_t *best[5] = {NULL, NULL, NULL, NULL, NULL};
    int best_score[5] = {1000, 1000, 1000, 1000, 1000};

    // First pass: exact match for each type
    for (size_t i = 0; i < chain->count; ++i) {
        fossil_jellyfish_block_t *b = &chain->memory[i];
        if (!b->valid) continue;
        if (strncmp(b->input, input, FOSSIL_JELLYFISH_INPUT_SIZE) == 0) {
            b->usage_count++;
            if (b->confidence < 1.0f)
                b->confidence += 0.05f;
            return b->output;
        }
    }

    // Second pass: fuzzy match for each type
    for (size_t i = 0; i < chain->count; ++i) {
        fossil_jellyfish_block_t *b = &chain->memory[i];
        if (!b->valid || b->block_type < 0 || b->block_type > 4) continue;
        int score = fossil_jellyfish_similarity(input, b->input);
        if (score < best_score[b->block_type]) {
            best_score[b->block_type] = score;
            best[b->block_type] = b;
        }
    }

    // Priority order: VERIFIED > DERIVED > BASIC > IMAGINED > EXPERIMENTAL
    static const int priority[] = {
        JELLY_BLOCK_VERIFIED, JELLY_BLOCK_DERIVED, JELLY_BLOCK_BASIC, JELLY_BLOCK_IMAGINED, JELLY_BLOCK_EXPERIMENTAL
    };
    for (size_t p = 0; p < 5; ++p) {
        int t = priority[p];
        if (best[t] && best_score[t] <= (int)(strlen(input) / 2)) {
            best[t]->usage_count++;
            if (best[t]->confidence < 1.0f)
                best[t]->confidence += 0.02f;
            return best[t]->output;
        }
    }

    return "Unknown";
}

void fossil_jellyfish_decay_confidence(fossil_jellyfish_chain_t *chain, float decay_rate) {
    if (!chain || chain->count == 0 || decay_rate <= 0.0f) return;

    // Per-type minimum confidence thresholds
    const float min_conf[5] = {
        0.05f, // BASIC
        0.10f, // IMAGINED
        0.10f, // DERIVED
        0.20f, // EXPERIMENTAL
        0.02f  // VERIFIED
    };
    const float max_conf[5] = {
        1.0f, // BASIC
        1.0f, // IMAGINED
        1.0f, // DERIVED
        1.0f, // EXPERIMENTAL
        1.0f  // VERIFIED
    };

    // Clamp half-life
    double half_life_seconds = fmax(1.0, (double)decay_rate);
    time_t now = time(NULL);

    for (size_t i = 0; i < chain->count; ++i) {
        fossil_jellyfish_block_t *block = &chain->memory[i];
        if (!block->valid || block->immutable) continue;

        int t = block->block_type;
        if (t < 0 || t > 4) t = 0; // fallback to BASIC

        time_t block_time = (time_t)(block->timestamp / 1000);
        time_t age_seconds = now - block_time;
        if (age_seconds <= 0) continue;

        double decay_factor = pow(0.5, (double)age_seconds / half_life_seconds);
        block->confidence *= (float)decay_factor;
        block->confidence = fmaxf(0.0f, fminf(block->confidence, max_conf[t]));
        if (block->confidence < min_conf[t]) {
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

const fossil_jellyfish_block_t *fossil_jellyfish_best_memory(const fossil_jellyfish_chain_t *chain) {
    if (!chain || chain->count == 0) return NULL;

    // Track best block for each type
    const fossil_jellyfish_block_t *best[5] = {NULL, NULL, NULL, NULL, NULL};
    float best_score[5] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

    for (size_t i = 0; i < chain->count; ++i) {
        const fossil_jellyfish_block_t *b = &chain->memory[i];
        if (!b->valid || b->block_type < 0 || b->block_type > 4) continue;
        int t = b->block_type;
        float score = b->confidence;

        // Optionally, boost score for immutable blocks
        if (b->immutable) score += 0.1f;

        if (!best[t] || score > best_score[t]) {
            best[t] = b;
            best_score[t] = score;
        }
    }

    // Priority order: VERIFIED > DERIVED > BASIC > IMAGINED > EXPERIMENTAL
    static const int priority[] = {
        JELLY_BLOCK_VERIFIED, JELLY_BLOCK_DERIVED, JELLY_BLOCK_BASIC, JELLY_BLOCK_IMAGINED, JELLY_BLOCK_EXPERIMENTAL
    };
    for (size_t p = 0; p < 5; ++p) {
        int t = priority[p];
        if (best[t]) return best[t];
    }

    return NULL;
}

float fossil_jellyfish_knowledge_coverage(const fossil_jellyfish_chain_t *chain) {
    if (!chain || chain->count == 0) return 0.0f;

    // Track valid counts per block type
    size_t valid_per_type[5] = {0, 0, 0, 0, 0};
    size_t total_per_type[5] = {0, 0, 0, 0, 0};

    for (size_t i = 0; i < chain->count; ++i) {
        const fossil_jellyfish_block_t *b = &chain->memory[i];
        int t = (b->block_type >= 0 && b->block_type <= 4) ? b->block_type : 0;
        total_per_type[t]++;

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

        valid_per_type[t]++;
    }

    // Compute per-type coverage, then average
    float sum_coverage = 0.0f;
    int type_count = 0;
    for (int t = 0; t < 5; ++t) {
        if (total_per_type[t] > 0) {
            sum_coverage += (float)valid_per_type[t] / (float)total_per_type[t];
            type_count++;
        }
    }
    return (type_count > 0) ? (sum_coverage / type_count) : 0.0f;
}

int fossil_jellyfish_detect_conflict(const fossil_jellyfish_chain_t *chain, const char *input, const char *output) {
    // Track conflict for each block type
    int conflict[5] = {0, 0, 0, 0, 0};

    for (size_t i = 0; i < chain->count; ++i) {
        const fossil_jellyfish_block_t *b = &chain->memory[i];
        if (!b->valid) continue;
        int t = (b->block_type >= 0 && b->block_type <= 4) ? b->block_type : 0;
        if (strncmp(b->input, input, FOSSIL_JELLYFISH_INPUT_SIZE) == 0) {
            if (strncmp(b->output, output, FOSSIL_JELLYFISH_OUTPUT_SIZE) != 0) {
                conflict[t] = 1;
            }
        }
    }

    // Return a bitmask: bit N set if conflict for block type N
    int result = 0;
    for (int t = 0; t < 5; ++t) {
        if (conflict[t]) result |= (1 << t);
    }
    return result;
}

const fossil_jellyfish_block_t* fossil_jellyfish_best_match(const fossil_jellyfish_chain_t *chain, const char *input) {
    // Score boost table for each block type
    static const float type_boost[5] = {
        0.0f,   // BASIC
        0.1f,   // IMAGINED
        0.2f,   // DERIVED
        -0.1f,  // EXPERIMENTAL
        0.3f    // VERIFIED
    };

    const fossil_jellyfish_block_t *best[5] = {NULL, NULL, NULL, NULL, NULL};
    float best_score[5] = {-1e9f, -1e9f, -1e9f, -1e9f, -1e9f};

    for (size_t i = 0; i < chain->count; ++i) {
        const fossil_jellyfish_block_t *block = &chain->memory[i];
        if (!block->valid) continue;
        if (strcmp(block->input, input) != 0) continue;

        int t = (block->block_type >= 0 && block->block_type <= 4) ? block->block_type : 0;
        float score = block->confidence + type_boost[t];
        if (block->immutable) score += 0.5f;

        if (score > best_score[t]) {
            best[t] = block;
            best_score[t] = score;
        }
    }

    // Priority order: VERIFIED > DERIVED > BASIC > IMAGINED > EXPERIMENTAL
    static const int priority[] = {
        JELLY_BLOCK_VERIFIED, JELLY_BLOCK_DERIVED, JELLY_BLOCK_BASIC, JELLY_BLOCK_IMAGINED, JELLY_BLOCK_EXPERIMENTAL
    };
    for (size_t p = 0; p < 5; ++p) {
        int t = priority[p];
        if (best[t]) return best[t];
    }

    return NULL;
}

void fossil_jellyfish_reflect(const fossil_jellyfish_chain_t *chain) {
    static const char *block_type_names[] = {
        "BASIC", "IMAGINED", "DERIVED", "EXPERIMENTAL", "VERIFIED"
    };
    if (!chain || chain->count == 0) {
        printf("== Jellyfish Self-Reflection ==\n");
        printf("No memories available.\n");
        printf("================================\n");
        return;
    }

    size_t valid[5] = {0}, total[5] = {0};
    float confidence_sum[5] = {0}, confidence_min[5], confidence_max[5];
    uint64_t usage_sum[5] = {0};
    for (int t = 0; t < 5; ++t) {
        confidence_min[t] = 1.0f;
        confidence_max[t] = 0.0f;
    }

    for (size_t i = 0; i < chain->count; ++i) {
        const fossil_jellyfish_block_t *block = &chain->memory[i];
        int t = (block->block_type >= 0 && block->block_type <= 4) ? block->block_type : 0;
        total[t]++;
        if (!block->valid) continue;
        valid[t]++;
        confidence_sum[t] += block->confidence;
        usage_sum[t] += block->usage_count;
        if (block->confidence < confidence_min[t]) confidence_min[t] = block->confidence;
        if (block->confidence > confidence_max[t]) confidence_max[t] = block->confidence;
    }

    printf("== Jellyfish Self-Reflection ==\n");
    for (int t = 0; t < 5; ++t) {
        float coverage = (total[t] > 0) ? (float)valid[t] / (float)total[t] : 0.0f;
        float confidence_avg = (valid[t] > 0) ? confidence_sum[t] / valid[t] : 0.0f;
        printf("[%s]\n", block_type_names[t]);
        printf("  Total Memories  : %llu\n", (unsigned long long)total[t]);
        printf("  Valid Memories  : %llu (%.1f%%)\n", (unsigned long long)valid[t], coverage * 100.0f);
        printf("  Avg Confidence  : %.3f\n", confidence_avg);
        printf("  Min Confidence  : %.3f\n", (valid[t] > 0) ? confidence_min[t] : 0.0f);
        printf("  Max Confidence  : %.3f\n", (valid[t] > 0) ? confidence_max[t] : 0.0f);
        printf("  Total Usage     : %llu\n", (unsigned long long)usage_sum[t]);

        // Find best block for this type
        const fossil_jellyfish_block_t *best = NULL;
        float best_conf = -1.0f;
        for (size_t i = 0; i < chain->count; ++i) {
            const fossil_jellyfish_block_t *b = &chain->memory[i];
            if (!b->valid || b->block_type != t) continue;
            if (b->confidence > best_conf) {
                best = b;
                best_conf = b->confidence;
            }
        }
        if (best) {
            printf("  Strongest Memory:\n");
            printf("    Input      : %s\n", best->input);
            printf("    Output     : %s\n", best->output);
            printf("    Confidence : %.3f\n", best->confidence);
            printf("    Usage Count: %u\n", best->usage_count);
            printf("    Timestamp  : %" PRIu64 "\n", best->timestamp);
            printf("    Delta ms   : %u\n", best->delta_ms);
            printf("    Duration ms: %u\n", best->duration_ms);

            printf("    Device ID  : ");
            for (size_t i = 0; i < FOSSIL_DEVICE_ID_SIZE; ++i) {
                printf("%02x", best->device_id[i]);
            }
            printf("\n");

            printf("    Signature  : ");
            for (size_t i = 0; i < FOSSIL_SIGNATURE_SIZE; ++i) {
                printf("%02x", best->signature[i]);
            }
            printf("\n");

            printf("    Hash       : ");
            for (int j = 0; j < FOSSIL_JELLYFISH_HASH_SIZE; ++j) {
                printf("%02x", best->hash[j]);
            }
            printf("\n");
            if (t == JELLY_BLOCK_IMAGINED || t == JELLY_BLOCK_DERIVED || t == JELLY_BLOCK_EXPERIMENTAL) {
                printf("    Imagined   : %d\n", best->imagined);
                printf("    Imagined From Index: %u\n", best->imagined_from_index);
                printf("    Imagination Reason : %s\n", best->imagination_reason);
            }
        } else {
            printf("  No confident memories found.\n");
        }
        printf("\n");
    }
    printf("================================\n");
}

bool fossil_jellyfish_verify_block(const fossil_jellyfish_block_t* block) {
    if (!block) return false;

    // Check input and output validity
    if (strlen(block->input) == 0 || strlen(block->output) == 0) return false;

    // Check hash validity (all zeros is invalid)
    int hash_nonzero = 0;
    for (size_t i = 0; i < FOSSIL_JELLYFISH_HASH_SIZE; i++) {
        if (block->hash[i] != 0) { hash_nonzero = 1; break; }
    }
    if (!hash_nonzero) return false;

    switch (block->block_type) {
        case JELLY_BLOCK_BASIC:
            // BASIC: Should not be imagined, confidence in [0,1]
            if (block->imagined) return false;
            if (block->confidence < 0.0f || block->confidence > 1.0f) return false;
            break;
        case JELLY_BLOCK_IMAGINED:
            // IMAGINED: Must be imagined, must have a reason
            if (!block->imagined) return false;
            if (strlen(block->imagination_reason) == 0) return false;
            if (block->confidence < 0.0f || block->confidence > 1.0f) return false;
            break;
        case JELLY_BLOCK_DERIVED:
            // DERIVED: If imagined, must have a source index and reason
            if (block->imagined && block->imagined_from_index == 0) return false;
            if (block->imagined && strlen(block->imagination_reason) == 0) return false;
            if (block->confidence < 0.0f || block->confidence > 1.0f) return false;
            break;
        case JELLY_BLOCK_EXPERIMENTAL:
            // EXPERIMENTAL: Should not have high confidence
            if (block->confidence > 0.9f) return false;
            break;
        case JELLY_BLOCK_VERIFIED:
            // VERIFIED: Should be immutable, signature must verify
            if (!block->immutable) return false;
            if (!fossil_jellyfish_block_verify_signature(block, block->device_id)) return false;
            break;
        default:
            // Unknown type: treat as invalid
            return false;
    }

    return true;
}

bool fossil_jellyfish_verify_chain(const fossil_jellyfish_chain_t* chain) {
    if (!chain || chain->count == 0) return false;

    // Track verification status for each block type
    bool type_ok[5] = {true, true, true, true, true};

    for (size_t i = 0; i < chain->count; i++) {
        const fossil_jellyfish_block_t *block = &chain->memory[i];
        int t = (block->block_type >= 0 && block->block_type <= 4) ? block->block_type : 0;
        if (!fossil_jellyfish_verify_block(block)) {
            type_ok[t] = false;
        }
    }

    // Only return true if all types are valid
    for (int t = 0; t < 5; ++t) {
        if (!type_ok[t]) return false;
    }
    return true;
}

void fossil_jellyfish_validation_report(const fossil_jellyfish_chain_t *chain) {
    static const char *block_type_names[] = {
        "BASIC", "IMAGINED", "DERIVED", "EXPERIMENTAL", "VERIFIED"
    };
    if (!chain) {
        printf("[Validation] Chain is NULL\n");
        return;
    }

    size_t type_total[5] = {0}, type_valid[5] = {0}, type_ok[5] = {0};

    printf("== Jellyfish Chain Validation Report ==\n");
    for (size_t i = 0; i < chain->count; ++i) {
        const fossil_jellyfish_block_t *block = &chain->memory[i];
        int t = (block->block_type >= 0 && block->block_type <= 4) ? block->block_type : 0;
        type_total[t]++;
        printf("Block %lu [%s]: ", (unsigned long)i, block_type_names[t]);

        if (!block->valid) {
            printf("Invalid\n");
            continue;
        }
        type_valid[t]++;

        bool ok = fossil_jellyfish_verify_block(block);
        printf("%s\n", ok ? "OK" : "Failed");
        if (ok) type_ok[t]++;
    }
    printf("=======================================\n");
    printf("Summary by Block Type:\n");
    for (int t = 0; t < 5; ++t) {
        printf("  %-12s: Total=%lu, Valid=%lu, Passed=%lu\n",
            block_type_names[t],
            (unsigned long)type_total[t],
            (unsigned long)type_valid[t],
            (unsigned long)type_ok[t]);
    }
    printf("=======================================\n");
}

float fossil_jellyfish_chain_trust_score(const fossil_jellyfish_chain_t *chain) {
    if (!chain || chain->count == 0) return 0.0f;

    float total_conf[5] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    size_t valid_count[5] = {0, 0, 0, 0, 0};

    for (size_t i = 0; i < chain->count; ++i) {
        const fossil_jellyfish_block_t *block = &chain->memory[i];
        if (!block->valid) continue;
        int t = (block->block_type >= 0 && block->block_type <= 4) ? block->block_type : 0;
        if (block->immutable && block->confidence >= 0.9f) {
            total_conf[t] += block->confidence;
            ++valid_count[t];
        }
    }

    float sum = 0.0f;
    int types_with_blocks = 0;
    for (int t = 0; t < 5; ++t) {
        if (valid_count[t]) {
            sum += total_conf[t] / valid_count[t];
            ++types_with_blocks;
        }
    }
    return types_with_blocks ? (sum / types_with_blocks) : 0.0f;
}

void fossil_jellyfish_mark_immutable(fossil_jellyfish_block_t *block) {
    if (!block) return;
    switch (block->block_type) {
        case JELLY_BLOCK_BASIC:
            block->immutable = 0;
            break;
        case JELLY_BLOCK_IMAGINED:
            block->immutable = 0;
            break;
        case JELLY_BLOCK_DERIVED:
            block->immutable = 0;
            break;
        case JELLY_BLOCK_EXPERIMENTAL:
            block->immutable = 0;
            break;
        case JELLY_BLOCK_VERIFIED:
            block->immutable = 1;
            break;
        default:
            block->immutable = 0;
            break;
    }
}

int fossil_jellyfish_prune_chain(fossil_jellyfish_chain_t *chain, float min_confidence) {
    if (!chain || chain->count == 0) return 0;
    int pruned = 0;

    // Per-type minimum confidence thresholds (can be adjusted as needed)
    const float min_conf[5] = {
        min_confidence,         // BASIC
        min_confidence + 0.05f, // IMAGINED
        min_confidence + 0.05f, // DERIVED
        min_confidence + 0.10f, // EXPERIMENTAL
        min_confidence - 0.03f  // VERIFIED (allow slightly lower)
    };

    for (size_t i = 0; i < chain->count; ) {
        fossil_jellyfish_block_t *block = &chain->memory[i];
        int t = (block->block_type >= 0 && block->block_type <= 4) ? block->block_type : 0;

        // Only prune if not immutable and confidence below per-type threshold
        if (!block->valid || block->immutable ||
            block->confidence >= min_conf[t]) {
            i++;
            continue;
        }

        // Shift the rest down
        memmove(&chain->memory[i], &chain->memory[i + 1], sizeof(fossil_jellyfish_block_t) * (chain->count - i - 1));
        chain->count--;
        pruned++;
        // Do not increment i, as we now have a new block at this index
    }

    return pruned;
}

int fossil_jellyfish_deduplicate_chain(fossil_jellyfish_chain_t *chain) {
    if (!chain || chain->count < 2) return 0;
    int removed = 0;

    // For each block type, deduplicate only within that type
    for (int t = JELLY_BLOCK_BASIC; t <= JELLY_BLOCK_VERIFIED; ++t) {
        for (size_t i = 0; i < chain->count; ++i) {
            fossil_jellyfish_block_t *a = &chain->memory[i];
            if (a->block_type != t) continue;

            for (size_t j = i + 1; j < chain->count; ) {
                fossil_jellyfish_block_t *b = &chain->memory[j];
                if (b->block_type == t &&
                    strcmp(a->input, b->input) == 0 &&
                    strcmp(a->output, b->output) == 0) {
                    // Remove duplicate b
                    memmove(&chain->memory[j], &chain->memory[j + 1], sizeof(fossil_jellyfish_block_t) * (chain->count - j - 1));
                    chain->count--;
                    removed++;
                } else {
                    j++;
                }
            }
        }
    }

    return removed;
}

static void trim_whitespace(char *str) {
    if (!str) return;
    
    // Trim leading
    char *start = str;
    while (isspace((unsigned char)*start)) start++;

    // Move trimmed start to front
    if (start != str) memmove(str, start, strlen(start) + 1);

    // Trim trailing
    size_t len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) str[--len] = '\0';
}

int fossil_jellyfish_compress_chain(fossil_jellyfish_chain_t *chain) {
    if (!chain) return 0;
    int modified = 0;

    // For each block type
    for (int t = JELLY_BLOCK_BASIC; t <= JELLY_BLOCK_VERIFIED; ++t) {
        for (size_t i = 0; i < chain->count; ++i) {
            fossil_jellyfish_block_t *block = &chain->memory[i];
            if (block->block_type != t) continue;

            size_t orig_input_len = strlen(block->input);
            size_t orig_output_len = strlen(block->output);

            trim_whitespace(block->input);
            trim_whitespace(block->output);

            if (strlen(block->input) != orig_input_len || strlen(block->output) != orig_output_len) {
                modified++;
            }
        }
    }

    return modified;
}

int fossil_jellyfish_redact_block(fossil_jellyfish_block_t *block) {
    if (!block) return -1;

    switch (block->block_type) {
        case JELLY_BLOCK_BASIC:
            strncpy(block->input, "***REDACTED_BASIC***", FOSSIL_JELLYFISH_INPUT_SIZE);
            strncpy(block->output, "***REDACTED_BASIC***", FOSSIL_JELLYFISH_OUTPUT_SIZE);
            break;
        case JELLY_BLOCK_IMAGINED:
            strncpy(block->input, "***REDACTED_IMAGINED***", FOSSIL_JELLYFISH_INPUT_SIZE);
            strncpy(block->output, "***REDACTED_IMAGINED***", FOSSIL_JELLYFISH_OUTPUT_SIZE);
            strncpy(block->imagination_reason, "***REDACTED***", sizeof(block->imagination_reason));
            block->imagined_from_index = 0;
            break;
        case JELLY_BLOCK_DERIVED:
            strncpy(block->input, "***REDACTED_DERIVED***", FOSSIL_JELLYFISH_INPUT_SIZE);
            strncpy(block->output, "***REDACTED_DERIVED***", FOSSIL_JELLYFISH_OUTPUT_SIZE);
            strncpy(block->imagination_reason, "***REDACTED***", sizeof(block->imagination_reason));
            block->imagined_from_index = 0;
            break;
        case JELLY_BLOCK_EXPERIMENTAL:
            strncpy(block->input, "***REDACTED_EXPERIMENTAL***", FOSSIL_JELLYFISH_INPUT_SIZE);
            strncpy(block->output, "***REDACTED_EXPERIMENTAL***", FOSSIL_JELLYFISH_OUTPUT_SIZE);
            strncpy(block->imagination_reason, "***REDACTED***", sizeof(block->imagination_reason));
            block->imagined_from_index = 0;
            break;
        case JELLY_BLOCK_VERIFIED:
            strncpy(block->input, "***REDACTED_VERIFIED***", FOSSIL_JELLYFISH_INPUT_SIZE);
            strncpy(block->output, "***REDACTED_VERIFIED***", FOSSIL_JELLYFISH_OUTPUT_SIZE);
            break;
        default:
            strncpy(block->input, "***REDACTED***", FOSSIL_JELLYFISH_INPUT_SIZE);
            strncpy(block->output, "***REDACTED***", FOSSIL_JELLYFISH_OUTPUT_SIZE);
            break;
    }

    memset(block->hash, 0, FOSSIL_JELLYFISH_HASH_SIZE);
    block->confidence = 0.0f;
    block->valid = 0;
    return 0;
}

void fossil_jellyfish_chain_stats(const fossil_jellyfish_chain_t *chain, size_t out_valid_count[5], float out_avg_confidence[5], float out_immutable_ratio[5]) {
    if (!chain) return;
    size_t valid[5] = {0}, immutable[5] = {0};
    float confidence_sum[5] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

    for (size_t i = 0; i < chain->count; ++i) {
        const fossil_jellyfish_block_t *b = &chain->memory[i];
        int t = (b->block_type >= 0 && b->block_type <= 4) ? b->block_type : 0;
        if (!b->valid) continue;
        valid[t]++;
        confidence_sum[t] += b->confidence;
        if (b->immutable) immutable[t]++;
    }

    for (int t = 0; t < 5; ++t) {
        if (out_valid_count) out_valid_count[t] = valid[t];
        if (out_avg_confidence) out_avg_confidence[t] = valid[t] ? (confidence_sum[t] / valid[t]) : 0.0f;
        if (out_immutable_ratio) out_immutable_ratio[t] = valid[t] ? ((float)immutable[t] / valid[t]) : 0.0f;
    }
}

int fossil_jellyfish_compare_chains(const fossil_jellyfish_chain_t *a, const fossil_jellyfish_chain_t *b) {
    if (!a || !b) return -1;
    int diff_count[5] = {0, 0, 0, 0, 0};

    // For each block type, compare blocks of that type
    for (int t = JELLY_BLOCK_BASIC; t <= JELLY_BLOCK_VERIFIED; ++t) {
        // Compare up to the max count for this type
        size_t ai = 0, bi = 0, ac = 0, bc = 0;
        while (ac < a->count || bc < b->count) {
            // Find next valid block of this type in a
            while (ai < a->count && (a->memory[ai].block_type != t || !a->memory[ai].valid)) ai++;
            // Find next valid block of this type in b
            while (bi < b->count && (b->memory[bi].block_type != t || !b->memory[bi].valid)) bi++;

            const fossil_jellyfish_block_t *ba = (ai < a->count) ? &a->memory[ai] : NULL;
            const fossil_jellyfish_block_t *bb = (bi < b->count) ? &b->memory[bi] : NULL;

            if (!ba && !bb) break; // Done with this type

            if (!ba || !bb || memcmp(ba->hash, bb->hash, FOSSIL_JELLYFISH_HASH_SIZE) != 0) {
                diff_count[t]++;
            }

            if (ba) { ai++; ac++; }
            if (bb) { bi++; bc++; }
        }
    }

    // Optionally, return total differences or encode per-type
    int total = 0;
    for (int t = 0; t < 5; ++t) total += diff_count[t];
    return total;
}

#define ROTL8(x, r) ((uint8_t)(((x) << (r)) | ((x) >> (8 - (r)))))

void fossil_jellyfish_chain_fingerprint(const fossil_jellyfish_chain_t *chain, uint8_t *out_hash) {
    if (!chain || !out_hash) return;

    // Per-type accumulators for better separation
    uint8_t type_hash[5][FOSSIL_JELLYFISH_HASH_SIZE] = {0};

    // Initialize each type's hash with a unique pattern
    for (int t = 0; t < 5; ++t)
        for (size_t i = 0; i < FOSSIL_JELLYFISH_HASH_SIZE; ++i)
            type_hash[t][i] = (uint8_t)(0xA5 ^ (i + t * 13));

    // Mix each block’s hash and timestamp into its type's buffer
    for (size_t i = 0; i < chain->count; ++i) {
        const fossil_jellyfish_block_t *b = &chain->memory[i];
        if (!b->valid) continue;
        int t = (b->block_type >= 0 && b->block_type <= 4) ? b->block_type : 0;

        for (size_t j = 0; j < FOSSIL_JELLYFISH_HASH_SIZE; ++j) {
            uint8_t h = b->hash[j];
            uint8_t ts = ((uint8_t *)&b->timestamp)[j % sizeof(uint64_t)];
            uint8_t rotated = ROTL8(h ^ ts, (j % 7) + 1);
            type_hash[t][j] ^= rotated ^ (j * 31 + i * 17 + t * 19);
        }
        // Mix in confidence and usage
        uint8_t conf = (uint8_t)(b->confidence * 255.0f);
        uint8_t usage = (uint8_t)(b->usage_count & 0xFF);
        type_hash[t][i % FOSSIL_JELLYFISH_HASH_SIZE] ^= conf ^ usage;
    }

    // Combine all type hashes into out_hash
    for (size_t j = 0; j < FOSSIL_JELLYFISH_HASH_SIZE; ++j) {
        uint8_t v = 0;
        for (int t = 0; t < 5; ++t)
            v ^= ROTL8(type_hash[t][j], t + 1);
        out_hash[j] = v;
    }
}

int fossil_jellyfish_trim(fossil_jellyfish_chain_t *chain, size_t max_blocks) {
    if (!chain || chain->count <= max_blocks) return 0;

    // For each block type, keep up to (max_blocks / 5) blocks of that type, sorted by confidence
    size_t per_type_max[5] = {0};
    size_t base = max_blocks / 5;
    size_t rem = max_blocks % 5;
    for (int t = 0; t < 5; ++t)
        per_type_max[t] = base + (t < (int)rem ? 1 : 0);

    // Temporary arrays to hold indices of blocks per type
    size_t indices[FOSSIL_JELLYFISH_MAX_MEM];
    size_t kept = 0;

    for (int t = 0; t < 5; ++t) {
        // Collect indices of valid blocks of this type
        size_t idx[FOSSIL_JELLYFISH_MAX_MEM];
        size_t n = 0;
        for (size_t i = 0; i < chain->count; ++i) {
            if (chain->memory[i].valid && chain->memory[i].block_type == t)
                idx[n++] = i;
        }
        // Sort indices by confidence descending
        for (size_t i = 0; i + 1 < n; ++i) {
            for (size_t j = i + 1; j < n; ++j) {
                if (chain->memory[idx[j]].confidence > chain->memory[idx[i]].confidence) {
                    size_t tmp = idx[i];
                    idx[i] = idx[j];
                    idx[j] = tmp;
                }
            }
        }
        // Keep up to per_type_max[t] blocks of this type
        size_t keep_n = (n > per_type_max[t]) ? per_type_max[t] : n;
        for (size_t k = 0; k < keep_n; ++k)
            indices[kept++] = idx[k];
    }

    // Rebuild chain with kept indices
    fossil_jellyfish_block_t new_mem[FOSSIL_JELLYFISH_MAX_MEM];
    size_t new_count = 0;
    for (size_t i = 0; i < kept; ++i)
        new_mem[new_count++] = chain->memory[indices[i]];

    // Zero out the rest
    for (size_t i = new_count; i < chain->count; ++i)
        memset(&new_mem[i], 0, sizeof(fossil_jellyfish_block_t));

    memcpy(chain->memory, new_mem, sizeof(new_mem));
    size_t removed = chain->count > new_count ? chain->count - new_count : 0;
    chain->count = new_count;
    return (int)removed;
}

int fossil_jellyfish_chain_compact(fossil_jellyfish_chain_t *chain) {
    if (!chain) return -1;

    size_t new_index[5] = {0, 0, 0, 0, 0};
    size_t moved = 0;

    // First, count how many valid blocks of each type and where to start writing them
    for (int t = JELLY_BLOCK_BASIC; t <= JELLY_BLOCK_VERIFIED; ++t) {
        for (size_t i = 0; i < chain->count; ++i) {
            if (chain->memory[i].valid && chain->memory[i].block_type == t) {
                ++new_index[t];
            }
        }
    }

    // Compute starting offsets for each type
    size_t offset[5] = {0, 0, 0, 0, 0};
    for (int t = 1; t < 5; ++t) {
        offset[t] = offset[t - 1] + new_index[t - 1];
    }

    // Temporary array to hold compacted blocks
    fossil_jellyfish_block_t temp[FOSSIL_JELLYFISH_MAX_MEM];
    memset(temp, 0, sizeof(temp));

    // Place valid blocks of each type in order
    size_t pos[5] = {offset[0], offset[1], offset[2], offset[3], offset[4]};
    for (size_t i = 0; i < chain->count; ++i) {
        fossil_jellyfish_block_t *b = &chain->memory[i];
        int t = (b->block_type >= 0 && b->block_type <= 4) ? b->block_type : 0;
        if (b->valid) {
            if (i != pos[t]) ++moved;
            temp[pos[t]++] = *b;
        }
    }

    // Copy back and zero the rest
    memcpy(chain->memory, temp, sizeof(temp));
    chain->count = offset[4] + new_index[4];
    return (int)moved;
}

uint64_t fossil_jellyfish_block_age(const fossil_jellyfish_block_t *block, uint64_t now) {
    if (!block || block->timestamp > now) return 0;

    switch (block->block_type) {
        case JELLY_BLOCK_BASIC:
        case JELLY_BLOCK_IMAGINED:
        case JELLY_BLOCK_DERIVED:
        case JELLY_BLOCK_EXPERIMENTAL:
        case JELLY_BLOCK_VERIFIED:
            return now - block->timestamp;
        default:
            return 0;
    }
}

void fossil_jellyfish_block_explain(const fossil_jellyfish_block_t *block, char *out, size_t size) {
    if (!block || !out || size == 0) return;

    static const char *type_names[] = {
        "BASIC", "IMAGINED", "DERIVED", "EXPERIMENTAL", "VERIFIED"
    };
    int t = (block->block_type >= 0 && block->block_type <= 4) ? block->block_type : 0;

    switch (t) {
        case JELLY_BLOCK_BASIC:
            snprintf(out, size,
                "[%s] Input: '%s' | Output: '%s' | Conf: %.2f | Used: %u | Immutable: %d | Valid: %d",
                type_names[t], block->input, block->output, block->confidence,
                block->usage_count, block->immutable, block->valid);
            break;
        case JELLY_BLOCK_IMAGINED:
            snprintf(out, size,
                "[%s] Input: '%s' | Output: '%s' | Conf: %.2f | Used: %u | Imagined: %d | Reason: '%s' | Immutable: %d | Valid: %d",
                type_names[t], block->input, block->output, block->confidence,
                block->usage_count, block->imagined, block->imagination_reason,
                block->immutable, block->valid);
            break;
        case JELLY_BLOCK_DERIVED:
            snprintf(out, size,
                "[%s] Input: '%s' | Output: '%s' | Conf: %.2f | Used: %u | Imagined: %d | From: %u | Reason: '%s' | Immutable: %d | Valid: %d",
                type_names[t], block->input, block->output, block->confidence,
                block->usage_count, block->imagined, block->imagined_from_index,
                block->imagination_reason, block->immutable, block->valid);
            break;
        case JELLY_BLOCK_EXPERIMENTAL:
            snprintf(out, size,
                "[%s] Input: '%s' | Output: '%s' | Conf: %.2f | Used: %u | Imagined: %d | Reason: '%s' | Immutable: %d | Valid: %d",
                type_names[t], block->input, block->output, block->confidence,
                block->usage_count, block->imagined, block->imagination_reason,
                block->immutable, block->valid);
            break;
        case JELLY_BLOCK_VERIFIED:
            snprintf(out, size,
                "[%s] Input: '%s' | Output: '%s' | Conf: %.2f | Used: %u | Immutable: %d | Valid: %d",
                type_names[t], block->input, block->output, block->confidence,
                block->usage_count, block->immutable, block->valid);
            break;
        default:
            snprintf(out, size,
                "[UNKNOWN] Input: '%s' | Output: '%s' | Conf: %.2f | Used: %u | Immutable: %d | Valid: %d",
                block->input, block->output, block->confidence,
                block->usage_count, block->immutable, block->valid);
            break;
    }
}

const fossil_jellyfish_block_t *fossil_jellyfish_find_by_hash(const fossil_jellyfish_chain_t *chain, const uint8_t *hash) {
    if (!chain || !hash) return NULL;

    // Track best match for each block type
    const fossil_jellyfish_block_t *best[5] = {NULL, NULL, NULL, NULL, NULL};

    for (size_t i = 0; i < chain->count; ++i) {
        const fossil_jellyfish_block_t *b = &chain->memory[i];
        int t = (b->block_type >= 0 && b->block_type <= 4) ? b->block_type : 0;
        if (b->valid && memcmp(b->hash, hash, FOSSIL_JELLYFISH_HASH_SIZE) == 0) {
            // Prefer highest confidence for each type
            if (!best[t] || b->confidence > best[t]->confidence) {
                best[t] = b;
            }
        }
    }

    // Priority order: VERIFIED > DERIVED > BASIC > IMAGINED > EXPERIMENTAL
    static const int priority[] = {
        JELLY_BLOCK_VERIFIED, JELLY_BLOCK_DERIVED, JELLY_BLOCK_BASIC, JELLY_BLOCK_IMAGINED, JELLY_BLOCK_EXPERIMENTAL
    };
    for (size_t p = 0; p < 5; ++p) {
        int t = priority[p];
        if (best[t]) return best[t];
    }

    return NULL;
}

int fossil_jellyfish_clone_chain(const fossil_jellyfish_chain_t *src, fossil_jellyfish_chain_t *dst) {
    if (!src || !dst) return -1;

    // For each block type, copy blocks of that type in order
    size_t dst_idx = 0;
    for (int t = JELLY_BLOCK_BASIC; t <= JELLY_BLOCK_VERIFIED; ++t) {
        for (size_t i = 0; i < src->count; ++i) {
            const fossil_jellyfish_block_t *block = &src->memory[i];
            if (block->block_type == t && block->valid) {
                if (dst_idx < FOSSIL_JELLYFISH_MAX_MEM) {
                    dst->memory[dst_idx++] = *block;
                }
            }
        }
    }
    dst->count = dst_idx;
    memcpy(dst->device_id, src->device_id, FOSSIL_DEVICE_ID_SIZE);
    dst->created_at = src->created_at;
    dst->updated_at = src->updated_at;
    // Zero unused memory
    for (size_t i = dst_idx; i < FOSSIL_JELLYFISH_MAX_MEM; ++i) {
        memset(&dst->memory[i], 0, sizeof(fossil_jellyfish_block_t));
    }
    return 0;
}

bool fossil_jellyfish_reason_verbose(const fossil_jellyfish_chain_t *chain, const char *input, char *out_output, float *out_confidence, const fossil_jellyfish_block_t **out_block) {
    if (!chain || !input) return false;

    // Track best match for each block type
    const fossil_jellyfish_block_t *best[5] = {NULL, NULL, NULL, NULL, NULL};
    float best_conf[5] = {-1.0f, -1.0f, -1.0f, -1.0f, -1.0f};

    for (size_t i = 0; i < chain->count; ++i) {
        const fossil_jellyfish_block_t *b = &chain->memory[i];
        int t = (b->block_type >= 0 && b->block_type <= 4) ? b->block_type : 0;
        if (!b->valid) continue;

        if (strncmp(b->input, input, FOSSIL_JELLYFISH_INPUT_SIZE) == 0) {
            if (b->confidence > best_conf[t] ||
                (b->confidence == best_conf[t] && b->immutable && (!best[t] || !best[t]->immutable))) {
                best[t] = b;
                best_conf[t] = b->confidence;
            }
        }
    }

    // Priority order: VERIFIED > DERIVED > BASIC > IMAGINED > EXPERIMENTAL
    static const int priority[] = {
        JELLY_BLOCK_VERIFIED, JELLY_BLOCK_DERIVED, JELLY_BLOCK_BASIC, JELLY_BLOCK_IMAGINED, JELLY_BLOCK_EXPERIMENTAL
    };
    for (size_t p = 0; p < 5; ++p) {
        int t = priority[p];
        if (best[t]) {
            if (out_output) strncpy(out_output, best[t]->output, FOSSIL_JELLYFISH_OUTPUT_SIZE);
            if (out_confidence) *out_confidence = best[t]->confidence;
            if (out_block) *out_block = best[t];
            return true;
        }
    }

    if (out_output) strncpy(out_output, "Unknown", FOSSIL_JELLYFISH_OUTPUT_SIZE);
    if (out_confidence) *out_confidence = 0.0f;
    if (out_block) *out_block = NULL;
    return false;
}

int fossil_jellyfish_block_sign(fossil_jellyfish_block_t *block, const uint8_t *priv_key) {
    if (!block) return -1;

    char key_string[64];
    if (priv_key) {
        for (size_t i = 0; i < 32 && i < FOSSIL_JELLYFISH_HASH_SIZE; ++i)
            sprintf(&key_string[i*2], "%02x", priv_key[i]);
    } else {
        snprintf(key_string, sizeof(key_string), "default-key");
    }

    // Generalize signature logic for all block types
    switch (block->block_type) {
        case JELLY_BLOCK_BASIC:
        case JELLY_BLOCK_IMAGINED:
        case JELLY_BLOCK_DERIVED:
        case JELLY_BLOCK_EXPERIMENTAL:
        case JELLY_BLOCK_VERIFIED:
            fossil_jellyfish_hash((const char *)block->hash, key_string, block->signature);
            break;
        default:
            // For unknown types, still sign but mark signature as all zeros
            memset(block->signature, 0, FOSSIL_SIGNATURE_SIZE);
            return -2;
    }
    return 0;
}

bool fossil_jellyfish_block_verify_signature(const fossil_jellyfish_block_t *block, const uint8_t *pub_key) {
    if (!block) return false;

    uint8_t expected[FOSSIL_SIGNATURE_SIZE];
    char key_string[64];

    if (pub_key) {
        for (size_t i = 0; i < 32 && i < FOSSIL_JELLYFISH_HASH_SIZE; ++i)
            sprintf(&key_string[i*2], "%02x", pub_key[i]);
    } else {
        snprintf(key_string, sizeof(key_string), "default-key");
    }

    // Generalize signature verification for all block types
    switch (block->block_type) {
        case JELLY_BLOCK_BASIC:
        case JELLY_BLOCK_IMAGINED:
        case JELLY_BLOCK_DERIVED:
        case JELLY_BLOCK_EXPERIMENTAL:
        case JELLY_BLOCK_VERIFIED:
            fossil_jellyfish_hash((const char *)block->hash, key_string, expected);
            break;
        default:
            // Unknown type: treat as invalid
            return false;
    }

    return memcmp(expected, block->signature, FOSSIL_SIGNATURE_SIZE) == 0;
}
