# Chess Engine Project

## Project Structure

```

chess-engine/
├── engine/           # Core engine logic, UCI interface, evaluation, and gameplay
├── movegen/          # Modular and tested move generation library
├── deps/             # External dependencies (Lua, Raylib)
├── build/            # Build artifacts (created automatically)
├── Makefile          # Central Makefile managing the modules
├── common.mk         # Shared Makefile configuration
└── README.md         # This file

````

## Quick Start

### Prerequisites

- `gcc` with support for `-fsanitize`
- `make`
- `bear` (optional, for generating `compile_commands.json`)
- Linux (tested), but should be portable with adjustments

### Building the Project

To build everything (engine, libraries, and tests):

```bash
make all -j3
````

### Build Types

You can choose between `DEBUG` and `RELEASE` build types:

```bash
make -C movegen type=RELEASE
make -C engine type=RELEASE
```

Defaults to `DEBUG` if unspecified.

### Running Tests

To run tests for each module:

```bash
make test_*
```
> [!NOTE]
> Data-driven testing handled by
> [IncludeOnly/test.h](https://github.com/KDesp73/IncludeOnly/blob/main/libs/test.h).

## Features

* **Correct Move Generation** — handles all legal chess moves
* **Modular Architecture** — `movegen` is a standalone library
* **Testable Components** — every module is independently testable
* **Evaluation** — early-stage evaluation with potential for learning-based scoring
* **Static/Shared Libraries** — both are supported for reuse and embedding

---

> ✨ This engine is a work-in-progress and being developed with learning, experimentation, and future research in mind.

