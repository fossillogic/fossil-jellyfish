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
#include "fossil/ai/imagine.h"
#include <string.h>
#include <time.h> // or platform-specific timestamp

static uint64_t now_ms() {
    return (uint64_t)(time(NULL)) * 1000;  // Replace with high-res timer if needed
}

int fossil_imagine_from_block(fossil_jellyfish_chain_t* chain, size_t source_index, const char* imagined_output, const char* reason) {
    if (!chain || source_index >= chain->count || !imagined_output)
        return -1;

    if (chain->count >= FOSSIL_JELLYFISH_MAX_MEM)
        return -1;

    fossil_jellyfish_block_t* src = &chain->memory[source_index];
    fossil_jellyfish_block_t* blk = &chain->memory[chain->count];
    memset(blk, 0, sizeof(fossil_jellyfish_block_t));

    strncpy(blk->input, src->input, sizeof(blk->input) - 1);
    strncpy(blk->output, imagined_output, sizeof(blk->output) - 1);
    blk->timestamp = now_ms();
    blk->valid = 1;
    blk->confidence = 0.5f;
    blk->imagined = 1;
    blk->imagined_from_index = (uint32_t)source_index;
    strncpy(blk->imagination_reason, reason ? reason : "unspecified", sizeof(blk->imagination_reason) - 1);

    chain->count++;
    chain->updated_at = blk->timestamp;
    return (int)(chain->count - 1);
}

int fossil_imagine_fresh(fossil_jellyfish_chain_t* chain, const char* imagined_input, const char* imagined_output, const char* reason) {
    if (!chain || !imagined_input || !imagined_output)
        return -1;

    if (chain->count >= FOSSIL_JELLYFISH_MAX_MEM)
        return -1;

    fossil_jellyfish_block_t* blk = &chain->memory[chain->count];
    memset(blk, 0, sizeof(fossil_jellyfish_block_t));

    strncpy(blk->input, imagined_input, sizeof(blk->input) - 1);
    strncpy(blk->output, imagined_output, sizeof(blk->output) - 1);
    blk->timestamp = now_ms();
    blk->valid = 1;
    blk->confidence = 0.3f;  // Lower confidence for fresh speculations
    blk->imagined = 1;
    blk->imagined_from_index = UINT32_MAX;
    strncpy(blk->imagination_reason, reason ? reason : "fresh speculation", sizeof(blk->imagination_reason) - 1);

    chain->count++;
    chain->updated_at = blk->timestamp;
    return (int)(chain->count - 1);
}

int fossil_imagine_is_imagined(const fossil_jellyfish_block_t* blk) {
    if (!blk) return 0;
    return blk->imagined != 0;
}

size_t fossil_imagine_count(const fossil_jellyfish_chain_t* chain) {
    if (!chain) return 0;
    size_t count = 0;
    for (size_t i = 0; i < chain->count; ++i) {
        if (chain->memory[i].imagined)
            count++;
    }
    return count;
}

size_t fossil_imagine_prune(fossil_jellyfish_chain_t* chain) {
    if (!chain) return 0;
    size_t new_count = 0;
    for (size_t i = 0; i < chain->count; ++i) {
        if (!chain->memory[i].imagined) {
            if (i != new_count)
                chain->memory[new_count] = chain->memory[i];
            new_count++;
        }
    }
    size_t removed = chain->count - new_count;
    chain->count = new_count;
    return removed;
}
