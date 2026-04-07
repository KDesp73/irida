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
#include <string.h>
#include "syzygy.h"
#include "tbprobe.h"
#include "tt.h"
#include "uci.h"

static int negamax(
    Board* board,
    EvalFn eval,
    OrderFn order,
    int depth, int ply,
    int a, int b,
    bool tt_exact_ok,
    SearchConfig* config
);

static int lmr_new_depth(
    int depth,
    size_t move_index,
    bool pv_node,
    bool parent_in_check,
    bool is_capture,
    bool gives_check,
    Move tt_move,
    Move move,
    SearchConfig* config)
{
    int newDepth = depth - 1;
    if (config->useLMR && depth >= 3 && move_index >= 4 && !pv_node &&
        !parent_in_check && !is_capture && !gives_check &&
        !(tt_move != NULL_MOVE && move == tt_move))
    {
        int R = 1 + depth / 6 + (int)move_index / 10;
        newDepth -= R;
        if (newDepth < 1)
            newDepth = 1;
    }
    return newDepth;
}

static bool nmp_try_cutoff(
    Board* board,
    EvalFn eval,
    OrderFn order,
    int depth,
    int ply,
    int beta,
    SearchConfig* config,
    int* out_score)
{
    if (!config->useNMP || depth < 3 || castro_IsInCheck(board) ||
        !castro_HasNonPawnMaterial(board, board->turn))
        return false;

    castro_MakeNullMove(board);
    int score = -negamax(board, eval, order, depth - 1 - 3, ply + 1, -beta, -beta + 1,
                         false, config);
    castro_UnmakeNullMove(board);

    if (score >= beta) {
        *out_score = (score >= MATE_SCORE) ? beta : score;
        return true;
    }
    return false;
}

/* Full window for the PV child; zero-window scout for siblings. Re-search when the scout
 * beats alpha and either LMR was used (verify at full depth) or score < beta (avoid
 * re-searching every fail-high, which explodes node count). */
static int pvs_negamax_move(
    Board* board,
    EvalFn eval,
    OrderFn order,
    int depth,
    int ply,
    int alpha,
    int beta,
    int newDepth,
    bool use_zw,
    bool tt_exact_ok,
    SearchConfig* config)
{
    int score;
    const bool lmr_reduced = (newDepth < depth - 1);

    if (!use_zw) {
        score = -negamax(board, eval, order, newDepth, ply + 1, -beta, -alpha,
                         tt_exact_ok, config);
        if (config->useLMR && lmr_reduced && score > alpha) {
            score = -negamax(board, eval, order, depth - 1, ply + 1, -beta, -alpha,
                             tt_exact_ok, config);
        }
    } else {
        score = -negamax(board, eval, order, newDepth, ply + 1,
                         -(alpha + 1), -alpha, false, config);

        if (score > alpha && (lmr_reduced || score < beta)) {
            score = -negamax(board, eval, order, depth - 1, ply + 1,
                             -beta, -alpha, tt_exact_ok, config);
        }
    }
    return score;
}

static void aspiration_init_window(
    SearchConfig* config,
    int currentDepth,
    int last_depth_score,
    int* delta,
    int* alpha,
    int* beta)
{
    *delta = 40 + abs(last_depth_score) / 4;
    *alpha = -INF;
    *beta = INF;
    if (config->useAspiration && currentDepth >= 5) {
        *alpha = last_depth_score - *delta;
        *beta = last_depth_score + *delta;
    }
}

/* Aspiration is enabled and currentDepth >= 5. Returns true when the loop should stop
 * (score inside window, or full-window fallback after repeated fails). */
static bool aspiration_try_finish(
    int bestScore,
    int originalAlpha,
    int originalBeta,
    int* alpha,
    int* beta,
    int* delta,
    int* failCount)
{
    if (bestScore <= originalAlpha) {
        *alpha = originalAlpha - *delta;
        *beta = originalBeta;
        *delta *= 2;
        (*failCount)++;
    } else if (bestScore >= originalBeta) {
        *alpha = originalAlpha;
        *beta = originalBeta + *delta;
        *delta *= 2;
        (*failCount)++;
    } else {
        return true;
    }

    if (*failCount > 2) {
        *alpha = -INF;
        *beta = INF;
        return true;
    }
    return false;
}

