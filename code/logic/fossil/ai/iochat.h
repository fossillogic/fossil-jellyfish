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
#ifndef FOSSIL_JELLYFISH_IOCHAT_H
#define FOSSIL_JELLYFISH_IOCHAT_H

#include "jellyfish.h"

#ifdef __cplusplus
extern "C"
{
#endif

// *****************************************************************************
// Type definitions
// *****************************************************************************

// Session state object (simple version)
typedef struct {
    fossil_jellyfish_chain *chain;  // pointer to the AI memory
    char last_input[FOSSIL_JELLYFISH_INPUT_SIZE];
    char last_output[FOSSIL_JELLYFISH_OUTPUT_SIZE];
    uint32_t session_id;
    float response_threshold;  // minimum confidence to use an existing memory
    bool enable_learning;
} fossil_iochat_session;

// *****************************************************************************
// Function prototypes
// *****************************************************************************

/**
 * @brief Initialize a new chat session
 */
void fossil_iochat_init(fossil_iochat_session *session, fossil_jellyfish_chain *chain);

/**
 * @brief Reset the session state (clears last input/output)
 */
void fossil_iochat_reset(fossil_iochat_session *session);

/**
 * @brief Shut down a chat session (optional cleanup)
 */
void fossil_iochat_shutdown(fossil_iochat_session *session);


// ======================================
// Chat Input / Output
// ======================================

/**
 * @brief Process user input and return AI output. May reuse or generate new.
 * 
 * @param session Active chat session
 * @param input Input string
 * @param output Output buffer (caller must allocate size >= FOSSIL_JELLYFISH_OUTPUT_SIZE)
 * @return true if response was generated or reused
 */
bool fossil_iochat_respond(fossil_iochat_session *session, const char *input, char *output);

/**
 * @brief Force the system to learn this input/output pair into memory.
 */
bool fossil_iochat_learn(fossil_iochat_session *session, const char *input, const char *output);


// ======================================
// Utility and Introspection
// ======================================

/**
 * @brief Returns true if the given input is already known with high confidence
 */
bool fossil_iochat_knows(const fossil_iochat_session *session, const char *input);

/**
 * @brief Gets the last response from the session (copied from internal buffer)
 */
const char* fossil_iochat_last_output(const fossil_iochat_session *session);

#ifdef __cplusplus
}
#include <stdexcept>
#include <vector>
#include <string>

namespace fossil {

namespace ai {



} // namespace ai

} // namespace fossil

#endif

#endif /* fossil_fish_FRAMEWORK_H */