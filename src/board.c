#include "board.h"
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "bitboard.h"
#include "move.h"
#include "notation.h"
#include "piece.h"
#include "square.h"
#include <chess/board.h>
#include <chess/piece.h>

Bitboard GetPseudoValidAttacks(const Board* board, Color color)
{
    size_t start = (color) ? 6 : 0;
    Bitboard enemy = GetEnemyColor(board, color);
    Bitboard empty = GetEmpty(board);
    return (color) 
        ? WhitePawnAttacks(board->bitboards[INDEX_WHITE_PAWN], enemy)
        : BlackPawnAttacks(board->bitboards[INDEX_BLACK_PAWN], enemy)
        | KnightAttacks(board->bitboards[start + 1], empty, enemy)
        | BishopAttacks(board->bitboards[start + 2], empty, enemy)
        | RookAttacks(board->bitboards[start + 3], empty, enemy)
        | QueenAttacks(board->bitboards[start + 4], empty, enemy)
        | KingAttacks(board->bitboards[start + 5], empty, enemy)
        ;
}

Board BoardCopy(const Board* board)
{
    Board b = {0};
    b.state = board->state;
    b.enpassant_square = board->enpassant_square;
    memcpy(b.bitboards, board->bitboards, sizeof(b.bitboards));

    return b;
}

void BoardToBoardT(const Board* board, board_t* board_t)
{
    // Initialize the board_t's grid to empty squares (' ')
    for (int rank = 0; rank < 8; ++rank) {
        for (int file = 0; file < 8; ++file) {
            board_t->grid[rank][file] = ' ';
        }
    }

    // Populate the grid from the bitboards
    for (int piece_type = 0; piece_type < PIECE_TYPES; ++piece_type) {
        Bitboard bitboard = board->bitboards[piece_type];
        while (bitboard) {
            // Find the least significant bit set
            Square square = __builtin_ctzll(bitboard);

            // Convert square to rank and file (adjusted for a1 -> 0 convention)
            int rank = square / 8;
            int file = square % 8;

            // Set the corresponding piece in the grid
            board_t->grid[7 - rank][file] = PIECES[piece_type];  // Adjust rank for printing

            // Clear the least significant bit
            bitboard &= bitboard - 1;
        }
    }

    // Copy state
    board_t->state = board->state;

    // Convert en passant square to a human-readable format
    if (board->enpassant_square != 64) {
        int rank = Rank(board->enpassant_square);
        int file = File(board->enpassant_square);

        board_t->enpassant_square[0] = 'a' + file; // Convert file to letter
        board_t->enpassant_square[1] = '8' - rank; // Adjust rank for printing (8-1)
        board_t->enpassant_square[2] = '\0';       // Null terminator
    } else {
        // No en passant square
        board_t->enpassant_square[0] = '-';
        board_t->enpassant_square[1] = '\0';
    }
}

/**
 * Parses a FEN string and initializes the Board structure.
 */
void BoardInitFen(Board *board, const char *fen)
{
    if(fen == NULL) {
        BoardInitFen(board, STARTING_FEN);
        return;
    }

    FenImport(board, fen);

    board->state.error = 0;
    board->state.result = RESULT_NONE;
    init_hash_table(&board->state.history, 1000, fen);
}


_Bool IsSquareOccupiedBy(const Board* board, Square square, Color color)
{
    uint64_t occupied = 0;
    for (int i = color * 6; i < (color + 1) * 6; ++i) {
        occupied |= board->bitboards[i];
    }
    return (occupied & (1ULL << square)) != 0;
}

_Bool IsSquareEmpty(const Board* board, Square square)
{
    for (int i = 0; i < 12; ++i) {
        if (board->bitboards[i] & (1ULL << square)) {
            return 0;
        }
    }
    return 1;
}

Square UpdateEnpassantSquare(Board* board, Move move)
{
    Piece piece = PieceAt(board, GetFrom(move));

    if(tolower(piece.type) != 'p') {
        goto no_enpassant;
    }

    square_t from, to;
    square_from_fr(&from, Rank(GetFrom(move)), File(GetFrom(move)));

    int file_diff = abs((int)from.file - (int)to.file);
    int rank_diff = (int)to.rank - (int)from.rank;

    if(file_diff != 0) goto no_enpassant;

    if(rank_diff != ((piece.color) ? 2 : -2)) {
        goto no_enpassant;
    }

    return ((piece.color) ? 3 : 6) * 8 + to.file;

no_enpassant:
    return 64;
}

