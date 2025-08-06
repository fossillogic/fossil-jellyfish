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

    char line[512];
    memset(out, 0, sizeof(fossil_jellyfish_jellydsl));

    while (fgets(line, sizeof(line), f)) {
        char *colon = strchr(line, ':');
        if (!colon) continue;

        *colon = '\0';
        char *key = trim(line);
        char *value = trim(colon + 1);

        strip_quotes(value);

        if (strcmp(key, "name") == 0) strncpy(out->name, value, sizeof(out->name));
        else if (strcmp(key, "description") == 0) strncpy(out->description, value, sizeof(out->description));
        else if (strcmp(key, "priority") == 0) out->priority = atoi(value);
        else if (strcmp(key, "confidence_threshold") == 0) out->confidence_threshold = atof(value);
        else if (strcmp(key, "activation_condition") == 0) strncpy(out->activation_condition, value, sizeof(out->activation_condition));
        else if (strcmp(key, "source_uri") == 0) strncpy(out->source_uri, value, sizeof(out->source_uri));
        else if (strcmp(key, "origin_device_id") == 0) strncpy(out->origin_device_id, value, sizeof(out->origin_device_id));
        else if (strcmp(key, "version") == 0) strncpy(out->version, value, sizeof(out->version));
        else if (strcmp(key, "content_hash") == 0) strncpy(out->content_hash, value, sizeof(out->content_hash));
        else if (strcmp(key, "created_at") == 0) out->created_at = strtoull(value, NULL, 10);
        else if (strcmp(key, "updated_at") == 0) out->updated_at = strtoull(value, NULL, 10);
        else if (strcmp(key, "trust_score") == 0) out->trust_score = atof(value);
        else if (strcmp(key, "immutable") == 0) out->immutable = atoi(value);
        else if (strcmp(key, "state_machine") == 0) strncpy(out->state_machine, value, sizeof(out->state_machine));

        // Handle list-style values for tags and models
        else if (strcmp(key, "tags") == 0) {
            char *tok = strtok(value, "[,]");
            while (tok && out->tag_count < FOSSIL_JELLYFISH_MAX_TAGS) {
                char *clean = trim(tok);
                strip_quotes(clean);
                strncpy(out->tags[out->tag_count++], clean, 32);
                tok = strtok(NULL, "[,]");
            }
        }
        else if (strcmp(key, "models") == 0) {
            char *tok = strtok(value, "[,]");
            while (tok && out->model_count < FOSSIL_JELLYFISH_MAX_MODELS) {
                char *clean = trim(tok);
                strip_quotes(clean);
                strncpy(out->models[out->model_count++], clean, 32);
                tok = strtok(NULL, "[,]");
            }
        }
    }

    fclose(f);
    return 0;
}

int fossil_jellydsl_save_model(const char *filepath, const fossil_jellyfish_jellydsl *model) {
    if (!filepath || !model) return -1;

    FILE *f = fopen(filepath, "w");
    if (!f) return -1;

    fprintf(f, "name: \"%s\"\n", model->name);
    fprintf(f, "description: \"%s\"\n", model->description);

    fprintf(f, "tags: [");
    for (int i = 0; i < model->tag_count; i++) {
        fprintf(f, "\"%s\"%s", model->tags[i], (i < model->tag_count - 1) ? ", " : "");
    }
    fprintf(f, "]\n");

    fprintf(f, "models: [");
    for (int i = 0; i < model->model_count; i++) {
        fprintf(f, "\"%s\"%s", model->models[i], (i < model->model_count - 1) ? ", " : "");
    }
    fprintf(f, "]\n");

    fprintf(f, "priority: %d\n", model->priority);
    fprintf(f, "confidence_threshold: %.4f\n", model->confidence_threshold);
    fprintf(f, "activation_condition: \"%s\"\n", model->activation_condition);
    fprintf(f, "source_uri: \"%s\"\n", model->source_uri);
    fprintf(f, "origin_device_id: \"%s\"\n", model->origin_device_id);
    fprintf(f, "version: \"%s\"\n", model->version);
    fprintf(f, "content_hash: \"%s\"\n", model->content_hash);
    fprintf(f, "created_at: %llu\n", (unsigned long long)model->created_at);
    fprintf(f, "updated_at: %llu\n", (unsigned long long)model->updated_at);
    fprintf(f, "trust_score: %.4f\n", model->trust_score);
    fprintf(f, "immutable: %d\n", model->immutable);
    fprintf(f, "state_machine: \"%s\"\n", model->state_machine);

    fclose(f);
    return 0;
}

