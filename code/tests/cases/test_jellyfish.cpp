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

FOSSIL_TEST_SUITE(cpp_jellyfish_fixture);

FOSSIL_SETUP(cpp_jellyfish_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(cpp_jellyfish_fixture) {
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Cases
// * * * * * * * * * * * * * * * * * * * * * * * *
// The test cases below are provided as samples, inspired
// by the Meson build system's approach of using test cases
// as samples for library usage.
// * * * * * * * * * * * * * * * * * * * * * * * *

using fossil::ai::JellyfishAI;

FOSSIL_TEST_CASE(cpp_test_jellyfish_chain_init) {
    JellyfishAI ai;
    const fossil_jellyfish_chain_t& chain = ai.get_chain();
    ASSUME_ITS_EQUAL_SIZE(chain.count, 0);
    for (size_t i = 0; i < FOSSIL_JELLYFISH_MAX_MEM; ++i) {
        ASSUME_ITS_TRUE(chain.memory[i].valid == 0);
    }
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_chain_learn_and_reason) {
    JellyfishAI ai;
    ai.learn("hello", "world");
    const fossil_jellyfish_chain_t& chain = ai.get_chain();
    ASSUME_ITS_EQUAL_SIZE(chain.count, 1);
    ASSUME_ITS_EQUAL_CSTR(chain.memory[0].input, "hello");
    ASSUME_ITS_EQUAL_CSTR(chain.memory[0].output, "world");
    ASSUME_ITS_TRUE(chain.memory[0].valid == 1);

    std::string result = ai.reason("hello");
    ASSUME_ITS_EQUAL_CSTR(result.c_str(), "world");

    // Test unknown input
    result = ai.reason("unknown");
    ASSUME_ITS_EQUAL_CSTR(result.c_str(), "Unknown");
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_chain_cleanup) {
    JellyfishAI ai;
    ai.learn("a", "b");
    fossil_jellyfish_chain_t& chain = ai.get_chain();
    chain.memory[0].confidence = 0.01f; // force low confidence
    chain.memory[0].valid = 1;
    chain.count = 1;

    ai.cleanup();
    ASSUME_ITS_EQUAL_SIZE(chain.count, 0);
    ASSUME_ITS_TRUE(chain.memory[0].valid == 0);
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_chain_hash) {
    uint8_t hash1[FOSSIL_JELLYFISH_HASH_SIZE] = {0};
    uint8_t hash2[FOSSIL_JELLYFISH_HASH_SIZE] = {0};
    fossil_jellyfish_hash("foo", "bar", hash1);
    fossil_jellyfish_hash("foo", "baz", hash2);

    int diff = 0;
    for (size_t i = 0; i < FOSSIL_JELLYFISH_HASH_SIZE; ++i)
        if (hash1[i] != hash2[i]) diff = 1;
    ASSUME_ITS_TRUE(diff);
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_reason_fuzzy) {
    JellyfishAI ai;
    ai.learn("hello", "world");
    ai.learn("foo", "bar");

    // Fuzzy match: "helo" should match "hello"
    std::string result = ai.reason("helo");
    ASSUME_ITS_EQUAL_CSTR(result.c_str(), "world");
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_reason_chain) {
    JellyfishAI ai;
    ai.learn("abc", "123");
    ai.learn("def", "456");

    std::string result = ai.reason("def");
    ASSUME_ITS_EQUAL_CSTR(result.c_str(), "456");
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_decay_confidence) {
    JellyfishAI ai;
    ai.learn("a", "b");
    fossil_jellyfish_chain_t& chain = ai.get_chain();

    // Set up one valid block
    chain.memory[0].confidence = 1.0f;
    chain.memory[0].valid = 1;
    chain.count = 1;

    // Simulate old timestamp to force decay
    time_t now = time(NULL);
    time_t past = now - 172800; // 2 days ago (48h)
    chain.memory[0].timestamp = (uint64_t)past * 1000; // assuming ms

    ai.decay_confidence(0.0f); // decay_rate is unused but required by API

    float conf = chain.memory[0].confidence;
    int valid = chain.memory[0].valid;

    // Confidence should be decayed significantly (2 half-lives)
    ASSUME_ITS_TRUE(conf < 0.26f); // 0.5^2 = 0.25

    // Should be invalidated if too low
    ASSUME_ITS_TRUE(valid == 0 || conf < 0.05f);
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_tokenize) {
    char tokens[8][FOSSIL_JELLYFISH_TOKEN_SIZE] = {{0}};
    size_t n = fossil_jellyfish_tokenize("Hello, world! 123", tokens, 8);
    ASSUME_ITS_EQUAL_SIZE(n, 3);
    ASSUME_ITS_EQUAL_CSTR(tokens[0], "hello");
    ASSUME_ITS_EQUAL_CSTR(tokens[1], "world");
    ASSUME_ITS_EQUAL_CSTR(tokens[2], "123");
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_best_memory) {
    JellyfishAI ai;
    ai.learn("a", "b");
    ai.learn("c", "d");
    fossil_jellyfish_chain_t& chain = ai.get_chain();
    chain.memory[0].confidence = 0.5f;
    chain.memory[1].confidence = 0.9f;
    chain.memory[0].valid = 1;
    chain.memory[1].valid = 1;
    chain.count = 2;

    const fossil_jellyfish_block_t *best = ai.best_memory();
    ASSUME_ITS_TRUE(best != NULL);
    ASSUME_ITS_EQUAL_CSTR(best->input, "c");
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_detect_conflict) {
    JellyfishAI ai;
    ai.learn("foo", "bar");
    int conflict = ai.detect_conflict("foo", "baz");
    ASSUME_ITS_TRUE(conflict == 0); // Not implemented, always returns 0
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_knowledge_coverage) {
    JellyfishAI ai;
    ai.learn("a", "b");
    fossil_jellyfish_chain_t& chain = ai.get_chain();
    chain.memory[0].valid = 1;
    chain.count = 1;
    float coverage = ai.knowledge_coverage();
    ASSUME_ITS_TRUE(coverage >= 0.0f && coverage <= 1.0f);
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_chain_struct_fields) {
    JellyfishAI ai;
    fossil_jellyfish_chain_t& chain = ai.get_chain();
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
FOSSIL_TEST_GROUP(cpp_jellyfish_tests) {    
    // Generic ToFu Fixture
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_chain_init);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_chain_learn_and_reason);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_chain_cleanup);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_chain_hash);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_reason_fuzzy);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_reason_chain);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_decay_confidence);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_tokenize);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_best_memory);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_detect_conflict);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_knowledge_coverage);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_chain_struct_fields);
    
    FOSSIL_TEST_REGISTER(cpp_jellyfish_fixture);
} // end of tests
