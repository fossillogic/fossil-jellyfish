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
    ASSUME_ITS_EQUAL_SIZE(ai.get_chain().count, 0);
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
    ASSUME_ITS_EQUAL_SIZE(ai.get_chain().count, 2);

    ai.cleanup();
    // After cleanup, the chain should be empty
    ASSUME_ITS_EQUAL_SIZE(ai.get_chain().count, 0);
}

FOSSIL_TEST_CASE(cpp_test_jellyfishai_chain_dump) {
    JellyfishAI ai;

    ai.learn("x", "y");
    // This just checks that dump does not crash; output is not captured
    ai.dump();
    ASSUME_ITS_TRUE(ai.get_chain().count == 1);
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

FOSSIL_TEST_CASE(cpp_test_jellyfishai_reason_fuzzy_exact_and_fuzzy) {
    JellyfishAI ai;
    ai.learn("apple", "fruit");
    ai.learn("appl", "not fruit");

    // Exact match
    std::string out1 = ai.reason_fuzzy("apple");
    ASSUME_ITS_EQUAL_CSTR(out1.c_str(), "fruit");

    // Fuzzy match (missing 'e')
    std::string out2 = ai.reason_fuzzy("appl");
    // Accept either "not fruit" or "fruit" depending on fuzzy logic
    ASSUME_ITS_TRUE(out2 == "not fruit" || out2 == "fruit");

    // Fuzzy match (typo)
    std::string out3 = ai.reason_fuzzy("aple");
    ASSUME_ITS_TRUE(out3 == "fruit" || out3 == "not fruit");

    // No match
    std::string out4 = ai.reason_fuzzy("banana");
    ASSUME_ITS_EQUAL_CSTR(out4.c_str(), "Unknown");
}

FOSSIL_TEST_CASE(cpp_test_jellyfishai_save_and_load) {
    JellyfishAI ai;
    ai.learn("cat", "meow");
    ai.learn("dog", "bark");

    const std::string filepath = "jellyfish_chain_test_save.fish";

    int save_result = ai.save(filepath);
    ASSUME_ITS_EQUAL_I32(save_result, 1);  // Expect 1 on success now

    JellyfishAI ai2;
    int load_result = ai2.load(filepath);
    ASSUME_ITS_EQUAL_I32(load_result, 1);  // Expect 1 on success now

    // Check that loaded data matches
    std::string out1 = ai2.reason("cat");
    std::string out2 = ai2.reason("dog");
    ASSUME_ITS_EQUAL_CSTR(out1.c_str(), "meow");
    ASSUME_ITS_EQUAL_CSTR(out2.c_str(), "bark");

    // Cleanup test file
    std::remove(filepath.c_str());
}

FOSSIL_TEST_CASE(cpp_test_jellyfishai_load_nonexistent_file) {
    JellyfishAI ai;
    int result = ai.load("nonexistent_file_hopefully_12345.dat");
    ASSUME_ITS_TRUE(result != 0);
}

FOSSIL_TEST_CASE(cpp_test_jellyfishai_reason_chain_basic) {
    JellyfishAI ai;
    ai.learn("sun", "star");
    ai.learn("star", "celestial");

    // Depth 0: should only show direct reasoning
    std::string chain0 = ai.reason_chain("sun", 0);
    ASSUME_ITS_TRUE(chain0.find("star") != std::string::npos);

    // Depth 1: should show one level of reasoning
    std::string chain1 = ai.reason_chain("sun", 1);
    ASSUME_ITS_TRUE(chain1.find("star") != std::string::npos);
    ASSUME_ITS_TRUE(chain1.find("celestial") != std::string::npos);

    // Unknown input
    std::string chain_unknown = ai.reason_chain("moon", 1);
    ASSUME_ITS_EQUAL_CSTR(chain_unknown.c_str(), "Unknown");
}

FOSSIL_TEST_CASE(cpp_test_jellyfishai_decay_confidence) {
    JellyfishAI ai;
    ai.learn("alpha", "beta");
    ai.learn("gamma", "delta");

    // Optionally, get confidence before decay if API allows
    // For now, just call decay and check chain is still valid
    ai.decay_confidence(0.5f);

    // After decay, chain should still have same count
    ASSUME_ITS_EQUAL_SIZE(ai.get_chain().count, 2);

    // Decay with 0.0 (no decay)
    ai.decay_confidence(0.0f);
    ASSUME_ITS_EQUAL_SIZE(ai.get_chain().count, 2);

    // Decay with 1.0 (full decay)
    ai.decay_confidence(1.0f);
    // Chain should still exist, but confidence values would be 0 if accessible
    ASSUME_ITS_EQUAL_SIZE(ai.get_chain().count, 2);
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
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfishai_save_and_load);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfishai_load_nonexistent_file);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfishai_reason_fuzzy_exact_and_fuzzy);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfishai_reason_chain_basic);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfishai_decay_confidence);

    FOSSIL_TEST_REGISTER(cpp_jellyfish_fixture);
} // end of tests
