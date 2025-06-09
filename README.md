# ⚔️ Chess Engine Project

This is a modular, high-performance chess engine written in **C**. The project is structured into separate components for move generation, game logic, evaluation, and a user interface. It is designed with maintainability, testability, and extensibility in mind.

## 📁 Project Structure

```

chess-engine/
├── engine/           # Core engine logic, UCI interface, evaluation, and gameplay
├── movegen/          # Modular and tested move generation library
├── deps/             # External dependencies (Lua, Raylib)
├── build/            # Build artifacts (created automatically)
├── common.mk         # Shared Makefile configuration
└── README.md         # This file

````

## 🚀 Quick Start

### 🔧 Prerequisites

- `gcc` with support for `-fsanitize`
- `make`
- `bear` (optional, for generating `compile_commands.json`)
- `raylib`
- `lua`
- Linux (tested), but should be portable with adjustments

### 🛠 Building the Project

To build everything (engine, libraries, and tests):

```bash
make -C movegen all
make -C engine all
````

You can also build from the top-level directory:

```bash
make -C movegen
make -C engine
```

### ⚙️ Build Types

You can choose between `DEBUG` and `RELEASE` build types:

```bash
make -C movegen type=RELEASE
make -C engine type=RELEASE
```

Defaults to `DEBUG` if unspecified.

### 🧪 Running Tests

To run unit tests for the move generator:

```bash
make -C movegen check
./build/bin/check
```

## 🧠 Features

* ✅ **Correct Move Generation** — handles all legal chess moves
* ⚙️ **Modular Architecture** — `movegen` is a standalone library
* 🧪 **Testable Components** — every module is independently testable
* 🧠 **Evaluation** — early-stage evaluation with potential for learning-based scoring
* 📦 **Lua Config Support** — integrates Lua for flexible runtime logic
* 🎨 **Raylib GUI** — visual debugging and gameplay (optional)
* 📦 **Static/Shared Libraries** — both are supported for reuse and embedding

## 🧰 Development Tools

Generate a `compile_commands.json` file for editor/IDE integration:

```bash
make -C engine compile_commands.json
make -C movegen compile_commands.json
```

## 🧹 Cleaning Build Artifacts

```bash
make -C movegen clean
make -C engine clean
```

## 🧾 License

This project is open-source and available under the MIT License.

---

> ✨ This engine is a work-in-progress and being developed with learning, experimentation, and future research in mind.

