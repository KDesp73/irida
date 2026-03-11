#include "search.h"
#include "castro.h"
#include "tt.h"
#include "moveordering.h"
#include "uci.h"
#include "draws.h"
#include "syzygy.h"
#include "eval_cache.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>

static bool search_should_stop(void)
{
    return search_time_up() || uci_state.stopRequested;
}

/* Cap iterative deepening so we don't run hundreds of trivial iterations (e.g. draw/TB positions). */
#define MAX_ID_DEPTH 64

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
    g_searchStats.rootChildTtHits = 0;

    tt_clear();
    tt_inc_generation();  /* only use TT entries from this search */
    eval_cache_clear();
    search_start_timer(config->timeLimitMs);

    const int aspirationWindow = 50;
    int prevScore = 0;
    bool fullWindowRetry = false;
    int maxDepth = config->maxDepth;
    if (maxDepth > MAX_ID_DEPTH)
        maxDepth = MAX_ID_DEPTH;

    uint64_t prevNodes = 0;

    for (int depth = 1; depth <= maxDepth; depth++) {

        if (search_should_stop())
            break;

        int alpha = -INF;
        int beta  = INF;
        bool usedNarrowWindow = false;

        /* After an aspiration fail we retry this depth with full window (don't check again) */
        if (fullWindowRetry) {
            fullWindowRetry = false;
            /* alpha/beta already -INF/INF */
        } else if (config->useAspiration && depth > 1) {
            alpha = prevScore - aspirationWindow;
            beta  = prevScore + aspirationWindow;
            if (alpha < -INF) alpha = -INF;
            if (beta > INF) beta = INF;
            usedNarrowWindow = true;
        }

        uint64_t rootHash = castro_CalculateZobristHash(board);
        PieceColor rootTurn = board->turn;
        uint8_t rootCastling = board->castling_rights;
        Square rootEp = board->enpassant_square;
        uint64_t rootBitboards[12];
        char rootGrid[8][8];
        memcpy(rootBitboards, board->bitboards, sizeof(rootBitboards));
        memcpy(rootGrid, board->grid, sizeof(rootGrid));

        Move rootTtMove = {0};
        int dummyScore;
        g_searchStats.rootChildTtHits = 0;  /* diagnostic: count TT hits at ply 1 this depth */
        if (config->useTT && tt_probe(rootHash, depth, -INF, INF, 0, &dummyScore, &rootTtMove))
            set_tt_move(rootTtMove);
        else
            set_tt_move((Move){0});

        Moves moves = castro_GenerateMoves(board, MOVE_LEGAL);
        order(board, moves.list, moves.count, 0);

        int localBestScore = -INF;
        Move localBestMove = {0};
        bool aspirationFail = false;

        for (size_t i = 0; i < moves.count; i++) {

            if (search_should_stop())
                break;

            Move move = moves.list[i];

            if (!castro_MakeMove(board, move))
                continue;

            int score;
            if (i == 0) {
                score = -search(board, depth - 1, -beta, -alpha, 1, eval, order);
            } else {
                score = -search(board, depth - 1, -alpha - 1, -alpha, 1, eval, order);
                if (score > alpha)
                    score = -search(board, depth - 1, -beta, -alpha, 1, eval, order);
            }

            /* Root: expect exactly one move on the stack (the root move we just made).
             * Always restore from saved root state to avoid UnmakeMove/turn bugs at depth 9+. */
            assert(board->history.count == 1 && "history stack mismatch at root before UnmakeMove");
            {
                uint64_t childHash = board->hash;
                board->turn = rootTurn;
                board->castling_rights = rootCastling;
                board->enpassant_square = rootEp;
                memcpy(board->bitboards, rootBitboards, sizeof(rootBitboards));
                memcpy(board->grid, rootGrid, sizeof(rootGrid));
                castro_BoardUpdateOccupancy(board);
                board->hash = castro_CalculateZobristHash(board);
                castro_HistoryRemove(&board->history, childHash);
            }
            assert(castro_CalculateZobristHash(board) == rootHash && "UnmakeMove did not restore board");

            if (search_should_stop())
                break;

            if (score > localBestScore) {
                localBestScore = score;
                localBestMove = move;
            }

            if (score > alpha)
                alpha = score;

            /* Aspiration fail-high or fail-low only when we used narrow window; then retry with full window */
            if (usedNarrowWindow && (score <= prevScore - aspirationWindow || score >= prevScore + aspirationWindow)) {
                aspirationFail = true;
                break;
            }
        }

        if (aspirationFail) {
            fullWindowRetry = true;
            depth--;  /* re-do same depth with full window next iteration */
            continue;
        }

        if (!search_should_stop()) {
            bestMove = localBestMove;
            bestScore = localBestScore;
        }
        prevScore = localBestScore;

        /* Early exit only when tree has clearly collapsed (e.g. draw/TB): high depth but almost no new nodes. */
        uint64_t nodesThisDepth = g_searchStats.nodes + g_searchStats.qnodes;
        if (depth >= 16 && (nodesThisDepth - prevNodes) < 50)
            break;
        prevNodes = nodesThisDepth;

    uint64_t timeMs = search_elapsed_ms();
    uint64_t nodes  = g_searchStats.nodes + g_searchStats.qnodes;
    uint64_t nps    = (timeMs > 0) ? (nodes * 1000ULL / timeMs) : 0;

    char move[12];
    castro_MoveToString(bestMove, move);

    uci_stdout_lock();
    printf("info depth %d seldepth %d score cp %d nodes %llu nps %llu time %llu pv %s\n",
           depth,
           g_searchStats.selDepth,
           bestScore,
           (unsigned long long)nodes,
           (unsigned long long)nps,
           (unsigned long long)timeMs,
           move
        );
    fflush(stdout);
    uci_stdout_unlock();
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
        if (search_should_stop())
            return 0;
    }

    if (ply > g_searchStats.selDepth)
        g_searchStats.selDepth = ply;

    int alphaOrig = alpha;

    Move ttMove = {0};
    int ttScore;

    uint64_t zobrist = castro_CalculateZobristHash(board);
    if (g_searchConfig.useTT && tt_probe(zobrist,
                 depth,
                 alpha,
                 beta,
                 ply,
                 &ttScore,
                 &ttMove))
    {
        g_searchStats.ttHits++;
        if (ply == 1)
            g_searchStats.rootChildTtHits++;
        return ttScore;
    }

    if (depth <= 0)
        return quiescence(board, alpha, beta, ply, eval, order);

    /* Skip draw check at root children (ply 1): ensures we always recurse so the tree grows.
     * If is_draw wrongly returns true at ply 1 (e.g. board/hash bug), we'd otherwise add only 20 nodes/depth. */
    if (ply != 1 && is_draw(board))
        return 0;

    /* Syzygy probe: in leaf-like positions within limit, use TB score */
    if (syzygy_available()
        && depth <= g_searchConfig.syzygyProbeDepth
        && syzygy_piece_count(board) <= (unsigned)g_searchConfig.syzygyProbeLimit) {
        int tb_score = syzygy_probe_wdl(board, g_searchConfig.syzygy50MoveRule);
        if (tb_score != SYZYGY_PROBE_FAILED)
            return tb_score;
    }

    Moves moves = castro_GenerateMoves(board, MOVE_LEGAL);

    if (moves.count == 0) {
        /* Checkmate or stalemate */
        if (castro_IsInCheck(board))
            return -MATE_SCORE + ply;
        else
            return 0;
    }

    /* Check extension: extend by one ply when in check */
    int extension = (castro_IsInCheck(board) && depth >= 1) ? 1 : 0;
    int effective_depth = depth + extension;

    const int R = 3;
    if (g_searchConfig.useNullMove && effective_depth >= R + 1 && !castro_IsInCheck(board)) {
        castro_MakeNullMove(board);
        int nullScore = -search(board, effective_depth - 1 - R, -beta, -beta + 1, ply + 1, eval, order);
        castro_UnmakeNullMove(board);
        if (nullScore >= beta)
            return beta;
    }

    if (g_searchConfig.useTT)
        set_tt_move(ttMove);
    order(board, moves.list, moves.count, ply);

    int bestScore = -INF;
    Move bestMove = {0};

    for (size_t i = 0; i < moves.count; i++) {

        Move move = moves.list[i];
        bool isCapture = castro_IsCapture(board, move);

        if (!castro_MakeMove(board, move))
            continue;

        int score;
        bool doFullSearch = true;
        int reduction = 0;

        if (g_searchConfig.useLMR && effective_depth >= 3 && i >= 2 && !isCapture)
            reduction = 2;

        if (reduction > 0) {
            /* Late move reduction: try reduced depth with zero window first */
            score = -search(board, effective_depth - 1 - reduction, -alpha - 1, -alpha, ply + 1, eval, order);
            if (score > alpha)
                doFullSearch = true;
            else
                doFullSearch = false;
        }

        if (doFullSearch) {
            if (g_searchConfig.useLMR && reduction > 0) {
                /* Re-search at full depth */
                score = -search(board, effective_depth - 1, -beta, -alpha, ply + 1, eval, order);
            } else if (i == 0) {
                /* PVS: first move with full window */
                score = -search(board, effective_depth - 1, -beta, -alpha, ply + 1, eval, order);
            } else {
                /* PVS: zero window for remaining moves */
                score = -search(board, effective_depth - 1, -alpha - 1, -alpha, ply + 1, eval, order);
                if (score > alpha)
                    score = -search(board, effective_depth - 1, -beta, -alpha, ply + 1, eval, order);
            }
        }

        castro_UnmakeMove(board);

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }

        if (score > alpha)
            alpha = score;

        if (alpha >= beta) {
            if (!isCapture) {
                update_killer(move, ply);
                update_history(move, depth);
            }
            break;  // beta cutoff
        }
    }

    TTNodeType type;

    if (bestScore <= alphaOrig)
        type = TT_UPPERBOUND;
    else if (bestScore >= beta)
        type = TT_LOWERBOUND;
    else
        type = TT_EXACT;

    if (g_searchConfig.useTT)
        tt_store(zobrist,
                 effective_depth,
                 bestScore,
                 type,
                 bestMove,
                 ply);

    return bestScore;
}

