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
#include <stdbool.h>
#include <sys/types.h>

enum {
    FOSSIL_JELLYFISH_MAX_MEM          = 128,
    FOSSIL_JELLYFISH_HASH_SIZE        = 32,
    FOSSIL_JELLYFISH_INPUT_SIZE       = 64,
    FOSSIL_JELLYFISH_OUTPUT_SIZE      = 64,
    FOSSIL_JELLYFISH_MAX_MODELS       = 32,
    FOSSIL_JELLYFISH_MAX_TOKENS       = 16,
    FOSSIL_JELLYFISH_TOKEN_SIZE       = 16,
    FOSSIL_JELLYFISH_MAX_MODEL_FILES  = 16,
    FOSSIL_JELLYFISH_MAX_TAGS         = 8
};

#define FOSSIL_DEVICE_ID_SIZE      16   // E.g., 128-bit hardware ID
#define FOSSIL_SIGNATURE_SIZE      64   // ECDSA, ED25519, etc.

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
    uint64_t timestamp;               // Absolute UNIX timestamp
    uint32_t delta_ms;                // New: time since last block in ms
    uint32_t duration_ms;             // New: time taken to process this block
    int valid;
    float confidence;
    uint32_t usage_count;

    uint8_t device_id[FOSSIL_DEVICE_ID_SIZE];
    uint8_t signature[FOSSIL_SIGNATURE_SIZE];
} fossil_jellyfish_block;

/**
 * Represents a chain of jellyfish blocks.
 * This structure holds the memory for the jellyfish AI and tracks the number of blocks.
 */
typedef struct {
    fossil_jellyfish_block memory[FOSSIL_JELLYFISH_MAX_MEM];
    size_t count;
} fossil_jellyfish_chain;

/**
 * Represents a parsed JellyDSL structure for describing AI mindsets or knowledge.
 * This structure can be used to hold parsed data from a .jellyfish (JellyDSL) file.
 * Suitable for use in Truth Intelligence (TI) or Artificial Intelligence (AI) modules.
 */
typedef struct {
    char name[64];                                      // Name of the mindset or knowledge set
    char tags[FOSSIL_JELLYFISH_MAX_TAGS][32];           // Tags for categorization
    size_t tag_count;                                   // Number of tags used
    char description[256];                              // Optional description or notes
    fossil_jellyfish_chain chain;                       // Associated memory chain
    char models[FOSSIL_JELLYFISH_MAX_MODELS][32];       // List of models associated with this mindset
    int priority;                                       // Priority level for processing
    float confidence_threshold;                        // Confidence threshold for decisions
    int model_count;                                     // Number of models in this mindset
} fossil_jellyfish_jellydsl;

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

/**
 * Save the jellyfish chain to a file.
 * This serializes the chain to a file for persistence.
 * 
 * @param chain Pointer to the jellyfish chain to save.
 * @param filepath The path to the file where the chain will be saved.
 * @return 0 on success, non-zero on failure.
 */
int fossil_jellyfish_save(const fossil_jellyfish_chain *chain, const char *filepath);

/**
 * Load a jellyfish chain from a file.
 * This deserializes the chain from a file.
 * 
 * @param chain Pointer to the jellyfish chain to load.
 * @param filepath The path to the file from which the chain will be loaded.
 * @return 0 on success, non-zero on failure.
 */
int fossil_jellyfish_load(fossil_jellyfish_chain *chain, const char *filepath);

/**
 * Decay the confidence of the jellyfish chain.
 * This reduces the confidence of all blocks in the chain by a specified decay rate.
 * 
 * @param chain Pointer to the jellyfish chain.
 * @param decay_rate The rate at which to decay confidence (0.0 - 1.0).
 */
void fossil_jellyfish_decay_confidence(fossil_jellyfish_chain *chain, float decay_rate);

/**
 * Tokenizes a given input string into lowercase word tokens.
 *
 * @param input       Null-terminated string to tokenize.
 * @param tokens      2D array to store output tokens (each max FOSSIL_JELLYFISH_TOKEN_SIZE).
 * @param max_tokens  Maximum number of tokens to extract.
 * @return            The number of tokens actually written to the tokens array.
 */
size_t fossil_jellyfish_tokenize(const char *input, char tokens[][FOSSIL_JELLYFISH_TOKEN_SIZE], size_t max_tokens);

/**
 * Returns a pointer to the memory block in the chain with the highest confidence score.
 *
 * @param chain  Pointer to the memory chain.
 * @return       Pointer to the best fossil_jellyfish_block, or NULL if no valid memory exists.
 */
const fossil_jellyfish_block *fossil_jellyfish_best_memory(const fossil_jellyfish_chain *chain);

/**
 * Calculates a normalized score representing how "full" or utilized the knowledge base is.
 *
 * @param chain  Pointer to the memory chain.
 * @return       Float between 0.0 and 1.0 indicating knowledge coverage.
 */
float fossil_jellyfish_knowledge_coverage(const fossil_jellyfish_chain *chain);

/**
 * Checks if adding a given input-output pair would contradict existing memory.
 *
 * @param chain   Pointer to the memory chain.
 * @param input   Input to check.
 * @param output  Output to check.
 * @return        1 if a conflict is found, 0 otherwise.
 */
