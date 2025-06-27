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
#include <time.h>

// Simple placeholder hash function (not cryptographic, just consistent)
void fossil_jellyfish_hash(const char *input, const char *output, uint8_t *hash_out) {
    // Enhanced: Use a simple FNV-1a-like mixing over input and output
    const uint32_t FNV_PRIME = 0x01000193;
    uint32_t hash = 0x811c9dc5;
    size_t in_len = strlen(input);
    size_t out_len = strlen(output);

    // Mix input
    for (size_t i = 0; i < in_len; ++i) {
        hash ^= (uint8_t)input[i];
        hash *= FNV_PRIME;
    }
    // Mix output
    for (size_t i = 0; i < out_len; ++i) {
        hash ^= (uint8_t)output[i];
        hash *= FNV_PRIME;
    }

    // Spread hash into hash_out buffer
    for (size_t i = 0; i < FOSSIL_JELLYFISH_HASH_SIZE; ++i) {
        hash ^= (hash >> 13);
        hash *= FNV_PRIME;
        hash_out[i] = (uint8_t)((hash >> (8 * (i % 4))) & 0xFF);
    }
}

void fossil_jellyfish_init(fossil_jellyfish_chain *chain) {
    chain->count = 0;
    memset(chain->memory, 0, sizeof(chain->memory));
}

void fossil_jellyfish_learn(fossil_jellyfish_chain *chain, const char *input, const char *output) {
    if (chain->count >= FOSSIL_JELLYFISH_MAX_MEM) {
        fossil_jellyfish_cleanup(chain); // Reclaim space
    }

    fossil_jellyfish_block *block = &chain->memory[chain->count++];
    strncpy(block->input, input, FOSSIL_JELLYFISH_INPUT_SIZE - 1);
    strncpy(block->output, output, FOSSIL_JELLYFISH_OUTPUT_SIZE - 1);
    block->timestamp = (uint64_t)time(NULL);
    block->valid = 1;

    fossil_jellyfish_hash(input, output, block->hash);
}

const char* fossil_jellyfish_reason(fossil_jellyfish_chain *chain, const char *input) {
    for (size_t i = 0; i < chain->count; ++i) {
        if (chain->memory[i].valid && strncmp(chain->memory[i].input, input, FOSSIL_JELLYFISH_INPUT_SIZE) == 0) {
            return chain->memory[i].output;
        }
    }
    return "Unknown";
}

// Removes blocks that are older and marked invalid (or just garbage collect oldest)
void fossil_jellyfish_cleanup(fossil_jellyfish_chain *chain) {
    size_t new_count = 0;
    for (size_t i = 0; i < chain->count; ++i) {
        if (chain->memory[i].valid && (time(NULL) - chain->memory[i].timestamp < 3600)) {
            chain->memory[new_count++] = chain->memory[i];
        }
    }
    chain->count = new_count;
}

void fossil_jellyfish_dump(const fossil_jellyfish_chain *chain) {
    for (size_t i = 0; i < chain->count; ++i) {
        printf("Block %zu:\n", i);
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
