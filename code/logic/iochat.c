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

int fossil_io_chat_start(const char *context_name) {
    (void)context_name;
    // Placeholder: allocate context memory or log session start
    return 0;
}

int fossil_io_chat_respond(fossil_jellyfish_chain *chain, const char *input, char *output, size_t size) {
    if (!chain || !input || !output || size == 0) return -1;

    float confidence;
    const fossil_jellyfish_block *block = NULL;

    if (fossil_jellyfish_reason_verbose(chain, input, output, &confidence, &block)) {
        return 0;
    } else {
        strncpy(output, "I'm not sure how to respond to that yet.", size - 1);
        output[size - 1] = '\0';
        return -1;
    }
}

int fossil_io_chat_end(void) {
    // Placeholder: cleanup session memory
    return 0;
}

int fossil_io_chat_inject_system_message(fossil_jellyfish_chain *chain, const char *message) {
    if (!chain || !message) return -1;

    fossil_jellyfish_learn(chain, "[system]", message);
    fossil_jellyfish_mark_immutable(&chain->memory[chain->count - 1]);
    return 0;
}

int fossil_io_chat_learn_response(fossil_jellyfish_chain *chain, const char *input, const char *output) {
    if (!chain || !input || !output) return -1;
    fossil_jellyfish_learn(chain, input, output);
    return 0;
}

int fossil_io_chat_turn_count(const fossil_jellyfish_chain *chain) {
    if (!chain) return 0;

    int count = 0;
    for (size_t i = 0; i < chain->count; ++i) {
        if (chain->memory[i].valid &&
            strncmp(chain->memory[i].input, "[system]", FOSSIL_JELLYFISH_INPUT_SIZE) != 0) {
            ++count;
        }
    }
    return count;
}

int fossil_io_chat_summarize_session(const fossil_jellyfish_chain *chain, char *summary, size_t size) {
    if (!chain || !summary || size == 0) return -1;

    size_t pos = 0;
    for (size_t i = 0; i < chain->count && pos + 64 < size; ++i) {
        const fossil_jellyfish_block *b = &chain->memory[i];
        if (!b->valid) continue;
        if (strncmp(b->input, "[system]", FOSSIL_JELLYFISH_INPUT_SIZE) == 0) continue;

        int written = snprintf(summary + pos, size - pos, "[%s] %s. ", b->input, b->output);
        if (written < 0) break;
        pos += written;
    }

    return pos > 0 ? 0 : -1;
}

int fossil_io_chat_filter_recent(const fossil_jellyfish_chain *chain, fossil_jellyfish_chain *out_chain, int turn_count) {
    if (!chain || !out_chain || turn_count <= 0) return -1;

    fossil_jellyfish_init_chain(out_chain);
    int added = 0;

    for (int i = (int)chain->count - 1; i >= 0 && added < turn_count; --i) {
        const fossil_jellyfish_block *b = &chain->memory[i];
        if (!b->valid) continue;
        if (strncmp(b->input, "[system]", FOSSIL_JELLYFISH_INPUT_SIZE) == 0) continue;

        // Copy recent valid user turn
        out_chain->memory[turn_count - added - 1] = *b;  // reverse order
        out_chain->memory[turn_count - added - 1].valid = true;
        added++;
    }

    out_chain->count = added;
    return 0;
}

int fossil_io_chat_export_history(const fossil_jellyfish_chain *chain, const char *filepath) {
    if (!chain || !filepath) return -1;

    FILE *f = fopen(filepath, "w");
    if (!f) return -1;

    for (size_t i = 0; i < chain->count; ++i) {
        const fossil_jellyfish_block *b = &chain->memory[i];
        if (!b->valid) continue;

        fprintf(f, "[%s] => %s\n", b->input, b->output);
    }

    fclose(f);
    return 0;
}

int fossil_io_chat_import_context(fossil_jellyfish_chain *chain, const char *filepath) {
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


