# ***Jellyfish AI Library by Fossil Logic***

Jellyfish is the core engine of Truthful Intelligence, a lightweight AI library written in pure C with no external dependencies. Designed for embedded and trust-critical systems, it records every input, output, and decision as a cryptographically hashed block, forming a fully auditable chain of reasoning. With support for adaptive learning, memory-based inference, and automatic pruning, Jellyfish ensures transparent, verifiable AI behavior where every conclusion is traceable to its source.

### Key Features

- **Cross-Platform**  
  Runs reliably on Windows, macOS, Linux, and embedded systems.

- **Zero Dependencies**  
  Written entirely in pure C for maximum portability, auditability, and minimal footprint.

- **Blockchain-Inspired Memory**  
  Every input, output, and decision is stored as a cryptographically hashed block, creating a tamper-resistant, traceable chain of logic.

- **Self-Pruning Engine**  
  Automatically removes obsolete or conflicting data, preserving clarity and consistency over time.

- **Fully Auditable Reasoning**  
  Supports forensic-level inspection of every decision, enabling ethical and transparent AI behavior.

- **Modular and Configurable**  
  Built to be embedded, extended, or customized for Truthful Intelligence applications across any platform.

## Prerequisites

To get started with Jellyfish, ensure you have the following installed:

- **Meson Build System**: If you don’t have Meson installed, follow the installation instructions on the official [Meson website](https://mesonbuild.com/Getting-meson.html).

### Adding Dependency

#### Adding via Meson Git Wrap

To add a git-wrap, place a `.wrap` file in `subprojects` with the Git repo URL and revision, then use `dependency('fossil-jellyfish')` in `meson.build` so Meson can fetch and build it automatically.

#### Adding via Conan GitHub repository

 packages directly from a GitHub repository if it contains a valid `conanfile.py`.

```bash
conan install git+https://github.com/fossillogic/fossil-jellyfish.git#v0.1.4 --name fossil_jellyfish --build=missing
```

#### Integrate the Dependency:

Add the `fossil-jellyfish.wrap` file in your `subprojects` directory and include the following content:

```ini
[wrap-git]
url = https://github.com/fossillogic/fossil-jellyfish.git
revision = v0.1.4

[provide]
dependency_names = fossil-jellyfish
```

**Note**: For the best experience, always use the latest releases. Visit the [releases](https://github.com/fossillogic/fossil-jellyfish/releases) page for the latest versions.

## Configure Options

Jellyfish offers configurable options to tailor the build process to your needs:

- **Running Tests**: To enable testing, configure the build with `-Dwith_test=enabled`.

Example:

```sh
meson setup builddir -Dwith_test=enabled
```

### Tests Double as Samples

The project is designed so that **test cases serve two purposes**:

- ✅ **Unit Tests** – validate the framework’s correctness.  
- 📖 **Usage Samples** – demonstrate how to use these libraries through test cases.  

This approach keeps the codebase compact and avoids redundant “hello world” style examples.  
Instead, the same code that proves correctness also teaches usage.  

This mirrors the **Meson build system** itself, which tests its own functionality by using Meson to test Meson.  
In the same way, Fossil Logic validates itself by demonstrating real-world usage in its own tests via Fossil Test.  

```bash
meson test -C builddir -v
```

Running the test suite gives you both verification and practical examples you can learn from.

## Contributing and Support

For those interested in contributing, reporting issues, or seeking support, please open an issue on the project repository or visit the [Fossil Logic Docs](https://fossillogic.com/docs) for more information. Your feedback and contributions are always welcome.
