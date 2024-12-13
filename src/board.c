#include "board.h"
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "move.h"
#include "notation.h"
#include "piece.h"
#include "square.h"
#include <chess/board.h>
#include <chess/piece.h>

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

/**
 * Prints a Board structure for debugging purposes.
 */
void BoardPrint(Board board)
{
    for (int rank = 0; rank <= 7; rank++) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            char piece = '.';
            for (int i = 0; i < PIECE_TYPES; i++) {
                if (board.bitboards[i] & (1ULL << square)) {
                    piece = "pnbrqkPNBRQK"[i];
                    break;
                }
            }
            printf("%c ", piece);
        }
        printf("\n");
    }
    printf("\n");
    printf("Active color: %c\n", board.state.turn ? 'w' : 'b');
    printf("Castling rights: %x\n", board.state.castling_rights);
    printf("En passant square: %d\n", board.enpassant_square);
    printf("Halfmove clock: %zu\n", board.state.halfmove);
    printf("Fullmove number: %zu\n", board.state.fullmove);
}
void BoardPrintBitboards(Board board)
{
    for(size_t i = 0; i < PIECE_TYPES; i++){
        printf("%2zu) %c ", i, PIECES[i]);
        Uint64Print(board.bitboards[i]);
    }
}


static inline _Bool IsSquareOccupiedBy(const Board* board, Square square, Color color)
{
    uint64_t occupied = 0;
    for (int i = color * 6; i < (color + 1) * 6; ++i) {
        occupied |= board->bitboards[i];
    }
    return (occupied & (1ULL << square)) != 0;
}

static inline _Bool IsSquareEmpty(const Board* board, Square square)
{
    for (int i = 0; i < 12; ++i) {
        if (board->bitboards[i] & (1ULL << square)) {
            return 0;
        }
    }
    return 1;
}

static inline _Bool IsSquareAttacked(Board board, Square square, Color color)
{
    if (!IsSquareValid(square)) {
        return 0;
    }

    // Check for pawn attacks
    int direction = (color == PIECE_COLOR_WHITE) ? -1 : 1;
    Square left_attack = square + direction * BOARD_SIZE - 1;
    Square right_attack = square + direction * BOARD_SIZE + 1;

    if (IsSquareValid(left_attack) && board.bitboards[color * 6 + 0] & (1ULL << left_attack)) {
        return 1;
    }
    if (IsSquareValid(right_attack) && board.bitboards[color * 6 + 0] & (1ULL << right_attack)) {
        return 1;
    }

    // Check for knight attacks
    for (int i = 0; i < 8; ++i) {
        Square knight_square = square + KNIGHT_OFFSETS[i];
        if (IsSquareValid(knight_square) && (board.bitboards[color * 6 + 1] & (1ULL << knight_square))) {
            return 1;
        }
    }

    // Check for bishop and queen diagonal attacks
    for (int i = 0; i < 4; ++i) {
        Square current = square;
        while (1) {
            current += BISHOP_OFFSETS[i];
            if (!IsSquareValid(current)) {
                break;
            }
            if (!IsSquareEmpty(&board, current)) {
                if (board.bitboards[color * 6 + 2] & (1ULL << current) || // Bishop
                    board.bitboards[color * 6 + 4] & (1ULL << current)) { // Queen
                    return 1;
                }
                break;
            }
        }
    }

    // Check for rook and queen straight attacks
    for (int i = 0; i < 4; ++i) {
        Square current = square;
        while (1) {
            current += ROOK_OFFSETS[i];
            if (!IsSquareValid(current)) {
                break;
            }
            if (!IsSquareEmpty(&board, current)) {
                if (board.bitboards[color * 6 + 3] & (1ULL << current) || // Rook
                    board.bitboards[color * 6 + 4] & (1ULL << current)) { // Queen
                    return 1;
                }
                break;
            }
        }
    }

    // Check for king attacks
    for (int i = 0; i < 8; ++i) {
        Square king_square = square + KING_OFFSETS[i];
        if (IsSquareValid(king_square) && (board.bitboards[color * 6 + 5] & (1ULL << king_square))) {
            return 1;
        }
    }

    return 0;
}

