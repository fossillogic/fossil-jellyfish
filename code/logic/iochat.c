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

static char current_context_name[64] = {0};
static time_t session_start_time = 0;
static uint64_t session_id = 0;
static FILE *session_log_file = NULL;

// Helper: Format timestamp string
static void format_timestamp(time_t t, char *buf, size_t size) {
    struct tm *tm_info = localtime(&t);
    strftime(buf, size, "%Y-%m-%d %H:%M:%S", tm_info);
}

// Helper: Open session log file for appending
static int open_session_log(uint64_t id) {
    char filename[128];
    snprintf(filename, sizeof(filename), "session_%llu.log", (unsigned long long)id);
    session_log_file = fopen(filename, "a");
    return session_log_file ? 0 : -1;
}

// Helper: Close session log file
static void close_session_log(void) {
    if (session_log_file) {
        fclose(session_log_file);
        session_log_file = NULL;
    }
}

// Logs a line to the session log (if open)
static void log_session_line(const char *line) {
    if (session_log_file) {
        fprintf(session_log_file, "%s\n", line);
        fflush(session_log_file);
    }
}

// Add a system block to the chain
static void record_system_block(fossil_jellyfish_chain_t *chain, const char *msg) {
    if (!chain || !msg) return;

    // Compose system input as [system] plus message
    char input[FOSSIL_JELLYFISH_INPUT_SIZE];
    snprintf(input, sizeof(input), "[system]");

    // Output is the msg (truncate if needed)
    char output[FOSSIL_JELLYFISH_OUTPUT_SIZE];
    strncpy(output, msg, sizeof(output) - 1);
    output[sizeof(output) - 1] = '\0';

    fossil_jellyfish_learn(chain, input, output);
}

// --- API Functions ---

int fossil_io_chat_start(const char *context_name, fossil_jellyfish_chain_t *chain) {
    if (context_name && strlen(context_name) < sizeof(current_context_name)) {
        strncpy(current_context_name, context_name, sizeof(current_context_name) - 1);
    } else {
        strncpy(current_context_name, "default", sizeof(current_context_name) - 1);
    }

    session_start_time = time(NULL);
    session_id = (uint64_t)session_start_time;

    if (open_session_log(session_id) != 0) {
        fprintf(stderr, "[fossil_io_chat] Warning: Could not open session log file.\n");
    }

    char ts[32];
    format_timestamp(session_start_time, ts, sizeof(ts));

    char log_line[256];
    snprintf(log_line, sizeof(log_line), "Session started: %s @ %s", current_context_name, ts);
    log_session_line(log_line);

    if (chain) {
        char system_msg[200];
        snprintf(system_msg, sizeof(system_msg), "Session started with context \"%s\" at %s", current_context_name, ts);
        record_system_block(chain, system_msg);
    }

    return 0;
}

int fossil_io_chat_respond(fossil_jellyfish_chain_t *chain, const char *input, char *output, size_t size) {
    if (!chain || !input || !output || size == 0) return -1;

    float confidence = 0.0f;
    const fossil_jellyfish_block_t *matched_block = NULL;

    bool found = fossil_jellyfish_reason_verbose(chain, input, output, &confidence, &matched_block);

    if (found && matched_block && confidence > 0.3f) {
        char log_line[256];
        snprintf(log_line, sizeof(log_line), "Input: \"%s\" → Output: \"%s\" (confidence: %.2f)", input, output, confidence);
        log_session_line(log_line);

        fossil_jellyfish_learn(chain, input, output);
        return 0;
    } else {
        const char *fallback = "I'm not sure how to respond to that yet.";
        strncpy(output, fallback, size - 1);
        output[size - 1] = '\0';

        char log_line[256];
        snprintf(log_line, sizeof(log_line), "Input: \"%s\" → Fallback response used", input);
        log_session_line(log_line);

        fossil_jellyfish_learn(chain, input, output);
        return -1;
    }
}

int fossil_io_chat_end(fossil_jellyfish_chain_t *chain) {
    time_t now = time(NULL);
    double duration = difftime(now, session_start_time);

    char ts[32];
    format_timestamp(now, ts, sizeof(ts));

    char log_line[256];
    snprintf(log_line, sizeof(log_line), "Session \"%s\" ended after %.2f seconds @ %s", current_context_name, duration, ts);
    log_session_line(log_line);

    if (chain) {
        char system_msg[128];
        snprintf(system_msg, sizeof(system_msg), "Session ended after %.2f seconds at %s", duration, ts);
        record_system_block(chain, system_msg);
    }

    close_session_log();

    memset(current_context_name, 0, sizeof(current_context_name));
    session_start_time = 0;
    session_id = 0;

    return 0;
}

