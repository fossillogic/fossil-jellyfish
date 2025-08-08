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

// Utility: Trims whitespace in-place
static void trim(char *str) {
    char *end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
}

// Utility: Reads entire file into buffer
static char *read_file(const char *filepath) {
    FILE *fp = fopen(filepath, "rb");
    if (!fp) return NULL;
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);
    char *data = malloc(size + 1);
    if (!data) { fclose(fp); return NULL; }
    fread(data, 1, size, fp);
    data[size] = 0;
    fclose(fp);
    return data;
}

// Utility: Writes a buffer to file
static int write_file(const char *filepath, const char *data) {
    FILE *fp = fopen(filepath, "wb");
    if (!fp) return -1;
    fwrite(data, 1, strlen(data), fp);
    fclose(fp);
    return 0;
}

// *****************************************************************************
// JellyDSL Parsers
// *****************************************************************************

int fossil_jellydsl_load_model(const char *filepath, fossil_jellyfish_jellydsl *out) {
    if (!filepath || !out) return -1;
    char *src = read_file(filepath);
    if (!src) return -2;

    memset(out, 0, sizeof(*out));

    char *line = strtok(src, "\n");
    while (line) {
        trim(line);
        if (strncmp(line, "name:", 5) == 0) {
            strncpy(out->name, line + 5, sizeof(out->name)-1);
        } else if (strncmp(line, "description:", 12) == 0) {
            strncpy(out->description, line + 12, sizeof(out->description)-1);
        } else if (strncmp(line, "priority:", 9) == 0) {
            out->priority = atoi(line + 9);
        } else if (strncmp(line, "confidence_threshold:", 21) == 0) {
            out->confidence_threshold = atof(line + 21);
        } else if (strncmp(line, "tags:", 5) == 0) {
            char *tag = strtok(line + 5, ",");
            while (tag && out->tag_count < FOSSIL_JELLYFISH_MAX_TAGS) {
                trim(tag);
                strncpy(out->tags[out->tag_count++], tag, 31);
                tag = strtok(NULL, ",");
            }
        } else if (strncmp(line, "models:", 7) == 0) {
            char *model = strtok(line + 7, ",");
            while (model && out->model_count < FOSSIL_JELLYFISH_MAX_MODELS) {
                trim(model);
                strncpy(out->models[out->model_count++], model, 31);
                model = strtok(NULL, ",");
            }
        } else if (strncmp(line, "activation_condition:", 21) == 0) {
            strncpy(out->activation_condition, line + 21, sizeof(out->activation_condition)-1);
        } else if (strncmp(line, "source_uri:", 11) == 0) {
            strncpy(out->source_uri, line + 11, sizeof(out->source_uri)-1);
        } else if (strncmp(line, "origin_device_id:", 17) == 0) {
            strncpy(out->origin_device_id, line + 17, sizeof(out->origin_device_id)-1);
        } else if (strncmp(line, "version:", 8) == 0) {
            strncpy(out->version, line + 8, sizeof(out->version)-1);
        } else if (strncmp(line, "content_hash:", 13) == 0) {
            strncpy(out->content_hash, line + 13, sizeof(out->content_hash)-1);
        } else if (strncmp(line, "created_at:", 11) == 0) {
            out->created_at = strtoull(line + 11, NULL, 10);
        } else if (strncmp(line, "updated_at:", 11) == 0) {
            out->updated_at = strtoull(line + 11, NULL, 10);
        } else if (strncmp(line, "trust_score:", 12) == 0) {
            out->trust_score = atof(line + 12);
        } else if (strncmp(line, "immutable:", 10) == 0) {
            out->immutable = atoi(line + 10);
        } else if (strncmp(line, "state_machine:", 14) == 0) {
            strncpy(out->state_machine, line + 14, sizeof(out->state_machine)-1);
        }
        line = strtok(NULL, "\n");
    }

    free(src);
    return 0;
}

