#ifndef SYZYGY_H
#define SYZYGY_H

#include "castro.h"
#include <stdbool.h>

// @module syzygy
// @desc Syzygy tablebases: init, free, probe WDL/DTZ, piece count.

// @function syzygy_init
// @desc Initialize Syzygy tablebases from path.
// @param path Path to Syzygy .rtbw/.rtbz files.
// @returns bool True on success.
bool syzygy_init(const char* path);

// @function syzygy_free
// @desc Free Syzygy resources.
void syzygy_free(void);

// @function syzygy_available
// @desc Whether tablebases are loaded and usable.
// @returns bool True if loaded.
bool syzygy_available(void);

// @const SYZYGY_PROBE_FAILED
// @desc Return value when probe fails; caller uses normal eval.
#define SYZYGY_PROBE_FAILED (-1000001)

// @function syzygy_probe_wdl
// @desc Probe WDL for current position. Returns score in centipawns (or draw/mate-like) if probe succeeds; otherwise SYZYGY_PROBE_FAILED. Call only when piece count <= limit, depth <= probe_depth, and (if 50-move rule) halfmove == 0 and castling == 0.
// @param board Board position.
// @param use_rule50 Whether to apply 50-move rule for TB.
// @returns int WDL score or SYZYGY_PROBE_FAILED.
int syzygy_probe_wdl(Board* board, bool use_rule50);

// @function syzygy_probe_root
// @desc At root: probe DTZ and fill in best move if position is in TB.
// @param board Board position.
// @param use_rule50 Whether to apply 50-move rule.
// @param best_move_out Output: best move if TB hit.
// @returns bool True if TB move was chosen (best_move_out set).
bool syzygy_probe_root(Board* board, bool use_rule50, Move* best_move_out);

// @function syzygy_piece_count
// @desc Total piece count (both sides).
// @param board Board position.
// @returns unsigned Piece count.
unsigned syzygy_piece_count(Board* board);

#define SYZYGY_DEFAULT_PATH "~/.irida/tb/dtz:~/.irida/tb/wdl"

#endif
