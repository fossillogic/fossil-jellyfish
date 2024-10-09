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
#include <fossil/unittest/framework.h>
#include <fossil/unittest/assume.h>

#include "fossil/jellyfish/framework.h"

#define TEST_FILE "test_network.fish"
#define NUM_LAYERS 2
#define NUM_NEURONS_LAYER1 3
#define NUM_NEURONS_LAYER2 2

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test
// * * * * * * * * * * * * * * * * * * * * * * * *

// Test case for creating a neural network
FOSSIL_TEST(test_create_network) {
    fossil_jellyfish_network_t* network = (fossil_jellyfish_network_t*)malloc(sizeof(fossil_jellyfish_network_t));
    network->num_layers = NUM_LAYERS;
    network->layers = (fossil_jellyfish_layer_t**)malloc(NUM_LAYERS * sizeof(fossil_jellyfish_layer_t*));

    // Create Layer 1
    network->layers[0] = (fossil_jellyfish_layer_t*)malloc(sizeof(fossil_jellyfish_layer_t));
    network->layers[0]->num_neurons = NUM_NEURONS_LAYER1;
    network->layers[0]->weights = (double*)malloc(NUM_NEURONS_LAYER1 * NUM_NEURONS_LAYER2 * sizeof(double));
    network->layers[0]->biases = (double*)malloc(NUM_NEURONS_LAYER1 * sizeof(double));
    network->layers[0]->deltas = (double*)malloc(NUM_NEURONS_LAYER1 * sizeof(double));
    network->layers[0]->activation = ACTIVATION_RELU;

    // Create Layer 2
    network->layers[1] = (fossil_jellyfish_layer_t*)malloc(sizeof(fossil_jellyfish_layer_t));
    network->layers[1]->num_neurons = NUM_NEURONS_LAYER2;
    network->layers[1]->weights = (double*)malloc(NUM_NEURONS_LAYER2 * NUM_NEURONS_LAYER1 * sizeof(double));
    network->layers[1]->biases = (double*)malloc(NUM_NEURONS_LAYER2 * sizeof(double));
    network->layers[1]->deltas = (double*)malloc(NUM_NEURONS_LAYER2 * sizeof(double));
    network->layers[1]->activation = ACTIVATION_SIGMOID;

    ASSUME_NOT_CNULL(network);
    ASSUME_NOT_CNULL(network->layers[0]);
    ASSUME_NOT_CNULL(network->layers[1]);

    fossil_jellyfish_free_network(network);
}

// Test case for saving a neural network
FOSSIL_TEST(test_save_network) {
    fossil_jellyfish_network_t* network = (fossil_jellyfish_network_t*)malloc(sizeof(fossil_jellyfish_network_t));
    network->num_layers = NUM_LAYERS;
    network->layers = (fossil_jellyfish_layer_t**)malloc(NUM_LAYERS * sizeof(fossil_jellyfish_layer_t*));

    // Layer 1
    network->layers[0] = (fossil_jellyfish_layer_t*)malloc(sizeof(fossil_jellyfish_layer_t));
    network->layers[0]->num_neurons = NUM_NEURONS_LAYER1;
    network->layers[0]->weights = (double*)malloc(NUM_NEURONS_LAYER1 * NUM_NEURONS_LAYER2 * sizeof(double));
    network->layers[0]->biases = (double*)malloc(NUM_NEURONS_LAYER1 * sizeof(double));
    network->layers[0]->deltas = (double*)malloc(NUM_NEURONS_LAYER1 * sizeof(double));
    network->layers[0]->activation = ACTIVATION_RELU;

    // Layer 2
    network->layers[1] = (fossil_jellyfish_layer_t*)malloc(sizeof(fossil_jellyfish_layer_t));
    network->layers[1]->num_neurons = NUM_NEURONS_LAYER2;
    network->layers[1]->weights = (double*)malloc(NUM_NEURONS_LAYER2 * NUM_NEURONS_LAYER1 * sizeof(double));
    network->layers[1]->biases = (double*)malloc(NUM_NEURONS_LAYER2 * sizeof(double));
    network->layers[1]->deltas = (double*)malloc(NUM_NEURONS_LAYER2 * sizeof(double));
    network->layers[1]->activation = ACTIVATION_SIGMOID;

    // Initialize random weights and biases for testing
    for (int i = 0; i < NUM_NEURONS_LAYER1; i++) {
        network->layers[0]->biases[i] = (double)(rand() % 10) / 10.0; // Example initialization
        for (int j = 0; j < NUM_NEURONS_LAYER2; j++) {
            network->layers[0]->weights[i * NUM_NEURONS_LAYER2 + j] = (double)(rand() % 10) / 10.0;
        }
    }
    for (int i = 0; i < NUM_NEURONS_LAYER2; i++) {
        network->layers[1]->biases[i] = (double)(rand() % 10) / 10.0;
        for (int j = 0; j < NUM_NEURONS_LAYER1; j++) {
            network->layers[1]->weights[i * NUM_NEURONS_LAYER1 + j] = (double)(rand() % 10) / 10.0;
        }
    }

    // Save network to file
    int result = fossil_jellyfish_save(network, TEST_FILE);
    ASSUME_ITS_EQUAL_I32(0, result); // Ensure save succeeded

    // Clean up
    fossil_jellyfish_free_network(network);
}

// Test case for loading a neural network
FOSSIL_TEST(test_load_network) {
    fossil_jellyfish_network_t* network = fossil_jellyfish_load(TEST_FILE);
    ASSUME_NOT_CNULL(network);
    ASSUME_ITS_EQUAL_I32(NUM_LAYERS, network->num_layers);
    ASSUME_NOT_CNULL(network->layers[0]);
    ASSUME_NOT_CNULL(network->layers[1]);
    
    // Check layer 1 properties
    ASSUME_ITS_EQUAL_I32(NUM_NEURONS_LAYER1, network->layers[0]->num_neurons);
    ASSUME_ITS_EQUAL_I32(ACTIVATION_RELU, network->layers[0]->activation);

    // Check layer 2 properties
    ASSUME_ITS_EQUAL_I32(NUM_NEURONS_LAYER2, network->layers[1]->num_neurons);
    ASSUME_ITS_EQUAL_I32(ACTIVATION_SIGMOID, network->layers[1]->activation);

    // Clean up
    fossil_jellyfish_free_network(network);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_GROUP(jellyfish_tests) {
    ADD_TEST(test_create_network);
    ADD_TEST(test_save_network);
    ADD_TEST(test_load_network);
}
