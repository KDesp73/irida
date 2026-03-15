# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).


## [0.4.0] - 2026-03-15 

### Added

- CLI
- Documentation with tinydocs
- Improved pesto evaluation
- NNUE Evaluation
- Tests
- Tinker executable
- Training/Tuning python module

### Changed

- Vendoring Dependencies


## [0.3.0] - 2025-07-04 

### Added

- Negamax search in Zig
- PeSTO evaluation in Zig
- Moveordering in Zig
- Documented castro.h
- Engine rewrite in Zig
- Uci protocol working in Zig

### Changed

- bumped version to 0.3.0
- Movegen library now using a single header

### Deprecated

- Old C engine


## [0.2.1] - 2025-06-11 

### Added

- pgntofen.py script and multiple pgn files
- LICENSE for movegen library
- Pawn structure evaluation
- More polyglot books

### Changed

- Bumped version to 0.2.1
- updated all READMEs

### Removed

- Gui code


## [0.2.0] - 2025-06-11 

### Added

- Move ordering functions
- Logfiles
- Reading book moves from polyglot book
- Alpha-Beta Pruning + Iterative Deepening with Negamax

### Changed

- Refactored movegen into its own library
- Slightly improved evaluation
- Bumped version to 0.2.0

### Removed

- Lua based config

### Fixed

- Heatmaps
- UCI communication issues
- Polyglot zobrist hashing


## [0.1.1] - 2025-02-11 

### Added

- Material evaluation function
- Minimax search function
- Using lua for uci configuration
- Added more evaluation parameters
- Loading tuning from tuning.lua
- Incorporated tuning into the evaluation
- More uci options
- Polyglot reader
- Added `book.bin`
- Printing uci info logs while searching
- Calculating zobrist hash the polyglot way

### Changed

- Prefixed uci methods with `uci_`
- Improved King moves castling logic using bitboards
- MovesCombine from 0.000022 to 0.000004 seconds
- Adding sanitizers on DEBUG build
- Included io libraries inside the project for stability

### Fixed

- Solved all warnings


## [0.1.0] - 2025-02-06 

### Added

- Added bitboard representation
- Introduced Color type
- Added history
- Do/Undo move mechanism
- Started UCI protocol
- Added gui
- Correct move generation

### Changed

- Pre-computing piece masks
- Using hybrid board representation

### Fixed

- Fixed movegen bugs (passing all perft tests)


[0.1.0]: https://github.com/KDesp73/chess-engine//releases/tag/v0.1.0
[0.1.1]: https://github.com/KDesp73/chess-engine//releases/tag/v0.1.1
[0.2.0]: https://github.com/KDesp73/chess-engine//releases/tag/v0.2.0
[0.2.1]: https://github.com/KDesp73/chess-engine//releases/tag/v0.2.1
[0.3.0]: https://github.com/KDesp73/chess-engine//releases/tag/v0.3.0
[0.4.0]: https://github.com/KDesp73/chess-engine//releases/tag/v0.4.0