// *****************************************************************************
// .jfchain: Jellyfish Memory Chain
// *****************************************************************************

int fossil_jellydsl_load_chain(const char *filepath, fossil_jellyfish_chain *out) {
    if (!filepath || !out) return -1;

    FILE *f = fopen(filepath, "r");
    if (!f) return -1;

    memset(out, 0, sizeof(fossil_jellyfish_chain));

    char line[1024];
    fossil_jellyfish_block temp = {0};
    int in_block = 0;

    while (fgets(line, sizeof(line), f)) {
        char *colon = strchr(line, ':');
        if (!colon) continue;

        *colon = '\0';
        char *key = trim(line);
        char *val = trim(colon + 1);
        strip_quotes(val);

        if (strcmp(key, "created_at") == 0)
            out->created_at = strtoull(val, NULL, 10);
        else if (strcmp(key, "updated_at") == 0)
            out->updated_at = strtoull(val, NULL, 10);
        else if (strcmp(key, "device_id") == 0)
            memcpy(out->device_id, val, strlen(val) < FOSSIL_DEVICE_ID_SIZE ? strlen(val) : FOSSIL_DEVICE_ID_SIZE);
        else if (strcmp(key, "blocks") == 0)
            in_block = 1;
        else if (in_block && strcmp(key, "input") == 0)
            strncpy(temp.input, val, FOSSIL_JELLYFISH_INPUT_SIZE);
        else if (in_block && strcmp(key, "output") == 0)
            strncpy(temp.output, val, FOSSIL_JELLYFISH_OUTPUT_SIZE);
        else if (in_block && strcmp(key, "timestamp") == 0)
            temp.timestamp = strtoull(val, NULL, 10);
        else if (in_block && strcmp(key, "delta_ms") == 0)
            temp.delta_ms = atoi(val);
        else if (in_block && strcmp(key, "duration_ms") == 0)
            temp.duration_ms = atoi(val);
        else if (in_block && strcmp(key, "valid") == 0)
            temp.valid = atoi(val);
        else if (in_block && strcmp(key, "confidence") == 0)
            temp.confidence = atof(val);
        else if (in_block && strcmp(key, "usage_count") == 0)
            temp.usage_count = atoi(val);
        else if (in_block && strcmp(key, "immutable") == 0)
            temp.immutable = atoi(val);
        else if (in_block && strstr(key, "}")) {
            // End of block
            if (out->count < FOSSIL_JELLYFISH_MAX_MEM)
                out->memory[out->count++] = temp;
            memset(&temp, 0, sizeof(temp));
        }
    }

    fclose(f);
    return 0;
}

int fossil_jellydsl_save_chain(const char *filepath, const fossil_jellyfish_chain *chain) {
    if (!filepath || !chain) return -1;

    FILE *f = fopen(filepath, "w");
    if (!f) return -1;

    fprintf(f, "created_at: %llu\n", (unsigned long long)chain->created_at);
    fprintf(f, "updated_at: %llu\n", (unsigned long long)chain->updated_at);
    fprintf(f, "device_id: \"%.*s\"\n", FOSSIL_DEVICE_ID_SIZE, chain->device_id);

    fprintf(f, "blocks: [\n");
    for (size_t i = 0; i < chain->count; ++i) {
        const fossil_jellyfish_block *b = &chain->memory[i];
        fprintf(f, "  {\n");
        fprintf(f, "    input: \"%s\"\n", b->input);
        fprintf(f, "    output: \"%s\"\n", b->output);
        fprintf(f, "    timestamp: %llu\n", (unsigned long long)b->timestamp);
        fprintf(f, "    delta_ms: %u\n", b->delta_ms);
        fprintf(f, "    duration_ms: %u\n", b->duration_ms);
        fprintf(f, "    valid: %d\n", b->valid);
        fprintf(f, "    confidence: %.4f\n", b->confidence);
        fprintf(f, "    usage_count: %u\n", b->usage_count);
        fprintf(f, "    immutable: %d\n", b->immutable);
        fprintf(f, "  }\n");
    }
    fprintf(f, "]\n");

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
