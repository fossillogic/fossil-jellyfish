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
#include "fossil/ai/jellydsl.h"
#include <ctype.h>

// Meson-style JellyDSL parser for .jellyfish "mindset" blocks

// --- Tokenizer/Lexer ---

typedef enum {
    TOK_EOF,
    TOK_IDENT,
    TOK_STRING,
    TOK_NUMBER,
    TOK_BOOL,
    TOK_COMMA,
    TOK_COLON,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACK,
    TOK_RBRACK
} jellydsl_token_type;

typedef struct {
    jellydsl_token_type type;
    char text[128];
} jellydsl_token;

typedef struct {
    const char *src;
    size_t pos;
    size_t len;
} jellydsl_lexer;

static void jellydsl_lexer_init(jellydsl_lexer *lx, const char *src) {
    lx->src = src;
    lx->pos = 0;
    lx->len = strlen(src);
}

static void jellydsl_lexer_skipws(jellydsl_lexer *lx) {
    while (lx->pos < lx->len && isspace((unsigned char)lx->src[lx->pos]))
        lx->pos++;
}

static int jellydsl_lexer_peek(jellydsl_lexer *lx) {
    return (lx->pos < lx->len) ? lx->src[lx->pos] : 0;
}

static int jellydsl_lexer_next(jellydsl_lexer *lx) {
    return (lx->pos < lx->len) ? lx->src[lx->pos++] : 0;
}

static jellydsl_token jellydsl_lexer_next_token(jellydsl_lexer *lx) {
    jellydsl_token tok = {0};
    jellydsl_lexer_skipws(lx);
    int c = jellydsl_lexer_peek(lx);

    if (!c) { tok.type = TOK_EOF; return tok; }

    if (isalpha(c) || c == '_') {
        // Identifier or boolean
        size_t start = lx->pos;
        while (isalnum(jellydsl_lexer_peek(lx)) || jellydsl_lexer_peek(lx) == '_')
            lx->pos++;
        size_t n = lx->pos - start;
        if (n >= sizeof(tok.text)) n = sizeof(tok.text) - 1;
        strncpy(tok.text, lx->src + start, n);
        tok.text[n] = 0;
        if (strcmp(tok.text, "true") == 0 || strcmp(tok.text, "false") == 0)
            tok.type = TOK_BOOL;
        else
            tok.type = TOK_IDENT;
        return tok;
    }

    if (isdigit(c) || c == '-') {
        // Number (int or float)
        size_t start = lx->pos;
        if (c == '-') lx->pos++;
        while (isdigit(jellydsl_lexer_peek(lx))) lx->pos++;
        if (jellydsl_lexer_peek(lx) == '.') {
            lx->pos++;
            while (isdigit(jellydsl_lexer_peek(lx))) lx->pos++;
        }
        size_t n = lx->pos - start;
        if (n >= sizeof(tok.text)) n = sizeof(tok.text) - 1;
        strncpy(tok.text, lx->src + start, n);
        tok.text[n] = 0;
        tok.type = TOK_NUMBER;
        return tok;
    }

    if (c == '"' || c == '\'') {
        // String
        int quote = jellydsl_lexer_next(lx);
        size_t i = 0;
        while (lx->pos < lx->len && lx->src[lx->pos] != quote && i < sizeof(tok.text) - 1) {
            if (lx->src[lx->pos] == '\\' && lx->pos + 1 < lx->len) lx->pos++; // skip escape
            tok.text[i++] = lx->src[lx->pos++];
        }
        tok.text[i] = 0;
        if (lx->src[lx->pos] == quote) lx->pos++;
        tok.type = TOK_STRING;
        return tok;
    }

    switch (c) {
        case ',': tok.type = TOK_COMMA;   lx->pos++; return tok;
        case ':': tok.type = TOK_COLON;   lx->pos++; return tok;
        case '(': tok.type = TOK_LPAREN;  lx->pos++; return tok;
        case ')': tok.type = TOK_RPAREN;  lx->pos++; return tok;
        case '[': tok.type = TOK_LBRACK;  lx->pos++; return tok;
        case ']': tok.type = TOK_RBRACK;  lx->pos++; return tok;
        default:  lx->pos++; return jellydsl_lexer_next_token(lx); // skip unknown
    }
}

