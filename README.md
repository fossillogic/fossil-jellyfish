# ***Jellyfish AI Library by Fossil Logic***

Jellyfish is a lightweight, portable AI library written in pure C with no external dependencies, purpose-built for embedded environments and trust-critical systems. Its architecture centers around transparent AI principles, using blockchain-inspired techniques to make all learning, memory, and reasoning fully auditable. Every interaction—input, decision, or output—is recorded as a cryptographically hashed block, creating an immutable, traceable chain of logic that preserves the system’s decision lineage. This enables developers to inspect exactly how and why the system reached a conclusion, down to the model, token, and rule used. The engine supports memory-based reasoning, adaptive learning from prior inputs, and automatic pruning of obsolete or conflicting knowledge, ensuring the system evolves without accumulating contradiction or noise. These features make Jellyfish a foundation for AI systems where verifiability, traceability, and long-term ethical transparency are non-negotiable.

## 🧠 `.jellyfish` File Example

This file defines a **Jellyfish mindset**, mapping named personalities to model chains (`.fish` files):

```meson
# This is the primary logic bundle for TAI personality

mindset('core_logic') {
  description: 'Fundamental AI building blocks'
  priority: 1
  models: [
    'logic.fish',
    'nlp.fish',
    'ethics.fish'
  ]
  tags: ['core']
  confidence_threshold: 0.5

  #:bootstrap
  #:taint-free
}

# Another mindset with conditional loading
mindset('persona_trump') {
  description: 'Simulates Donald Trump speech pattern'
  models: ['trump_speech.fish']
  activation_condition: 'input contains "Trump"'
  priority: 10
  #:persona
}
```

* `mindset(name)` declares a named personality or capability.
* `models` list refers to one or more `.fish` model files.
* Optional attributes like `tags`, `priority`, and `activation_condition` allow selective and conditional loading.

---

## 🧠 `.fish` File Example

A `.fish` file stores learned associations (called *thought blocks*) in JSON format:

```json
{
  "signature": "JFS1",
  "blocks": [
    {
      "input": "fire",
      "output": "hot",
      "timestamp": 1620000000
    },
    {
      "input": "ice",
      "output": "cold",
      "timestamp": 1620001000
    },
    {
      "input": "wind",
      "output": "fast",
      "timestamp": 1620002000
    }
  ]
}
```

* `signature`: Identifies the file format version (`JFS1`).
* Each `block` represents a learned input → output association, optionally timestamped for ordering or pruning.

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
