#include "search.h"
#include "castro.h"
#include "tt.h"
#include <stdint.h>

Move search_root(Board* board,
                 EvalFn eval,
                 OrderFn order,
                 SearchConfig* config)
{
    Move bestMove = {0};
    int bestScore = -INF;

    g_searchStats.nodes = 0;
    g_searchStats.qnodes = 0;
    g_searchStats.selDepth = 0;

    search_start_timer(config->timeLimitMs);

    for (int depth = 1; depth <= config->maxDepth; depth++) {

        if (search_time_up())
            break;

        int alpha = -INF;
        int beta  = INF;

        Moves moves = castro_GenerateMoves(board, MOVE_LEGAL);
        order(board, moves.list, moves.count, 0);

        int localBestScore = -INF;
        Move localBestMove = {0};

        for (size_t i = 0; i < moves.count; i++) {

            if (search_time_up())
                break;

            Move move = moves.list[i];

            if (!castro_MakeMove(board, move))
                continue;

            int score = -search(board,
                                depth - 1,
                                -beta,
                                -alpha,
                                1,
                                eval,
                                order);

            castro_UnmakeMove(board);

            if (search_time_up())
                break;

            if (score > localBestScore) {
                localBestScore = score;
                localBestMove = move;
            }

            if (score > alpha)
                alpha = score;
        }

        if (!search_time_up()) {
            bestMove = localBestMove;
            bestScore = localBestScore;
        }

    uint64_t timeMs = search_elapsed_ms();
    uint64_t nodes  = g_searchStats.nodes;
    uint64_t nps    = (timeMs > 0) ? (nodes * 1000ULL / timeMs) : 0;

    char move[12];
    castro_MoveToString(bestMove, move);

    printf("info depth %d seldepth %d score cp %d nodes %llu nps %llu time %llu pv %s\n",
           depth,
           g_searchStats.selDepth,
           bestScore,
           (unsigned long long)nodes,
           (unsigned long long)nps,
           (unsigned long long)timeMs,
           move
        );
    }

    return bestMove;
}


int search(Board* board,
           int depth,
           int alpha,
           int beta,
           int ply,
           EvalFn eval,
           OrderFn order)
{
    if ((g_searchStats.nodes++ & 2047) == 0) {
        if (search_time_up())
            return 0;
    }

    if (ply > g_searchStats.selDepth)
        g_searchStats.selDepth = ply;

    int alphaOrig = alpha;

    Move ttMove = {0};
    int ttScore;

    uint64_t zobrist = castro_CalculateZobristHash(board);
    if (tt_probe(zobrist,
                 depth,
                 alpha,
                 beta,
                 &ttScore,
                 &ttMove))
    {
        g_searchStats.ttHits++;
        return ttScore;
    }

    if (depth <= 0)
        return quiescence(board, alpha, beta, ply, eval, order);

    Moves moves = castro_GenerateMoves(board, MOVE_LEGAL);

    if (moves.count == 0) {
        // Checkmate or stalemate
        if (castro_IsInCheck(board))
            return -MATE_SCORE + ply;
        else
            return 0;
    }

    order(board, moves.list, moves.count, ply);

    int bestScore = -INF;
    Move bestMove = {0};

    for (size_t i = 0; i < moves.count; i++) {

        Move move = moves.list[i];

        if (!castro_MakeMove(board, move))
            continue;

        int score = -search(board,
                            depth - 1,
                            -beta,
                            -alpha,
                            ply + 1,
                            eval,
                            order);

        castro_UnmakeMove(board);

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }

        if (score > alpha)
            alpha = score;

        if (alpha >= beta)
            break;  // beta cutoff
    }

    TTNodeType type;

    if (bestScore <= alphaOrig)
        type = TT_UPPERBOUND;
    else if (bestScore >= beta)
        type = TT_LOWERBOUND;
    else
        type = TT_EXACT;

    tt_store(zobrist,
             depth,
             bestScore,
             type,
             bestMove);

    return bestScore;
}

