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
#include "fossil/ai/iochat.h"

// =========================================
// Session Initialization and Management
// =========================================

void fossil_iochat_init(fossil_iochat_session *session, fossil_jellyfish_chain *chain) {
    if (!session || !chain) return;
    memset(session, 0, sizeof(fossil_iochat_session));
    session->chain = chain;
    session->session_id = (uint32_t)(uintptr_t)session; // weak uniqueness
    session->response_threshold = 0.5f;
    session->enable_learning = true;
}

void fossil_iochat_reset(fossil_iochat_session *session) {
    if (!session) return;
    memset(session->last_input, 0, sizeof(session->last_input));
    memset(session->last_output, 0, sizeof(session->last_output));
}

void fossil_iochat_shutdown(fossil_iochat_session *session) {
    // No dynamic allocation yet
    (void)session;
}

// =========================================
// Core Chat Logic
// =========================================

bool fossil_iochat_respond(fossil_iochat_session *session, const char *input, char *output) {
    if (!session || !input || !output || !session->chain) return false;

    // Store input
    strncpy(session->last_input, input, FOSSIL_JELLYFISH_INPUT_SIZE - 1);

    // Attempt to find a matching block
    const fossil_jellyfish_block *block = fossil_jellyfish_best_match(session->chain, input);
    if (block && block->confidence >= session->response_threshold) {
        strncpy(session->last_output, block->output, FOSSIL_JELLYFISH_OUTPUT_SIZE - 1);
        strncpy(output, block->output, FOSSIL_JELLYFISH_OUTPUT_SIZE - 1);
        return true;
    }

    // Fallback: echo or fixed default response
    const char *fallback = "I'm still learning. Could you rephrase?";
    strncpy(session->last_output, fallback, FOSSIL_JELLYFISH_OUTPUT_SIZE - 1);
    strncpy(output, fallback, FOSSIL_JELLYFISH_OUTPUT_SIZE - 1);

    // Optionally learn new pair
    if (session->enable_learning) {
        fossil_jellyfish_learn(session->chain, input, fallback);
    }

    return true;
}

bool fossil_iochat_learn(fossil_iochat_session *session, const char *input, const char *output) {
    if (!session || !input || !output || !session->chain) return false;
    return fossil_jellyfish_learn(session->chain, input, output);
}

// =========================================
// Utility Functions
// =========================================

bool fossil_iochat_knows(const fossil_iochat_session *session, const char *input) {
    if (!session || !input || !session->chain) return false;
    const fossil_jellyfish_block *block = fossil_jellyfish_best_match(session->chain, input);
    return (block && block->confidence >= session->response_threshold);
}

const char* fossil_iochat_last_output(const fossil_iochat_session *session) {
    if (!session) return NULL;
    return session->last_output;
}
