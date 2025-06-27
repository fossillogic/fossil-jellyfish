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
#ifndef FOSSIL_JELLYFISH_AI_H
#define FOSSIL_JELLYFISH_AI_H

#include <stdint.h>
#include <stddef.h>

#define FOSSIL_JELLYFISH_MAX_MEM 128
#define FOSSIL_JELLYFISH_HASH_SIZE 32
#define FOSSIL_JELLYFISH_INPUT_SIZE 64
#define FOSSIL_JELLYFISH_OUTPUT_SIZE 64

#ifdef __cplusplus
extern "C"
{
#endif

// *****************************************************************************
// Type definitions
// *****************************************************************************

/**
 * Represents a single jellyfish block in the AI memory.
 * Each block contains an input, output, hash, timestamp, and validity flag.
 */
typedef struct {
    char input[FOSSIL_JELLYFISH_INPUT_SIZE];
    char output[FOSSIL_JELLYFISH_OUTPUT_SIZE];
    uint8_t hash[FOSSIL_JELLYFISH_HASH_SIZE];
    uint64_t timestamp;
    int valid;
} fossil_jellyfish_block;

/**
 * Represents a chain of jellyfish blocks.
 * This structure holds the memory for the jellyfish AI and tracks the number of blocks.
 */
typedef struct {
    fossil_jellyfish_block memory[FOSSIL_JELLYFISH_MAX_MEM];
    size_t count;
} fossil_jellyfish_chain;

// *****************************************************************************
// Function prototypes
// *****************************************************************************

/**
 * Initialize the jellyfish chain.
 * This sets up the initial state of the chain.
 * 
 * @param chain Pointer to the jellyfish chain to initialize.
 */
void fossil_jellyfish_init(fossil_jellyfish_chain *chain);

/**
 * Learn a new input-output pair.
 * This adds a new block to the chain with the given input and output.
 * 
 * @param chain Pointer to the jellyfish chain.
 * @param input The input string to learn.
 * @param output The output string corresponding to the input.
 */
void fossil_jellyfish_learn(fossil_jellyfish_chain *chain, const char *input, const char *output);

/**
 * Reason about an input.
 * This searches the chain for a matching input and returns the corresponding output.
 * 
 * @param chain Pointer to the jellyfish chain.
 * @param input The input string to reason about.
 * @return The output string if found, or "Unknown" if not found.
 */
const char* fossil_jellyfish_reason(fossil_jellyfish_chain *chain, const char *input);

/**
 * Cleanup the jellyfish chain.
 * This removes old or invalid blocks from the chain to reclaim space.
 * 
 * @param chain Pointer to the jellyfish chain to clean up.
 */
void fossil_jellyfish_cleanup(fossil_jellyfish_chain *chain);

/**
 * Dump the contents of the jellyfish chain.
 * This prints the current state of the chain to standard output.
 * 
 * @param chain Pointer to the jellyfish chain to dump.
 */
void fossil_jellyfish_dump(const fossil_jellyfish_chain *chain);

/**
 * Generate a hash for the given input and output.
 * This computes a hash based on the input and output strings.
 * 
 * @param input The input string to hash.
 * @param output The output string to hash.
 * @param hash_out Pointer to an array where the resulting hash will be stored.
 */
void fossil_jellyfish_hash(const char *input, const char *output, uint8_t *hash_out);

#ifdef __cplusplus
}
#include <stdexcept>
#include <vector>
#include <string>

namespace fossil {

namespace ai {

    // C++ wrapper for the jellyfish AI
    class JellyfishAI {
    public:
        /**
         * Constructor for JellyfishAI.
         * Initializes the jellyfish chain.
         */
        JellyfishAI() {
            fossil_jellyfish_init(&chain);
        }

        /**
         * Destructor for JellyfishAI.
         * Cleans up the jellyfish chain.
         */
        ~JellyfishAI() {
            cleanup();
        }

        /**
         * Learn a new input-output pair.
         * This adds a new block to the chain with the given input and output.
         *
         * @param input The input string to learn.
         * @param output The output string corresponding to the input.
         */
        void learn(const std::string &input, const std::string &output) {
            fossil_jellyfish_learn(&chain, input.c_str(), output.c_str());
        }

        /**
         * Reason about an input.
         * This searches the chain for a matching input and returns the corresponding output.
         *
         * @param input The input string to reason about.
         * @return The output string if found, or "Unknown" if not found.
         */
        std::string reason(const std::string &input) {
            const char *result = fossil_jellyfish_reason(&chain, input.c_str());
            return std::string(result);
        }

        /**
         * Cleanup the jellyfish chain.
         * This removes old or invalid blocks from the chain to reclaim space.
         */
        void cleanup() {
            fossil_jellyfish_cleanup(&chain);
        }

        /**
         * Dump the contents of the jellyfish chain.
         * This prints the current state of the chain to standard output.
         */
        void dump() const {
            fossil_jellyfish_dump(&chain);
        }

        /**
         * Generate a hash for the given input and output.
         * This computes a hash based on the input and output strings.
         *
         * @param input The input string to hash.
         * @param output The output string to hash.
         * @param hash_out Reference to a vector where the resulting hash will be stored.
         */
        void hash(const std::string &input, const std::string &output, std::vector<uint8_t> &hash_out) {
            hash_out.resize(FOSSIL_JELLYFISH_HASH_SIZE);
            fossil_jellyfish_hash(input.c_str(), output.c_str(), hash_out.data());
        }

        /**
         * Get the underlying jellyfish chain.
         * This provides access to the raw chain data.
         *
         * @return Reference to the jellyfish chain.
         */
        const fossil_jellyfish_chain& get_chain() const {
            return chain;
        }

        /**
         * Reset the jellyfish chain.
         * This reinitializes the chain to its default state.
         */
        void reset() {
            fossil_jellyfish_init(&chain);
        }

    private:
        fossil_jellyfish_chain chain; // The jellyfish chain instance
    };

} // namespace ai

} // namespace fossil

#endif

#endif /* fossil_fish_FRAMEWORK_H */