// --- Parser ---

static int jellydsl_parse_list(jellydsl_lexer *lx, char arr[][32], int max, int *out_count) {
    *out_count = 0;
    jellydsl_token tok = jellydsl_lexer_next_token(lx);
    if (tok.type != TOK_LBRACK) return -1;
    while (1) {
        tok = jellydsl_lexer_next_token(lx);
        if (tok.type == TOK_RBRACK) break;
        if (tok.type == TOK_STRING || tok.type == TOK_IDENT) {
            if (*out_count < max) {
                strncpy(arr[*out_count], tok.text, 31);
                arr[*out_count][31] = 0;
                (*out_count)++;
            }
        }
        tok = jellydsl_lexer_next_token(lx);
        if (tok.type == TOK_RBRACK) break;
        if (tok.type != TOK_COMMA) return -1;
    }
    return 0;
}

// Utility internal function
static bool file_exists(const char *path) {
    FILE *f = fopen(path, "rb");
    if (f) { fclose(f); return true; }
    return false;
}

// Helper to strip quotes from a quoted string
static void strip_quotes(char *s) {
    if (s[0] == '"' || s[0] == '\'') {
        memmove(s, s + 1, strlen(s));
        size_t len = strlen(s);
        if (len > 0 && (s[len - 1] == '"' || s[len - 1] == '\''))
            s[len - 1] = '\0';
    }
}

