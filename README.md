# ***Jellyfish AI Library by Fossil Logic***

Jellyfish is a cutting-edge, platform-independent library offering a robust suite of data structures, algorithms, and utilities designed to power AI-driven applications. Written in C and C++, Jellyfish is optimized for high performance and reliability across diverse operating systems. The library leverages the Meson build system for a seamless development experience.

## Key Features

1. **Cross-Platform Compatibility**: Ensures consistent performance across major operating systems, including Windows, macOS, and Linux.
2. **AI-Optimized Algorithms**: Provides specialized algorithms tailored for AI workloads, such as data transformation, filtering, and searching.
3. **Modular and Extensible**: Offers modular components that can be easily customized to meet specific AI project requirements.
4. **High Performance**: Implemented in C and C++ to maximize computational efficiency, suitable for both desktop and embedded AI applications.
5. **Comprehensive Documentation**: Includes detailed documentation and examples to help developers integrate Jellyfish quickly and effectively.

## Core Capabilities

Jellyfish introduces a versatile set of tools and utilities to accelerate AI development:

- **Data Transformation**: Functions for transforming and accumulating data in arrays, ideal for preprocessing AI datasets.
- **Filtering and Searching**: Advanced capabilities for filtering elements and searching with custom predicates.
- **Array Operations**: Utilities for reversing, swapping, and shuffling array elements, enabling efficient data manipulation.
- **Iterator Support**: Provides iterator functionality for seamless traversal of data structures.
- **Memory Management**: Custom allocation, reallocation, and free functions to optimize resource usage.
- **String Utilities**: Includes functions like `jf_strdup` for efficient string manipulation.

## Prerequisites

To get started with Jellyfish, ensure you have the following installed:

- **Meson Build System**: If you don’t have Meson installed, follow the installation instructions on the official [Meson website](https://mesonbuild.com/Getting-meson.html).

### Adding Jellyfish Dependency

#### Adding Jellyfish Dependency With Meson

1. **Install Meson Build System**:
   Install Meson version `1.3` or newer:
   ```sh
   python -m pip install meson           # To install Meson
   python -m pip install --upgrade meson # To upgrade Meson
   ```

2. **Create a `.wrap` File**:
   Add the `fossil-jellyfish.wrap` file in your `subprojects` directory and include the following content:

   ```ini
   # ======================
   # Git Wrap package definition
   # ======================
   [wrap-git]
   url = https://github.com/fossillogic/fossil-jellyfish.git
   revision = v0.1.0

   [provide]
   fossil-jellyfish = fossil_fish_dep
   ```

3. **Integrate the Dependency**:
   In your `meson.build` file, integrate Jellyfish by adding the following line:
   ```ini
   dep = dependency('fossil-jellyfish')
   ```

## Configure Options

Jellyfish offers configurable options to tailor the build process to your needs:

- **Running Tests**: To enable testing, configure the build with `-Dwith_test=enabled`.

Example:

```sh
meson setup builddir -Dwith_test=enabled
```

## Contributing and Support

For those interested in contributing, reporting issues, or seeking support, please open an issue on the project repository or visit the [Fossil Logic Docs](https://fossillogic.com/docs) for more information. Your feedback and contributions are always welcome.
