# Jellyfish by Fossil Logic

**Jellyfish** is a flexible and high-performance AI library designed for building neural networks and machine learning models. Developed by **Fossil Logic**, Jellyfish provides modular, cross-platform support with a focus on neural network construction, training, and optimization. The library is written in C to ensure speed and efficiency, making it suitable for both desktop and embedded systems.

## Key Features

- **Neural Network Support**: Essential functions for constructing and training neural networks, including layers, activation functions, and optimizers.
- **Cross-Platform Compatibility**: Seamless operation across Windows, macOS, and Linux, ensuring consistent behavior across different platforms.
- **Modular Architecture**: Easy integration of various components, including custom optimizers, loss functions, and activation functions.
- **Efficient Build System**: Utilizes Meson for fast and straightforward builds, with support for easy project configuration.
- **High Performance**: Written in C for optimal speed and resource efficiency, ideal for performance-critical applications.
- **Comprehensive Documentation**: Detailed documentation and examples to facilitate quick integration and customization.

## Prerequisites

Before using Jellyfish, ensure you have the following installed:

- **Meson Build System**: Jellyfish uses Meson for the build process. Install or upgrade it using the following commands:

```bash
python -m pip install meson           # To install Meson
python -m pip install --upgrade meson # To upgrade Meson
```

## Adding Jellyfish as a Dependency

To integrate Jellyfish into your project, follow these steps:

1. **Install Meson**: Ensure Meson 1.3 or newer is installed.

2. **Add Wrap File**: Create a `.wrap` file (e.g., `fossil-jellyfish.wrap`) in the `subprojects` directory with the following content:

    ```ini
    # ======================
    # Git Wrap package definition
    # ======================
    [wrap-git]
    url = https://github.com/fossillogic/fossil-jellyfish.git
    revision = v0.1.0

    [provide]
    fossil-jellyfish = fossil_jellyfish_dep
    ```

3. **Integrate Dependency**: Add the dependency to your `meson.build` file:

    ```meson
    dep = dependency('fossil-jellyfish')
    ```

## Build and Configuration Options

You can configure the build options to enable tests and other features:

- **Enable Testing**: Run tests by configuring the project with the following option:

    ```bash
    meson setup builddir -Dwith_test=enabled
    ```

## Contributing and Support

Contributions, feedback, and support are always welcome. If you encounter issues or wish to contribute, please open an issue on the project repository. For more information, refer to the Fossil Logic documentation.
