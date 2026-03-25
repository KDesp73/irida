// Search Implemenation:
// Negamax
//   + Iterative Deepening
//   + Alpha-Beta Pruning
//   + Quiescence
//   + Move Ordering
//   + Transposition Table
//   + Null Move Pruning
//   + Syzygy

#include "search.h"
#include "castro_additions.h"
#include "syzygy.h"
#include "tt.h"
#include "uci.h"

static int negamax(
    Board* board,
    EvalFn eval,
    OrderFn order,
    int depth, int ply,
    int a, int b,
    SearchConfig* config
);

Move search(Board* board, EvalFn eval, OrderFn order, SearchConfig* config)
{
    printf("info using new search with Syzygy\n");

    Move tb_move = NULL_MOVE;
    size_t piece_count = castro_PieceCount(board);
    if (config->useSyzygy && piece_count <= config->syzygyProbeLimit) {
        if (syzygy_probe_root(board, config->syzygy50MoveRule, &tb_move)) {
            printf("info string tablebase hit\n");
            return tb_move; 
        }
    }

    Move best_move = NULL_MOVE;
    g_searchStats.nodes = 0;

    search_start_timer(config->timeLimitMs);

    // Iterative Deepening
    for (int currentDepth = 1; currentDepth <= config->maxDepth; currentDepth++) {
        if (search_time_up()) break;

        Moves legal = castro_GenerateLegalMoves(board);
        order(board, legal.list, legal.count, 0, best_move);

        int alpha = -INF;
        int beta = INF;
        int bestScore = -INF;
        Move currentBestMove = NULL_MOVE;

        for (size_t i = 0; i < legal.count; i++) {
            if (!castro_MakeMove(board, legal.list[i])) continue;

            int score = -negamax(board, eval, order, currentDepth - 1, 1, -beta, -alpha, config);
            castro_UnmakeMove(board);

            // If time ran out, discard results from this depth to avoid playing a blunder
            if (search_should_stop()) break;

            if (score > bestScore) {
                bestScore = score;
                currentBestMove = legal.list[i];
            }
            if (score > alpha) alpha = score;
        }

        // Only update the global best_move if we completed the depth (or at least the first move)
        if (!search_should_stop()) {
            best_move = currentBestMove;
            
            char moveBuf[10];
            castro_MoveToString(best_move, moveBuf);
            uci_report_search(currentDepth, bestScore, g_searchStats.nodes, search_elapsed_ms(), moveBuf);
            
            // If we found a mate, no need to search deeper
            if (bestScore > (INF - MAX_PLY)) break; 
        } else {
            break; 
        }
    }

    return best_move;
}

#include "fathom/tbprobe.h"
static int wdl_to_score(int wdl, int ply) {
    if (wdl == TB_WIN)  return  MATE_SCORE - ply;
    if (wdl == TB_LOSS) return -MATE_SCORE + ply;
    return 0; // Draw
}

static int negamax(Board* board, EvalFn eval, OrderFn order, int depth, int ply, int a, int b, SearchConfig* config)
{
    // 1. Static Checks (Repetition / 50-move rule)
    if (ply > 0 && (castro_IsThreefoldRepetition(board) || board->halfmove >= 100)) {
        return 0;
    }

    // 2. TT Probe: Fastest check, do this first.
    Move tt_move = NULL_MOVE;
    int tt_score = 0;
    if (config->useTT && tt_probe(board->hash, depth, a, b, ply, &tt_score, &tt_move)) {
        return tt_score;
    }

    // 3. Syzygy Probe: Only if TT didn't give us a result.
    int piece_count = castro_PieceCount(board);
    if (config->useSyzygy && ply > 0 && piece_count <= config->syzygyProbeLimit) {
        int wdl;
        if (syzygy_probe_wdl(board, &wdl)) {
            int score = wdl_to_score(wdl, ply);
            if(config->useTT) tt_store(board->hash, depth, score, TT_EXACT, NULL_MOVE, ply);
            return score;
        }
    }

    // 4. Base Case: Leaf Node
    if (depth <= 0) return config->useQuiescence ? quiescence(board, a, b, ply, eval, order) : 0;

    // 5. Null Move Pruning
    if (config->useNMP && depth >= 3 && !castro_IsInCheck(board) && castro_HasNonPawnMaterial(board, board->turn)) {
        castro_MakeNullMove(board);
        int score = -negamax(board, eval, order, depth - 1 - 3, ply + 1, -b, -b + 1, config);
        castro_UnmakeNullMove(board);

        if (score >= b) {
            // Prune if the null move search fails high
            return (score >= MATE_SCORE) ? b : score; 
        }
    }

    // 6. Move Generation and Search
    int original_alpha = a;
    Moves legal = castro_GenerateLegalMoves(board);
    if (legal.count == 0) {
        if (castro_IsInCheck(board)) return -INF + ply;
        return 0;
    }

    order(board, legal.list, legal.count, ply, tt_move);

    Move bestMove = NULL_MOVE;
    int bestScore = -INF;

    for (size_t i = 0; i < legal.count; ++i) {
        if (!castro_MakeMove(board, legal.list[i])) continue;
        
        int score = -negamax(board, eval, order, depth - 1, ply + 1, -b, -a, config);
        castro_UnmakeMove(board);

        if (search_should_stop()) return 0;

        if (score > bestScore) {
            bestScore = score;
            bestMove = legal.list[i];
        }

        if (score > a) a = score;
        if (a >= b) break; 
    }

    // 7. Store Result
    if (config->useTT) {
        TTNodeType type = (bestScore <= original_alpha) ? TT_UPPERBOUND : 
            (bestScore >= b) ? TT_LOWERBOUND : TT_EXACT;

        tt_store(board->hash, depth, bestScore, type, bestMove, ply);
    }

    return bestScore;
}
