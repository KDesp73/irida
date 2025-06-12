#include "search.h"
#include "evaluation.h"
#include "move.h"
#include "movegen.h"
#include "moveordering.h"
#include "result.h"
#include "uci.h"
#include "zobrist.h"
#include <limits.h>
#include <stdio.h>

static int nodes = 0;

#define TT_SIZE (1 << 20)
TTEntry tt[TT_SIZE]; 

int Negamax(Board* board, int depth, int alpha, int beta)
{
    nodes++;
    int alphaOrig = alpha;
    uint64_t key = board->hash;

    // Transposition Table Lookup
    TTEntry* entry = &tt[key % TT_SIZE];
    if (entry->key == key && entry->depth >= depth) {
        if (entry->flag == EXACT) return entry->score;
        if (entry->flag == LOWERBOUND && entry->score >= beta) return entry->score;
        if (entry->flag == UPPERBOUND && entry->score <= alpha) return entry->score;
    }

    // Terminal node or depth limit
    if (depth == 0 || IsResult(board))
        return Evaluation(board).total;

    // Null Move Pruning
    const int R = 2;
    if (depth >= 3 && !IsInCheck(board)) {
        MakeNullMove(board);
        int score = -Negamax(board, depth - 1 - R, -beta, -beta + 1);
        UnmakeNullMove(board);

        if (score >= beta)
            return score;
    }

    Moves moves = GenerateMoves(board, MOVE_LEGAL);
    SortMoves(board, &moves);

    int bestScore = INT_MIN;

    for (size_t i = 0; i < moves.count; i++) {
        Move move = moves.list[i];
        MakeMove(board, move);
        int score = -Negamax(board, depth - 1, -beta, -alpha);
        UnmakeMove(board);

        if (score > bestScore) {
            bestScore = score;
            if (score > alpha) {
                alpha = score;
                if (alpha >= beta)
                    break;
            }
        }
    }

    if (bestScore == INT_MIN && moves.count == 0) {
        return IsInCheck(board) ? -MATE_SCORE + (-depth) : 0;
    }

    // Transposition Table Store
    entry->key = key;
    entry->depth = depth;
    entry->score = bestScore;
    entry->flag = (bestScore >= beta)      ? LOWERBOUND :
                  (bestScore <= alphaOrig) ? UPPERBOUND :
                                             EXACT;

    return bestScore;
}

Move FindBest(Board* board, int depth, int* outScore) {
    nodes = 0;
    int alpha = -MATE_SCORE;  // Or -INFINITY
    int beta = MATE_SCORE;    // Or +INFINITY
    int bestScore = -MATE_SCORE;
    Move bestMove = NULL_MOVE;

    Moves moves = GenerateMoves(board, MOVE_LEGAL);
    SortMoves(board, &moves);

    for (size_t i = 0; i < moves.count; i++) {
        Move move = moves.list[i];
        MakeMove(board, move);
        int score = -Negamax(board, depth - 1, -beta, -alpha);
        UnmakeMove(board);

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
            alpha = score;  // Narrow the window

            // UCI info
            char mStr[16];
            MoveToString(bestMove, mStr);
            LogPrintf("info depth %d score cp %d nodes %d pv %s\n",
                   depth, bestScore, nodes, mStr);
        }

        if (alpha >= beta)
            break;  // Beta cutoff
    }

    *outScore = bestScore;
    return bestMove;
}

Move FindBestIterative(Board* board, int maxDepth, int* outScore) {
    Move bestMove = NULL_MOVE;
    int bestScore = -MATE_SCORE;

    for (int depth = 1; depth <= maxDepth; depth++) {
        int score;

        Move mv = FindBest(board, depth, &score);

        if (mv != NULL_MOVE) {
            bestMove = mv;
            bestScore = score;
        }

        // TODO: Time control check
        // if (TimeElapsed() > MAX_TIME_MS) break;
    }

    *outScore = bestScore;
    return bestMove;
}
