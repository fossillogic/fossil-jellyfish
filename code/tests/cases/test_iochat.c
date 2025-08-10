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

FOSSIL_TEST_SUITE(c_iochat_fixture);

FOSSIL_SETUP(c_iochat_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(c_iochat_fixture) {
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Cases
// * * * * * * * * * * * * * * * * * * * * * * * *
// The test cases below are provided as samples, inspired
// by the Meson build system's approach of using test cases
// as samples for library usage.
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_CASE(c_test_iochat_start_and_end_session) {
    fossil_jellyfish_chain_t chain;
    fossil_jellyfish_init(&chain);

    int start_result = fossil_io_chat_start("test_session", &chain);
    ASSUME_ITS_EQUAL_I32(start_result, 0);

    int end_result = fossil_io_chat_end(&chain);
    ASSUME_ITS_EQUAL_I32(end_result, 0);
}

FOSSIL_TEST_CASE(c_test_iochat_respond_known_and_unknown) {
    fossil_jellyfish_chain_t chain;
    fossil_jellyfish_init(&chain);

    fossil_io_chat_start("chat", &chain);
    fossil_io_chat_learn_response(&chain, "hi", "hello there!");

    char output[64] = {0};
    int found = fossil_io_chat_respond(&chain, "hi", output, sizeof(output));
    ASSUME_ITS_EQUAL_I32(found, 0);
    ASSUME_ITS_EQUAL_CSTR(output, "hello there!");

    int not_found = fossil_io_chat_respond(&chain, "unknown", output, sizeof(output));
    ASSUME_ITS_EQUAL_I32(not_found, -1);
    ASSUME_ITS_TRUE(strstr(output, "not sure") != NULL);

    fossil_io_chat_end(&chain);
}

FOSSIL_TEST_CASE(c_test_iochat_inject_system_message_and_immutable) {
    fossil_jellyfish_chain_t chain;
    fossil_jellyfish_init(&chain);

    int result = fossil_io_chat_inject_system_message(&chain, "System Ready");
    ASSUME_ITS_EQUAL_I32(result, 0);

    // Last block should be immutable and input "[system]"
    size_t idx = chain.count - 1;
    ASSUME_ITS_TRUE(chain.memory[idx].attributes.immutable);
    ASSUME_ITS_EQUAL_CSTR(chain.memory[idx].io.input, "[system]");
    ASSUME_ITS_EQUAL_CSTR(chain.memory[idx].io.output, "System Ready");
}

FOSSIL_TEST_CASE(c_test_iochat_learn_response_and_turn_count) {
    fossil_jellyfish_chain_t chain;
    fossil_jellyfish_init(&chain);

    fossil_io_chat_learn_response(&chain, "foo", "bar");
    fossil_io_chat_learn_response(&chain, "baz", "qux");

    int turns = fossil_io_chat_turn_count(&chain);
    ASSUME_ITS_EQUAL_I32(turns, 2);
}

FOSSIL_TEST_CASE(c_test_iochat_summarize_session_basic) {
    fossil_jellyfish_chain_t chain;
    fossil_jellyfish_init(&chain);

    fossil_io_chat_learn_response(&chain, "hello", "world");
    fossil_io_chat_learn_response(&chain, "how are you", "fine");

    char summary[256] = {0};
    int result = fossil_io_chat_summarize_session(&chain, summary, sizeof(summary));
    ASSUME_ITS_EQUAL_I32(result, 0);
    ASSUME_ITS_TRUE(strstr(summary, "hello") != NULL);
    ASSUME_ITS_TRUE(strstr(summary, "world") != NULL);
    ASSUME_ITS_TRUE(strstr(summary, "how are you") != NULL);
}

FOSSIL_TEST_CASE(c_test_iochat_filter_recent_turns) {
    fossil_jellyfish_chain_t chain, filtered;
    fossil_jellyfish_init(&chain);
    fossil_jellyfish_init(&filtered);

    fossil_io_chat_learn_response(&chain, "a", "1");
    fossil_io_chat_learn_response(&chain, "b", "2");
    fossil_io_chat_learn_response(&chain, "c", "3");

    int result = fossil_io_chat_filter_recent(&chain, &filtered, 2);
    ASSUME_ITS_EQUAL_I32(result, 0);
    ASSUME_ITS_EQUAL_I32(filtered.count, 2);
    ASSUME_ITS_EQUAL_CSTR(filtered.memory[0].io.input, "b");
    ASSUME_ITS_EQUAL_CSTR(filtered.memory[1].io.input, "c");
}

FOSSIL_TEST_CASE(c_test_iochat_export_and_import_history) {
    fossil_jellyfish_chain_t chain, imported;
    fossil_jellyfish_init(&chain);
    fossil_jellyfish_init(&imported);

    fossil_io_chat_learn_response(&chain, "x", "y");
    fossil_io_chat_learn_response(&chain, "z", "w");

    const char *filepath = "test_iochat_history.txt";
    int export_result = fossil_io_chat_export_history(&chain, filepath);
    ASSUME_ITS_EQUAL_I32(export_result, 0);

    int import_result = fossil_io_chat_import_context(&imported, filepath);
    ASSUME_ITS_EQUAL_I32(import_result, 0);

    ASSUME_ITS_EQUAL_I32(imported.count, 2);
    ASSUME_ITS_EQUAL_CSTR(imported.memory[0].io.input, "x");
    ASSUME_ITS_EQUAL_CSTR(imported.memory[0].io.output, "y");
    ASSUME_ITS_EQUAL_CSTR(imported.memory[1].io.input, "z");
    ASSUME_ITS_EQUAL_CSTR(imported.memory[1].io.output, "w");

    remove(filepath);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(c_iochat_tests) {
    FOSSIL_TEST_ADD(c_iochat_fixture, c_test_iochat_start_and_end_session);
    FOSSIL_TEST_ADD(c_iochat_fixture, c_test_iochat_respond_known_and_unknown);
    FOSSIL_TEST_ADD(c_iochat_fixture, c_test_iochat_inject_system_message_and_immutable);
    FOSSIL_TEST_ADD(c_iochat_fixture, c_test_iochat_learn_response_and_turn_count);
    FOSSIL_TEST_ADD(c_iochat_fixture, c_test_iochat_summarize_session_basic);
    FOSSIL_TEST_ADD(c_iochat_fixture, c_test_iochat_filter_recent_turns);
    FOSSIL_TEST_ADD(c_iochat_fixture, c_test_iochat_export_and_import_history);

    FOSSIL_TEST_REGISTER(c_iochat_fixture);
} // end of tests
