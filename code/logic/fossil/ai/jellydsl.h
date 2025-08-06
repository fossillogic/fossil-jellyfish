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
#include "jellyfish.h"  // Defines fossil_jellyfish_chain, block, jellydsl, etc.

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

    fossil_jellyfish_chain chain;                       // Associated memory chain
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

// Model descriptor (.jellyfish)
int fossil_jellydsl_load_model(const char *filepath, fossil_jellyfish_jellydsl *out);
int fossil_jellydsl_save_model(const char *filepath, const fossil_jellyfish_jellydsl *model);

// Memory chain (.jfchain)
int fossil_jellydsl_load_chain(const char *filepath, fossil_jellyfish_chain *out);
int fossil_jellydsl_save_chain(const char *filepath, const fossil_jellyfish_chain *chain);

int fossil_jellydsl_load_idea(const char *filepath, fossil_jellydsl_idea *out);
int fossil_jellydsl_save_idea(const char *filepath, const fossil_jellydsl_idea *idea);

int fossil_jellydsl_load_metadata(const char *filepath, fossil_jellydsl_metadata *out);
int fossil_jellydsl_save_metadata(const char *filepath, const fossil_jellydsl_metadata *meta);

int fossil_jellydsl_load_signature(const char *filepath, fossil_jellydsl_signature *out);
int fossil_jellydsl_save_signature(const char *filepath, const fossil_jellydsl_signature *sig);

// Utility helpers
bool fossil_jellydsl_path_endswith(const char *path, const char *ext);
const char *fossil_jellydsl_guess_type(const char *filepath);

#ifdef __cplusplus
}
#endif

#endif // FOSSIL_JELLYDSL_H
