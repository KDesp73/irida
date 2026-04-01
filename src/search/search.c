// Search Implemenation:
// Negamax
//   + Iterative Deepening
//   + Alpha-Beta Pruning
//   + Quiescence
//   + Move Ordering
//   + Transposition Table
//   + Null Move Pruning
//   + Late Move Reductions
//   + Aspiration Windows
//   + PVS
//   + Syzygy

#include "search.h"
#include "castro.h"
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

/* PVS is sound only if the principal continuation is searched first with a full
 * window. Ordering should already rank the TT / iteration move highest; we also
 * swap it to index 0 when present so i==0 always matches that hint. */
static void prioritize_hash_move(Move list[], size_t count, Move hash_move)
{
    if (hash_move == NULL_MOVE || count == 0)
        return;
    for (size_t j = 0; j < count; j++) {
        if (list[j] != hash_move)
            continue;
        if (j != 0) {
            Move tmp = list[0];
            list[0] = list[j];
            list[j] = tmp;
        }
        return;
    }
}

Move search(Board* board, EvalFn eval, OrderFn order, SearchConfig* config)
{
    tt_inc_generation();

    // 1. Syzygy Tablebase Probe
    Move tb_move = NULL_MOVE;
    size_t piece_count = castro_PieceCount(board);
    if (config->useSyzygy && piece_count <= config->syzygyProbeLimit) {
        if (syzygy_probe_root(board, config->syzygy50MoveRule, &tb_move)) {
            printf("info string tablebase hit\n");
            return tb_move; 
        }
    }

    // 2. Statistics and State Initialization
    g_searchStats.nodes = 0;
    g_searchStats.qnodes = 0;
    g_searchStats.selDepth = 0;
    
    Move best_move = NULL_MOVE;
    int last_depth_score = 0; 

    search_start_timer(config->timeLimitMs);

    char root_fen_snapshot[256];
    castro_FenExport(board, root_fen_snapshot);
    root_fen_snapshot[255] = '\0';

    // 3. Iterative Deepening Loop
    for (int currentDepth = 1; currentDepth <= config->maxDepth; currentDepth++) {
        /* Deep search can leave side-to-move / fullmove out of sync with the
         * piece grid while history.count is 0 (castro undo/metadata edge cases).
         * Reload the root each depth so iterative deepening stays consistent. */
        castro_BoardFree(board);
        castro_BoardInitFen(board, root_fen_snapshot);

        if (search_time_up()) break;

        int delta = 40 + abs(last_depth_score) / 4;
        int alpha = -INF;
        int beta = INF;

        if (config->useAspiration && currentDepth >= 5) {
            alpha = last_depth_score - delta;
            beta  = last_depth_score + delta;
        }

        Move currentBestMove = NULL_MOVE;
        Move iterationBestMove = best_move;
        int bestScore = -INF;

        int failCount = 0;
        bool done = false;

        while (!done) {
            if (search_should_stop()) break;

            int originalAlpha = alpha;
            int originalBeta  = beta;

            bestScore = -INF;
            currentBestMove = NULL_MOVE;

            /* Aspiration re-searches and deep negamax can leave root metadata wrong.
             * Always rebuild root before generating moves. */
            castro_BoardFree(board);
            castro_BoardInitFen(board, root_fen_snapshot);

            // Generate and order moves
            Moves legal = castro_GenerateLegalMoves(board);
            order(board, legal.list, legal.count, 0, iterationBestMove);
            prioritize_hash_move(legal.list, legal.count, iterationBestMove);

            // Root move loop: fresh root before each line so a bad unmake cannot
            // poison the next sibling (or MoveToString / legality).
            for (size_t i = 0; i < legal.count; i++) {
                castro_BoardFree(board);
                castro_BoardInitFen(board, root_fen_snapshot);

                if (!castro_MakeMove(board, legal.list[i])) continue;
                
                int score = -negamax(board, eval, order,
                                     currentDepth - 1, 1,
                                     -beta, -alpha, config);

                if (search_should_stop()) break;

                if (score > bestScore) {
                    bestScore = score;
                    currentBestMove = legal.list[i];
                }

                if (score > alpha) alpha = score;

                if (alpha >= beta) break; // beta cutoff
            }

            if (search_should_stop()) break;

            // If aspiration disabled, accept immediately
            if (!config->useAspiration || currentDepth < 5) {
                done = true;
                break;
            }

            // Correct fail detection using ORIGINAL bounds
            if (bestScore <= originalAlpha) {
                // Fail low
                alpha = originalAlpha - delta;
                beta  = originalBeta;
                delta *= 2;
                failCount++;
            }
            else if (bestScore >= originalBeta) {
                // Fail high
                alpha = originalAlpha;
                beta  = originalBeta + delta;
                delta *= 2;
                failCount++;
            }
            else {
                // Success
                done = true;
            }

            // Update ordering hint for next re-search
            if (currentBestMove != NULL_MOVE) {
                iterationBestMove = currentBestMove;
            }

            // Emergency fallback: switch to full window
            if (failCount > 2) {
                alpha = -INF;
                beta  = INF;
                done = true;
            }
        }

        if (search_should_stop()) break;

        best_move = currentBestMove;
        last_depth_score = bestScore;

        // Report search results to UCI
        char moveBuf[10];
        castro_MoveToString(best_move, moveBuf);
        uci_report_search(currentDepth, last_depth_score,
                          search_elapsed_ms(), moveBuf);

        // Exit if mate found
        if (last_depth_score > (INF - MAX_PLY)) break; 
    }

    return best_move;
}