static inline _Bool IsKingInCheck(Board board, Color color) {
    uint64_t king_bitboard = board.bitboards[!color * 6 + 5];
    if (king_bitboard == 0) {
        fprintf(stderr, "No king found for color %d\n", color);
        return 0;
    }

    Square king_square = __builtin_ctzll(king_bitboard); // Get index of the first set bit
    return IsSquareAttacked(board, king_square, 1 - color);
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

Square* SquareIsAttackedBy(const Board* board, Square square, int attacked_by, size_t* count)
{
    *count = 0;
    static Square attackedSquares[64];  // Array to hold attacked squares

    // Define bitboard for the attacking piece
    uint64_t attackedBitboard = 0;

    switch (attacked_by) {
        case PIECE_COLOR_WHITE:
        case PIECE_COLOR_BLACK:
            break;
        default:
            return NULL;
    }

    // Store attacked squares in attackedSquares array
    for (int i = 0; i < 64; i++) {
        if (attackedBitboard & (1ULL << i)) {
            attackedSquares[*count] = (Square)i;
            (*count)++;
        }
    }

    return attackedSquares;
}

Square* SquareIsAccessibleBy(const Board* board, Square square, char piece, size_t* count)
{
    *count = 0;
    static Square accessibleSquares[64];  // Array to hold accessible squares

    int color = piece_color(piece); 
    uint64_t accessibleBitboard = 0;

    switch (piece) {
        case 'R':
        case 'r':  // Rook
            accessibleBitboard = GenerateRookMoves(board, square, color);
            break;
        case 'B':
        case 'b':  // Bishop
            accessibleBitboard = GenerateBishopMoves(board, square, color);
            break;
        case 'N':
        case 'n':  // Knight
            accessibleBitboard = GenerateKnightMoves(board, square, color);
            break;
        case 'Q':
        case 'q':  // Queen
            accessibleBitboard = GenerateQueenMoves(board, square, color);
            break;
        case 'K':
        case 'k':  // King
            accessibleBitboard = GenerateKingMoves(board, square, color);
            break;
        case 'P':
        case 'p':  // Pawn
            accessibleBitboard = GeneratePawnMoves(board, square, color);
            break;
        default:
            return NULL;
    }

    // Store accessible squares in accessibleSquares array
    for (int i = 0; i < 64; i++) {
        if (accessibleBitboard & (1ULL << i)) {
            accessibleSquares[*count] = (Square)i;
            (*count)++;
        }
    }

    return accessibleSquares;
}

Square* AttackPathToKing(Board* board, Square king, Square attacker, size_t* path_count)
{
    *path_count = 0;
    static Square path[64];

    char attackerPiece = PieceAt(board, attacker).type;

    if (attackerPiece == 'r' || attackerPiece == 'q') {
        if (attacker % 8 == king % 8) {  // Same column
            int step = (king < attacker) ? -1 : 1;
            for (int i = attacker + step; i != king; i += step) {
                path[*path_count] = (Square)i;
                (*path_count)++;
            }
        } else if (attacker / 8 == king / 8) {  // Same row
            int step = (king < attacker) ? -1 : 1;
            for (int i = attacker + step; i != king; i += step) {
                path[*path_count] = (Square)i;
                (*path_count)++;
            }
        }
    } else if (attackerPiece == 'b' || attackerPiece == 'q') {
        int stepRow = (king / 8 > attacker / 8) ? 1 : -1;
        int stepCol = (king % 8 > attacker % 8) ? 1 : -1;
        int i = attacker + stepRow * 8 + stepCol;
        while (i != king) {
            path[*path_count] = (Square)i;
            (*path_count)++;
            i += stepRow * 8 + stepCol;
        }
    }

    return path;
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

