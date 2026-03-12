#ifndef MOVE_ORDERING_H
#define MOVE_ORDERING_H

#include "castro.h"

// @module moveordering
// @desc Move ordering: OrderFn type, MVV-LVA, killer, history, TT move.

// @type OrderFn
// @desc Function pointer: order moves in place (board, moves, count, ply).
typedef void (*OrderFn)(Board* board, Move moves[], size_t count, size_t ply);

// @function init_mvv_lva
void init_mvv_lva(void);

// @function mvv_lva_score
// @param board Board (for piece types).
// @param m Move to score.
// @returns int MVV-LVA score (higher = try first).
int mvv_lva_score(Board *board, Move m);

// @function order_moves
// @param board Board position.
// @param moves Array of moves to order (modified in place).
// @param count Number of moves.
// @param ply Current ply.
void order_moves(Board *board, Move moves[], size_t count, size_t ply);

// @function set_tt_move
// @desc Set the transposition table move for the next order_moves call (search it first).
// @param move TT best move.
void set_tt_move(Move move);

// @function update_killer
// @desc Killer moves heuristic.
// @param move Move to record as killer.
// @param ply Ply of the move.
void update_killer(Move move, int ply);

// @function update_history
// @desc History heuristic.
// @param move Move to update.
// @param depth Depth of the search that caused the cutoff.
void update_history(Move move, int depth);

#endif // MOVE_ORDERING_H
