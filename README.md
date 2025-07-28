# ***Jellyfish AI Library by Fossil Logic***

Jellyfish is the core engine of Truthful Intelligence, a lightweight AI library written in pure C with no external dependencies. Designed for embedded and trust-critical systems, it records every input, output, and decision as a cryptographically hashed block, forming a fully auditable chain of reasoning. With support for adaptive learning, memory-based inference, and automatic pruning, Jellyfish ensures transparent, verifiable AI behavior where every conclusion is traceable to its source.

## Key Features

- **Cross-Platform**: Runs consistently on Windows, macOS, and Linux.
- **No External Dependencies**: Written in pure C for maximum portability and minimal footprint.
- **Blockchain-Inspired Memory**: Stores each interaction as a cryptographically hashed block, forming a tamper-resistant chain for transparent and verifiable learning.
- **Self-Pruning**: Automatically removes outdated or irrelevant data to maintain efficiency.
- **Traceable Reasoning**: Every decision and memory is auditable, supporting trust-critical applications.
- **Configurable and Modular**: Easily tailored for embedded, desktop, or custom AI projects.

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
   revision = v0.1.1

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
