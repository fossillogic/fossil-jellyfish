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

    const char *filepath = "test_jellyfish_chain.fish";  // Updated to .fish extension

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

FOSSIL_TEST_CASE(c_test_jellyfish_mind_load_model) {
    fossil_jellyfish_mind mind;
    memset(&mind, 0, sizeof(fossil_jellyfish_mind));
    fossil_jellyfish_chain chain;
    fossil_jellyfish_init(&chain);
    fossil_jellyfish_learn(&chain, "cpu", "central processing unit");

    // Save the chain to simulate a model file
    const char *path = "cpu_model.fish";
    fossil_jellyfish_save(&chain, path);

    // Load it into the mind
    int ok = fossil_jellyfish_mind_load_model(&mind, path, "hardware");
    ASSUME_ITS_TRUE(ok == 1);
    ASSUME_ITS_EQUAL_SIZE(mind.model_count, 1);
    ASSUME_ITS_EQUAL_CSTR(mind.model_names[0], "hardware");

    // Validate content loaded into model slot
    const fossil_jellyfish_chain *loaded = &mind.models[0];
    ASSUME_ITS_EQUAL_SIZE(loaded->count, 1);
    ASSUME_ITS_EQUAL_CSTR(loaded->memory[0].input, "cpu");

    remove(path);
}

FOSSIL_TEST_CASE(c_test_jellyfish_mind_reason) {
    fossil_jellyfish_mind mind;
    memset(&mind, 0, sizeof(fossil_jellyfish_mind));

    fossil_jellyfish_chain logic;
    fossil_jellyfish_init(&logic);
    fossil_jellyfish_learn(&logic, "sun", "a star");

    // Simulate preloaded model
    mind.models[0] = logic;
    strncpy(mind.model_names[0], "astronomy", 64);
    mind.model_count = 1;

    const char *result = fossil_jellyfish_mind_reason(&mind, "sun");
    ASSUME_ITS_EQUAL_CSTR(result, "a star");
}

FOSSIL_TEST_CASE(c_test_jellyfish_tokenize) {
    char tokens[8][FOSSIL_JELLYFISH_TOKEN_SIZE];
    const char *text = "What is a GPU?";

    size_t count = fossil_jellyfish_tokenize(text, tokens, 8);
    ASSUME_ITS_EQUAL_SIZE(count, 4);
    ASSUME_ITS_EQUAL_CSTR(tokens[0], "what");
    ASSUME_ITS_EQUAL_CSTR(tokens[1], "is");
    ASSUME_ITS_EQUAL_CSTR(tokens[2], "a");
    ASSUME_ITS_EQUAL_CSTR(tokens[3], "gpu");
}

FOSSIL_TEST_CASE(c_test_jellyfish_best_memory) {
    fossil_jellyfish_chain chain;
    fossil_jellyfish_init(&chain);

    fossil_jellyfish_learn(&chain, "1", "one");
    fossil_jellyfish_learn(&chain, "2", "two");

    chain.memory[0].confidence = 0.3f;
    chain.memory[1].confidence = 0.9f;

    const fossil_jellyfish_block *best = fossil_jellyfish_best_memory(&chain);
    ASSUME_ITS_EQUAL_CSTR(best->input, "2");
}

FOSSIL_TEST_CASE(c_test_jellyfish_detect_conflict) {
    fossil_jellyfish_chain chain;
    fossil_jellyfish_init(&chain);
    fossil_jellyfish_learn(&chain, "earth", "round");

    int conflict = fossil_jellyfish_detect_conflict(&chain, "earth", "flat");
    ASSUME_ITS_TRUE(conflict == 1);

    int no_conflict = fossil_jellyfish_detect_conflict(&chain, "earth", "round");
    ASSUME_ITS_TRUE(no_conflict == 0);
}

FOSSIL_TEST_CASE(c_test_parse_jellyfish_file_valid) {
    const char *test_file = "valid.jellyfish";

    FILE *f = fopen(test_file, "w");
    fprintf(f,
        "{\n"
        "  \"signature\": \"JFS1\",\n"
        "  \"blocks\": [\n"
        "    {\n"
        "      \"input\": \"apple.fish\",\n"
        "      \"output\": \"fruit\",\n"
        "      \"timestamp\": 12345678\n"
        "    }\n"
        "  ]\n"
        "}\n"
    );
    fclose(f);

    fossil_jellyfish_mindset out[4] = {0};
    int count = fossil_jellyfish_parse_jellyfish_file(test_file, out, 4);

    ASSUME_ITS_EQUAL_SIZE(count, 1);
    ASSUME_ITS_EQUAL_CSTR(out[0].model_files[0], "apple.fish");
    ASSUME_ITS_EQUAL_SIZE(out[0].model_count, 1);

    remove(test_file);
}

