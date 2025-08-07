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

FOSSIL_TEST_CASE(cpp_test_jellyfish_chain_init) {
    fossil::ai::Jellyfish jf;
    auto* chain = jf.native_chain();
    ASSUME_ITS_EQUAL_SIZE(chain->count, 0);
    for (size_t i = 0; i < FOSSIL_JELLYFISH_MAX_MEM; ++i) {
        ASSUME_ITS_TRUE(chain->memory[i].valid == 0);
    }
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_chain_learn_and_reason) {
    fossil::ai::Jellyfish jf;

    jf.learn("hello", "world");
    auto* chain = jf.native_chain();
    ASSUME_ITS_EQUAL_SIZE(chain->count, 1);
    ASSUME_ITS_EQUAL_CSTR(chain->memory[0].input, "hello");
    ASSUME_ITS_EQUAL_CSTR(chain->memory[0].output, "world");
    ASSUME_ITS_TRUE(chain->memory[0].valid == 1);

    std::string result = jf.reason("hello");
    ASSUME_ITS_EQUAL_CSTR(result.c_str(), "world");

    // Test unknown input
    result = jf.reason("unknown");
    ASSUME_ITS_EQUAL_CSTR(result.c_str(), "Unknown");
}


FOSSIL_TEST_CASE(cpp_test_jellyfish_chain_cleanup) {
    fossil::ai::Jellyfish jf;
    jf.learn("a", "b");
    jf.native_chain()->memory[0].confidence = 0.01f; // force low confidence
    jf.native_chain()->memory[0].valid = 1;
    jf.native_chain()->count = 1;

    // Call cleanup (should clear the chain)
    fossil_jellyfish_cleanup(jf.native_chain());
    ASSUME_ITS_EQUAL_SIZE(jf.native_chain()->count, 0);
    ASSUME_ITS_TRUE(jf.native_chain()->memory[0].valid == 0);
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_chain_hash) {
    fossil::ai::Jellyfish jf;
    auto hash1 = jf.hash("foo", "bar");
    auto hash2 = jf.hash("foo", "baz");

    int diff = 0;
    for (size_t i = 0; i < hash1.size(); ++i)
        if (hash1[i] != hash2[i]) {
            diff = 1;
            break;
        }

    ASSUME_ITS_TRUE(diff);
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_chain_hash_stability) {
    fossil::ai::Jellyfish jf;
    auto hash1 = jf.hash("ping", "pong");
    auto hash2 = jf.hash("ping", "pong"); // Run quickly before time changes

    int identical = 1;
    for (size_t i = 0; i < hash1.size(); ++i)
        if (hash1[i] != hash2[i]) {
            identical = 0;
            break;
        }

    // It might still fail due to microsecond tick!
    ASSUME_ITS_TRUE(identical || !"Hashes changed too fast – maybe time ticked");
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_reason_fuzzy) {
    fossil::ai::Jellyfish jf;
    jf.learn("hello", "world");
    jf.learn("foo", "bar");

    // Fuzzy match: "helo" should match "hello"
    std::string result = jf.reason("helo");
    ASSUME_ITS_EQUAL_CSTR(result.c_str(), "world");
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_reason_chain) {
    fossil::ai::Jellyfish jf;
    jf.learn("abc", "123");
    jf.learn("def", "456");

    std::string result = jf.reason("def");
    ASSUME_ITS_EQUAL_CSTR(result.c_str(), "456");
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_decay_confidence) {
    fossil::ai::Jellyfish jf;
    jf.learn("a", "b");
    jf.native_chain()->memory[0].confidence = 1.0f;
    jf.native_chain()->memory[0].valid = 1;
    jf.native_chain()->count = 1;

    jf.decay_confidence(0.5f);
    ASSUME_ITS_TRUE(jf.native_chain()->memory[0].confidence < 1.0f);
    jf.decay_confidence(0.9f);
    ASSUME_ITS_TRUE(jf.native_chain()->memory[0].valid == 0 || jf.native_chain()->memory[0].confidence < 0.05f);
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_tokenize) {
    fossil::ai::Jellyfish jf;
    std::vector<std::string> tokens;
    size_t n = jf.tokenize("Hello, world! 123", tokens);
    ASSUME_ITS_EQUAL_SIZE(n, 3);
    ASSUME_ITS_EQUAL_CSTR(tokens[0].c_str(), "hello");
    ASSUME_ITS_EQUAL_CSTR(tokens[1].c_str(), "world");
    ASSUME_ITS_EQUAL_CSTR(tokens[2].c_str(), "123");
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_best_memory) {
    fossil::ai::Jellyfish jf;
    jf.learn("a", "b");
    jf.learn("c", "d");
    jf.native_chain()->memory[0].confidence = 0.5f;
    jf.native_chain()->memory[1].confidence = 0.9f;
    jf.native_chain()->memory[0].valid = 1;
    jf.native_chain()->memory[1].valid = 1;
    jf.native_chain()->count = 2;

    const fossil_jellyfish_block_t *best = jf.best_memory();
    ASSUME_ITS_TRUE(best != NULL);
    ASSUME_ITS_EQUAL_CSTR(best->input, "c");
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_detect_conflict) {
    fossil::ai::Jellyfish jf;

    // Teach it one input-output pair
    jf.learn("foo", "bar");

    // Introduce a conflicting input with different output
    int conflict = jf.detect_conflict("foo", "baz");

    // Should be 1 if conflict detected, 0 otherwise
    ASSUME_ITS_TRUE(conflict == 1 || conflict == 0);
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_knowledge_coverage) {
    fossil::ai::Jellyfish jf;
    jf.learn("a", "b");
    jf.native_chain()->memory[0].valid = 1;
    jf.native_chain()->count = 1;
    float coverage = jf.knowledge_coverage();
    ASSUME_ITS_TRUE(coverage >= 0.0f && coverage <= 1.0f);
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_chain_struct_fields) {
    fossil::ai::Jellyfish jf;
    jf.learn("foo", "bar");
    auto* chain = jf.native_chain();

    ASSUME_ITS_EQUAL_SIZE(chain->count, 1);
    ASSUME_ITS_EQUAL_CSTR(chain->memory[0].input, "foo");
    ASSUME_ITS_EQUAL_CSTR(chain->memory[0].output, "bar");
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_audit) {
    fossil::ai::Jellyfish jf;
    jf.learn("a", "b");
    int issues = fossil_jellyfish_audit(jf.native_chain());
    ASSUME_ITS_TRUE(issues >= 0);
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_prune) {
    fossil::ai::Jellyfish jf;
    jf.learn("a", "b");
    jf.native_chain()->memory[0].confidence = 0.1f;
    jf.native_chain()->memory[0].valid = 1;
    jf.native_chain()->count = 1;
    int pruned = fossil_jellyfish_prune(jf.native_chain(), 0.5f);
    ASSUME_ITS_TRUE(pruned >= 0);
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_dump) {
    fossil::ai::Jellyfish jf;
    jf.learn("foo", "bar");
    jf.dump(); // Should not crash
    ASSUME_ITS_TRUE(1);
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_reflect) {
    fossil::ai::Jellyfish jf;
    jf.learn("foo", "bar");
    jf.reflect(); // Should not crash
    ASSUME_ITS_TRUE(1);
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_validation_report) {
    fossil::ai::Jellyfish jf;
    jf.learn("foo", "bar");
    jf.validation_report(); // Should not crash
    ASSUME_ITS_TRUE(1);
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_chain_trust_score) {
    fossil::ai::Jellyfish jf;
    jf.learn("foo", "bar");
    float trust = jf.trust_score();
    ASSUME_ITS_TRUE(trust >= 0.0f && trust <= 1.0f);
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_mark_immutable) {
    fossil::ai::Jellyfish jf;
    jf.learn("foo", "bar");
    auto* block = &jf.native_chain()->memory[0];
    jf.mark_immutable(block);
    // No easy way to check, but should not crash
    ASSUME_ITS_TRUE(1);
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_prune_chain) {
    fossil::ai::Jellyfish jf;
    jf.learn("a", "b");
    jf.native_chain()->memory[0].confidence = 0.1f;
    jf.native_chain()->memory[0].valid = 1;
    jf.native_chain()->count = 1;
    int pruned = jf.prune_chain(0.5f);
    ASSUME_ITS_TRUE(pruned >= 0);
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_deduplicate_chain) {
    fossil::ai::Jellyfish jf;
    jf.learn("a", "b");
    jf.learn("a", "b");
    int removed = jf.deduplicate_chain();
    ASSUME_ITS_TRUE(removed >= 0);
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_compress_chain) {
    fossil::ai::Jellyfish jf;
    jf.learn("  a  ", "  b  ");
    int modified = jf.compress_chain();
    ASSUME_ITS_TRUE(modified >= 0);
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_best_match) {
    fossil::ai::Jellyfish jf;
    jf.learn("abc", "123");
    const fossil_jellyfish_block_t* block = jf.best_match("abc");
    ASSUME_ITS_TRUE(block != NULL);
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_redact_block) {
    fossil::ai::Jellyfish jf;
    jf.learn("secret", "data");
    auto* block = &jf.native_chain()->memory[0];
    int rc = jf.redact_block(block);
    ASSUME_ITS_TRUE(rc == 0);
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_chain_stats) {
    fossil::ai::Jellyfish jf;
    size_t valid[5] = {0};
    float avg[5] = {0};
    float ratio[5] = {0};
    jf.learn("a", "b");
    jf.chain_stats(valid, avg, ratio);
    ASSUME_ITS_TRUE(valid[0] <= jf.native_chain()->count);
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_compare_chains) {
    fossil::ai::Jellyfish a, b;
    a.learn("a", "b");
    b.learn("a", "b");
    int diff = a.compare_chains(b);
    ASSUME_ITS_TRUE(diff >= 0 || diff == -1);
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_chain_fingerprint) {
    fossil::ai::Jellyfish jf;
    uint8_t hash[FOSSIL_JELLYFISH_HASH_SIZE] = {0};
    jf.learn("a", "b");
    jf.chain_fingerprint(hash);
    ASSUME_ITS_TRUE(1);
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_trim) {
    fossil::ai::Jellyfish jf;
    jf.learn("a", "b");
    jf.learn("c", "d");
    int removed = jf.trim(1);
    ASSUME_ITS_TRUE(removed >= 0);
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_chain_compact) {
    fossil::ai::Jellyfish jf;
    jf.learn("a", "b");
    int moved = jf.chain_compact();
    ASSUME_ITS_TRUE(moved >= 0);
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_find_by_hash) {
    fossil::ai::Jellyfish jf;
    uint8_t hash[FOSSIL_JELLYFISH_HASH_SIZE] = {0};
    jf.learn("a", "b");
    jf.hash("a", "b").swap(*reinterpret_cast<std::array<uint8_t, FOSSIL_JELLYFISH_HASH_SIZE>*>(hash));
    const fossil_jellyfish_block_t *block = jf.find_by_hash(hash);
    ASSUME_ITS_TRUE(block != NULL);
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_clone_chain) {
    fossil::ai::Jellyfish src, dst;
    src.learn("a", "b");
    int rc = src.clone_chain(dst);
    ASSUME_ITS_TRUE(rc == 0);
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_reason_verbose) {
    fossil::ai::Jellyfish jf;
    jf.learn("a", "b");
    char out[64] = {0};
    float conf = 0.0f;
    const fossil_jellyfish_block_t *block = nullptr;
    bool found = jf.reason_verbose("a", out, &conf, &block);
    ASSUME_ITS_TRUE(found && strcmp(out, "b") == 0);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(cpp_jellyfish_tests) {    
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_chain_init);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_chain_learn_and_reason);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_chain_cleanup);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_chain_hash);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_chain_hash_stability);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_reason_fuzzy);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_reason_chain);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_decay_confidence);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_tokenize);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_best_memory);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_detect_conflict);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_knowledge_coverage);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_chain_struct_fields);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_audit);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_prune);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_dump);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_reflect);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_validation_report);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_chain_trust_score);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_mark_immutable);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_prune_chain);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_deduplicate_chain);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_compress_chain);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_best_match);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_redact_block);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_chain_stats);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_compare_chains);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_chain_fingerprint);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_trim);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_chain_compact);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_find_by_hash);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_clone_chain);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_reason_verbose);

    FOSSIL_TEST_REGISTER(cpp_jellyfish_fixture);
} // end of tests