// Helper to trim whitespace
static char *trim(char *str) {
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

// *****************************************************************************
// .jellyfish: JellyDSL Model Mindset
// *****************************************************************************

int fossil_jellydsl_load_model(const char *filepath, fossil_jellyfish_jellydsl *out) {
    if (!filepath || !out) return -1;
    FILE *f = fopen(filepath, "r");
    if (!f) return -1;

    // Read file into buffer
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(sz + 1);
    if (!buf) { fclose(f); return -1; }
    fread(buf, 1, sz, f);
    buf[sz] = 0;
    fclose(f);

    memset(out, 0, sizeof(*out));
    jellydsl_lexer lx;
    jellydsl_lexer_init(&lx, buf);

    // --- Skip comments at the start ---
    while (1) {
        jellydsl_lexer_skipws(&lx);
        if (lx.src[lx.pos] == '#') {
            while (lx.pos < lx.len && lx.src[lx.pos] != '\n') lx.pos++;
        } else {
            break;
        }
    }

    jellydsl_token tok = jellydsl_lexer_next_token(&lx);
    if (tok.type != TOK_IDENT || strcmp(tok.text, "mindset") != 0) { free(buf); return -1; }
    if (jellydsl_lexer_next_token(&lx).type != TOK_LPAREN) { free(buf); return -1; }

    // Parse: mindset('name', key: value, ...)
    tok = jellydsl_lexer_next_token(&lx);
    if (tok.type == TOK_STRING || tok.type == TOK_IDENT) {
        strncpy(out->name, tok.text, sizeof(out->name) - 1);
        tok = jellydsl_lexer_next_token(&lx);
        if (tok.type == TOK_COMMA) tok = jellydsl_lexer_next_token(&lx);
    } else { free(buf); return -1; }

    while (tok.type != TOK_RPAREN && tok.type != TOK_EOF) {
        // --- Skip comments between fields ---
        while (tok.type == TOK_IDENT && tok.text[0] == '#') {
            // Skip until end of line
            while (lx.pos < lx.len && lx.src[lx.pos] != '\n') lx.pos++;
            tok = jellydsl_lexer_next_token(&lx);
        }
        if (tok.type != TOK_IDENT) { free(buf); return -1; }
        char key[64]; strncpy(key, tok.text, 63); key[63] = 0;
        if (jellydsl_lexer_next_token(&lx).type != TOK_COLON) { free(buf); return -1; }
        tok = jellydsl_lexer_next_token(&lx);

        if (strcmp(key, "description") == 0 && (tok.type == TOK_STRING || tok.type == TOK_IDENT)) {
            strncpy(out->description, tok.text, sizeof(out->description) - 1);
        } else if (strcmp(key, "priority") == 0 && tok.type == TOK_NUMBER) {
            out->priority = atoi(tok.text);
        } else if (strcmp(key, "confidence_threshold") == 0 && tok.type == TOK_NUMBER) {
            out->confidence_threshold = atof(tok.text);
        } else if (strcmp(key, "immutable") == 0 && tok.type == TOK_BOOL) {
            out->immutable = (strcmp(tok.text, "true") == 0) ? 1 : 0;
        } else if (strcmp(key, "activation_condition") == 0 && (tok.type == TOK_STRING || tok.type == TOK_IDENT)) {
            strncpy(out->activation_condition, tok.text, sizeof(out->activation_condition) - 1);
        } else if (strcmp(key, "version") == 0 && (tok.type == TOK_STRING || tok.type == TOK_IDENT)) {
            strncpy(out->version, tok.text, sizeof(out->version) - 1);
        } else if (strcmp(key, "source_uri") == 0 && (tok.type == TOK_STRING || tok.type == TOK_IDENT)) {
            strncpy(out->source_uri, tok.text, sizeof(out->source_uri) - 1);
        } else if (strcmp(key, "origin_device_id") == 0 && (tok.type == TOK_STRING || tok.type == TOK_IDENT)) {
            strncpy(out->origin_device_id, tok.text, sizeof(out->origin_device_id) - 1);
        } else if (strcmp(key, "content_hash") == 0 && (tok.type == TOK_STRING || tok.type == TOK_IDENT)) {
            strncpy(out->content_hash, tok.text, sizeof(out->content_hash) - 1);
        } else if (strcmp(key, "created_at") == 0 && tok.type == TOK_NUMBER) {
            out->created_at = strtoull(tok.text, NULL, 10);
        } else if (strcmp(key, "updated_at") == 0 && tok.type == TOK_NUMBER) {
            out->updated_at = strtoull(tok.text, NULL, 10);
        } else if (strcmp(key, "trust_score") == 0 && tok.type == TOK_NUMBER) {
            out->trust_score = atof(tok.text);
        } else if (strcmp(key, "state_machine") == 0 && (tok.type == TOK_STRING || tok.type == TOK_IDENT)) {
            strncpy(out->state_machine, tok.text, sizeof(out->state_machine) - 1);
        } else if (strcmp(key, "tags") == 0 && tok.type == TOK_LBRACK) {
            lx.pos--; // rewind so parse_list sees '['
            if (jellydsl_parse_list(&lx, out->tags, FOSSIL_JELLYFISH_MAX_TAGS, &out->tag_count) != 0) { free(buf); return -1; }
        } else if (strcmp(key, "models") == 0 && tok.type == TOK_LBRACK) {
            lx.pos--;
            if (jellydsl_parse_list(&lx, out->models, FOSSIL_JELLYFISH_MAX_MODELS, &out->model_count) != 0) { free(buf); return -1; }
        } else if (strcmp(key, "chain") == 0 && (tok.type == TOK_STRING || tok.type == TOK_IDENT)) {
            // Optionally store chain path if your struct supports it
        }
        // skip trailing comma and comments
        tok = jellydsl_lexer_next_token(&lx);
        while (tok.type == TOK_IDENT && tok.text[0] == '#') {
            while (lx.pos < lx.len && lx.src[lx.pos] != '\n') lx.pos++;
            tok = jellydsl_lexer_next_token(&lx);
        }
        if (tok.type == TOK_COMMA) tok = jellydsl_lexer_next_token(&lx);
    }

    free(buf);
    return 0;
}

int fossil_jellydsl_save_model(const char *filepath, const fossil_jellyfish_jellydsl *model) {
    if (!filepath || !model) return -1;

    FILE *f = fopen(filepath, "w");
    if (!f) return -1;

    // Meson-style: mindset('name', key: value, ...)
    fprintf(f, "mindset('%s',\n", model->name);

    // description
    fprintf(f, "  description: '%s',\n", model->description);

    // tags
    fprintf(f, "  tags: [");
    for (int i = 0; i < model->tag_count; i++) {
        fprintf(f, "'%s'%s", model->tags[i], (i < model->tag_count - 1) ? ", " : "");
    }
    fprintf(f, "],\n");

    // models
    fprintf(f, "  models: [");
    for (int i = 0; i < model->model_count; i++) {
        fprintf(f, "'%s'%s", model->models[i], (i < model->model_count - 1) ? ", " : "");
    }
    fprintf(f, "],\n");

    // other fields
    fprintf(f, "  priority: %d,\n", model->priority);
    fprintf(f, "  confidence_threshold: %.4f,\n", model->confidence_threshold);
    fprintf(f, "  immutable: %s,\n", model->immutable ? "true" : "false");
    fprintf(f, "  activation_condition: '%s',\n", model->activation_condition);
    fprintf(f, "  version: '%s',\n", model->version);
    fprintf(f, "  source_uri: '%s',\n", model->source_uri);
    fprintf(f, "  origin_device_id: '%s',\n", model->origin_device_id);
    fprintf(f, "  content_hash: '%s',\n", model->content_hash);
    fprintf(f, "  created_at: %llu,\n", (unsigned long long)model->created_at);
    fprintf(f, "  updated_at: %llu,\n", (unsigned long long)model->updated_at);
    fprintf(f, "  trust_score: %.4f,\n", model->trust_score);
    fprintf(f, "  state_machine: '%s'", model->state_machine);

    // If you want to support chain, add here (if present)
    // fprintf(f, ",\n  chain: '%s'", model->chain);

    fprintf(f, "\n)\n");

    fclose(f);
    return 0;
}

// *****************************************************************************
// .jfchain: Jellyfish Memory Chain
// *****************************************************************************

int fossil_jellydsl_load_chain(const char *filepath, fossil_jellyfish_chain_t *out) {
    if (!filepath || !out) return -1;

    FILE *f = fopen(filepath, "r");
    if (!f) return -1;

    memset(out, 0, sizeof(fossil_jellyfish_chain_t));

    char line[1024];
    fossil_jellyfish_block_t temp;
    memset(&temp, 0, sizeof(temp));
    int in_block = 0;

    while (fgets(line, sizeof(line), f)) {
        char *colon = strchr(line, ':');
        if (!colon) {
            // Detect block start/end
            if (strchr(line, '{')) {
                memset(&temp, 0, sizeof(temp));
                in_block = 1;
            } else if (strchr(line, '}')) {
                if (in_block && out->count < FOSSIL_JELLYFISH_MAX_MEM) {
                    out->memory[out->count++] = temp;
                }
                in_block = 0;
            }
            continue;
        }

        *colon = '\0';
        char *key = trim(line);
        char *val = trim(colon + 1);
        strip_quotes(val);

        if (!in_block) {
            if (strcmp(key, "created_at") == 0)
                out->created_at = strtoull(val, NULL, 10);
            else if (strcmp(key, "updated_at") == 0)
                out->updated_at = strtoull(val, NULL, 10);
            else if (strcmp(key, "device_id") == 0) {
                // device_id as hex string
                size_t len = strlen(val) / 2;
                if (len > FOSSIL_DEVICE_ID_SIZE) len = FOSSIL_DEVICE_ID_SIZE;
                for (size_t i = 0; i < len; ++i) {
                    unsigned int byte = 0;
                    sscanf(val + 2 * i, "%2x", &byte);
                    out->device_id[i] = (uint8_t)byte;
                }
            }
            continue;
        }

        // Block fields
        if (strcmp(key, "input") == 0)
            strncpy(temp.io.input, val, FOSSIL_JELLYFISH_INPUT_SIZE - 1);
        else if (strcmp(key, "output") == 0)
            strncpy(temp.io.output, val, FOSSIL_JELLYFISH_OUTPUT_SIZE - 1);
        else if (strcmp(key, "timestamp") == 0)
            temp.time.timestamp = strtoull(val, NULL, 10);
        else if (strcmp(key, "delta_ms") == 0)
            temp.time.delta_ms = (uint32_t)atoi(val);
        else if (strcmp(key, "duration_ms") == 0)
            temp.time.duration_ms = (uint32_t)atoi(val);
        else if (strcmp(key, "valid") == 0)
            temp.attributes.valid = atoi(val);
        else if (strcmp(key, "confidence") == 0)
            temp.attributes.confidence = (float)atof(val);
        else if (strcmp(key, "usage_count") == 0)
            temp.attributes.usage_count = (uint32_t)atoi(val);
        else if (strcmp(key, "immutable") == 0)
            temp.attributes.immutable = atoi(val);
        else if (strcmp(key, "block_type") == 0)
            temp.block_type = (fossil_jellyfish_block_type_t)atoi(val);
        else if (strcmp(key, "imagined") == 0)
            temp.classify.is_hallucinated = atoi(val);
        else if (strcmp(key, "imagined_from_index") == 0)
            temp.classify.derived_from_index = (uint32_t)atoi(val);
        else if (strcmp(key, "imagination_reason") == 0)
            strncpy(temp.classify.classification_reason, val, sizeof(temp.classify.classification_reason) - 1);
        else if (strcmp(key, "hash") == 0) {
            // hex string to bytes
            size_t len = strlen(val) / 2;
            if (len > FOSSIL_JELLYFISH_HASH_SIZE) len = FOSSIL_JELLYFISH_HASH_SIZE;
            for (size_t i = 0; i < len; ++i) {
                unsigned int byte = 0;
                sscanf(val + 2 * i, "%2x", &byte);
                temp.identity.hash[i] = (uint8_t)byte;
            }
        }
        else if (strcmp(key, "signature") == 0) {
            size_t len = strlen(val) / 2;
            if (len > FOSSIL_SIGNATURE_SIZE) len = FOSSIL_SIGNATURE_SIZE;
            for (size_t i = 0; i < len; ++i) {
                unsigned int byte = 0;
                sscanf(val + 2 * i, "%2x", &byte);
                temp.identity.signature[i] = (uint8_t)byte;
            }
        }
        else if (strcmp(key, "device_id") == 0) {
            // block-level device_id as hex string
            size_t len = strlen(val) / 2;
            if (len > FOSSIL_DEVICE_ID_SIZE) len = FOSSIL_DEVICE_ID_SIZE;
            for (size_t i = 0; i < len; ++i) {
                unsigned int byte = 0;
                sscanf(val + 2 * i, "%2x", &byte);
                temp.identity.device_id[i] = (uint8_t)byte;
            }
        }
    }

    fclose(f);
    return 0;
}

int fossil_jellydsl_save_chain(const char *filepath, const fossil_jellyfish_chain_t *chain) {
    if (!filepath || !chain) return -1;

    FILE *f = fopen(filepath, "w");
    if (!f) return -1;

    fprintf(f, "created_at: %llu\n", (unsigned long long)chain->created_at);
    fprintf(f, "updated_at: %llu\n", (unsigned long long)chain->updated_at);

    // Write device_id as hex
    fprintf(f, "device_id: \"");
    for (size_t i = 0; i < FOSSIL_DEVICE_ID_SIZE; ++i)
        fprintf(f, "%02x", chain->device_id[i]);
    fprintf(f, "\"\n");

    for (size_t i = 0; i < chain->count; ++i) {
        const fossil_jellyfish_block_t *b = &chain->memory[i];
        fprintf(f, "{\n");

        // IO
        fprintf(f, "  input: \"%s\"\n", b->io.input);
        fprintf(f, "  output: \"%s\"\n", b->io.output);

        // Identity
        fprintf(f, "  hash: \"");
        for (size_t j = 0; j < FOSSIL_JELLYFISH_HASH_SIZE; ++j)
            fprintf(f, "%02x", b->identity.hash[j]);
        fprintf(f, "\"\n");

        fprintf(f, "  device_id: \"");
        for (size_t j = 0; j < FOSSIL_DEVICE_ID_SIZE; ++j)
            fprintf(f, "%02x", b->identity.device_id[j]);
        fprintf(f, "\"\n");

        fprintf(f, "  signature: \"");
        for (size_t j = 0; j < FOSSIL_SIGNATURE_SIZE; ++j)
            fprintf(f, "%02x", b->identity.signature[j]);
        fprintf(f, "\"\n");

        fprintf(f, "  block_index: %u\n", b->identity.block_index);
        fprintf(f, "  prev_block_index: %u\n", b->identity.prev_block_index);

        fprintf(f, "  prev_hash: \"");
        for (size_t j = 0; j < FOSSIL_JELLYFISH_HASH_SIZE; ++j)
            fprintf(f, "%02x", b->identity.prev_hash[j]);
        fprintf(f, "\"\n");

        fprintf(f, "  signature_len: %u\n", b->identity.signature_len);

        // Time
        fprintf(f, "  timestamp: %llu\n", (unsigned long long)b->time.timestamp);
        fprintf(f, "  delta_ms: %u\n", b->time.delta_ms);
        fprintf(f, "  duration_ms: %u\n", b->time.duration_ms);
        fprintf(f, "  updated_at: %llu\n", (unsigned long long)b->time.updated_at);
        fprintf(f, "  expires_at: %llu\n", (unsigned long long)b->time.expires_at);
        fprintf(f, "  validated_at: %llu\n", (unsigned long long)b->time.validated_at);

        // Attributes
        fprintf(f, "  immutable: %d\n", b->attributes.immutable);
        fprintf(f, "  valid: %d\n", b->attributes.valid);
        fprintf(f, "  confidence: %.4f\n", b->attributes.confidence);
        fprintf(f, "  usage_count: %u\n", b->attributes.usage_count);
        fprintf(f, "  pruned: %d\n", b->attributes.pruned);
        fprintf(f, "  redacted: %d\n", b->attributes.redacted);
        fprintf(f, "  deduplicated: %d\n", b->attributes.deduplicated);
        fprintf(f, "  compressed: %d\n", b->attributes.compressed);
        fprintf(f, "  expired: %d\n", b->attributes.expired);
        fprintf(f, "  trusted: %d\n", b->attributes.trusted);
        fprintf(f, "  conflicted: %d\n", b->attributes.conflicted);

        // Block type
        fprintf(f, "  block_type: %d\n", (int)b->block_type);

        // Classification
        fprintf(f, "  derived_from_index: %u\n", b->classify.derived_from_index);
        fprintf(f, "  classification_reason: \"%s\"\n", b->classify.classification_reason);
        fprintf(f, "  similarity_score: %.4f\n", b->classify.similarity_score);
        fprintf(f, "  is_hallucinated: %d\n", b->classify.is_hallucinated);
        fprintf(f, "  is_contradicted: %d\n", b->classify.is_contradicted);

        // Tags
        fprintf(f, "  tags: [");
        for (int t = 0; t < FOSSIL_JELLYFISH_MAX_TAGS; ++t) {
            if (b->classify.tags[t][0]) {
                if (t > 0) fprintf(f, ", ");
                fprintf(f, "\"%s\"", b->classify.tags[t]);
            }
        }
        fprintf(f, "]\n");

        fprintf(f, "}\n");
    }

    fclose(f);
    return 0;
}

// *****************************************************************************
// .jfidea: Imagination Blocks
// *****************************************************************************

int fossil_jellydsl_load_idea(const char *filepath, fossil_jellydsl_idea *out) {
    if (!filepath || !out || !file_exists(filepath)) return -1;

    FILE *f = fopen(filepath, "r");
    if (!f) return -1;

    memset(out, 0, sizeof(fossil_jellydsl_idea));

    char line[1024];
    int in_generated = 0;
    int in_tags = 0;

    while (fgets(line, sizeof(line), f)) {
        char *key = strtok(line, ":");
        char *val = strtok(NULL, "\n");

        if (!key || !val) continue;

        key = trim(key);
        val = trim(val);
        strip_quotes(val);

        if (strcmp(key, "seed") == 0) {
            strncpy(out->seed, val, sizeof(out->seed));
        } else if (strcmp(key, "prompt_type") == 0) {
            strncpy(out->prompt_type, val, sizeof(out->prompt_type));
        } else if (strcmp(key, "last_used") == 0) {
            out->last_used = strtoull(val, NULL, 10);
        } else if (strcmp(key, "generated") == 0) {
            in_generated = 1;
            in_tags = 0;
            out->count = 0;
        } else if (strcmp(key, "tags") == 0) {
            in_tags = 1;
            in_generated = 0;
            out->tag_count = 0;
        } else if (in_generated && val[0] == '"') {
            strip_quotes(val);
            if (out->count < 8)
                strncpy(out->generated[out->count++], val, sizeof(out->generated[0]));
        } else if (in_tags && val[0] == '"') {
            strip_quotes(val);
            if (out->tag_count < 8)
                strncpy(out->tags[out->tag_count++], val, sizeof(out->tags[0]));
        }
    }

    fclose(f);
    return 0;
}

int fossil_jellydsl_save_idea(const char *filepath, const fossil_jellydsl_idea *idea) {
    if (!filepath || !idea) return -1;

    FILE *f = fopen(filepath, "w");
    if (!f) return -1;

    fprintf(f, "seed: \"%s\"\n", idea->seed);
    fprintf(f, "prompt_type: \"%s\"\n", idea->prompt_type);
    fprintf(f, "last_used: %llu\n", (unsigned long long)idea->last_used);

    fprintf(f, "generated: [\n");
    for (int i = 0; i < idea->count; ++i) {
        fprintf(f, "  \"%s\"\n", idea->generated[i]);
    }
    fprintf(f, "]\n");

    fprintf(f, "tags: [\n");
    for (int i = 0; i < idea->tag_count; ++i) {
        fprintf(f, "  \"%s\"\n", idea->tags[i]);
    }
    fprintf(f, "]\n");

    fclose(f);
    return 0;
}

// *****************************************************************************
// .jfmeta: Metadata
// *****************************************************************************

int fossil_jellydsl_load_metadata(const char *filepath, fossil_jellydsl_metadata *out) {
    if (!filepath || !out || !file_exists(filepath)) return -1;

    FILE *f = fopen(filepath, "r");
    if (!f) return -1;

    memset(out, 0, sizeof(fossil_jellydsl_metadata));
    char line[1024];
    int in_related = 0;

    while (fgets(line, sizeof(line), f)) {
        char *key = strtok(line, ":");
        char *val = strtok(NULL, "\n");

        if (!key || !val) continue;
        key = trim(key);
        val = trim(val);
        strip_quotes(val);

        if (strcmp(key, "origin") == 0) {
            strncpy(out->origin, val, sizeof(out->origin));
        } else if (strcmp(key, "source_uri") == 0) {
            strncpy(out->source_uri, val, sizeof(out->source_uri));
        } else if (strcmp(key, "device") == 0) {
            strncpy(out->device, val, sizeof(out->device));
        } else if (strcmp(key, "license") == 0) {
            strncpy(out->license, val, sizeof(out->license));
        } else if (strcmp(key, "content_hash") == 0) {
            strncpy(out->content_hash, val, sizeof(out->content_hash));
        } else if (strcmp(key, "related_files") == 0) {
            in_related = 1;
            out->file_count = 0;
        } else if (strcmp(key, "file_count") == 0) {
            out->file_count = atoi(val);
        } else if (in_related && val[0] == '"') {
            strip_quotes(val);
            if (out->file_count < 8) {
                strncpy(out->related_files[out->file_count++], val, sizeof(out->related_files[0]));
            }
        }
    }

    fclose(f);
    return 0;
}

int fossil_jellydsl_save_metadata(const char *filepath, const fossil_jellydsl_metadata *meta) {
    if (!filepath || !meta) return -1;

    FILE *f = fopen(filepath, "w");
    if (!f) return -1;

    fprintf(f, "origin: \"%s\"\n", meta->origin);
    fprintf(f, "source_uri: \"%s\"\n", meta->source_uri);
    fprintf(f, "device: \"%s\"\n", meta->device);
    fprintf(f, "license: \"%s\"\n", meta->license);
    fprintf(f, "content_hash: \"%s\"\n", meta->content_hash);

    fprintf(f, "related_files: [\n");
    for (int i = 0; i < meta->file_count; ++i) {
        fprintf(f, "  \"%s\"\n", meta->related_files[i]);
    }
    fprintf(f, "]\n");

    fprintf(f, "file_count: %d\n", meta->file_count);

    fclose(f);
    return 0;
}

// *****************************************************************************
// .jfsig: Signatures
// *****************************************************************************

int fossil_jellydsl_load_signature(const char *filepath, fossil_jellydsl_signature *out) {
    if (!filepath || !out || !file_exists(filepath)) return -1;

    FILE *f = fopen(filepath, "r");
    if (!f) return -1;

    memset(out, 0, sizeof(fossil_jellydsl_signature));

    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char *key = strtok(line, ":");
        char *val = strtok(NULL, "\n");

        if (!key || !val) continue;
        key = trim(key);
        val = trim(val);
        strip_quotes(val);

        if (strcmp(key, "signed_by") == 0) {
            strncpy(out->signed_by, val, sizeof(out->signed_by));
        } else if (strcmp(key, "signature") == 0) {
            strncpy(out->signature, val, sizeof(out->signature));
        } else if (strcmp(key, "hash") == 0) {
            strncpy(out->hash, val, sizeof(out->hash));
        } else if (strcmp(key, "key_fingerprint") == 0) {
            strncpy(out->key_fingerprint, val, sizeof(out->key_fingerprint));
        } else if (strcmp(key, "timestamp") == 0) {
            out->timestamp = strtoull(val, NULL, 10);
        }
    }

    fclose(f);
    return 0;
}

