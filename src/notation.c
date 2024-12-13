#include "notation.h"
#include "square.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/**
 * Maps a piece character to its corresponding bitboard index.
 * Uppercase characters are white pieces, lowercase are black.
 */
int char_to_piece_index(char c) {
    switch (c) {
        case 'P': return INDEX_WHITE_PAWN;
        case 'N': return INDEX_WHITE_KNIGHT;
        case 'B': return INDEX_WHITE_BISHOP;
        case 'R': return INDEX_WHITE_ROOK;
        case 'Q': return INDEX_WHITE_QUEEN;
        case 'K': return INDEX_WHITE_KING;
        case 'p': return INDEX_BLACK_PAWN;
        case 'n': return INDEX_BLACK_KNIGHT;
        case 'b': return INDEX_BLACK_BISHOP;
        case 'r': return INDEX_BLACK_ROOK;
        case 'q': return INDEX_BLACK_QUEEN;
        case 'k': return INDEX_BLACK_KING;
        default: return -1;
    }
}


void FenImport(Board *board, const char *fen)
{
    memset(board, 0, sizeof(Board));

    const char *ptr = fen;
    int rank = 0;
    int file = 0;

    // Parse board state
    while (*ptr && *ptr != ' ') {
        if (*ptr == '/') {
            rank++;
            file = 0;
        } else if (isdigit(*ptr)) {
            file += *ptr - '0';
        } else {
            int piece_index = char_to_piece_index(*ptr);
            if (piece_index == -1) {
                return; // Invalid piece character
            }

            int square = rank * 8 + file; // Calculate square index (0 = a8, ..., 63 = h1)
            int inverted = (7 - (square/ 8)) * 8 + (square % 8);
            board->bitboards[piece_index] |= (1ULL << inverted);
            file++;
        }
        ptr++;
    }

    if (*ptr != ' ') return;
    ptr++;

    // Parse active color
    board->state.turn = (*ptr == 'w') ? 1 : 0;
    ptr += 2;

    // Parse castling rights
    board->state.castling_rights = 0;
    while (*ptr && *ptr != ' ') {
        switch (*ptr) {
            case 'K': board->state.castling_rights |= 0x1; break; // White kingside
            case 'Q': board->state.castling_rights |= 0x2; break; // White queenside
            case 'k': board->state.castling_rights |= 0x4; break; // Black kingside
            case 'q': board->state.castling_rights |= 0x8; break; // Black queenside
        }
        ptr++;
    }

    if (*ptr != ' ') return;
    ptr++;

    // Parse en passant square
    if (*ptr == '-') {
        board->enpassant_square = 64; // No en passant
        ptr++;
    } else {
        char file_char = ptr[0];
        char rank_char = ptr[1];
        if (file_char < 'a' || file_char > 'h' || rank_char < '1' || rank_char > '8') {
            return; // Invalid FEN
        }
        // Convert 'a1'-'h8' to 0-63 (bitboard index)
        board->enpassant_square = (7 - (rank_char - '1')) * 8 + (file_char - 'a');
        ptr += 2;
    }

    if (*ptr != ' ') return;
    ptr++;

    // Parse halfmove clock
    board->state.halfmove = atoi(ptr);
    while (*ptr && *ptr != ' ') ptr++;

    if (*ptr != ' ') return;
    ptr++;

    // Parse fullmove number
    board->state.fullmove = atoi(ptr);
}

void FenExport(const Board* board, char buffer[])
{
    char* ptr = buffer;

    // Loop through each rank
    for (int rank = 7; rank >= 0; rank--) {
        int empty_squares = 0;

        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            _Bool piece_found = 0;

            for (int piece = 0; piece < PIECE_TYPES; piece++) {
                if (board->bitboards[piece] & (1ULL << square)) {
                    if (empty_squares > 0) {
                        *ptr++ = '0' + empty_squares;
                        empty_squares = 0;
                    }
                    *ptr++ = PIECES[piece];
                    piece_found = 1;
                    break;
                }
            }

            if (!piece_found) {
                empty_squares++;
            }
        }

        if (empty_squares > 0) {
            *ptr++ = '0' + empty_squares;
        }

        if (rank > 0) {
            *ptr++ = '/';
        }
    }

    *ptr++ = ' ';

    // Side to move
    *ptr++ = (board->state.turn) ? 'w' : 'b';
    *ptr++ = ' ';

    // Castling rights
    _Bool castling_written = 0;
    if (board->state.castling_rights & 0x1) { *ptr++ = 'K'; castling_written = 1; }
    if (board->state.castling_rights & 0x2) { *ptr++ = 'Q'; castling_written = 1; }
    if (board->state.castling_rights & 0x4) { *ptr++ = 'k'; castling_written = 1; }
    if (board->state.castling_rights & 0x8) { *ptr++ = 'q'; castling_written = 1; }
    if (!castling_written) {
        *ptr++ = '-';
    }
    *ptr++ = ' ';

    // En passant square
    if (board->enpassant_square < BOARD_SIZE) {
        int file = board->enpassant_square % 8;
        int rank = board->enpassant_square / 8;
        *ptr++ = 'a' + file;
        *ptr++ = '1' + rank;
    } else {
        *ptr++ = '-';
    }
    *ptr++ = ' ';

    // Halfmove clock
    ptr += sprintf(ptr, "%zu ", board->state.halfmove);

    // Fullmove number
    ptr += sprintf(ptr, "%zu", board->state.fullmove);

    // Null-terminate the buffer
    *ptr = '\0';
}

void MoveToSan(Board board, Move move, san_move_t* san)
{
    Square from, to;
    uint8_t promotion, flag;
    MoveDecode(move, &from, &to, &promotion, &flag);

    char fen[64];
    FenExport(&board, fen);
    board_t board_;
    fen_import(&board_, fen);
    square_t from_square, to_square;
    SquareToSquareT(&from_square, from);
    SquareToSquareT(&to_square, to);

    move_to_san(&board_, from_square, to_square, PromotionToChar(promotion), san);
}

Move SanToMove(Board board, san_move_t san)
{
    char fen[64];
    FenExport(&board, fen);
    board_t board_;
    fen_import(&board_, fen);

    square_t from, to;
    char promotion;

    san_to_move(&board_, san, &from, &to, &promotion);
    return SquaresToMove(from, to, CharToPromotion(promotion), FLAG_NORMAL);
}
