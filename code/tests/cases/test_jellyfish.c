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

FOSSIL_TEST_SUITE(c_jellyfish_fixture);

FOSSIL_SETUP(c_jellyfish_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(c_jellyfish_fixture) {
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Cases
// * * * * * * * * * * * * * * * * * * * * * * * *
// The test cases below are provided as samples, inspired
// by the Meson build system's approach of using test cases
// as samples for library usage.
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_CASE(c_test_jellyfish_chain_init) {
    fossil_jellyfish_chain chain;
    fossil_jellyfish_init(&chain);
    // After init, the chain should be empty and valid
    ASSUME_ITS_EQUAL_SIZE(chain.count, 0);
}

FOSSIL_TEST_CASE(c_test_jellyfish_chain_learn_and_reason) {
    fossil_jellyfish_chain chain;
    fossil_jellyfish_init(&chain);

    fossil_jellyfish_learn(&chain, "hello", "world");
    fossil_jellyfish_learn(&chain, "foo", "bar");

    const char *out1 = fossil_jellyfish_reason(&chain, "hello");
    const char *out2 = fossil_jellyfish_reason(&chain, "foo");
    const char *out3 = fossil_jellyfish_reason(&chain, "unknown");

    ASSUME_ITS_EQUAL_CSTR(out1, "world");
    ASSUME_ITS_EQUAL_CSTR(out2, "bar");
    ASSUME_ITS_EQUAL_CSTR(out3, "Unknown");
}

FOSSIL_TEST_CASE(c_test_jellyfish_chain_cleanup) {
    fossil_jellyfish_chain chain;
    fossil_jellyfish_init(&chain);

    fossil_jellyfish_learn(&chain, "a", "1");
    fossil_jellyfish_learn(&chain, "b", "2");
    ASSUME_ITS_EQUAL_SIZE(chain.count, 2);

    fossil_jellyfish_cleanup(&chain);
    // After cleanup, the chain should be empty
    ASSUME_ITS_EQUAL_SIZE(chain.count, 0);
}

FOSSIL_TEST_CASE(c_test_jellyfish_chain_dump) {
    fossil_jellyfish_chain chain;
    fossil_jellyfish_init(&chain);

    fossil_jellyfish_learn(&chain, "x", "y");
    // This just checks that dump does not crash; output is not captured
    fossil_jellyfish_dump(&chain);
    ASSUME_ITS_TRUE(chain.count == 1);
}

FOSSIL_TEST_CASE(c_test_jellyfish_chain_hash) {
    uint8_t hash1[32] = {0};
    uint8_t hash2[32] = {0};
    fossil_jellyfish_hash("input", "output", hash1);
    fossil_jellyfish_hash("input", "output", hash2);

    // Hashes for same input/output should match
    int same = memcmp(hash1, hash2, 32) == 0;
    ASSUME_ITS_TRUE(same);

    uint8_t hash3[32] = {0};
    fossil_jellyfish_hash("input", "different", hash3);
    // Hashes for different input/output should not match
    int diff = memcmp(hash1, hash3, 32) != 0;
    ASSUME_ITS_TRUE(diff);
}

FOSSIL_TEST_CASE(c_test_jellyfish_chain_save_and_load) {
    fossil_jellyfish_chain chain1, chain2;
    fossil_jellyfish_init(&chain1);
    fossil_jellyfish_init(&chain2);

    fossil_jellyfish_learn(&chain1, "alpha", "beta");
    fossil_jellyfish_learn(&chain1, "gamma", "delta");

    const char *filepath = "test_jellyfish_chain_save.dat";
    int save_result = fossil_jellyfish_save(&chain1, filepath);
    ASSUME_ITS_TRUE(save_result == 1);

    int load_result = fossil_jellyfish_load(&chain2, filepath);
    ASSUME_ITS_TRUE(load_result == 1);

    ASSUME_ITS_EQUAL_SIZE(chain2.count, 2);
    ASSUME_ITS_EQUAL_CSTR(chain2.memory[0].input, "alpha");
    ASSUME_ITS_EQUAL_CSTR(chain2.memory[0].output, "beta");
    ASSUME_ITS_EQUAL_CSTR(chain2.memory[1].input, "gamma");
    ASSUME_ITS_EQUAL_CSTR(chain2.memory[1].output, "delta");

    // Clean up test file
    remove(filepath);
}

FOSSIL_TEST_CASE(c_test_jellyfish_chain_save_fail) {
    fossil_jellyfish_chain chain;
    fossil_jellyfish_init(&chain);
    // Try to save to an invalid path
    int save_result = fossil_jellyfish_save(&chain, "/invalid/path/should_fail.dat");
    ASSUME_ITS_TRUE(save_result == 0);
}

FOSSIL_TEST_CASE(c_test_jellyfish_chain_load_fail) {
    fossil_jellyfish_chain chain;
    fossil_jellyfish_init(&chain);
    // Try to load from a non-existent file
    int load_result = fossil_jellyfish_load(&chain, "nonexistent_file.dat");
    ASSUME_ITS_TRUE(load_result == 0);
}

FOSSIL_TEST_CASE(c_test_jellyfish_reason_fuzzy) {
    fossil_jellyfish_chain chain;
    fossil_jellyfish_init(&chain);

    fossil_jellyfish_learn(&chain, "cat", "meow");
    fossil_jellyfish_learn(&chain, "dog", "bark");
    fossil_jellyfish_learn(&chain, "bird", "tweet");

    // Exact match
    const char *out1 = fossil_jellyfish_reason_fuzzy(&chain, "cat");
    ASSUME_ITS_EQUAL_CSTR(out1, "meow");

    // Fuzzy match (one char off)
    const char *out2 = fossil_jellyfish_reason_fuzzy(&chain, "cot");
    ASSUME_ITS_EQUAL_CSTR(out2, "meow");

    // Fuzzy match (closest)
    const char *out3 = fossil_jellyfish_reason_fuzzy(&chain, "bog");
    ASSUME_ITS_EQUAL_CSTR(out3, "bark");

    // No close match
    const char *out4 = fossil_jellyfish_reason_fuzzy(&chain, "elephant");
    ASSUME_ITS_EQUAL_CSTR(out4, "Unknown");
}

FOSSIL_TEST_CASE(c_test_jellyfish_reason_chain) {
    fossil_jellyfish_chain chain;
    fossil_jellyfish_init(&chain);

    fossil_jellyfish_learn(&chain, "a", "b");
    fossil_jellyfish_learn(&chain, "b", "c");
    fossil_jellyfish_learn(&chain, "c", "d");

    // Depth 0 returns input
    const char *out0 = fossil_jellyfish_reason_chain(&chain, "a", 0);
    ASSUME_ITS_EQUAL_CSTR(out0, "a");

    // Depth 1 returns first reasoning
    const char *out1 = fossil_jellyfish_reason_chain(&chain, "a", 1);
    ASSUME_ITS_EQUAL_CSTR(out1, "b");

    // Depth 2 returns second reasoning
    const char *out2 = fossil_jellyfish_reason_chain(&chain, "a", 2);
    ASSUME_ITS_EQUAL_CSTR(out2, "c");

    // Depth 3 returns third reasoning
    const char *out3 = fossil_jellyfish_reason_chain(&chain, "a", 3);
    ASSUME_ITS_EQUAL_CSTR(out3, "d");

    // Depth greater than chain returns last found
    const char *out4 = fossil_jellyfish_reason_chain(&chain, "a", 10);
    ASSUME_ITS_EQUAL_CSTR(out4, "d");

    // Unknown input returns "Unknown"
    const char *out5 = fossil_jellyfish_reason_chain(&chain, "z", 2);
    ASSUME_ITS_EQUAL_CSTR(out5, "Unknown");
}

FOSSIL_TEST_CASE(c_test_jellyfish_decay_confidence) {
    fossil_jellyfish_chain chain;
    fossil_jellyfish_init(&chain);

    fossil_jellyfish_learn(&chain, "x", "y");
    fossil_jellyfish_learn(&chain, "foo", "bar");

    // Set confidence to a known value
    chain.memory[0].confidence = 0.5f;
    chain.memory[1].confidence = 0.1f;

    fossil_jellyfish_decay_confidence(&chain, 0.2f);

    // First block should have confidence 0.3
    ASSUME_ITS_TRUE(chain.memory[0].confidence > 0.29f && chain.memory[0].confidence < 0.31f);
    // Second block should be marked invalid (confidence < 0.05)
    ASSUME_ITS_TRUE(chain.memory[1].valid == 0);

    // After cleanup, only valid blocks remain
    fossil_jellyfish_cleanup(&chain);
    ASSUME_ITS_EQUAL_SIZE(chain.count, 1);
    ASSUME_ITS_EQUAL_CSTR(chain.memory[0].input, "x");
}

FOSSIL_TEST_CASE(c_test_jellyfish_tokenize_basic) {
    char tokens[8][FOSSIL_JELLYFISH_TOKEN_SIZE] = {0};
    const char *text = "The quick brown fox jumps";
    size_t count = fossil_jellyfish_tokenize(text, tokens, 8);

    ASSUME_ITS_EQUAL_SIZE(count, 5);
    ASSUME_ITS_EQUAL_CSTR(tokens[0], "the");
    ASSUME_ITS_EQUAL_CSTR(tokens[1], "quick");
    ASSUME_ITS_EQUAL_CSTR(tokens[2], "brown");
    ASSUME_ITS_EQUAL_CSTR(tokens[3], "fox");
    ASSUME_ITS_EQUAL_CSTR(tokens[4], "jumps");
}

FOSSIL_TEST_CASE(c_test_jellyfish_best_memory) {
    fossil_jellyfish_chain chain;
    fossil_jellyfish_init(&chain);

    fossil_jellyfish_learn(&chain, "sky", "blue");
    fossil_jellyfish_learn(&chain, "grass", "green");

    chain.memory[0].confidence = 0.4f;
    chain.memory[1].confidence = 0.9f;

    const fossil_jellyfish_block *best = fossil_jellyfish_best_memory(&chain);
    ASSUME_ITS_EQUAL_CSTR(best->input, "grass");
    ASSUME_ITS_EQUAL_CSTR(best->output, "green");
}

FOSSIL_TEST_CASE(c_test_jellyfish_detect_conflict) {
    fossil_jellyfish_chain chain;
    fossil_jellyfish_init(&chain);
    fossil_jellyfish_learn(&chain, "apple", "fruit");

    // Not a conflict
    int result1 = fossil_jellyfish_detect_conflict(&chain, "orange", "fruit");
    ASSUME_ITS_TRUE(result1 == 0);

    // Conflict with different output
    int result2 = fossil_jellyfish_detect_conflict(&chain, "apple", "company");
    ASSUME_ITS_TRUE(result2 == 1);
}

FOSSIL_TEST_CASE(c_test_jellyfish_knowledge_coverage) {
    fossil_jellyfish_chain chain;
    fossil_jellyfish_init(&chain);
    fossil_jellyfish_learn(&chain, "sun", "hot");
    fossil_jellyfish_learn(&chain, "moon", "cold");

    float coverage = fossil_jellyfish_knowledge_coverage(&chain);
    ASSUME_ITS_TRUE(coverage > 0.9f && coverage <= 1.0f);

    // Invalidate one
    chain.memory[1].valid = 0;
    coverage = fossil_jellyfish_knowledge_coverage(&chain);
    ASSUME_ITS_TRUE(coverage > 0.4f && coverage < 0.6f);
}

FOSSIL_TEST_CASE(c_test_jellyfish_reflect) {
    fossil_jellyfish_chain chain;
    fossil_jellyfish_init(&chain);
    fossil_jellyfish_learn(&chain, "code", "logic");
    chain.memory[0].confidence = 0.75f;

    // Should print reflection output
    fossil_jellyfish_reflect(&chain);
    ASSUME_ITS_TRUE(chain.memory[0].valid == 1);
}

FOSSIL_TEST_CASE(c_test_jellyfish_mind_reason_basic) {
    fossil_jellyfish_mind mind;
    fossil_jellyfish_mind_init(&mind);

    fossil_jellyfish_chain *core = &mind.core;
    fossil_jellyfish_learn(core, "ice", "cold");

    const char *out = fossil_jellyfish_mind_reason(&mind, "ice");
    ASSUME_ITS_EQUAL_CSTR(out, "cold");
}

FOSSIL_TEST_CASE(c_test_jellyfish_mind_load_model) {
    fossil_jellyfish_mind mind;
    fossil_jellyfish_mind_init(&mind);

    fossil_jellyfish_chain source;
    fossil_jellyfish_init(&source);
    fossil_jellyfish_learn(&source, "model", "test");

    const char *filename = "test_model.fish";
    fossil_jellyfish_save(&source, filename);

    int load_result = fossil_jellyfish_mind_load_model(&mind, filename, "test-model");
    ASSUME_ITS_TRUE(load_result == 1);

    const char *result = fossil_jellyfish_mind_reason(&mind, "model");
    ASSUME_ITS_EQUAL_CSTR(result, "test");

    remove(filename);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(c_jellyfish_tests) {    
    // Generic ToFu Fixture
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_chain_init);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_chain_learn_and_reason);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_chain_cleanup);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_chain_dump);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_chain_hash);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_chain_save_and_load);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_chain_save_fail);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_chain_load_fail);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_reason_fuzzy);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_reason_chain);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_decay_confidence);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_tokenize_basic);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_best_memory);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_detect_conflict);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_knowledge_coverage);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_reflect);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_mind_reason_basic);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_mind_load_model);

    FOSSIL_TEST_REGISTER(c_jellyfish_fixture);
} // end of tests
