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
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <float.h>
#include <time.h>
#include <math.h>

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
 * @brief Represents a single memory block in the Jellyfish AI chain.
 *
 * Each block stores a learned input-output pair, metadata about usage,
 * cryptographic fingerprinting, and timing. The `immutable` flag ensures
 * the block is protected from decay, pruning, or accidental overwrites.
 *
 * Indicates whether this block is imagined (not from real interaction),
 * and optionally links to a "source" block or reasoning trail.
 */
typedef struct {
    char input[FOSSIL_JELLYFISH_INPUT_SIZE];
    char output[FOSSIL_JELLYFISH_OUTPUT_SIZE];
    uint8_t hash[FOSSIL_JELLYFISH_HASH_SIZE];
    uint64_t timestamp;
    uint32_t delta_ms;
    uint32_t duration_ms;
    int valid;
    float confidence;
    uint32_t usage_count;

    uint8_t device_id[FOSSIL_DEVICE_ID_SIZE];
    uint8_t signature[FOSSIL_SIGNATURE_SIZE];
    int immutable;

    // Imagination-related fields:
    int imagined;                             // New: 1 = imagined, 0 = real
    uint32_t imagined_from_index;             // Optional: index of source block
    char imagination_reason[128];             // Optional: short explanation ("extrapolated from similar pattern", etc.)
} fossil_jellyfish_block;

/**
 * Represents a chain of jellyfish blocks.
 * This structure holds the memory for the jellyfish AI and tracks the number of blocks.
 */
