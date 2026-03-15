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


/* Helper to verify if a move is actually legal in the current position */
static bool is_move_legal(Board* board, Move move) {
    Moves legalMoves = castro_GenerateMoves(board, MOVE_LEGAL);
    for (size_t i = 0; i < legalMoves.count; i++) {
        if (memcmp(&move, &legalMoves.list[i], sizeof(Move)) == 0) return true;
    }
    return false;
}

#define MAX_ID_DEPTH 64

static int search(Board* board, int depth, int alpha, int beta, int ply, EvalFn eval, OrderFn order);

Move search_v040(Board* board, EvalFn eval, OrderFn order, SearchConfig* config)
{
    Move bestMove = {0};
    int bestScore = -INF;

    g_searchStats.nodes = 0;
    g_searchStats.qnodes = 0;
    g_searchStats.selDepth = 0;
    g_searchStats.tbHits = 0;

    tt_clear();
    tt_inc_generation(); 
    eval_cache_clear();
    search_start_timer(config->timeLimitMs);

    /* Initial legal move generation to ensure we have a fallback */
    Moves rootMoves = castro_GenerateMoves(board, MOVE_LEGAL);
    if (rootMoves.count > 0) bestMove = rootMoves.list[0];

    const int aspirationWindow = 50;
    int prevScore = 0;
    bool fullWindowRetry = false;
    int maxDepth = (config->maxDepth > MAX_ID_DEPTH) ? MAX_ID_DEPTH : config->maxDepth;
    uint64_t prevNodes = 0;

    for (int depth = 1; depth <= maxDepth; depth++) {
        if (search_should_stop()) break;

        int alpha = -INF;
        int beta  = INF;
        bool usedNarrowWindow = false;

        if (fullWindowRetry) {
            fullWindowRetry = false;
        } else if (config->useAspiration && depth > 1) {
            alpha = prevScore - aspirationWindow;
            beta  = prevScore + aspirationWindow;
            usedNarrowWindow = true;
        }

        Move rootTtMove = {0};
        int dummyScore;
        if (config->useTT && tt_probe(board->hash, depth, -INF, INF, 0, &dummyScore, &rootTtMove)) {
            if (is_move_legal(board, rootTtMove)) set_tt_move(rootTtMove);
        } else {
            set_tt_move((Move){0});
        }

        order(board, rootMoves.list, rootMoves.count, 0);

        int localBestScore = -INF;
        Move localBestMove = rootMoves.list[0];
        bool aspirationFail = false;

        for (size_t i = 0; i < rootMoves.count; i++) {
            if (search_should_stop()) break;

            Move move = rootMoves.list[i];
            if (!castro_MakeMove(board, move)) continue;

            int score;
            if (i == 0) {
                score = -search(board, depth - 1, -beta, -alpha, 1, eval, order);
            } else {
                score = -search(board, depth - 1, -alpha - 1, -alpha, 1, eval, order);
                if (score > alpha)
                    score = -search(board, depth - 1, -beta, -alpha, 1, eval, order);
            }

            castro_UnmakeMove(board);

            if (search_should_stop()) break;

            if (score > localBestScore) {
                localBestScore = score;
                localBestMove = move;
            }

            if (score > alpha) alpha = score;

            if (usedNarrowWindow && (score <= alpha || score >= beta)) {
                aspirationFail = true;
                break;
            }
        }

        if (aspirationFail) {
            fullWindowRetry = true;
            depth--; 
            continue;
        }

        if (!search_should_stop()) {
            bestMove = localBestMove;
            bestScore = localBestScore;
            prevScore = localBestScore;
        }

        /* --- UCI Info Reporting --- */
        uint64_t timeMs = search_elapsed_ms();
        uint64_t nodes  = g_searchStats.nodes + g_searchStats.qnodes;
        uint64_t nps    = (timeMs > 0) ? (nodes * 1000ULL / timeMs) : 0;
        int hashfull    = tt_hashfull();

        /* Extract PV for printing */
        char pvBuf[1024] = {0};
        size_t pvLen = 0;
        Move pvMove = bestMove;
        int pvDepth = depth;
        int pvCount = 0;
        while (pvDepth > 0 && pvLen < sizeof(pvBuf) - 10 && pvCount < MAX_PLY) {
            char moveStr[12];
            castro_MoveToString(pvMove, moveStr);
            size_t len = strlen(moveStr);
            if (pvLen) pvBuf[pvLen++] = ' ';
            memcpy(pvBuf + pvLen, moveStr, len);
            pvLen += len;
            if (!castro_MakeMove(board, pvMove)) break;
            pvCount++;
            pvDepth--;
            if (!g_searchConfig.useTT || !tt_probe_pv(board->hash, &pvMove)) break;
        }
        for (int j = 0; j < pvCount; j++) castro_UnmakeMove(board);
        pvBuf[pvLen] = '\0';

        const int mateThreshold = MATE_SCORE - MAX_PLY;

        uci_stdout_lock();
        printf("info depth %d seldepth %d score ", depth, g_searchStats.selDepth);
        if (bestScore > mateThreshold)
            printf("mate %d ", (MATE_SCORE - bestScore + 1) / 2);
        else if (bestScore < -mateThreshold)
            printf("mate %d ", -((MATE_SCORE + bestScore + 1) / 2));
        else
            printf("cp %d ", bestScore);

        printf("nodes %llu nps %llu hashfull %d tbhits %llu time %llu pv %s\n",
               (unsigned long long)nodes, (unsigned long long)nps, hashfull, 
               (unsigned long long)g_searchStats.tbHits, (unsigned long long)timeMs, pvBuf);
        fflush(stdout);
        uci_stdout_unlock();

        if (depth >= 16 && (nodes - prevNodes) < 50) break;
        prevNodes = nodes;
    }

    return bestMove;
}

