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
#include "fossil/jellyfish/jellyfish.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

// Utility functions for activations
double fossil_jellyfish_activate(double value, fossil_jellyfish_activation_t activation) {
    switch (activation) {
        case ACTIVATION_RELU:
            return value > 0 ? value : 0;
        case ACTIVATION_SIGMOID:
            return 1.0 / (1.0 + exp(-value));
        case ACTIVATION_TANH:
            return tanh(value);
        default:
            return value;
    }
}

double fossil_jellyfish_activate_derivative(double value, fossil_jellyfish_activation_t activation) {
    switch (activation) {
        case ACTIVATION_RELU:
            return value > 0 ? 1 : 0;
        case ACTIVATION_SIGMOID:
            return value * (1 - value);  // Sigmoid derivative
        case ACTIVATION_TANH:
            return 1 - value * value;  // Tanh derivative
        default:
            return value;
    }
}

// Creates a new neural network
fossil_jellyfish_network_t* fossil_jellyfish_create_network(int32_t num_layers, int32_t* neurons_per_layer, fossil_jellyfish_activation_t* activations) {
    fossil_jellyfish_network_t* network = (fossil_jellyfish_network_t*)malloc(sizeof(fossil_jellyfish_network_t));
    network->num_layers = num_layers;
    network->layers = (fossil_jellyfish_layer_t**)malloc(num_layers * sizeof(fossil_jellyfish_layer_t*));

    for (size_t i = 0; i < num_layers; i++) {
        fossil_jellyfish_layer_t* layer = (fossil_jellyfish_layer_t*)malloc(sizeof(fossil_jellyfish_layer_t));
        layer->num_neurons = neurons_per_layer[i];
        layer->activation = activations[i];
        if (i > 0) {  // Skip the input layer
            layer->weights = (double*)malloc(neurons_per_layer[i] * neurons_per_layer[i-1] * sizeof(double));
            layer->biases = (double*)malloc(neurons_per_layer[i] * sizeof(double));
            layer->deltas = (double*)calloc(neurons_per_layer[i], sizeof(double));
        } else {
            layer->weights = NULL;
            layer->biases = NULL;
            layer->deltas = NULL;
        }
        layer->outputs = (double*)calloc(neurons_per_layer[i], sizeof(double));

        network->layers[i] = layer;
    }

    return network;
}

// Frees up memory allocated for the network
void fossil_jellyfish_free_network(fossil_jellyfish_network_t* network) {
    for (int i = 0; i < network->num_layers; i++) {
        fossil_jellyfish_layer_t* layer = network->layers[i];
        free(layer->biases);
        free(layer->weights);
        free(layer->deltas);
        free(layer);
    }
    free(network->layers);
    free(network);
}

// Forward pass through the network
void fossil_jellyfish_forward(fossil_jellyfish_network_t* network, double* input) {
    // Load input into the first layer
    memcpy(network->layers[0]->outputs, input, network->layers[0]->num_neurons * sizeof(double));

    for (size_t i = 1; i < network->num_layers; i++) {
        fossil_jellyfish_layer_t* prev_layer = network->layers[i - 1];
        fossil_jellyfish_layer_t* layer = network->layers[i];

        for (size_t j = 0; j < layer->num_neurons; j++) {
            double weighted_sum = 0;
            for (size_t k = 0; k < prev_layer->num_neurons; k++) {
                weighted_sum += prev_layer->outputs[k] * layer->weights[j * prev_layer->num_neurons + k];
            }
            weighted_sum += layer->biases[j];
            layer->outputs[j] = fossil_jellyfish_activate(weighted_sum, layer->activation);
        }
    }
}

// Backpropagation algorithm to adjust weights and biases
void fossil_jellyfish_backpropagate(fossil_jellyfish_network_t* network, double* expected_output, double learning_rate) {
    fossil_jellyfish_layer_t* output_layer = network->layers[network->num_layers - 1];

    // Calculate deltas for the output layer
    for (size_t i = 0; i < output_layer->num_neurons; i++) {
        double error = expected_output[i] - output_layer->outputs[i];
        output_layer->deltas[i] = error * fossil_jellyfish_activate_derivative(output_layer->outputs[i], output_layer->activation);
    }

    // Propagate the error backward
    for (size_t i = network->num_layers - 2; i >= 0; i--) {
        fossil_jellyfish_layer_t* layer = network->layers[i];
        fossil_jellyfish_layer_t* next_layer = network->layers[i + 1];

        for (size_t j = 0; j < layer->num_neurons; j++) {
            double error = 0;
            for (size_t k = 0; k < next_layer->num_neurons; k++) {
                error += next_layer->weights[k * layer->num_neurons + j] * next_layer->deltas[k];
            }
            layer->deltas[j] = error * fossil_jellyfish_activate_derivative(layer->outputs[j], layer->activation);
        }

        // Update weights and biases
        for (size_t j = 0; j < next_layer->num_neurons; j++) {
            for (size_t k = 0; k < layer->num_neurons; k++) {
                next_layer->weights[j * layer->num_neurons + k] += learning_rate * next_layer->deltas[j] * layer->outputs[k];
            }
            next_layer->biases[j] += learning_rate * next_layer->deltas[j];
        }
    }
}

