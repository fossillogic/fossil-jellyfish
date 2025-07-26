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
#include "fossil/ai/language.h"

static const char *stopwords[] = {
    "the", "and", "is", "a", "an", "to", "of", "in", "on", "with", "for", NULL
};

void fossil_lang_normalize(char *text) {
    for (char *p = text; *p; ++p) {
        *p = (char)tolower((unsigned char)*p);
        if (!isalnum(*p) && !isspace(*p)) *p = ' ';
    }
}

size_t fossil_lang_tokenize(const char *input, fossil_lang_token *tokens, size_t max_tokens) {
    size_t count = 0;
    const char *start = NULL;
    for (const char *p = input; *p && count < max_tokens; ++p) {
        if (isalnum(*p)) {
            if (!start) start = p;
        } else if (start) {
            tokens[count++] = (fossil_lang_token){start, (size_t)(p - start)};
            start = NULL;
        }
    }
    if (start && count < max_tokens) {
        tokens[count++] = (fossil_lang_token){start, strlen(start)};
    }
    return count;
}

int fossil_lang_is_stopword(const char *word, size_t length) {
    for (int i = 0; stopwords[i]; ++i) {
        if (strlen(stopwords[i]) == length && strncmp(word, stopwords[i], length) == 0) {
            return 1;
        }
    }
    return 0;
}

const char* fossil_lang_guess_pos(const char *word, size_t length) {
    if (length > 3 && strcmp(word + length - 3, "ing") == 0) return "verb";
    if (length > 2 && strcmp(word + length - 2, "ed") == 0) return "verb";
    if (length > 1 && word[length - 1] == 'y') return "adj";
    if (length > 2 && word[length - 1] == 's') return "noun";
    return "unknown";
}

size_t fossil_lang_word_frequency(const char *text, const char *word) {
    size_t count = 0;
    size_t word_len = strlen(word);
    const char *p = text;
    while ((p = strstr(p, word)) != NULL) {
        if ((p == text || !isalnum(p[-1])) && !isalnum(p[word_len])) {
            count++;
        }
        p += word_len;
    }
    return count;
}

size_t fossil_lang_split_sentences(const char *text, fossil_lang_token *sentences, size_t max_sentences) {
    size_t count = 0;
    const char *start = text;
    for (const char *p = text; *p && count < max_sentences; ++p) {
        if (*p == '.' || *p == '?' || *p == '!') {
            while (*(p+1) == ' ') ++p;
            sentences[count++] = (fossil_lang_token){start, (size_t)(p - start + 1)};
            start = p + 1;
        }
    }
    if (*start && count < max_sentences) {
        sentences[count++] = (fossil_lang_token){start, strlen(start)};
    }
    return count;
}

size_t fossil_lang_stem(char *word, size_t length) {
    if (length > 4 && strcmp(&word[length - 3], "ing") == 0) return length - 3;
    if (length > 3 && strcmp(&word[length - 2], "ed") == 0) return length - 2;
    if (length > 3 && strcmp(&word[length - 1], "s") == 0) return length - 1;
    return length;
}

int fossil_lang_is_question(const char *sentence) {
    size_t len = strlen(sentence);
    return (len > 0 && sentence[len - 1] == '?');
}

const char* fossil_lang_detect_emotion(const char *sentence) {
    if (strstr(sentence, "thank") || strstr(sentence, "great") || strstr(sentence, "love"))
        return "positive";
    if (strstr(sentence, "hate") || strstr(sentence, "terrible") || strstr(sentence, "angry"))
        return "negative";
    return "neutral";
}

int fossil_lang_match_phrase(const char *text, const char *phrase) {
    char buf_text[512];
    char buf_phrase[128];

    strncpy(buf_text, text, sizeof(buf_text));
    strncpy(buf_phrase, phrase, sizeof(buf_phrase));
    fossil_lang_normalize(buf_text);
    fossil_lang_normalize(buf_phrase);

    return strstr(buf_text, buf_phrase) != NULL;
}