static int search(Board* board, int depth, int alpha, int beta, int ply, EvalFn eval, OrderFn order)
{
    if ((g_searchStats.nodes++ & 2047) == 0 && search_should_stop()) return 0;

    if (ply > g_searchStats.selDepth) g_searchStats.selDepth = ply;
    int alphaOrig = alpha;

    Move ttMove = {0};
    int ttScore;
    uint64_t zobrist = board->hash;

    if (g_searchConfig.useTT && tt_probe(zobrist, depth, alpha, beta, ply, &ttScore, &ttMove)) {
        return ttScore;
    }

    if (depth <= 0) return quiescence(board, alpha, beta, ply, eval, order);
    if (ply != 0 && is_draw(board)) return 0;

    if (syzygy_available() && depth <= g_searchConfig.syzygyProbeDepth) {
        int tb_score = syzygy_probe_wdl(board, g_searchConfig.syzygy50MoveRule);
        if (tb_score != SYZYGY_PROBE_FAILED) return tb_score;
    }

    if (g_searchConfig.useNullMove && depth >= 3 && !castro_IsInCheck(board)) {
        castro_MakeNullMove(board);
        int nullScore = -search(board, depth - 1 - 3, -beta, -beta + 1, ply + 1, eval, order);
        castro_UnmakeNullMove(board);
        if (nullScore >= beta) return beta;
    }

    Moves moves = castro_GenerateMoves(board, MOVE_LEGAL);
    if (moves.count == 0) {
        return castro_IsInCheck(board) ? (-MATE_SCORE + ply) : 0;
    }

    set_tt_move(ttMove); 
    order(board, moves.list, moves.count, ply);

    int bestScore = -INF;
    Move bestMove = {0};

    for (size_t i = 0; i < moves.count; i++) {
        Move move = moves.list[i];
        if (!castro_MakeMove(board, move)) continue;

        int score;
        int reduction = 0;
        if (g_searchConfig.useLMR && depth >= 3 && i >= 3 && !castro_IsCapture(board, move)) {
            reduction = 1;
        }

        if (i == 0) {
            score = -search(board, depth - 1, -beta, -alpha, ply + 1, eval, order);
        } else {
            score = -search(board, depth - 1 - reduction, -alpha - 1, -alpha, ply + 1, eval, order);
            if (score > alpha && reduction > 0)
                score = -search(board, depth - 1, -alpha - 1, -alpha, ply + 1, eval, order);
            if (score > alpha)
                score = -search(board, depth - 1, -beta, -alpha, ply + 1, eval, order);
        }

        castro_UnmakeMove(board);
        if (search_should_stop()) return 0;

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }
        if (score > alpha) alpha = score;
        if (alpha >= beta) break; 
    }

    TTNodeType type = (bestScore <= alphaOrig) ? TT_UPPERBOUND : (bestScore >= beta) ? TT_LOWERBOUND : TT_EXACT;
    if (g_searchConfig.useTT) tt_store(zobrist, depth, bestScore, type, bestMove, ply);

    return bestScore;
}