static int negamax(Board* board, EvalFn eval, OrderFn order,
                   int depth, int ply, int alpha, int beta,
                   SearchConfig* config)
{
    int alphaOrig = alpha;

    // --- 1. Static Checks ---
    if (ply > 0 && (castro_IsThreefoldRepetition(board) || board->halfmove >= 100))
        return 0;

    // --- 2. TT Probe ---
    Move tt_move = NULL_MOVE;
    int tt_score = 0;
    if (config->useTT && tt_probe(board->hash, depth, alpha, beta, ply, &tt_score, &tt_move))
        return tt_score;

    int mated_score = -INF + ply;
    if (alpha < mated_score) alpha = mated_score;
    if (beta <= alpha) return alpha;

    if ((g_searchStats.nodes & 2047) == 0 && search_time_up())
        uci_state.stopRequested = true;
    if (search_should_stop()) return 0;

    g_searchStats.nodes++;
    if (ply > g_searchStats.selDepth) g_searchStats.selDepth = ply;

    // --- 3. Syzygy Probe ---
    int piece_count = castro_PieceCount(board);
    if (config->useSyzygy && ply > 0 && piece_count <= config->syzygyProbeLimit) {
        int tb_score = syzygy_probe_wdl(board, config->syzygy50MoveRule);
        if (tb_score != SYZYGY_PROBE_FAILED) {
            if (config->useTT)
                tt_store(board->hash, depth, tb_score, TT_EXACT, NULL_MOVE, ply);
            g_searchStats.tbHits++;
            return tb_score;
        }
    }

    // --- 4. Base Case ---
    if (depth <= 0)
        return config->useQuiescence ? quiescence(board, alpha, beta, ply, eval, order) : 0;

    // --- 5. Null Move Pruning ---
    if (config->useNMP && depth >= 3 && !castro_IsInCheck(board) &&
        castro_HasNonPawnMaterial(board, board->turn))
    {
        castro_MakeNullMove(board);
        int score = -negamax(board, eval, order, depth - 1 - 3, ply + 1, -beta, -beta + 1, config);
        castro_UnmakeNullMove(board);

        if (score >= beta)
            return (score >= MATE_SCORE) ? beta : score;
    }

    // --- 6. Move Generation ---
    Moves legal = castro_GenerateLegalMoves(board);
    if (legal.count == 0)
        return castro_IsInCheck(board) ? -INF + ply : 0;

    order(board, legal.list, legal.count, ply, tt_move);
    prioritize_hash_move(legal.list, legal.count, tt_move);

    Move bestMove = NULL_MOVE;
    int bestScore = -INF;
    const bool parent_in_check = castro_IsInCheck(board);

    // Store quiet moves searched (for penalties)
    Move quiets_tried[MAX_MOVES];
    int quiet_count = 0;

    for (size_t i = 0; i < legal.count; ++i) {
        Move move = legal.list[i];
        int side = board->turn;

        bool is_capture = castro_IsCapture(board, move);

        if (!castro_MakeMove(board, move)) continue;

        bool gives_check = castro_IsInCheck(board);

        int score;

        const bool pv_node = (i == 0)
            || (tt_move != NULL_MOVE && move == tt_move);

        if (pv_node || !config->usePVS) {
            score = -negamax(board, eval, order, depth - 1, ply + 1, -beta, -alpha, config);
        } 
        else {
            int newDepth = depth - 1;

            if (config->useLMR && depth >= 3 && i >= 4 &&
                    !parent_in_check && !is_capture && !gives_check &&
                    !(tt_move != NULL_MOVE && move == tt_move))
            {
                int R = 1 + depth / 6 + i / 10;
                newDepth -= R;
                if (newDepth < 1) newDepth = 1;
            }

            score = -negamax(board, eval, order, newDepth, ply + 1,
                    -(alpha + 1), -alpha, config);

            if (score > alpha) {
                score = -negamax(board, eval, order, depth - 1, ply + 1,
                        -beta, -alpha, config);
            }
        }

        castro_UnmakeMove(board);

        if (search_should_stop()) return 0;

        // Quiet tracking
        if (!is_capture && !gives_check) {
            quiets_tried[quiet_count++] = move;
        }

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }

        if (score > alpha) alpha = score;

        if (alpha >= beta) {
            if (!is_capture) {
                int from = castro_GetFrom(move);
                int to   = castro_GetTo(move);

                int *h = &history_heuristic[side][from][to];
                *h += depth * depth;

                if (*h > HISTORY_MAX) *h = HISTORY_MAX;

                for (int j = 0; j < quiet_count - 1; j++) {
                    int f = castro_GetFrom(quiets_tried[j]);
                    int t = castro_GetTo(quiets_tried[j]);

                    int *hp = &history_heuristic[side][f][t];
                    *hp -= depth * depth;

                    if (*hp < -HISTORY_MAX) *hp = -HISTORY_MAX;
                }
            }
            break;
        }
    }

    // --- 7. TT Store ---
    if (config->useTT) {
        TTNodeType type;
        if (bestScore <= alphaOrig) type = TT_UPPERBOUND;
        else if (bestScore >= beta) type = TT_LOWERBOUND;
        else type = TT_EXACT;

        tt_store(board->hash, depth, bestScore, type, bestMove, ply);
    }

    return bestScore;
}