FOSSIL_TEST_CASE(c_test_load_mindset_file_valid) {
    const char *model_file = "test_model.fish";

    // Save a test model to a file
    fossil_jellyfish_chain chain;
    fossil_jellyfish_init(&chain);
    fossil_jellyfish_learn(&chain, "sun", "star");
    fossil_jellyfish_save(&chain, model_file);

    // Create mindset file referencing model
    const char *jelly_file = "test_valid.jellyfish";
    FILE *f = fopen(jelly_file, "w");
    fprintf(f,
        "{\n"
        "  \"signature\": \"JFS1\",\n"
        "  \"blocks\": [\n"
        "    {\n"
        "      \"input\": \"%s\",\n"
        "      \"output\": \"space\",\n"
        "      \"timestamp\": 12345678\n"
        "    }\n"
        "  ]\n"
        "}\n", model_file);
    fclose(f);

    // Load mind from mindset file
    fossil_jellyfish_mind mind;
    memset(&mind, 0, sizeof(mind));
    int ok = fossil_jellyfish_load_mindset_file(jelly_file, &mind);

    ASSUME_ITS_TRUE(ok == 1);
    ASSUME_ITS_EQUAL_SIZE(mind.model_count, 1);
    ASSUME_ITS_EQUAL_CSTR(mind.models[0].memory[0].input, "sun");
    ASSUME_ITS_EQUAL_CSTR(mind.models[0].memory[0].output, "star");

    remove(jelly_file);
    remove(model_file);
}

FOSSIL_TEST_CASE(c_test_load_mindset_file_missing_model) {
    const char *jelly_file = "bad_model_ref.jellyfish";

    FILE *f = fopen(jelly_file, "w");
    fprintf(f,
        "{\n"
        "  \"signature\": \"JFS1\",\n"
        "  \"blocks\": [\n"
        "    {\n"
        "      \"input\": \"missing.fish\",\n"
        "      \"output\": \"logic\",\n"
        "      \"timestamp\": 0\n"
        "    }\n"
        "  ]\n"
        "}\n");
    fclose(f);

    fossil_jellyfish_mind mind = {0};
    int ok = fossil_jellyfish_load_mindset_file(jelly_file, &mind);

    ASSUME_ITS_TRUE(ok == 0);  // Should fail to load missing model

    remove(jelly_file);
}

FOSSIL_TEST_CASE(c_test_load_mindset_multiple_models) {
    const char *model_a = "model_a.fish";
    const char *model_b = "model_b.fish";

    fossil_jellyfish_chain a = {0}, b = {0};
    fossil_jellyfish_init(&a);
    fossil_jellyfish_init(&b);

    fossil_jellyfish_learn(&a, "red", "color");
    fossil_jellyfish_learn(&b, "triangle", "shape");

    fossil_jellyfish_save(&a, model_a);
    fossil_jellyfish_save(&b, model_b);

    const char *jelly_file = "complex_mindset.jellyfish";
    FILE *f = fopen(jelly_file, "w");
    fprintf(f,
        "{\n"
        "  \"signature\": \"JFS1\",\n"
        "  \"blocks\": [\n"
        "    { \"input\": \"%s\", \"output\": \"visual\", \"timestamp\": 1 },\n"
        "    { \"input\": \"%s\", \"output\": \"geometry\", \"timestamp\": 2 }\n"
        "  ]\n"
        "}\n", model_a, model_b);
    fclose(f);

    fossil_jellyfish_mind mind = {0};
    int ok = fossil_jellyfish_load_mindset_file(jelly_file, &mind);

    ASSUME_ITS_TRUE(ok == 1);
    ASSUME_ITS_EQUAL_SIZE(mind.model_count, 2);

    ASSUME_ITS_EQUAL_CSTR(mind.models[0].memory[0].input, "red");
    ASSUME_ITS_EQUAL_CSTR(mind.models[0].memory[0].output, "color");

    ASSUME_ITS_EQUAL_CSTR(mind.models[1].memory[0].input, "triangle");
    ASSUME_ITS_EQUAL_CSTR(mind.models[1].memory[0].output, "shape");

    remove(model_a);
    remove(model_b);
    remove(jelly_file);
}

FOSSIL_TEST_CASE(c_test_load_model_with_multiple_thoughts) {
    const char *model_file = "multi_thoughts.fish";

    fossil_jellyfish_chain chain = {0};
    fossil_jellyfish_init(&chain);

    fossil_jellyfish_learn(&chain, "day", "light");
    fossil_jellyfish_learn(&chain, "night", "dark");
    fossil_jellyfish_learn(&chain, "moon", "reflect");

    fossil_jellyfish_save(&chain, model_file);

    const char *jelly_file = "mind_multi_thoughts.jellyfish";
    FILE *f = fopen(jelly_file, "w");
    fprintf(f,
        "{\n"
        "  \"signature\": \"JFS1\",\n"
        "  \"blocks\": [\n"
        "    { \"input\": \"%s\", \"output\": \"cycle\", \"timestamp\": 123 }\n"
        "  ]\n"
        "}\n", model_file);
    fclose(f);

    fossil_jellyfish_mind mind = {0};
    int ok = fossil_jellyfish_load_mindset_file(jelly_file, &mind);

    ASSUME_ITS_TRUE(ok == 1);
    ASSUME_ITS_EQUAL_SIZE(mind.model_count, 1);

    // Multiple thoughts
    ASSUME_ITS_EQUAL_CSTR(mind.models[0].memory[0].input, "day");
    ASSUME_ITS_EQUAL_CSTR(mind.models[0].memory[1].input, "night");
    ASSUME_ITS_EQUAL_CSTR(mind.models[0].memory[2].input, "moon");

    remove(model_file);
    remove(jelly_file);
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
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_mind_load_model);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_mind_reason);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_tokenize);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_best_memory);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_detect_conflict);

    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_parse_jellyfish_file_valid);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_load_mindset_file_valid);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_load_mindset_file_missing_model);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_load_mindset_multiple_models);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_load_model_with_multiple_thoughts);

    
    FOSSIL_TEST_REGISTER(c_jellyfish_fixture);
} // end of tests