// Train the network with gradient descent
void fossil_jellyfish_train(fossil_jellyfish_network_t* network, double* inputs, double* expected_output, int32_t num_samples, int32_t num_epochs, double learning_rate) {
    for (size_t epoch = 0; epoch < num_epochs; epoch++) {
        for (size_t i = 0; i < num_samples; i++) {
            fossil_jellyfish_forward(network, &inputs[i * network->layers[0]->num_neurons]);
            fossil_jellyfish_backpropagate(network, &expected_output[i * network->layers[network->num_layers - 1]->num_neurons], learning_rate);
        }
    }
}

int32_t fossil_jellyfish_save(fossil_jellyfish_network_t* network, const char* file_path) {
    FILE *file = fopen(file_path, "wb");
    if (!file) {
        return -1;  // Error opening the file
    }

    // Write the number of layers
    fwrite(&network->num_layers, sizeof(int32_t), 1, file);

    // Write each layer's data
    for (int i = 0; i < network->num_layers; i++) {
        fossil_jellyfish_layer_t* layer = network->layers[i];

        // Write number of neurons
        fwrite(&layer->num_neurons, sizeof(int32_t), 1, file);

        // Write activation function
        fwrite(&layer->activation, sizeof(fossil_jellyfish_activation_t), 1, file);

        // Write biases (size is num_neurons)
        fwrite(layer->biases, sizeof(double), layer->num_neurons, file);

        // Write weights (size is num_neurons * number of neurons in previous layer)
        int32_t prev_layer_neurons = (i == 0) ? 0 : network->layers[i - 1]->num_neurons;
        fwrite(layer->weights, sizeof(double), layer->num_neurons * prev_layer_neurons, file);

        // Optionally save deltas (used in backpropagation)
        fwrite(layer->deltas, sizeof(double), layer->num_neurons, file);
    }

    fclose(file);
    return 0;  // Success
}

fossil_jellyfish_network_t* fossil_jellyfish_load(const char* file_path) {
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        return NULL;  // Error opening the file
    }

    // Read the number of layers
    int32_t num_layers;
    fread(&num_layers, sizeof(int32_t), 1, file);

    // Allocate memory for the network
    fossil_jellyfish_network_t* network = (fossil_jellyfish_network_t*)malloc(sizeof(fossil_jellyfish_network_t));
    network->num_layers = num_layers;
    network->layers = (fossil_jellyfish_layer_t**)malloc(num_layers * sizeof(fossil_jellyfish_layer_t*));

    // Read each layer's data
    for (int i = 0; i < num_layers; i++) {
        // Allocate memory for the layer
        fossil_jellyfish_layer_t* layer = (fossil_jellyfish_layer_t*)malloc(sizeof(fossil_jellyfish_layer_t));

        // Read number of neurons
        fread(&layer->num_neurons, sizeof(int32_t), 1, file);

        // Read activation function
        fread(&layer->activation, sizeof(fossil_jellyfish_activation_t), 1, file);

        // Allocate and read biases
        layer->biases = (double*)malloc(layer->num_neurons * sizeof(double));
        fread(layer->biases, sizeof(double), layer->num_neurons, file);

        // Allocate and read weights (size is num_neurons * number of neurons in previous layer)
        int32_t prev_layer_neurons = (i == 0) ? 0 : network->layers[i - 1]->num_neurons;
        layer->weights = (double*)malloc(layer->num_neurons * prev_layer_neurons * sizeof(double));
        fread(layer->weights, sizeof(double), layer->num_neurons * prev_layer_neurons, file);

        // Allocate and read deltas (if used)
        layer->deltas = (double*)malloc(layer->num_neurons * sizeof(double));
        fread(layer->deltas, sizeof(double), layer->num_neurons, file);

        // Assign the layer to the network
        network->layers[i] = layer;
    }

    fclose(file);
    return network;
}