int fossil_jellyfish_detect_conflict(const fossil_jellyfish_chain *chain, const char *input, const char *output);

/**
 * Prints a self-reflection report of the current memory chain to stdout.
 * Includes memory size, confidence distribution, usage patterns, and top entries.
 *
 * @param chain  Pointer to the memory chain to reflect on.
 */
void fossil_jellyfish_reflect(const fossil_jellyfish_chain *chain);

/**
 * Verifies the integrity of a jellyfish block.
 * This checks if the block has valid input, output, and hash.
 * 
 * @param block Pointer to the jellyfish block to verify.
 * @return True if the block is valid, false otherwise.
 */
bool fossil_jellyfish_verify_block(const fossil_jellyfish_block* block);

/**
 * Verifies the integrity of a jellyfish chain.
 * This checks if all blocks are valid and properly linked.
 * 
 * @param chain Pointer to the jellyfish chain to verify.
 * @return True if the chain is valid, false otherwise.
 */
bool fossil_jellyfish_verify_chain(const fossil_jellyfish_chain* chain);

/**
 * Parses a .jellyfish file and extracts mindsets.
 * 
 * @param filepath       Path to the .jellyfish file.
 * @param out_mindsets   Array to store parsed mindsets.
 * @param max_mindsets   Maximum number of mindsets to store.
 * @return               Number of mindsets parsed, or 0 on failure.
 */
int fossil_jellyfish_parse_jellyfish_file(const char *filepath, fossil_jellyfish_jellydsl *out, int max_chains);

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
        // Constructor and destructor
        JellyfishAI() { fossil_jellyfish_init(&chain_); }

        // Disable copy and move semantics
        ~JellyfishAI() {}

        /**
         * Learn a new input-output pair.
         * 
         * @param input The input string to learn.
         * @param output The output string corresponding to the input.
         */
        void learn(const char* input, const char* output) {
            fossil_jellyfish_learn(&chain_, input, output);
        }

        /**
         * Get a reasoned response for a given input.
         * 
         * @param input The input string to reason about.
         * @return The output string corresponding to the input.
         */
        std::string reason(const char* input) {
            const char* result = fossil_jellyfish_reason(&chain_, input);
            return std::string(result ? result : "Unknown");
        }

        /**
         * Initialize the jellyfish AI.
         */
        void init() {
            fossil_jellyfish_init(&chain_);
        }

        /**
         * Cleanup the jellyfish AI.
         */
        void cleanup() {
            fossil_jellyfish_cleanup(&chain_);
        }

        /**
         * Dump the current state of the jellyfish AI.
         */
        void dump() const {
            fossil_jellyfish_dump(&chain_);
        }

        /**
         * Save the current state of the jellyfish AI to a file.
         * 
         * @param filepath The path to the file to save to.
         * @return True if the save was successful, false otherwise.
         */
        bool save(const char* filepath) const {
            return fossil_jellyfish_save(&chain_, filepath) == 0;
        }

        /**
         * Load the jellyfish AI state from a file.
         * 
         * @param filepath The path to the file to load from.
         * @return True if the load was successful, false otherwise.
         */
        bool load(const char* filepath) {
            return fossil_jellyfish_load(&chain_, filepath) == 0;
        }

        /**
         * Decay the confidence of the AI's knowledge.
         * 
         * @param decay_rate The rate at which to decay confidence.
         */
        void decay_confidence(float decay_rate) {
            fossil_jellyfish_decay_confidence(&chain_, decay_rate);
        }

        /**
         * Get the knowledge coverage of the AI.
         * 
         * @return The knowledge coverage as a float.
         */
        float knowledge_coverage() const {
            return fossil_jellyfish_knowledge_coverage(&chain_);
        }

        /**
         * Get the best memory block of the AI.
         * 
         * @return A pointer to the best memory block.
         */
        const fossil_jellyfish_block* best_memory() const {
            return fossil_jellyfish_best_memory(&chain_);
        }

        /**
         * Detect if a given input-output pair would conflict with existing memories.
         * 
         * @param input The input string to check.
         * @param output The output string to check.
         * @return 1 if a conflict is detected, 0 otherwise.
         */
        int detect_conflict(const char* input, const char* output) const {
            return fossil_jellyfish_detect_conflict(&chain_, input, output);
        }

        /**
         * Reflect on the current state of the AI.
         * This prints a self-reflection report to stdout.
         */
        void reflect() const {
            fossil_jellyfish_reflect(&chain_);
        }

        /**
         * Parse a .jellyfish file and extract mindsets.
         * 
         * @param filepath The path to the .jellyfish file.
         * @param out_mindsets Vector to store parsed mindsets.
         * @return The number of mindsets parsed, or 0 on failure.
         */
        const fossil_jellyfish_chain& get_chain() const { return chain_; }

        /**         * Parse a .jellyfish file and extract mindsets.
         * 
         * @param filepath The path to the .jellyfish file.
         * @param out_mindsets Vector to store parsed mindsets.
         * @return The number of mindsets parsed, or 0 on failure.
         */
        fossil_jellyfish_chain& get_chain() { return chain_; }

    private:
        fossil_jellyfish_chain chain_;
    };

} // namespace ai

} // namespace fossil

#endif

#endif /* fossil_fish_FRAMEWORK_H */
