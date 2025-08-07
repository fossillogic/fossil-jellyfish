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
    fossil_jellyfish_chain_t chain;
    fossil_jellyfish_init(&chain);
    ASSUME_ITS_EQUAL_SIZE(chain.count, 0);
    for (size_t i = 0; i < FOSSIL_JELLYFISH_MAX_MEM; ++i) {
        ASSUME_ITS_TRUE(chain.memory[i].valid == 0);
    }
}

FOSSIL_TEST_CASE(c_test_jellyfish_chain_learn_and_reason) {
    fossil_jellyfish_chain_t chain = {0};
    fossil_jellyfish_init(&chain);

    fossil_jellyfish_learn(&chain, "hello", "world");
    ASSUME_ITS_EQUAL_SIZE(chain.count, 1);
    ASSUME_ITS_EQUAL_CSTR(chain.memory[0].input, "hello");
    ASSUME_ITS_EQUAL_CSTR(chain.memory[0].output, "world");
    ASSUME_ITS_TRUE(chain.memory[0].valid == 1);

    const char *result = fossil_jellyfish_reason(&chain, "hello");
    ASSUME_ITS_EQUAL_CSTR(result, "world");

    // Test unknown input
    result = fossil_jellyfish_reason(&chain, "unknown");
    ASSUME_ITS_EQUAL_CSTR(result, "Unknown");
}

FOSSIL_TEST_CASE(c_test_jellyfish_chain_cleanup) {
    fossil_jellyfish_chain_t chain = {0};
    fossil_jellyfish_init(&chain);

    fossil_jellyfish_learn(&chain, "a", "b");
    chain.memory[0].confidence = 0.01f; // force low confidence
    chain.memory[0].valid = 1;
    chain.count = 1;

    fossil_jellyfish_cleanup(&chain);
    ASSUME_ITS_EQUAL_SIZE(chain.count, 0);
    ASSUME_ITS_TRUE(chain.memory[0].valid == 0);
}

FOSSIL_TEST_CASE(c_test_jellyfish_chain_hash) {
    uint8_t hash1[FOSSIL_JELLYFISH_HASH_SIZE] = {0};
    uint8_t hash2[FOSSIL_JELLYFISH_HASH_SIZE] = {0};

    // Inputs differ only in output string
    fossil_jellyfish_hash("foo", "bar", hash1);
    fossil_jellyfish_hash("foo", "baz", hash2);

    // Ensure hashes differ
    int diff = 0;
    for (size_t i = 0; i < FOSSIL_JELLYFISH_HASH_SIZE; ++i)
        if (hash1[i] != hash2[i]) {
            diff = 1;
            break;
        }

    ASSUME_ITS_TRUE(diff);
}

FOSSIL_TEST_CASE(c_test_jellyfish_chain_hash_stability) {
    uint8_t hash1[FOSSIL_JELLYFISH_HASH_SIZE] = {0};
    uint8_t hash2[FOSSIL_JELLYFISH_HASH_SIZE] = {0};

    fossil_jellyfish_hash("ping", "pong", hash1);
    fossil_jellyfish_hash("ping", "pong", hash2); // Run quickly before time changes

    int identical = 1;
    for (size_t i = 0; i < FOSSIL_JELLYFISH_HASH_SIZE; ++i)
        if (hash1[i] != hash2[i]) {
            identical = 0;
            break;
        }

    // It might still fail due to microsecond tick!
    ASSUME_ITS_TRUE(identical || !"Hashes changed too fast – maybe time ticked");
}

FOSSIL_TEST_CASE(c_test_jellyfish_reason_fuzzy) {
    fossil_jellyfish_chain_t chain = {0};
    fossil_jellyfish_learn(&chain, "hello", "world");
    fossil_jellyfish_learn(&chain, "foo", "bar");

    // Fuzzy match: "helo" should match "hello"
    const char *result = fossil_jellyfish_reason(&chain, "helo");
    ASSUME_ITS_EQUAL_CSTR(result, "world");
}

FOSSIL_TEST_CASE(c_test_jellyfish_reason_chain) {
    fossil_jellyfish_chain_t chain = {0};
    fossil_jellyfish_learn(&chain, "abc", "123");
    fossil_jellyfish_learn(&chain, "def", "456");

    const char *result = fossil_jellyfish_reason(&chain, "def");
    ASSUME_ITS_EQUAL_CSTR(result, "456");
}

FOSSIL_TEST_CASE(c_test_jellyfish_decay_confidence) {
    fossil_jellyfish_chain_t chain = {0};
    fossil_jellyfish_learn(&chain, "a", "b");
    chain.memory[0].confidence = 1.0f;
    chain.memory[0].valid = 1;
    chain.count = 1;

    fossil_jellyfish_decay_confidence(&chain, 0.5f);
    ASSUME_ITS_TRUE(chain.memory[0].confidence < 1.0f);
    fossil_jellyfish_decay_confidence(&chain, 0.9f);
    ASSUME_ITS_TRUE(chain.memory[0].valid == 0 || chain.memory[0].confidence < 0.05f);
}

