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
#ifndef FOSSIL_JELLYFISH_LANG_H
#define FOSSIL_JELLYFISH_LANG_H

#include "jellyfish.h"

#ifdef __cplusplus
extern "C"
{
#endif

// *****************************************************************************
// Type definitions
// *****************************************************************************

typedef struct {
    const char *start;
    size_t length;
} fossil_lang_token;

// *****************************************************************************
// Function prototypes
// *****************************************************************************

// Tokenize input into word tokens
size_t fossil_lang_tokenize(const char *input, fossil_lang_token *tokens, size_t max_tokens);

// Normalize: lowercase and remove punctuation
void fossil_lang_normalize(char *text);

// Stopword check
int fossil_lang_is_stopword(const char *word, size_t length);

// POS guess (noun/verb/etc.)
const char* fossil_lang_guess_pos(const char *word, size_t length);

// Word frequency count
size_t fossil_lang_word_frequency(const char *text, const char *word);

// Sentence splitting
size_t fossil_lang_split_sentences(const char *text, fossil_lang_token *sentences, size_t max_sentences);

// Basic stemmer (suffix trimming)
size_t fossil_lang_stem(char *word, size_t length);

// Detect questions
int fossil_lang_is_question(const char *sentence);

// Detect tone or emotion
const char* fossil_lang_detect_emotion(const char *sentence);

// Phrase matcher (lenient)
int fossil_lang_match_phrase(const char *text, const char *phrase);

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