int fossil_jellydsl_save_signature(const char *filepath, const fossil_jellydsl_signature *sig) {
    if (!filepath || !sig) return -1;

    FILE *f = fopen(filepath, "w");
    if (!f) return -1;

    fprintf(f, "signed_by: \"%s\"\n", sig->signed_by);
    fprintf(f, "signature: \"%s\"\n", sig->signature);
    fprintf(f, "hash: \"%s\"\n", sig->hash);
    fprintf(f, "key_fingerprint: \"%s\"\n", sig->key_fingerprint);
    fprintf(f, "timestamp: %llu\n", (unsigned long long)sig->timestamp);

    fclose(f);
    return 0;
}

// *****************************************************************************
// Utility Helpers
// *****************************************************************************

bool fossil_jellydsl_path_endswith(const char *path, const char *ext) {
    if (!path || !ext) return false;
    size_t len1 = strlen(path);
    size_t len2 = strlen(ext);
    if (len1 < len2) return false;
    return strcmp(path + len1 - len2, ext) == 0;
}

const char *fossil_jellydsl_guess_type(const char *filepath) {
    if (fossil_jellydsl_path_endswith(filepath, ".jellyfish")) return "model";
    if (fossil_jellydsl_path_endswith(filepath, ".jfchain"))    return "memory_chain";
    if (fossil_jellydsl_path_endswith(filepath, ".jfidea"))     return "imagination";
    if (fossil_jellydsl_path_endswith(filepath, ".jfmeta"))     return "metadata";
    if (fossil_jellydsl_path_endswith(filepath, ".jfsig"))      return "signature";
    return "unknown";
}
