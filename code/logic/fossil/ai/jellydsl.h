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
#ifndef FOSSIL_JELLYDSL_H
#define FOSSIL_JELLYDSL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "jellyfish.h"  // Defines fossil_jellyfish_chain_t, block, jellydsl, etc.

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Constants
// *****************************************************************************

#define FOSSIL_JELLYFISH_MAX_TAGS     16
#define FOSSIL_JELLYFISH_MAX_MODELS   16

// *****************************************************************************
// JellyDSL Master Mindset Struct
// *****************************************************************************

/**
 * @brief Represents a parsed JellyDSL structure for describing AI mindsets or knowledge.
 *
 * This structure is used to represent one complete mindset, including metadata,
 * activation logic, provenance, memory chain, trust settings, and more.
 * Designed to support the Truthful Intelligence (TI) framework.
 */
typedef struct {
    char name[64];                                      // Name of the model
    char tags[FOSSIL_JELLYFISH_MAX_TAGS][32];           // Tags for categorization
    size_t tag_count;                                   // Number of tags
    char description[256];                              // Description or notes

    fossil_jellyfish_chain_t chain;                       // Associated memory chain
    char models[FOSSIL_JELLYFISH_MAX_MODELS][32];       // List of model filenames
    int model_count;                                    // Number of models
    int priority;                                       // Processing priority
    float confidence_threshold;                         // Confidence threshold

    char activation_condition[128];                     // Optional logic to activate
    char source_uri[256];                               // Source reference for provenance
    char origin_device_id[64];                          // Device ID that originated the model
    char version[32];                                   // Version string
    char content_hash[64];                              // Integrity hash

    uint64_t created_at;                                // Timestamp of creation
    uint64_t updated_at;                                // Last update timestamp
    float trust_score;                                  // Trust score (0.0 - 1.0)
    int immutable;                                      // Nonzero = model cannot change
    char state_machine[128];                            // Optional state machine ref
} fossil_jellyfish_jellydsl;

// Metadata (.jfmeta)
typedef struct {
    char origin[128];
    char source_uri[256];
    char device[64];
    char license[128];
    char content_hash[64];
    char related_files[8][128];
    int file_count;
} fossil_jellydsl_metadata;

// Imagination file (.jfidea)
typedef struct {
    char seed[256];
    char generated[8][512];
    int count;
    char prompt_type[64];
    uint64_t last_used;
    char tags[8][32];
    int tag_count;
} fossil_jellydsl_idea;

// Signature (.jfsig)
typedef struct {
    char signed_by[64];
    char signature[256];
    char hash[64];
    char key_fingerprint[64];
    uint64_t timestamp;
} fossil_jellydsl_signature;

// *****************************************************************************
// Function Prototypes (modular loaders/savers follow)
// *****************************************************************************

/**
 * @brief Load a JellyDSL model descriptor from a file (.jellyfish).
 * @param filepath Path to the model file.
 * @param out Pointer to output structure.
 * @return 0 on success, nonzero on failure.
 */
int fossil_jellydsl_load_model(const char *filepath, fossil_jellyfish_jellydsl *out);

/**
 * @brief Save a JellyDSL model descriptor to a file (.jellyfish).
 * @param filepath Path to the model file.
 * @param model Pointer to the model structure.
 * @return 0 on success, nonzero on failure.
 */
int fossil_jellydsl_save_model(const char *filepath, const fossil_jellyfish_jellydsl *model);

/**
 * @brief Load a memory chain from a file (.jfchain).
 * @param filepath Path to the chain file.
 * @param out Pointer to output chain structure.
 * @return 0 on success, nonzero on failure.
 */
int fossil_jellydsl_load_chain(const char *filepath, fossil_jellyfish_chain_t *out);

/**
 * @brief Save a memory chain to a file (.jfchain).
 * @param filepath Path to the chain file.
 * @param chain Pointer to the chain structure.
 * @return 0 on success, nonzero on failure.
 */
int fossil_jellydsl_save_chain(const char *filepath, const fossil_jellyfish_chain_t *chain);

/**
 * @brief Load an imagination idea from a file (.jfidea).
 * @param filepath Path to the idea file.
 * @param out Pointer to output idea structure.
 * @return 0 on success, nonzero on failure.
 */
int fossil_jellydsl_load_idea(const char *filepath, fossil_jellydsl_idea *out);

/**
 * @brief Save an imagination idea to a file (.jfidea).
 * @param filepath Path to the idea file.
 * @param idea Pointer to the idea structure.
 * @return 0 on success, nonzero on failure.
 */
int fossil_jellydsl_save_idea(const char *filepath, const fossil_jellydsl_idea *idea);

/**
 * @brief Load metadata from a file (.jfmeta).
 * @param filepath Path to the metadata file.
 * @param out Pointer to output metadata structure.
 * @return 0 on success, nonzero on failure.
 */
int fossil_jellydsl_load_metadata(const char *filepath, fossil_jellydsl_metadata *out);

/**
 * @brief Save metadata to a file (.jfmeta).
 * @param filepath Path to the metadata file.
 * @param meta Pointer to the metadata structure.
 * @return 0 on success, nonzero on failure.
 */
int fossil_jellydsl_save_metadata(const char *filepath, const fossil_jellydsl_metadata *meta);