int fossil_jellydsl_save_model(const char *filepath, const fossil_jellyfish_jellydsl *model) {
    if (!filepath || !model) return -1;

    char buf[8192];
    snprintf(buf, sizeof(buf),
        "name: %s\n"
        "description: %s\n"
        "priority: %d\n"
        "confidence_threshold: %.3f\n"
        "tags: ",
        model->name,
        model->description,
        model->priority,
        model->confidence_threshold
    );

    for (int i = 0; i < model->tag_count; i++) {
        strncat(buf, model->tags[i], sizeof(buf) - strlen(buf) - 1);
        if (i < model->tag_count - 1) strncat(buf, ", ", sizeof(buf) - strlen(buf) - 1);
    }
    strncat(buf, "\nmodels: ", sizeof(buf) - strlen(buf) - 1);
    for (int i = 0; i < model->model_count; i++) {
        strncat(buf, model->models[i], sizeof(buf) - strlen(buf) - 1);
        if (i < model->model_count - 1) strncat(buf, ", ", sizeof(buf) - strlen(buf) - 1);
    }

    char meta[2048];
    snprintf(meta, sizeof(meta),
        "\nactivation_condition: %s\n"
        "source_uri: %s\n"
        "origin_device_id: %s\n"
        "version: %s\n"
        "content_hash: %s\n"
        "created_at: %llu\n"
        "updated_at: %llu\n"
        "trust_score: %.3f\n"
        "immutable: %d\n"
        "state_machine: %s\n",
        model->activation_condition,
        model->source_uri,
        model->origin_device_id,
        model->version,
        model->content_hash,
        model->created_at,
        model->updated_at,
        model->trust_score,
        model->immutable,
        model->state_machine
    );

    strncat(buf, meta, sizeof(buf) - strlen(buf) - 1);
    return write_file(filepath, buf);
}

// ─────────────────────────────────────────────────────────────
// .jfmeta: Save/Load Metadata Subset of JellyDSL
// ─────────────────────────────────────────────────────────────

int fossil_jellydsl_save_meta(const char *filepath, const fossil_jellyfish_jellydsl *meta) {
    if (!filepath || !meta) return -1;

    char buf[2048];
    snprintf(buf, sizeof(buf),
        "name: %s\n"
        "version: %s\n"
        "origin_device_id: %s\n"
        "source_uri: %s\n"
        "content_hash: %s\n"
        "created_at: %llu\n"
        "updated_at: %llu\n"
        "trust_score: %.3f\n"
        "immutable: %d\n",
        meta->name,
        meta->version,
        meta->origin_device_id,
        meta->source_uri,
        meta->content_hash,
        meta->created_at,
        meta->updated_at,
        meta->trust_score,
        meta->immutable
    );

    return write_file(filepath, buf);
}

int fossil_jellydsl_load_meta(const char *filepath, fossil_jellyfish_jellydsl *meta) {
    if (!filepath || !meta) return -1;
    char *src = read_file(filepath);
    if (!src) return -2;

    memset(meta, 0, sizeof(*meta));

    char *line = strtok(src, "\n");
    while (line) {
        trim(line);
        if (strncmp(line, "name:", 5) == 0) {
            strncpy(meta->name, line + 5, sizeof(meta->name)-1);
        } else if (strncmp(line, "version:", 8) == 0) {
            strncpy(meta->version, line + 8, sizeof(meta->version)-1);
        } else if (strncmp(line, "origin_device_id:", 17) == 0) {
            strncpy(meta->origin_device_id, line + 17, sizeof(meta->origin_device_id)-1);
        } else if (strncmp(line, "source_uri:", 11) == 0) {
            strncpy(meta->source_uri, line + 11, sizeof(meta->source_uri)-1);
        } else if (strncmp(line, "content_hash:", 13) == 0) {
            strncpy(meta->content_hash, line + 13, sizeof(meta->content_hash)-1);
        } else if (strncmp(line, "created_at:", 11) == 0) {
            meta->created_at = strtoull(line + 11, NULL, 10);
        } else if (strncmp(line, "updated_at:", 11) == 0) {
            meta->updated_at = strtoull(line + 11, NULL, 10);
        } else if (strncmp(line, "trust_score:", 12) == 0) {
            meta->trust_score = atof(line + 12);
        } else if (strncmp(line, "immutable:", 10) == 0) {
            meta->immutable = atoi(line + 10);
        }
        line = strtok(NULL, "\n");
    }

    free(src);
    return 0;
}

// ─────────────────────────────────────────────────────────────
// .jfidea: Save/Load Freeform Text Ideas
// ─────────────────────────────────────────────────────────────