/* PVS: the principal continuation must be searched with a full window first.
 * We swap the TT move to list slot 0 when present; the actual PV flag is set on
 * the first MakeMove that succeeds, not on i==0 (slot 0 can fail if the TT move
 * is stale). LMR must not apply to that first successful extension. */
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
    if (config->useSyzygy && piece_count <= config->syzygyProbeLimit &&
        TB_LARGEST > 0 && piece_count <= TB_LARGEST) {
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

        int delta, alpha, beta;
        aspiration_init_window(config, currentDepth, last_depth_score, &delta, &alpha, &beta);

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
                                     -beta, -alpha, true, config);

                if (search_should_stop()) break;

                if (score > bestScore) {
                    bestScore = score;
                    currentBestMove = legal.list[i];
                }

                if (score > alpha) alpha = score;

                if (alpha >= beta) break; // beta cutoff
            }

            if (search_should_stop()) break;

            if (!config->useAspiration || currentDepth < 5) {
                done = true;
                break;
            }

            if (currentBestMove != NULL_MOVE)
                iterationBestMove = currentBestMove;

            if (aspiration_try_finish(bestScore, originalAlpha, originalBeta,
                                     &alpha, &beta, &delta, &failCount))
                done = true;
        }

        /* Commit this depth before checking stop: if movetime/stop fires right after
         * the aspiration loop, we must not skip assignment or UCI sends bestmove (none). */
        best_move = currentBestMove;
        last_depth_score = bestScore;

        char moveBuf[10];
        castro_MoveToString(best_move, moveBuf);
        uci_report_search(currentDepth, last_depth_score,
                          search_elapsed_ms(), moveBuf);

        if (search_should_stop()) break;

        // Exit if mate found
        if (last_depth_score > (INF - MAX_PLY)) break;
    }

    if (best_move == NULL_MOVE) {
        Board fb;
        memset(&fb, 0, sizeof(fb));
        castro_BoardInitFen(&fb, root_fen_snapshot);
        Moves m = castro_GenerateLegalMoves(&fb);
        if (m.count > 0)
            best_move = m.list[0];
        castro_BoardFree(&fb);
    }

    return best_move;
}

static int negamax(Board* board, EvalFn eval, OrderFn order,
                   int depth, int ply, int alpha, int beta,
                   bool tt_exact_ok,
                   SearchConfig* config)
{
    int alphaOrig = alpha;

    // --- 1. Static Checks ---
    if (ply > 0 && (castro_IsThreefoldRepetition(board) || board->halfmove >= 100))
        return config->contempt;

    // --- 2. TT Probe ---
    Move tt_move = NULL_MOVE;
    int tt_score = 0;
    if (config->useTT && tt_probe(board->hash, depth, alpha, beta, ply, &tt_score, &tt_move))
        return tt_score;

    int mated_score = -INF + ply;
    if (alpha < mated_score) alpha = mated_score;
    if (beta <= alpha) return alpha;

    if ((g_searchStats.nodes & 1023) == 0 && search_time_up())
        uci_state.stopRequested = true;
    if (search_should_stop()) return 0;

    g_searchStats.nodes++;
    if (ply > g_searchStats.selDepth) g_searchStats.selDepth = ply;

    // --- 3. Syzygy Probe ---
    /* Stockfish-style: always probe below the user piece cap; at the cap, require
     * remaining depth >= SyzygyProbeDepth so shallow nodes do not hammer TB. */
    size_t piece_count = castro_PieceCount(board);
    bool tb_wdl_eligible = config->useSyzygy && ply > 0 &&
        piece_count <= config->syzygyProbeLimit &&
        (piece_count < config->syzygyProbeLimit ||
         (depth >= 0 && (size_t)depth >= config->syzygyProbeDepth));
    if (tb_wdl_eligible) {
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
    int nmp_score = 0;
    if (nmp_try_cutoff(board, eval, order, depth, ply, beta, config, &nmp_score))
        return nmp_score;

    // --- 6. Move Generation ---
    Moves legal = castro_GenerateLegalMoves(board);
    if (legal.count == 0)
        return castro_IsInCheck(board) ? -INF + ply : 0;

    order(board, legal.list, legal.count, ply, tt_move);
    prioritize_hash_move(legal.list, legal.count, tt_move);

    Move bestMove = NULL_MOVE;
    int bestScore = -INF;
    const bool parent_in_check = castro_IsInCheck(board);

    /* PVS: the first *successfully played* legal move must use a full window.
     * Using list index i==0 is wrong when MakeMove fails for that slot (e.g. TT
     * best move inconsistent with the board); the next sibling would incorrectly
     * get a zero-window scout and the tree can blow up. */
    bool first_legal_extension = true;

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

        const bool pv_node = first_legal_extension;
        first_legal_extension = false;

        int newDepth = lmr_new_depth(depth, i, pv_node, parent_in_check,
                                     is_capture, gives_check, tt_move, move, config);

        const bool use_zw = config->usePVS && !pv_node;

        score = pvs_negamax_move(board, eval, order, depth, ply, alpha, beta,
                                 newDepth, use_zw, tt_exact_ok, config);

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
                killer_store(move, ply);
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

        if (type == TT_EXACT && !tt_exact_ok)
            type = TT_LOWERBOUND;

        tt_store(board->hash, depth, bestScore, type, bestMove, ply);
    }

    return bestScore;
}