/**
 * @brief Load a signature from a file (.jfsig).
 * @param filepath Path to the signature file.
 * @param out Pointer to output signature structure.
 * @return 0 on success, nonzero on failure.
 */
int fossil_jellydsl_load_signature(const char *filepath, fossil_jellydsl_signature *out);

/**
 * @brief Save a signature to a file (.jfsig).
 * @param filepath Path to the signature file.
 * @param sig Pointer to the signature structure.
 * @return 0 on success, nonzero on failure.
 */
int fossil_jellydsl_save_signature(const char *filepath, const fossil_jellydsl_signature *sig);

/**
 * @brief Check if a path ends with a given extension.
 * @param path File path.
 * @param ext Extension to check (including dot).
 * @return true if path ends with ext, false otherwise.
 */
bool fossil_jellydsl_path_endswith(const char *path, const char *ext);

/**
 * @brief Guess the file type based on the file path.
 * @param filepath File path.
 * @return String representing the guessed type.
 */
const char *fossil_jellydsl_guess_type(const char *filepath);

#ifdef __cplusplus
}

namespace fossil {
namespace ai {

    class jellydsl {
    public:
        /**
         * @brief Loads a JellyDSL model from a file.
         * 
         * @param filepath Path to the model file.
         * @param out Pointer to output structure.
         * @return 0 on success, nonzero on failure.
         */
        static int load_model(const char *filepath, fossil_jellyfish_jellydsl *out) {
            return fossil_jellydsl_load_model(filepath, out);
        }

        /**
         * @brief Saves a JellyDSL model to a file.
         * 
         * @param filepath Path to the model file.
         * @param model Pointer to the model structure.
         * @return 0 on success, nonzero on failure.
         */
        static int save_model(const char *filepath, const fossil_jellyfish_jellydsl *model) {
            return fossil_jellydsl_save_model(filepath, model);
        }

        /**
         * @brief Loads a memory chain from a file.
         * 
         * @param filepath Path to the chain file.
         * @param out Pointer to output chain structure.
         * @return 0 on success, nonzero on failure.
         */
        static int load_chain(const char *filepath, fossil_jellyfish_chain_t *out) {
            return fossil_jellydsl_load_chain(filepath, out);
        }

        /**
         * @brief Saves a memory chain to a file.
         * 
         * @param filepath Path to the chain file.
         * @param chain Pointer to the chain structure.
         * @return 0 on success, nonzero on failure.
         */
        static int save_chain(const char *filepath, const fossil_jellyfish_chain_t *chain) {
            return fossil_jellydsl_save_chain(filepath, chain);
        }

        /**
         * @brief Loads an imagination idea from a file.
         * 
         * @param filepath Path to the idea file.
         * @param out Pointer to output idea structure.
         * @return 0 on success, nonzero on failure.
         */
        static int load_idea(const char *filepath, fossil_jellydsl_idea *out) {
            return fossil_jellydsl_load_idea(filepath, out);
        }

        /**
         * @brief Saves an imagination idea to a file.
         * 
         * @param filepath Path to the idea file.
         * @param idea Pointer to the idea structure.
         * @return 0 on success, nonzero on failure.
         */
        static int save_idea(const char *filepath, const fossil_jellydsl_idea *idea) {
            return fossil_jellydsl_save_idea(filepath, idea);
        }

        /**
         * @brief Loads metadata from a file.
         * 
         * @param filepath Path to the metadata file.
         * @param out Pointer to output metadata structure.
         * @return 0 on success, nonzero on failure.
         */
        static int load_metadata(const char *filepath, fossil_jellydsl_metadata *out) {
            return fossil_jellydsl_load_metadata(filepath, out);
        }

        /**
         * @brief Saves metadata to a file.
         * 
         * @param filepath Path to the metadata file.
         * @param meta Pointer to the metadata structure.
         * @return 0 on success, nonzero on failure.
         */
        static int save_metadata(const char *filepath, const fossil_jellydsl_metadata *meta) {
            return fossil_jellydsl_save_metadata(filepath, meta);
        }

        /**
         * @brief Loads a signature from a file.
         * 
         * @param filepath Path to the signature file.
         * @param out Pointer to output signature structure.
         * @return 0 on success, nonzero on failure.
         */
        static int load_signature(const char *filepath, fossil_jellydsl_signature *out) {
            return fossil_jellydsl_load_signature(filepath, out);
        }

        /**
         * @brief Saves a signature to a file.
         * 
         * @param filepath Path to the signature file.
         * @param sig Pointer to the signature structure.
         * @return 0 on success, nonzero on failure.
         */
        static int save_signature(const char *filepath, const fossil_jellydsl_signature *sig) {
            return fossil_jellydsl_save_signature(filepath, sig);
        }

        /**
         * @brief Checks if a path ends with a specific extension.
         * 
         * @param path File path.
         * @param ext Extension to check (including dot).
         * @return true if path ends with ext, false otherwise.
         */
        static bool path_ends_with(const char *path, const char *ext) {
            return fossil_jellydsl_path_endswith(path, ext);
        }

        /**
         * @brief Guesses the file type based on the file path.
         * 
         * @param filepath File path.
         * @return String representing the guessed type.
         */
        static const char *guess_type(const char *filepath) {
            return fossil_jellydsl_guess_type(filepath);
        }
    };

}

}

#endif

#endif // FOSSIL_JELLYDSL_H
