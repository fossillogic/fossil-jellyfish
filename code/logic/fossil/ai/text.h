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
#ifndef FOSSIL_JELLYFISH_TEXT_H
#define FOSSIL_JELLYFISH_TEXT_H

#include "jellyfish.h"

#ifdef __cplusplus
extern "C"
{
#endif

// *****************************************************************************
// Function prototypes
// *****************************************************************************

// Trim leading/trailing whitespace in place
void fossil_text_trim(char *text);

// Collapse multiple spaces/tabs/newlines into single space
void fossil_text_collapse_whitespace(char *text);

// Remove non-printable ASCII/control characters (in-place)
void fossil_text_remove_nonprintable(char *text);

// Compute Levenshtein distance between two strings
int fossil_text_levenshtein(const char *a, const char *b);

// Return length of common prefix
size_t fossil_text_prefix_match(const char *a, const char *b);

// Return length of common suffix
size_t fossil_text_suffix_match(const char *a, const char *b);

// Simple djb2-style checksum
unsigned long fossil_text_checksum(const char *text);

// Sanitize brackets, quotes, emoji, and markup symbols
void fossil_text_sanitize(char *text);

#ifdef __cplusplus
}
#include <stdexcept>
#include <vector>
#include <string>

namespace fossil {

namespace ai {



} // namespace ai

} // namespace fossil

#endif

#endif /* fossil_fish_FRAMEWORK_H */