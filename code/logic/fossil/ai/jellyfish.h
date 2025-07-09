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
#define FOSSIL_JELLYFISH_MAX_MODELS 32
#define FOSSIL_JELLYFISH_MAX_TOKENS 16
#define FOSSIL_JELLYFISH_TOKEN_SIZE 16

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
    float confidence;   // New: how trusted this block is (0.0 - 1.0)
    uint32_t usage_count; // New: how often it's used
} fossil_jellyfish_block;


/**
 * Represents a chain of jellyfish blocks.
 * This structure holds the memory for the jellyfish AI and tracks the number of blocks.
 */
typedef struct {
    fossil_jellyfish_block memory[FOSSIL_JELLYFISH_MAX_MEM];
    size_t count;
} fossil_jellyfish_chain;

typedef struct {
    fossil_jellyfish_chain models[FOSSIL_JELLYFISH_MAX_MODELS];
    char model_names[FOSSIL_JELLYFISH_MAX_MODELS][64];
    size_t model_count;
} fossil_jellyfish_mind;

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
 * Fuzzy reasoning for jellyfish AI.
 * This function attempts to find a close match for the input string
 * and returns the corresponding output if found.
 * 
 * @param chain Pointer to the jellyfish chain.
 * @param input The input string to reason about.
 * @return The output string if a close match is found, or "Unknown" if not found.
 */
const char* fossil_jellyfish_reason_fuzzy(fossil_jellyfish_chain *chain, const char *input);

/**
 * Get the reason chain for a given input.
 * This function provides a detailed explanation of how the AI arrived at its reasoning.
 * 
 * @param chain Pointer to the jellyfish chain.
 * @param input The input string to reason about.
 * @param depth The depth of reasoning to explore (0 for no depth).
 * @return A string explaining the reasoning process, or "Unknown" if not found.
 */
const char* fossil_jellyfish_reason_chain(fossil_jellyfish_chain *chain, const char *input, int depth);

/**
 * Decay the confidence of the jellyfish chain.
 * This reduces the confidence of all blocks in the chain by a specified decay rate.
 * 
 * @param chain Pointer to the jellyfish chain.
 * @param decay_rate The rate at which to decay confidence (0.0 - 1.0).
 */
void fossil_jellyfish_decay_confidence(fossil_jellyfish_chain *chain, float decay_rate);

/**
 * Loads a named memory model from a file into the specified mind.
 *
 * @param mind      Pointer to the target mind structure.
 * @param filepath  Path to the .fish or binary model file.
 * @param name      Name to associate with the loaded model.
 * @return          1 if successful, 0 on failure.
 */
int fossil_jellyfish_mind_load_model(fossil_jellyfish_mind *mind, const char *filepath, const char *name);

/**
 * Performs reasoning using the mind's active memory chain.
 *
 * @param mind   Pointer to the mind structure.
 * @param input  Input question or statement to reason about.
 * @return       Output string representing the closest known answer or reasoning result.
 */
