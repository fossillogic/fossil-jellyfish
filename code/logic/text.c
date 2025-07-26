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
#include "fossil/ai/text.h"

// Strip leading and trailing whitespace in-place
void fossil_text_trim(char *text) {
    if (!text) return;
    size_t len = strlen(text);
    size_t start = 0;
    while (isspace((unsigned char)text[start])) ++start;
    while (len > start && isspace((unsigned char)text[len - 1])) --len;
    memmove(text, text + start, len - start);
    text[len - start] = '\0';
}

// Collapse all whitespace runs into single space
void fossil_text_collapse_whitespace(char *text) {
    char *read = text, *write = text;
    int in_space = 0;
    while (*read) {
        if (isspace((unsigned char)*read)) {
            if (!in_space) *write++ = ' ';
            in_space = 1;
        } else {
            *write++ = *read;
            in_space = 0;
        }
        ++read;
    }
    *write = '\0';
}

// Remove non-printable characters
void fossil_text_remove_nonprintable(char *text) {
    char *read = text, *write = text;
    while (*read) {
        if (isprint((unsigned char)*read) || isspace((unsigned char)*read)) {
            *write++ = *read;
        }
        ++read;
    }
    *write = '\0';
}

// Compute Levenshtein edit distance between two strings
int fossil_text_levenshtein(const char *a, const char *b) {
    size_t len_a = strlen(a), len_b = strlen(b);
    int *cost = malloc((len_b + 1) * sizeof(int));
    if (!cost) return -1;

    for (size_t j = 0; j <= len_b; j++) cost[j] = (int)j;

    for (size_t i = 1; i <= len_a; i++) {
        int prev = cost[0];
        cost[0] = (int)i;
        for (size_t j = 1; j <= len_b; j++) {
            int insert = cost[j] + 1;
            int del = cost[j - 1] + 1;
            int subst = prev + (a[i - 1] != b[j - 1]);
            prev = cost[j];
            cost[j] = (insert < del ? insert : del);
            if (subst < cost[j]) cost[j] = subst;
        }
    }
    int result = cost[len_b];
    free(cost);
    return result;
}

// Return length of common prefix
size_t fossil_text_prefix_match(const char *a, const char *b) {
    size_t count = 0;
    while (*a && *b && *a == *b) {
        ++count;
        ++a;
        ++b;
    }
    return count;
}

// Return length of common suffix
size_t fossil_text_suffix_match(const char *a, const char *b) {
    size_t len_a = strlen(a), len_b = strlen(b);
    size_t count = 0;
    while (len_a > 0 && len_b > 0 && a[len_a - 1] == b[len_b - 1]) {
        ++count;
        --len_a;
        --len_b;
    }
    return count;
}

// Simple djb2 hash
unsigned long fossil_text_checksum(const char *text) {
    unsigned long hash = 5381;
    int c;
    while ((c = *text++)) {
        hash = ((hash << 5) + hash) + (unsigned char)c;
    }
    return hash;
}

// Remove common punctuation, symbols, and brackets
void fossil_text_sanitize(char *text) {
    char *read = text, *write = text;
    while (*read) {
        if (isalnum((unsigned char)*read) || isspace((unsigned char)*read)) {
            *write++ = *read;
        } else {
            *write++ = ' ';
        }
        ++read;
    }
    *write = '\0';
}