int fossil_io_chat_inject_system_message(fossil_jellyfish_chain_t *chain, const char *message) {
    if (!chain || !message || strlen(message) == 0) {
        return -1;
    }

    if (chain->count >= FOSSIL_JELLYFISH_MAX_MEM) {
        fprintf(stderr, "[fossil_io_chat] Chain memory full, cannot inject system message.\n");
        return -1;
    }

    // Add system message with input key "[system]"
    fossil_jellyfish_learn(chain, "[system]", message);

    // Mark the newly added block as immutable to protect it
    fossil_jellyfish_mark_immutable(&chain->memory[chain->count - 1]);

    printf("[fossil_io_chat] Injected system message: \"%s\"\n", message);

    return 0;
}

int fossil_io_chat_learn_response(fossil_jellyfish_chain_t *chain, const char *input, const char *output) {
    if (!chain || !input || !output || strlen(input) == 0 || strlen(output) == 0) {
        return -1;
    }

    if (chain->count >= FOSSIL_JELLYFISH_MAX_MEM) {
        fprintf(stderr, "[fossil_io_chat] Chain memory full, cannot learn new response.\n");
        return -1;
    }

    fossil_jellyfish_learn(chain, input, output);

    printf("[fossil_io_chat] Learned new response for input: \"%s\"\n", input);

    return 0;
}

int fossil_io_chat_turn_count(const fossil_jellyfish_chain_t *chain) {
    if (!chain) return 0;

    int count = 0;
    for (size_t i = 0; i < chain->count; ++i) {
        const fossil_jellyfish_block_t *b = &chain->memory[i];
        if (b->attributes.valid &&
            strncmp(b->io.input, "[system]", sizeof("[system]") - 1) != 0) {
            ++count;
        }
    }
    return count;
}

int fossil_io_chat_summarize_session(const fossil_jellyfish_chain_t *chain, char *summary, size_t size) {
    if (!chain || !summary || size == 0) return -1;

    size_t pos = 0;
    for (size_t i = 0; i < chain->count && pos + 64 < size; ++i) {
        const fossil_jellyfish_block_t *b = &chain->memory[i];
        if (!b->attributes.valid) continue;
        if (strncmp(b->io.input, "[system]", FOSSIL_JELLYFISH_INPUT_SIZE) == 0) continue;

        int written = snprintf(summary + pos, size - pos, "[%s] %s. ", b->io.input, b->io.output);
        if (written < 0) break;
        pos += written;
    }

    return pos > 0 ? 0 : -1;
}

int fossil_io_chat_filter_recent(const fossil_jellyfish_chain_t *chain, fossil_jellyfish_chain_t *out_chain, int turn_count) {
    if (!chain || !out_chain || turn_count <= 0) return -1;

    fossil_jellyfish_init(out_chain);
    int added = 0;

    for (int i = (int)chain->count - 1; i >= 0 && added < turn_count; --i) {
        const fossil_jellyfish_block_t *b = &chain->memory[i];
        if (!b->attributes.valid) continue;
        if (strncmp(b->io.input, "[system]", FOSSIL_JELLYFISH_INPUT_SIZE) == 0) continue;

        // Copy recent valid user turn
        out_chain->memory[turn_count - added - 1] = *b;  // reverse order
        out_chain->memory[turn_count - added - 1].attributes.valid = 1;
        added++;
    }

    out_chain->count = added;
    return 0;
}

int fossil_io_chat_export_history(const fossil_jellyfish_chain_t *chain, const char *filepath) {
    if (!chain || !filepath) return -1;

    FILE *f = fopen(filepath, "w");
    if (!f) return -1;

    for (size_t i = 0; i < chain->count; ++i) {
        const fossil_jellyfish_block_t *b = &chain->memory[i];
        if (!b->attributes.valid) continue;

        fprintf(f, "[%s] => %s\n", b->io.input, b->io.output);
    }

    fclose(f);
    return 0;
}

int fossil_io_chat_import_context(fossil_jellyfish_chain_t *chain, const char *filepath) {
    if (!chain || !filepath) return -1;

    FILE *f = fopen(filepath, "r");
    if (!f) return -1;

    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char *arrow = strstr(line, "=>");
        if (!arrow) continue;

        *arrow = '\0'; // split input/output
        char *input = line;
        char *output = arrow + 2;

        // Trim surrounding whitespace
        while (*input == '[') ++input;
        char *end = strchr(input, ']');
        if (end) *end = '\0';

        while (*output == ' ') ++output;
        end = strchr(output, '\n');
        if (end) *end = '\0';

        fossil_jellyfish_learn(chain, input, output);
    }

    fclose(f);
    return 0;
}