const char* fossil_jellyfish_mind_reason(fossil_jellyfish_mind *mind, const char *input);

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

        /**
         * Fuzzy reasoning for jellyfish AI.
         * This function attempts to find a close match for the input string
         * and returns the corresponding output if found.
         *
         * @param input The input string to reason about.
         * @return The output string if a close match is found, or "Unknown" if not found.
         */
        std::string reason_fuzzy(const std::string &input) {
            const char *result = fossil_jellyfish_reason_fuzzy(&chain, input.c_str());
            return std::string(result);
        }

        /**
         * Save the jellyfish chain to a file.
         * This serializes the chain to a file for persistence.
         *
         * @param filepath The path to the file where the chain will be saved.
         * @return 0 on success, non-zero on failure.
         */
        int save(const std::string &filepath) const {
            return fossil_jellyfish_save(&chain, filepath.c_str());
        }

        /**
         * Load a jellyfish chain from a file.
         * This deserializes the chain from a file.
         *
         * @param filepath The path to the file from which the chain will be loaded.
         * @return 0 on success, non-zero on failure.
         */
        int load(const std::string &filepath) {
            return fossil_jellyfish_load(&chain, filepath.c_str());
        }

        /**
         * Get the reason chain for a given input.
         * This function provides a detailed explanation of how the AI arrived at its reasoning.
         *
         * @param input The input string to reason about.
         * @param depth The depth of reasoning to explore (0 for no depth).
         * @return A string explaining the reasoning process, or "Unknown" if not found.
         */
        std::string reason_chain(const std::string &input, int depth = 0) {
            const char *result = fossil_jellyfish_reason_chain(&chain, input.c_str(), depth);
            return std::string(result);
        }

        /**
         * Decay the confidence of the jellyfish chain.
         * This reduces the confidence of all blocks in the chain by a specified decay rate.
         *
         * @param decay_rate The rate at which to decay confidence (0.0 - 1.0).
         */
        void decay_confidence(float decay_rate) {
            fossil_jellyfish_decay_confidence(&chain, decay_rate);
        }
        
        /**
     * @brief Load a named model into the mind from a .fish or binary file.
     *
     * @param filepath Path to the model file.
     * @param name     Logical name for the model within the mind.
     * @return True if loaded successfully, false on failure.
     */
    bool load_model(const std::string &filepath, const std::string &name) {
        return fossil_jellyfish_mind_load_model(&mind, filepath.c_str(), name.c_str()) != 0;
    }

    /**
     * @brief Perform reasoning across all loaded models.
     *
     * @param input Input question or query.
     * @return The best known answer string, or "Unknown".
     */
    std::string reason(const std::string &input) {
        return std::string(fossil_jellyfish_mind_reason(&mind, input.c_str()));
    }

    /**
     * @brief Tokenize the input string into lowercase word tokens.
     *
     * @param input The input string to tokenize.
     * @return A vector of token strings (max size determined by macro).
     */
    std::vector<std::string> tokenize(const std::string &input) {
        char tokens[FOSSIL_JELLYFISH_MAX_TOKENS][FOSSIL_JELLYFISH_TOKEN_SIZE] = {};
        size_t count = fossil_jellyfish_tokenize(input.c_str(), tokens, FOSSIL_JELLYFISH_MAX_TOKENS);

        std::vector<std::string> result;
        for (size_t i = 0; i < count; ++i) {
            result.emplace_back(tokens[i]);
        }
        return result;
    }

    /**
     * @brief Get the memory block with the highest confidence.
     *
     * @param chain Reference to a memory chain.
     * @return Pointer to the best memory block, or nullptr if none.
     */
    const fossil_jellyfish_block* best_memory(const fossil_jellyfish_chain &chain) const {
        return fossil_jellyfish_best_memory(&chain);
    }

    /**
     * @brief Get a knowledge coverage score from the given chain.
     *
     * @param chain Reference to the chain.
     * @return A float from 0.0 to 1.0 indicating how full the chain is.
     */
    float knowledge_coverage(const fossil_jellyfish_chain &chain) const {
        return fossil_jellyfish_knowledge_coverage(&chain);
    }

    /**
     * @brief Check if the input-output pair conflicts with existing memory.
     *
     * @param chain  The chain to check against.
     * @param input  Input string.
     * @param output Output string.
     * @return True if a contradiction exists, false otherwise.
     */
    bool detect_conflict(const fossil_jellyfish_chain &chain, const std::string &input, const std::string &output) const {
        return fossil_jellyfish_detect_conflict(&chain, input.c_str(), output.c_str()) != 0;
    }

    /**
     * @brief Print a self-reflection report about a chain.
     *
     * @param chain Reference to the memory chain to reflect on.
     */
    void reflect(const fossil_jellyfish_chain &chain) const {
        fossil_jellyfish_reflect(&chain);
    }

    /**
     * @brief Access the underlying C mind object.
     *
     * @return Reference to the mind struct.
     */
    fossil_jellyfish_mind& raw() { return mind; }

    private:
        fossil_jellyfish_mind mind; ///< Internal mind state holding multiple chains/models.
        fossil_jellyfish_chain chain; // The jellyfish chain instance
    };

} // namespace ai

} // namespace fossil

#endif

#endif /* fossil_fish_FRAMEWORK_H */