int fossil_jellydsl_save_idea(const char *filepath, const fossil_jellydsl_idea *idea) {
    if (!filepath || !idea) return -1;
    FILE *f = fopen(filepath, "w");
    if (!f) return errno;
    fprintf(f, "seed=%s\n", idea->seed);
    fprintf(f, "count=%d\n", idea->count);
    // Write generated strings
    for (int i = 0; i < idea->count && i < 8; i++) {
        fprintf(f, "generated[%d]=%s\n", i, idea->generated[i]);
    }
    fclose(f);
    return 0;
}

int fossil_jellydsl_load_idea(const char *filepath, fossil_jellydsl_idea *out) {
    if (!filepath || !out) return -1;
    FILE *f = fopen(filepath, "r");
    if (!f) return errno;
    fscanf(f, "seed=%255[^\n]\n", out->seed);
    fscanf(f, "count=%d\n", &out->count);
    for (int i = 0; i < out->count && i < 8; i++) {
        fscanf(f, "generated[%d]=%511[^\n]\n", &i, out->generated[i]);
    }
    fclose(f);
    return 0;
}

// ─────────────────────────────────────────────────────────────
// .jfchain: Save/Load Jellyfish Chain
// ─────────────────────────────────────────────────────────────
// Requires chain functions to be implemented separately

int fossil_jellydsl_save_chain(const char *filepath, const fossil_jellyfish_chain_t *chain) {
    if (!filepath || !chain) return -1;
    // Assumes chain->to_string() or similar exists
    char *chain_text = fossil_jellyfish_chain_export_text(chain);
    if (!chain_text) return -2;
    int result = write_file(filepath, chain_text);
    free(chain_text);
    return result;
}

int fossil_jellydsl_load_chain(const char *filepath, fossil_jellyfish_chain_t *out_chain) {
    if (!filepath || !out_chain) return -1;
    char *src = read_file(filepath);
    if (!src) return -2;
    int result = fossil_jellyfish_chain_import_text(out_chain, src);
    free(src);
    return result;
}

// ─────────────────────────────────────────────────────────────
// .jfsig: Save/Load Signature (text or binary as base64)
// ─────────────────────────────────────────────────────────────

int fossil_jellydsl_save_signature(const char *filepath, const fossil_jellydsl_signature *sig) {
    if (!filepath || !sig) return -1;
    FILE *f = fopen(filepath, "w");
    if (!f) return errno;
    fprintf(f, "signed_by=%s\n", sig->signed_by);
    fprintf(f, "signature=%s\n", sig->signature);
    fprintf(f, "hash=%s\n", sig->hash);
    fprintf(f, "key_fingerprint=%s\n", sig->key_fingerprint);
    fprintf(f, "timestamp=%llu\n", (unsigned long long)sig->timestamp);
    fclose(f);
    return 0;
}

int fossil_jellydsl_load_signature(const char *filepath, fossil_jellydsl_signature *out) {
    if (!filepath || !out) return -1;
    FILE *f = fopen(filepath, "r");
    if (!f) return errno;
    fscanf(f, "signed_by=%63[^\n]\n", out->signed_by);
    fscanf(f, "signature=%255[^\n]\n", out->signature);
    fscanf(f, "hash=%63[^\n]\n", out->hash);
    fscanf(f, "key_fingerprint=%63[^\n]\n", out->key_fingerprint);
    fscanf(f, "timestamp=%llu\n", (unsigned long long *)&out->timestamp);
    fclose(f);
    return 0;
}

// Helpers

bool fossil_jellydsl_path_endswith(const char *path, const char *ext) {
    if (!path || !ext) return false;
    size_t len_path = strlen(path), len_ext = strlen(ext);
    return len_path >= len_ext && strcmp(path + len_path - len_ext, ext) == 0;
}

const char *fossil_jellydsl_guess_type(const char *filepath) {
    if (fossil_jellydsl_path_endswith(filepath, ".jellyfish")) return "model";
    if (fossil_jellydsl_path_endswith(filepath, ".jfmeta")) return "metadata";
    if (fossil_jellydsl_path_endswith(filepath, ".jfidea")) return "idea";
    if (fossil_jellydsl_path_endswith(filepath, ".jfchain")) return "chain";
    if (fossil_jellydsl_path_endswith(filepath, ".jfsig")) return "signature";
    return "unknown";
}