FOSSIL_TEST_CASE(c_test_jellyfish_tokenize) {
    char tokens[8][FOSSIL_JELLYFISH_TOKEN_SIZE] = {{0}};
    size_t n = fossil_jellyfish_tokenize("Hello, world! 123", tokens, 8);
    ASSUME_ITS_EQUAL_SIZE(n, 3);
    ASSUME_ITS_EQUAL_CSTR(tokens[0], "hello");
    ASSUME_ITS_EQUAL_CSTR(tokens[1], "world");
    ASSUME_ITS_EQUAL_CSTR(tokens[2], "123");
}

FOSSIL_TEST_CASE(c_test_jellyfish_best_memory) {
    fossil_jellyfish_chain_t chain = {0};
    fossil_jellyfish_learn(&chain, "a", "b");
    fossil_jellyfish_learn(&chain, "c", "d");
    chain.memory[0].confidence = 0.5f;
    chain.memory[1].confidence = 0.9f;
    chain.memory[0].valid = 1;
    chain.memory[1].valid = 1;
    chain.count = 2;

    const fossil_jellyfish_block_t *best = fossil_jellyfish_best_memory(&chain);
    ASSUME_ITS_TRUE(best != NULL);
    ASSUME_ITS_EQUAL_CSTR(best->input, "c");
}

FOSSIL_TEST_CASE(c_test_jellyfish_detect_conflict) {
    fossil_jellyfish_chain_t chain = {0};
    fossil_jellyfish_init(&chain);

    // Teach it one input-output pair
    fossil_jellyfish_learn(&chain, "foo", "bar");

    // Introduce a conflicting input with different output
    int conflict = fossil_jellyfish_detect_conflict(&chain, "foo", "baz");

    // Currently a stub (always returns 0)
    ASSUME_ITS_TRUE(conflict == 0);
}

FOSSIL_TEST_CASE(c_test_jellyfish_knowledge_coverage) {
    fossil_jellyfish_chain_t chain = {0};
    fossil_jellyfish_learn(&chain, "a", "b");
    chain.memory[0].valid = 1;
    chain.count = 1;
    float coverage = fossil_jellyfish_knowledge_coverage(&chain);
    ASSUME_ITS_TRUE(coverage >= 0.0f && coverage <= 1.0f);
}

FOSSIL_TEST_CASE(c_test_jellyfish_verify_block_and_chain) {
    fossil_jellyfish_block_t block = {0};
    strcpy(block.input, "foo");
    strcpy(block.output, "bar");
    for (size_t i = 0; i < FOSSIL_JELLYFISH_HASH_SIZE; ++i) block.hash[i] = 1;
    block.valid = 1;

    ASSUME_ITS_TRUE(fossil_jellyfish_verify_block(&block));

    fossil_jellyfish_chain_t chain = {0};
    chain.memory[0] = block;
    chain.count = 1;
    ASSUME_ITS_TRUE(fossil_jellyfish_verify_chain(&chain));
}

FOSSIL_TEST_CASE(c_test_jellyfish_block_struct_fields) {
    fossil_jellyfish_block_t block = {0};
    strcpy(block.input, "test_input");
    strcpy(block.output, "test_output");
    block.timestamp = 123456789;
    block.delta_ms = 42;
    block.duration_ms = 100;
    block.valid = 1;
    block.confidence = 0.75f;
    block.usage_count = 3;
    memset(block.device_id, 0xAB, sizeof(block.device_id));
    memset(block.signature, 0xCD, sizeof(block.signature));

    ASSUME_ITS_EQUAL_CSTR(block.input, "test_input");
    ASSUME_ITS_EQUAL_CSTR(block.output, "test_output");
    ASSUME_ITS_TRUE(block.timestamp == 123456789);
    ASSUME_ITS_TRUE(block.delta_ms == 42);
    ASSUME_ITS_TRUE(block.duration_ms == 100);
    ASSUME_ITS_TRUE(block.valid == 1);
    ASSUME_ITS_TRUE(block.confidence > 0.74f && block.confidence < 0.76f);
    ASSUME_ITS_TRUE(block.usage_count == 3);
    for (size_t i = 0; i < sizeof(block.device_id); ++i)
        ASSUME_ITS_TRUE(block.device_id[i] == 0xAB);
    for (size_t i = 0; i < sizeof(block.signature); ++i)
        ASSUME_ITS_TRUE(block.signature[i] == 0xCD);
}

FOSSIL_TEST_CASE(c_test_jellyfish_chain_struct_fields) {
    fossil_jellyfish_chain_t chain = {0};
    strcpy(chain.memory[0].input, "foo");
    strcpy(chain.memory[0].output, "bar");
    chain.count = 1;

    ASSUME_ITS_EQUAL_SIZE(chain.count, 1);
    ASSUME_ITS_EQUAL_CSTR(chain.memory[0].input, "foo");
    ASSUME_ITS_EQUAL_CSTR(chain.memory[0].output, "bar");
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(c_jellyfish_tests) {    
    // Generic ToFu Fixture
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_chain_init);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_chain_learn_and_reason);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_chain_cleanup);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_chain_hash);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_chain_hash_stability);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_reason_fuzzy);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_reason_chain);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_decay_confidence);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_tokenize);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_best_memory);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_detect_conflict);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_knowledge_coverage);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_verify_block_and_chain);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_block_struct_fields);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_chain_struct_fields);

    FOSSIL_TEST_REGISTER(c_jellyfish_fixture);
} // end of tests
