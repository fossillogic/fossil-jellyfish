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
// Model Descriptor (.jellyfish)
// *****************************************************************************

int fossil_jellydsl_load_model(const char *filepath, fossil_jellyfish_jellydsl *out);
int fossil_jellydsl_save_model(const char *filepath, const fossil_jellyfish_jellydsl *model);

// *****************************************************************************
// Memory Chain (.jfchain)
// *****************************************************************************

int fossil_jellydsl_load_chain(const char *filepath, fossil_jellyfish_chain *out);
int fossil_jellydsl_save_chain(const char *filepath, const fossil_jellyfish_chain *chain);

// *****************************************************************************
// Imagination (.jfidea)
// *****************************************************************************

typedef struct {
    char seed[256];
    char generated[8][512];
    int count;
    char prompt_type[64];
    uint64_t last_used;
    char tags[8][32];
    int tag_count;
} fossil_jellydsl_idea;

int fossil_jellydsl_load_idea(const char *filepath, fossil_jellydsl_idea *out);
int fossil_jellydsl_save_idea(const char *filepath, const fossil_jellydsl_idea *idea);

// *****************************************************************************
// Signature (.jfsig)
// *****************************************************************************

typedef struct {
    char signed_by[64];
    char signature[256];
    char hash[64];
    char key_fingerprint[64];
    uint64_t timestamp;
} fossil_jellydsl_signature;

int fossil_jellydsl_load_signature(const char *filepath, fossil_jellydsl_signature *out);
int fossil_jellydsl_save_signature(const char *filepath, const fossil_jellydsl_signature *sig);

// *****************************************************************************
// Metadata (.jfmeta)
// *****************************************************************************

typedef struct {
    char origin[128];
    char source_uri[256];
    char device[64];
    char license[128];
    char content_hash[64];
    char related_files[8][128];
    int file_count;
} fossil_jellydsl_metadata;

int fossil_jellydsl_load_metadata(const char *filepath, fossil_jellydsl_metadata *out);
int fossil_jellydsl_save_metadata(const char *filepath, const fossil_jellydsl_metadata *meta);

// *****************************************************************************
// Utilities
// *****************************************************************************

bool fossil_jellydsl_path_endswith(const char *path, const char *ext);
const char *fossil_jellydsl_guess_type(const char *filepath);

#ifdef __cplusplus
}
#endif

#endif // FOSSIL_JELLYDSL_H
