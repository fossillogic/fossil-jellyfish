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

FOSSIL_TEST_CASE(cpp_test_jellyfishai_chain_init) {
    JellyfishAI ai;
    // After init, the chain should be empty and valid
    ASSUME_ITS_EQUAL_SIZE(ai.getChain().count, 0);
}

FOSSIL_TEST_CASE(cpp_test_jellyfishai_chain_learn_and_reason) {
    JellyfishAI ai;

    ai.learn("hello", "world");
    ai.learn("foo", "bar");

    std::string out1 = ai.reason("hello");
    std::string out2 = ai.reason("foo");
    std::string out3 = ai.reason("unknown");

    ASSUME_ITS_EQUAL_CSTR(out1.c_str(), "world");
    ASSUME_ITS_EQUAL_CSTR(out2.c_str(), "bar");
    ASSUME_ITS_EQUAL_CSTR(out3.c_str(), "Unknown");
}

FOSSIL_TEST_CASE(cpp_test_jellyfishai_chain_cleanup) {
    JellyfishAI ai;

    ai.learn("a", "1");
    ai.learn("b", "2");
    ASSUME_ITS_EQUAL_SIZE(ai.getChain().count, 2);

    ai.cleanup();
    // After cleanup, the chain should be empty
    ASSUME_ITS_EQUAL_SIZE(ai.getChain().count, 0);
}

FOSSIL_TEST_CASE(cpp_test_jellyfishai_chain_dump) {
    JellyfishAI ai;

    ai.learn("x", "y");
    // This just checks that dump does not crash; output is not captured
    ai.dump();
    ASSUME_ITS_TRUE(ai.getChain().count == 1);
}

FOSSIL_TEST_CASE(cpp_test_jellyfishai_chain_hash) {
    JellyfishAI ai;
    std::vector<uint8_t> hash1, hash2;
    ai.hash("input", "output", hash1);
    ai.hash("input", "output", hash2);

    // Hashes for same input/output should match
    int same = memcmp(hash1.data(), hash2.data(), hash1.size()) == 0;
    ASSUME_ITS_TRUE(same);

    std::vector<uint8_t> hash3;
    ai.hash("input", "different", hash3);
    // Hashes for different input/output should not match
    int diff = memcmp(hash1.data(), hash3.data(), hash1.size()) != 0;
    ASSUME_ITS_TRUE(diff);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(cpp_jellyfish_tests) {    
    // Generic Fish Fixture
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfishai_chain_init);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfishai_chain_learn_and_reason);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfishai_chain_cleanup);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfishai_chain_dump);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfishai_chain_hash);

    FOSSIL_TEST_REGISTER(cpp_jellyfish_fixture);
} // end of tests