uint8_t UpdateCastlingRights(Board* board, Square from)
{
    Piece piece = PieceAt(board, from);
    int color = piece.color;
    uint8_t castling_rights = board->state.castling_rights;

    // Handle rook moves: disable relevant castling rights
    if (piece.type == 'r' || piece.type == 'R') {
        if (color == PIECE_COLOR_WHITE) {
            if (from == 0) {
                castling_rights &= ~CASTLE_WHITE_QUEENSIDE;
            }
            if (from == 7) {
                castling_rights &= ~CASTLE_WHITE_KINGSIDE;
            }
        } else if (color == PIECE_COLOR_BLACK) {
            if (from == 56) {
                castling_rights &= ~CASTLE_BLACK_QUEENSIDE;
            }
            if (from == 63) {
                castling_rights &= ~CASTLE_BLACK_KINGSIDE;
            }
        }
    }

    if (piece.type == 'k') {
        if (color == PIECE_COLOR_BLACK) {
            castling_rights &= ~CASTLE_BLACK_KINGSIDE;
            castling_rights &= ~CASTLE_BLACK_QUEENSIDE;
        }
    } else if (piece.type == 'K') {
        if (color == PIECE_COLOR_WHITE) {
            castling_rights &= ~CASTLE_WHITE_KINGSIDE;
            castling_rights &= ~CASTLE_WHITE_QUEENSIDE;
        }
    }

    return castling_rights;
}

void UpdateHalfmove(Board* board, Move move, size_t piece_count_before, size_t piece_count_after, char piece)
{
    int color = piece_color(piece);
    int direction = (color == PIECE_COLOR_WHITE) ? 1 : -1;
    _Bool is_pawn = tolower(piece) == 'p';
    _Bool is_capture = (piece_count_after < piece_count_before);
    int from_rank = Rank(GetFrom(move));
    int to_rank = Rank(GetTo(move));

    // Check if it's a pawn move
    _Bool is_pawn_advancement = is_pawn && (from_rank == (color == PIECE_COLOR_WHITE ? 7 : 2)) &&
                                (to_rank == from_rank + direction);

    // If it's a pawn move, pawn advancement, or a capture, reset halfmove
    if (is_pawn_advancement || is_capture || is_pawn) {
        board->state.halfmove = 0;
    } else {
        board->state.halfmove++;
    }
}

size_t NumberOfPieces(const Board* board, Color color)
{
    size_t count = 0;

    for (size_t i = 0; i < PIECE_TYPES; i++) {
        if ((color == PIECE_COLOR_BLACK && i < 6) || (color == PIECE_COLOR_WHITE && i >= 6)) {
            count += __builtin_popcountll(board->bitboards[i]);
        }
    }

    return count;
}

Bitboard GetWhite(const Board* board)
{
    return board->bitboards[INDEX_WHITE_PAWN]
         | board->bitboards[INDEX_WHITE_KNIGHT]
         | board->bitboards[INDEX_WHITE_BISHOP]
         | board->bitboards[INDEX_WHITE_ROOK]
         | board->bitboards[INDEX_WHITE_QUEEN]
         | board->bitboards[INDEX_WHITE_KING];
}
Bitboard GetBlack(const Board* board)
{
    return board->bitboards[INDEX_BLACK_PAWN]
         | board->bitboards[INDEX_BLACK_KNIGHT]
         | board->bitboards[INDEX_BLACK_BISHOP]
         | board->bitboards[INDEX_BLACK_ROOK]
         | board->bitboards[INDEX_BLACK_QUEEN]
         | board->bitboards[INDEX_BLACK_KING];
}

Bitboard GetEnemyColor(const Board *board, Color color)
{
    return (color) 
        ? GetBlack(board)
        : GetWhite(board);
}

Bitboard GetEnemy(const Board *board)
{
    return GetEnemyColor(board, board->state.turn);
}

Bitboard GetEmpty(const Board* board)
{
    return ~(GetWhite(board) | GetBlack(board));
}
