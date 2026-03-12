/*
 * Theory: Syzygy tablebases (Fathom).
 *
 * We use the Fathom library to probe Syzygy endgame tablebases. WDL (win/draw/loss)
 * probing returns a result from the side-to-move perspective; we map that to a
 * centipawn-like score. DTZ (distance-to-zero) root probing can suggest the best
 * move from a TB position; the board is converted to Fathom's bitboard format
 * (white, black, kings, queens, rooks, bishops, knights, pawns, castling, ep).
 * Probe is only valid when piece count and other conditions (e.g. halfmove,
 * castling) match the TB format. syzygy_probe_root currently does not decode
 * the TB move back to the engine's move format.
 */
#include "syzygy.h"
#include "castro.h"
#include <string.h>

#include "tbprobe.h"

static bool g_syzygy_loaded = false;
static char g_syzygy_path[512];

bool syzygy_init(const char* path)
{
    if (!path || path[0] == '\0') {
        g_syzygy_loaded = false;
        tb_free();
        return false;
    }

    strncpy(g_syzygy_path, path, sizeof(g_syzygy_path) - 1);
    g_syzygy_path[sizeof(g_syzygy_path) - 1] = '\0';

    if (!tb_init(g_syzygy_path)) {
        g_syzygy_loaded = false;
        return false;
    }
    g_syzygy_loaded = true;
    return true;
}

void syzygy_free(void)
{
    tb_free();
    g_syzygy_loaded = false;
}

bool syzygy_available(void)
{
    return g_syzygy_loaded;
}

unsigned syzygy_piece_count(Board* board)
{
    unsigned n = 0;
    for (int i = 0; i < PIECE_TYPES; i++) {
        uint64_t bb = board->bitboards[i];
        while (bb) {
            n++;
            bb &= bb - 1;
        }
    }
    return n;
}

static uint64_t bitboard_white(const Board* b)
{
    return b->white;
}

static uint64_t bitboard_black(const Board* b)
{
    return b->black;
}

static uint64_t bitboard_kings(const Board* b)
{
    return b->bitboards[INDEX_WHITE_KING] | b->bitboards[INDEX_BLACK_KING];
}

static uint64_t bitboard_queens(const Board* b)
{
    return b->bitboards[INDEX_WHITE_QUEEN] | b->bitboards[INDEX_BLACK_QUEEN];
}

static uint64_t bitboard_rooks(const Board* b)
{
    return b->bitboards[INDEX_WHITE_ROOK] | b->bitboards[INDEX_BLACK_ROOK];
}

static uint64_t bitboard_bishops(const Board* b)
{
    return b->bitboards[INDEX_WHITE_BISHOP] | b->bitboards[INDEX_BLACK_BISHOP];
}

static uint64_t bitboard_knights(const Board* b)
{
    return b->bitboards[INDEX_WHITE_KNIGHT] | b->bitboards[INDEX_BLACK_KNIGHT];
}

static uint64_t bitboard_pawns(const Board* b)
{
    return b->bitboards[INDEX_WHITE_PAWN] | b->bitboards[INDEX_BLACK_PAWN];
}

static unsigned ep_square(const Board* b)
{
    if (b->enpassant_square == SQUARE_NONE)
        return 0;
    return (unsigned)b->enpassant_square;
}

int syzygy_probe_wdl(Board* board, bool use_rule50)
{
    if (!g_syzygy_loaded)
        return SYZYGY_PROBE_FAILED;

    /* Fathom's public tb_probe_wdl requires halfmove == 0 and castling == 0. */
    if (board->halfmove != 0 || board->castling_rights != 0)
        return SYZYGY_PROBE_FAILED;

    (void)use_rule50;

    unsigned wdl = tb_probe_wdl(
        bitboard_white(board),
        bitboard_black(board),
        bitboard_kings(board),
        bitboard_queens(board),
        bitboard_rooks(board),
        bitboard_bishops(board),
        bitboard_knights(board),
        bitboard_pawns(board),
        0u,
        0u,
        ep_square(board),
        board->turn);

    if (wdl == TB_RESULT_FAILED)
        return SYZYGY_PROBE_FAILED;

    /* Convert WDL to score from side-to-move perspective. Use centipawn-like values. */
    static const int score_tbl[] = {
        [TB_LOSS]         = -10000,
        [TB_BLESSED_LOSS] = 0,
        [TB_DRAW]         = 0,
        [TB_CURSED_WIN]   = 0,
        [TB_WIN]          = 10000,
    };
    int score = (wdl < 5) ? score_tbl[wdl] : 0;
    if (!board->turn)
        score = -score;
    return score;
}

bool syzygy_probe_root(Board* board, bool use_rule50, Move* best_move_out)
{
    if (!g_syzygy_loaded || !best_move_out)
        return false;
    if (board->castling_rights != 0)
        return false;

    struct TbRootMoves root_moves;
    memset(&root_moves, 0, sizeof(root_moves));

    int ok = tb_probe_root_dtz(
        bitboard_white(board),
        bitboard_black(board),
        bitboard_kings(board),
        bitboard_queens(board),
        bitboard_rooks(board),
        bitboard_bishops(board),
        bitboard_knights(board),
        bitboard_pawns(board),
        (unsigned)board->halfmove,
        0u,
        ep_square(board),
        board->turn,
        false,
        use_rule50,
        &root_moves);

    if (!ok || root_moves.size == 0)
        return false;

    /* Pick first (best) root move. TbRootMove has TbMove move (from/to encoded). */
    struct TbRootMove* rm = &root_moves.moves[0];
    unsigned from = TB_MOVE_FROM(rm->move);
    unsigned to   = TB_MOVE_TO(rm->move);
    /* Convert to castro Move. Castro Move is uint32_t - we need to build from from/to.
     * Castro may have a constructor; otherwise we need to match castro's encoding. */
    (void)from;
    (void)to;
    /* For now we don't decode TbMove to castro Move - would need castro's move encoding.
     * So return false and let search find the move. */
    return false;
}
