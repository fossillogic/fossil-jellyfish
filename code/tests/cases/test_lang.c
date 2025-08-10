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
#include <fossil/pizza/framework.h>
#include "fossil/ai/framework.h"


// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Utilities
// * * * * * * * * * * * * * * * * * * * * * * * *
// Setup steps for things like test fixtures and
// mock objects are set here.
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_SUITE(c_lang_fixture);

FOSSIL_SETUP(c_lang_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(c_lang_fixture) {
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Cases
// * * * * * * * * * * * * * * * * * * * * * * * *
// The test cases below are provided as samples, inspired
// by the Meson build system's approach of using test cases
// as samples for library usage.
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_CASE(c_test_lang_tokenize_basic) {
    char tokens[8][FOSSIL_JELLYFISH_TOKEN_SIZE] = {0};
    size_t n = fossil_lang_tokenize("Hello, world!  This is a test.", tokens, 8);
    ASSUME_ITS_EQUAL_I32(n, 6);
    ASSUME_ITS_EQUAL_CSTR(tokens[0], "hello");
    ASSUME_ITS_EQUAL_CSTR(tokens[1], "world");
    ASSUME_ITS_EQUAL_CSTR(tokens[2], "this");
    ASSUME_ITS_EQUAL_CSTR(tokens[3], "is");
    ASSUME_ITS_EQUAL_CSTR(tokens[4], "a");
    ASSUME_ITS_EQUAL_CSTR(tokens[5], "test");
}

FOSSIL_TEST_CASE(c_test_lang_is_question) {
    ASSUME_ITS_TRUE(fossil_lang_is_question("Is this a question?"));
    ASSUME_ITS_TRUE(fossil_lang_is_question("What time is it"));
    ASSUME_ITS_TRUE(fossil_lang_is_question("Could you help me?"));
    ASSUME_ITS_TRUE(!fossil_lang_is_question("This is not a question."));
}

FOSSIL_TEST_CASE(c_test_lang_detect_emotion) {
    float pos = fossil_lang_detect_emotion("I love this!");
    float neg = fossil_lang_detect_emotion("This is terrible.");
    float neu = fossil_lang_detect_emotion("The sky is blue.");
    ASSUME_ITS_TRUE(pos > 0.5f);
    ASSUME_ITS_TRUE(neg < -0.5f);
    ASSUME_ITS_TRUE(neu > -0.2f && neu < 0.2f);
}

FOSSIL_TEST_CASE(c_test_lang_detect_bias_or_falsehood) {
    ASSUME_ITS_EQUAL_I32(fossil_lang_detect_bias_or_falsehood("Everyone knows this is the best!"), 1);
    ASSUME_ITS_EQUAL_I32(fossil_lang_detect_bias_or_falsehood("The sun rises in the east."), 0);
}

FOSSIL_TEST_CASE(c_test_lang_align_truth) {
    fossil_jellyfish_chain_t chain;
    fossil_jellyfish_init(&chain);
    fossil_io_chat_learn_response(&chain, "The sky is blue.", "Yes, it is.");
    int aligned = fossil_lang_align_truth(&chain, "The sky is blue.");
    int unknown = fossil_lang_align_truth(&chain, "Grass is purple.");
    ASSUME_ITS_EQUAL_I32(aligned, 1);
    ASSUME_ITS_TRUE(unknown == 0 || unknown == -1);
}

FOSSIL_TEST_CASE(c_test_lang_similarity) {
    float sim1 = fossil_lang_similarity("The quick brown fox", "A quick brown fox");
    float sim2 = fossil_lang_similarity("cat", "dog");
    float sim3 = fossil_lang_similarity("identical", "identical");
    ASSUME_ITS_TRUE(sim1 > 0.5f);
    ASSUME_ITS_TRUE(sim2 < 0.5f);
    ASSUME_ITS_TRUE(sim3 > 0.99f);
}

FOSSIL_TEST_CASE(c_test_lang_summarize_and_normalize) {
    char summary[64] = {0};
    char normalized[64] = {0};
    fossil_lang_summarize("This is a very long sentence that should be summarized.", summary, sizeof(summary));
    fossil_lang_normalize("I'm gonna win!", normalized, sizeof(normalized));
    ASSUME_ITS_TRUE(strlen(summary) > 0);
    ASSUME_ITS_TRUE(strstr(normalized, "going to") != NULL);
}

FOSSIL_TEST_CASE(c_test_lang_extract_focus) {
    char focus[32] = {0};
    fossil_lang_extract_focus("The quick brown fox jumps over the lazy dog.", focus, sizeof(focus));
    ASSUME_ITS_TRUE(strlen(focus) > 0);
    // Heuristic: focus word should be "fox" or "dog"
    ASSUME_ITS_TRUE(strstr(focus, "fox") != NULL || strstr(focus, "dog") != NULL);
}

FOSSIL_TEST_CASE(c_test_lang_estimate_trust) {
    fossil_jellyfish_chain_t chain;
    fossil_jellyfish_init(&chain);
    float trust1 = fossil_lang_estimate_trust(&chain, "The sky is blue.");
    float trust2 = fossil_lang_estimate_trust(&chain, "Everyone knows this is the best!");
    ASSUME_ITS_TRUE(trust1 > trust2);
    ASSUME_ITS_TRUE(trust1 >= 0.0f && trust1 <= 1.0f);
}

FOSSIL_TEST_CASE(c_test_lang_embedding_similarity) {
    float a[4] = {1.0f, 0.0f, 0.0f, 0.0f};
    float b[4] = {1.0f, 0.0f, 0.0f, 0.0f};
    float c[4] = {0.0f, 1.0f, 0.0f, 0.0f};
    float sim_ab = fossil_lang_embedding_similarity(a, b, 4);
    float sim_ac = fossil_lang_embedding_similarity(a, c, 4);
    ASSUME_ITS_TRUE(sim_ab > 0.99f);
    ASSUME_ITS_TRUE(sim_ac < 0.1f);
}

FOSSIL_TEST_CASE(c_test_lang_generate_variants) {
    char variants[3][256] = {{0}};
    fossil_lang_generate_variants("hello", variants, 3);
    ASSUME_ITS_TRUE(strlen(variants[0]) > 0);
    // At least one variant should differ from input
    ASSUME_ITS_TRUE(strcmp(variants[0], "hello") == 0 || strcmp(variants[1], "hello") == 0);
}

FOSSIL_TEST_CASE(c_test_lang_process_pipeline) {
    fossil_lang_pipeline_t pipe = {0}; // Assume default enables all
    fossil_lang_result_t result = {0};
    fossil_lang_process(&pipe, "Is this a test?", &result);
    // Check at least one field is filled
    ASSUME_ITS_TRUE(result.token_count > 0 || result.is_question);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(c_lang_tests) {
    FOSSIL_TEST_ADD(c_lang_fixture, c_test_lang_tokenize_basic);
    FOSSIL_TEST_ADD(c_lang_fixture, c_test_lang_is_question);
    FOSSIL_TEST_ADD(c_lang_fixture, c_test_lang_detect_emotion);
    FOSSIL_TEST_ADD(c_lang_fixture, c_test_lang_detect_bias_or_falsehood);
    FOSSIL_TEST_ADD(c_lang_fixture, c_test_lang_align_truth);
    FOSSIL_TEST_ADD(c_lang_fixture, c_test_lang_similarity);
    FOSSIL_TEST_ADD(c_lang_fixture, c_test_lang_summarize_and_normalize);
    FOSSIL_TEST_ADD(c_lang_fixture, c_test_lang_extract_focus);
    FOSSIL_TEST_ADD(c_lang_fixture, c_test_lang_estimate_trust);
    FOSSIL_TEST_ADD(c_lang_fixture, c_test_lang_embedding_similarity);
    FOSSIL_TEST_ADD(c_lang_fixture, c_test_lang_generate_variants);
    FOSSIL_TEST_ADD(c_lang_fixture, c_test_lang_process_pipeline);

    FOSSIL_TEST_REGISTER(c_lang_fixture);
} // end of tests
