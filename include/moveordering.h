#ifndef MOVE_ORDERING_H
#define MOVE_ORDERING_H

#include "castro.h"

// @module moveordering
// @desc Move ordering: OrderFn type, MVV-LVA, killer, history, TT move.

// @type OrderFn
// @desc Function pointer: order moves in place (board, moves, count, ply).
typedef void (*OrderFn)(Board* board, Move moves[], size_t count, size_t ply, Move tt_move);

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
void order_moves(Board *board, Move moves[], size_t count, size_t ply, Move tt_move);

#define HISTORY_MAX 100000
extern int history_heuristic[2][64][64]; 

#endif // MOVE_ORDERING_H