typedef struct {
    fossil_jellyfish_block memory[FOSSIL_JELLYFISH_MAX_MEM];
    size_t count;

    uint8_t device_id[FOSSIL_DEVICE_ID_SIZE];  // ← Needed for file I/O
    uint64_t created_at;                       // ← Timestamp when this chain was created
    uint64_t updated_at;                       // ← Timestamp when it was last updated
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
 * @brief Generates a speculative (imagined) block based on an existing one.
 *
 * @param chain The Jellyfish memory chain to insert into.
 * @param source_index Index of the source block in the chain.
 * @param new_output The imagined output string.
 * @param reason Short explanation of the imagination logic.
 * @return int Index of the new imagined block, or -1 on failure.
 */
int fossil_jellyfish_imagine_block(fossil_jellyfish_chain* chain, size_t source_index, const char* new_output, const char* reason);

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
 * @brief Prints a validation report for each memory block in the Jellyfish chain.
 *
 * Iterates through the chain and invokes `fossil_jellyfish_verify_block` on each valid block,
 * outputting a status line for each entry to standard output.
 *
 * This helps in debugging chain integrity, visualizing where corruption or verification
 * failures occur, and understanding the structure of the chain.
 *
 * @param chain Pointer to the Jellyfish chain to validate.
 */
void fossil_jellyfish_validation_report(const fossil_jellyfish_chain *chain);

/**
 * @brief Performs full-chain validation by verifying each block.
 *
 * Calls `fossil_jellyfish_verify_block` for every block in the chain and returns
 * false if any verification fails.
 *
 * This function is useful for asserting the integrity of a deserialized chain,
 * checking for tampering, or before accepting input from external sources.
 *
 * @param chain Pointer to the Jellyfish chain to verify.
 * @return true if all blocks pass verification, false otherwise.
 */
bool fossil_jellyfish_verify_chain(const fossil_jellyfish_chain *chain);

/**
 * @brief Computes a normalized trust score for the Jellyfish chain.
 *
 * Only includes valid and immutable blocks in the scoring. Confidence values from
 * eligible blocks are averaged to produce a floating-point trust score from 0.0 to 1.0.
 *
 * This metric can help determine the chain’s overall credibility, for instance before
 * deploying, merging, or persisting long-term knowledge.
 *
 * @param chain Pointer to the Jellyfish chain to analyze.
 * @return Trust score (0.0f to 1.0f), or 0.0f if the chain is null or has no valid entries.
 */
float fossil_jellyfish_chain_trust_score(const fossil_jellyfish_chain *chain);

/**
 * @brief Marks a Jellyfish memory block as immutable.
 *
 * This flag indicates that the block should not be altered or pruned, and will be
 * included in trust score calculations. Immutable blocks are useful for storing core
 * logic, critical responses, or verified inputs that must persist through decay or pruning.
 *
 * @param block Pointer to the memory block to mark as immutable.
 */
void fossil_jellyfish_mark_immutable(fossil_jellyfish_block *block);

/**
 * @brief Prunes invalid or low-confidence blocks from the chain.
 * @param chain The jellyfish memory chain.
 * @param min_confidence Threshold below which memories are removed.
 * @return Number of blocks pruned.
 */
int fossil_jellyfish_prune_chain(fossil_jellyfish_chain *chain, float min_confidence);

/**
 * @brief Deduplicates blocks with identical input/output pairs.
 * @param chain The jellyfish memory chain.
 * @return Number of duplicates removed.
 */
int fossil_jellyfish_deduplicate_chain(fossil_jellyfish_chain *chain);

/**
 * @brief Compresses the memory chain by trimming whitespace and optionally shrinking fields.
 * @param chain The jellyfish memory chain.
 * @return Number of blocks modified.
 */
int fossil_jellyfish_compress_chain(fossil_jellyfish_chain *chain);

/**
 * @brief Finds the best matching memory block in the chain for a given input string.
 * 
 * It selects the most confident valid response, favoring immutable blocks if tied,
 * and optionally factoring in future enhancements like string similarity or recency.
 * 
 * @param chain The Jellyfish chain to search.
 * @param input The input string to match.
 * @return Pointer to the best matching block, or NULL if none found.
 */
const fossil_jellyfish_block* fossil_jellyfish_best_match(const fossil_jellyfish_chain *chain, const char *input);

/**
 * @brief Redacts sensitive data in a memory block while retaining structural integrity.
 *
 * Overwrites the input and/or output fields with fixed tokens (e.g. "***REDACTED***").
 * May be used before public export or sharing across systems.
 *
 * @param block Pointer to the block to redact.
 * @return 0 on success, non-zero on error.
 */
int fossil_jellyfish_redact_block(fossil_jellyfish_block *block);

/**
 * @brief Computes statistics over the Jellyfish chain.
 *
 * Populates out parameters with stats like count, valid count, confidence mean,
 * trust score, block age distribution, and immutable ratio.
 *
 * @param chain Pointer to the chain to analyze.
 * @param out_valid_count Pointer to store number of valid blocks.
 * @param out_avg_confidence Pointer to store average confidence score.
 * @param out_immutable_ratio Pointer to store immutable block ratio.
 */
void fossil_jellyfish_chain_stats(const fossil_jellyfish_chain *chain, size_t *out_valid_count, float *out_avg_confidence, float *out_immutable_ratio);

/**
 * @brief Compares two Jellyfish chains and identifies block-level differences.
 *
 * May be used for verifying synchronization, deduplication between devices,
 * or forensic audits (e.g., tampering or divergence).
 *
 * @param a First chain.
 * @param b Second chain.
 * @return Number of differing blocks, or -1 on error.
 */
int fossil_jellyfish_compare_chains(const fossil_jellyfish_chain *a, const fossil_jellyfish_chain *b);

/**
 * @brief Computes a single fingerprint hash for the entire chain.
 *
 * Hashes block hashes, timestamps, and content summary to produce a deterministic
 * digest for the chain’s current state.
 *
 * @param chain The Jellyfish chain to hash.
 * @param out_hash Output buffer of FOSSIL_JELLYFISH_HASH_SIZE bytes.
 */
void fossil_jellyfish_chain_fingerprint(const fossil_jellyfish_chain *chain, uint8_t *out_hash);

/**
 * @brief Trims the chain to retain only the N most recently used or most confident blocks.
 *
 * Used for constrained environments or audit-controlled exports.
 *
 * @param chain The Jellyfish chain.
 * @param max_blocks Number of blocks to retain.
 * @return Number of blocks removed.
 */
int fossil_jellyfish_trim(fossil_jellyfish_chain *chain, size_t max_blocks);

/**
 * @brief Reorders valid blocks to the front of the chain and removes gaps.
 *
 * Maintains block order by timestamp and shrinks memory footprint after pruning or trimming.
 *
 * @param chain The Jellyfish chain.
 * @return Number of blocks moved.
 */
int fossil_jellyfish_chain_compact(fossil_jellyfish_chain *chain);

/**
 * @brief Computes the age of a block relative to current time.
 *
 * @param block The memory block.
 * @param now   Current UNIX timestamp.
 * @return      Age in milliseconds.
 */
uint64_t fossil_jellyfish_block_age(const fossil_jellyfish_block *block, uint64_t now);

/**
 * @brief Returns a short diagnostic string for a block.
 *
 * Outputs a line including input, output, confidence, usage, and trust status.
 * Useful for human-readable debug tools or logging systems.
 *
 * @param block The block to describe.
 * @param out   Output buffer.
 * @param size  Size of the buffer.
 */
void fossil_jellyfish_block_explain(const fossil_jellyfish_block *block, char *out, size_t size);

/**
 * @brief Finds a memory block by its hash.
 *
 * @param chain The Jellyfish chain.
 * @param hash  The 32-byte hash to search for.
 * @return Pointer to the matching block, or NULL.
 */
const fossil_jellyfish_block *fossil_jellyfish_find_by_hash(const fossil_jellyfish_chain *chain, const uint8_t *hash);

/**
 * @brief Creates a deep copy of a Jellyfish chain.
 *
 * @param src Source chain.
 * @param dst Destination chain.
 * @return 0 on success, non-zero on error.
 */
int fossil_jellyfish_clone_chain(const fossil_jellyfish_chain *src, fossil_jellyfish_chain *dst);

/**
 * @brief Like `reason`, but includes match confidence, source block, and hash.
 *
 * Useful for debug, inspection, or high-trust outputs.
 *
 * @param chain  Pointer to the memory chain.
 * @param input  Input string to reason with.
 * @param out_output  Output string buffer.
 * @param out_confidence Optional confidence return pointer.
 * @param out_block Optional pointer to store matching block.
 * @return True if a match was found, false otherwise.
 */
bool fossil_jellyfish_reason_verbose(const fossil_jellyfish_chain *chain, const char *input, char *out_output, float *out_confidence, const fossil_jellyfish_block **out_block);

/**
 * @brief Signs a Jellyfish block using a private key.
 * @param block The block to sign.
 * @param priv_key The private key (implementation-defined).
 * @return 0 on success.
 */
int fossil_jellyfish_block_sign(fossil_jellyfish_block *block, const uint8_t *priv_key);

/**
 * @brief Verifies a Jellyfish block's signature.
 * @param block The block to verify.
 * @param pub_key The public key.
 * @return True if signature is valid.
 */
bool fossil_jellyfish_block_verify_signature(const fossil_jellyfish_block *block, const uint8_t *pub_key);

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
