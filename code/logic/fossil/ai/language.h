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
// Function prototypes
// *****************************************************************************

/**
 * Tokenizes input into normalized lowercase tokens.
 * Removes punctuation and collapses whitespace.
 */
size_t fossil_lang_tokenize(const char *input, char tokens[][FOSSIL_JELLYFISH_TOKEN_SIZE], size_t max_tokens);

/**
 * Determines whether a given input is a question.
 * Looks for terminal punctuation and question phrases.
 */
bool fossil_lang_is_question(const char *input);

/**
 * Guesses the emotional tone of a sentence.
 * Returns a score (-1.0 sad → 0.0 neutral → +1.0 positive).
 */
float fossil_lang_detect_emotion(const char *input);

/**
 * Attempts to identify bias, exaggeration, or unverified claims in input.
 * Returns 1 if detected, 0 if not.
 */
int fossil_lang_detect_bias_or_falsehood(const char *input);

/**
 * Performs truth alignment by comparing input to known chain knowledge.
 * Returns:
 *   1  → consistent/truth-aligned
 *   0  → unknown
 *  -1  → contradiction detected
 */
int fossil_lang_align_truth(const fossil_jellyfish_chain_t *chain, const char *input);

/**
 * Computes semantic similarity between two input strings.
 * Returns a float between 0.0 (no match) to 1.0 (identical meaning).
 */
float fossil_lang_similarity(const char *a, const char *b);

/**
 * Generates a compressed summary of the input string.
 * Result written to `out`, which must be preallocated.
 */
void fossil_lang_summarize(const char *input, char *out, size_t out_size);

/**
 * Attempts to normalize slang, contractions, or informal expressions.
 * Output is written to `out`, which must be preallocated.
 */
void fossil_lang_normalize(const char *input, char *out, size_t out_size);

/**
 * Extracts the most meaningful phrase from input for matching.
 * Good for chaining to Jellyfish reasoning.
 */
void fossil_lang_extract_focus(const char *input, char *out, size_t out_size);

/**
 * Estimates trustworthiness of the input text based on
 * structure, word choice, exaggeration, and alignment.
 * Score is in [0.0, 1.0].
 */
float fossil_lang_estimate_trust(const fossil_jellyfish_chain_t *chain, const char *input);

/**
 * Replace slang and contractions with formal equivalents.
 * This is a fixed-rule version (extendable).
 */
void fossil_lang_normalize(const char *input, char *out, size_t out_size);

/**
 * Extracts key content from the first few meaningful tokens.
 * Simple lead-based summarization.
 */
void fossil_lang_summarize(const char *input, char *out, size_t out_size);

/**
 * Extracts a "focus word" — usually a noun or key concept — from the input.
 * Current version uses simple heuristics and common stopwords.
 */
void fossil_lang_extract_focus(const char *input, char *out, size_t out_size);

/**
 * Simple bag-of-words overlap similarity between two strings.
 * Returns a float between 0.0 (no overlap) and 1.0 (identical sets).
 */
float fossil_lang_similarity(const char *a, const char *b);

#ifdef __cplusplus
}
#include <stdexcept>
#include <vector>
#include <string>

namespace fossil {

namespace ai {

    // C++ wrapper for language processing
    class Language {
    public:
        /**
         * @brief Tokenizes an input string into lowercase word tokens.
         * 
         * @param input Null-terminated string to tokenize.
         * @return Vector of tokens extracted from the input.
         */
        static std::vector<std::string> tokenize(const std::string& input) {
            char tokens[64][FOSSIL_JELLYFISH_TOKEN_SIZE] = {};
            size_t count = fossil_lang_tokenize(input.c_str(), tokens, 64);
            std::vector<std::string> result;
            for (size_t i = 0; i < count; ++i) {
                result.emplace_back(tokens[i]);
            }
            return result;
        }

        /**
         * @brief Checks if the input is a question.
         * 
         * @param input The input string to check.
         * @return True if the input is a question, false otherwise.
         */
        static bool is_question(const std::string& input) {
            return fossil_lang_is_question(input.c_str());
        }

        /**
         * @brief Detects the emotional tone of the input.
         * 
         * @param input The input string to analyze.
         * @return A float score indicating the emotional tone (-1.0 sad → 0.0 neutral → +1.0 positive).
         */
        static float detect_emotion(const std::string& input) {
            return fossil_lang_detect_emotion(input.c_str());
        }

        /**
         * @brief Detects bias or falsehood in the input.
         * 
         * @param input The input string to analyze.
         * @return 1 if bias/falsehood detected, 0 otherwise.
         */
        static int detect_bias_or_falsehood(const std::string& input) {
            return fossil_lang_detect_bias_or_falsehood(input.c_str());
        }

        /**
         * @brief Aligns the input with known truth in the Jellyfish chain.
         * 
         * @param chain Pointer to the Jellyfish chain.
         * @param input The input string to align.
         * @return 1 if aligned, 0 if unknown, -1 if contradiction detected.
         */
        static int align_truth(const fossil_jellyfish_chain_t* chain, const std::string& input) {
            return fossil_lang_align_truth(chain, input.c_str());
        }

        /**
         * @brief Computes semantic similarity between two strings.
         * 
         * @param a First string.
         * @param b Second string.
         * @return A float score between 0.0 (no match) and 1.0 (identical meaning).
         */        
        static float similarity(const std::string& a, const std::string& b) {
            return fossil_lang_similarity(a.c_str(), b.c_str());
        }

        /**
         * @brief Summarizes the input text.
         * 
         * @param input The input text to summarize.
         * @return A summary of the input text.
         */
        static std::string summarize(const std::string& input) {
            char out[512] = {};
            fossil_lang_summarize(input.c_str(), out, sizeof(out));
            return std::string(out);
        }

        /**
         * @brief Normalizes slang and contractions in the input text.
         * 
         * @param input The input text to normalize.
         * @return A normalized version of the input text.
         */
        static std::string normalize(const std::string& input) {
            char out[512] = {};
            fossil_lang_normalize(input.c_str(), out, sizeof(out));
            return std::string(out);
        }

        /**
         * @brief Extracts the most meaningful phrase from the input.
         * 
         * @param input The input text to extract focus from.
         * @return The extracted focus phrase.
         */
        static std::string extract_focus(const std::string& input) {
            char out[256] = {};
            fossil_lang_extract_focus(input.c_str(), out, sizeof(out));
            return std::string(out);
        }

        /**
         * @brief Estimates the trustworthiness of the input text.
         * 
         * @param chain Pointer to the Jellyfish chain.
         * @param input The input text to analyze.
         * @return A trust score between 0.0 (untrustworthy) and 1.0 (trustworthy).
         */
        static float estimate_trust(const fossil_jellyfish_chain_t* chain, const std::string& input) {
            return fossil_lang_estimate_trust(chain, input.c_str());
        }
    };

} // namespace ai

} // namespace fossil

#endif

#endif /* fossil_fish_FRAMEWORK_H */
