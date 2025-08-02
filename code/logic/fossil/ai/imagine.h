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
#ifndef FOSSIL_JELLYFISH_IMAGINE_H
#define FOSSIL_JELLYFISH_IMAGINE_H

#include "jellyfish.h"

#ifdef __cplusplus
extern "C"
{
#endif

// *****************************************************************************
// Function prototypes
// *****************************************************************************

/**
 * @brief Generate an imagined block from a known input or source block.
 *
 * @param chain Jellyfish chain to insert into.
 * @param source_index Index of the original block (for context).
 * @param imagined_output Speculative output to store.
 * @param reason Short explanation (for traceability).
 * @return Index of the new imagined block, or -1 on failure.
 */
int fossil_imagine_from_block(fossil_jellyfish_chain* chain, size_t source_index, const char* imagined_output, const char* reason);

/**
 * @brief Create an imagined block from a raw input without a known source.
 *
 * @param chain Jellyfish chain to insert into.
 * @param imagined_input Input being speculated on.
 * @param imagined_output Speculative output.
 * @param reason Short explanation (for traceability).
 * @return Index of the new imagined block, or -1 on failure.
 */
int fossil_imagine_fresh(fossil_jellyfish_chain* chain, const char* imagined_input, const char* imagined_output, const char* reason);

/**
 * @brief Check if a block is imagined.
 */
int fossil_imagine_is_imagined(const fossil_jellyfish_block* blk);

/**
 * @brief Count how many imagined blocks exist in the chain.
 */
size_t fossil_imagine_count(const fossil_jellyfish_chain* chain);

/**
 * @brief Remove all imagined blocks (for pruning or cleanup).
 *
 * @return Number of blocks removed.
 */
size_t fossil_imagine_prune(fossil_jellyfish_chain* chain);

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
