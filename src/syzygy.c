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
#include "IncludeOnly/logging.h"
#include "castro.h"
#include <stdint.h>
#include <string.h>

#include "tbprobe.h"

/* Fathom tbchess.c:is_valid — pawns may not sit on 1st/8th rank. */
#define SYZYGY_BOARD_FILE_EDGE (0xFF000000000000FFull)

static bool g_syzygy_loaded = false;
static char g_syzygy_path[512];

bool irida_syzygy_init(const char* path)
{
    if (!path || path[0] == '\0') {
        g_syzygy_loaded = false;
        tb_free();
        return false;
    }

    const char* home = getenv("HOME");
    char expanded_path[sizeof(g_syzygy_path)];
    size_t out_pos = 0;

    if (home) {
        size_t home_len = strlen(home);

        for (size_t i = 0; path[i] != '\0'; ++i) {
            if (path[i] == '~') {
                // Copy HOME into output
                if (out_pos + home_len >= sizeof(expanded_path))
                    break; // prevent overflow

                memcpy(expanded_path + out_pos, home, home_len);
                out_pos += home_len;
            } else {
                if (out_pos + 1 >= sizeof(expanded_path))
                    break; // prevent overflow

                expanded_path[out_pos++] = path[i];
            }
        }
        expanded_path[out_pos] = '\0';
    } else {
        // Fallback: just copy original path
        strncpy(expanded_path, path, sizeof(expanded_path) - 1);
        expanded_path[sizeof(expanded_path) - 1] = '\0';
    }

    strncpy(g_syzygy_path, expanded_path, sizeof(g_syzygy_path) - 1);
    g_syzygy_path[sizeof(g_syzygy_path) - 1] = '\0';

    if (!tb_init(g_syzygy_path)) {
        g_syzygy_loaded = false;
        return false;
    }

    g_syzygy_loaded = true;
    return true;
}

void irida_syzygy_free(void)
{
    tb_free();
    g_syzygy_loaded = false;
}

bool irida_syzygy_available(void)
{
    return g_syzygy_loaded;
}

unsigned irida_syzygy_piece_count(Board* board)
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

/* Fathom builds a material key from (white & queens), etc. If board->white/black
 * lag behind bitboards[], keys disagree with piece masks and tbprobe can hit
 * lsb(0) in fill_squares (assert in tbprobe.c). Mirror tbchess is_valid
 * occupancy rules; skip is_legal() to avoid duplicating Fathom movegen. */
static bool syzygy_fathom_layout_ok(Board* board)
{
    castro_BoardUpdateOccupancy(board);

    if (TB_LARGEST == 0)
        return false;

    unsigned n = irida_syzygy_piece_count(board);
    if (n == 0 || n > TB_LARGEST)
        return false;

    uint64_t w = bitboard_white(board);
    uint64_t blk = bitboard_black(board);
    if (w & blk)
        return false;

    uint64_t k = bitboard_kings(board);
    uint64_t q = bitboard_queens(board);
    uint64_t r = bitboard_rooks(board);
    uint64_t bs = bitboard_bishops(board);
    uint64_t n2 = bitboard_knights(board);
    uint64_t p = bitboard_pawns(board);

    if (__builtin_popcountll(k) != 2u)
        return false;
    if (__builtin_popcountll(k & w) != 1u || __builtin_popcountll(k & blk) != 1u)
        return false;

    if ((k & q) || (k & r) || (k & bs) || (k & n2) || (k & p))
        return false;
    if ((q & r) || (q & bs) || (q & n2) || (q & p))
        return false;
    if ((r & bs) || (r & n2) || (r & p))
        return false;
    if ((bs & n2) || (bs & p) || (n2 & p))
        return false;

    if (p & SYZYGY_BOARD_FILE_EDGE)
        return false;

    if ((w | blk) != (k | q | r | bs | n2 | p))
        return false;

    return true;
}

int irida_syzygy_probe_wdl(Board* board, bool use_rule50)
{
    if (!g_syzygy_loaded)
        return SYZYGY_PROBE_FAILED;

    if (!syzygy_fathom_layout_ok(board))
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
    /* Fathom WDL is from the side-to-move perspective (see tbprobe.h). */
    return (wdl < 5) ? score_tbl[wdl] : 0;
}

bool irida_syzygy_probe_root(Board* board, bool use_rule50, Move* best_move_out)
{
    if (!g_syzygy_loaded || !best_move_out) {
        return false;
    }
    /* Fathom only has tables through TB_LARGEST pieces; skip root work otherwise. */
    if (TB_LARGEST == 0 || irida_syzygy_piece_count(board) > TB_LARGEST) {
        return false;
    }
    if (board->castling_rights != 0) {
        return false;
    }
    if (!syzygy_fathom_layout_ok(board))
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

    if (!ok || root_moves.size == 0){
        return false;
    }

    /* Pick first (best) root move. TbRootMove has TbMove move (from/to encoded). */
    struct TbRootMove* rm = &root_moves.moves[0];
    unsigned from = TB_MOVE_FROM(rm->move);
    unsigned to   = TB_MOVE_TO(rm->move);
    unsigned promotion = TB_MOVE_PROMOTES(rm->move);

    // TODO: check that fathom's promotion encoding matches ours
    *best_move_out = castro_MoveEncode(from, to, promotion, FLAG_NORMAL);

    return true;
}
