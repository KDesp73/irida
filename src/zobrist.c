#include "zobrist.h"
#include "board.h"
#include <chess/board.h>
#include <stdio.h>
#include <chess/zobrist.h>


void InitZobrist()
{
    srand(12345);  // Fixed seed for reproducibility

    // Initialize zobrist_table
    for (int piece = 0; piece < PIECE_TYPES; piece++) {
        for (int rank = 0; rank < BOARD_SIZE; rank++) {
            for (int file = 0; file < BOARD_SIZE; file++) {
                uint64_t high = ((uint64_t)rand() & 0xFFFF) << 48;
                uint64_t mid = ((uint64_t)rand() & 0xFFFF) << 32;
                uint64_t low = ((uint64_t)rand() & 0xFFFFFFFF);
                zobrist_table[piece][rank][file] = high | mid | low;
            }
        }
    }

    // Initialize castling rights
    for (int i = 0; i < CASTLING_OPTIONS; i++) {
        zobrist_castling[i] = ((uint64_t)rand() << 32) | rand();
    }

    // Initialize en passant files
    for (int file = 0; file < BOARD_SIZE; file++) {
        zobrist_en_passant[file] = ((uint64_t)rand() << 32) | rand();
    }

    zobrist_black_to_move = ((uint64_t)rand() << 32) | rand();
}

int PieceToIndex(char piece)
{
    switch (piece) {
        case 'P': return 0;  // White Pawn
        case 'N': return 1;  // White Knight
        case 'B': return 2;  // White Bishop
        case 'R': return 3;  // White Rook
        case 'Q': return 4;  // White Queen
        case 'K': return 5;  // White King
        case 'p': return 6;  // Black Pawn
        case 'n': return 7;  // Black Knight
        case 'b': return 8;  // Black Bishop
        case 'r': return 9;  // Black Rook
        case 'q': return 10; // Black Queen
        case 'k': return 11; // Black King
        default:  return -1; // Invalid piece
    }
}

uint64_t CalculateZobristHash(const Board* board)
{
    uint64_t hash = 0;

    for (int rank = 0; rank < BOARD_SIZE; rank++) {
        for (int file = 0; file < BOARD_SIZE; file++) {
            int square_index = rank * BOARD_SIZE + file;

            char piece = EMPTY_SQUARE;
            for (int i = 0; i < PIECE_TYPES; ++i) {
                if (board->bitboards[i] & (1ULL << square_index)) {
                    piece = "pnbrqkPNBRQK"[i];
                    break;
                }
            }

            if (piece != EMPTY_SQUARE) {
                int piece_index = PieceToIndex(piece);
                if (piece_index >= 0 && piece_index < PIECE_TYPES) {
                    hash ^= zobrist_table[piece_index][rank][file];
                } else {
                    fprintf(stderr, "Invalid piece '%c' at rank %d, file %d\n", piece, rank, file);
                }
            }
        }
    }

    if (board->castling_rights & CASTLE_WHITE_KINGSIDE) hash ^= zobrist_castling[0];
    if (board->castling_rights & CASTLE_WHITE_QUEENSIDE) hash ^= zobrist_castling[1];
    if (board->castling_rights & CASTLE_BLACK_KINGSIDE) hash ^= zobrist_castling[2];
    if (board->castling_rights & CASTLE_BLACK_QUEENSIDE) hash ^= zobrist_castling[3];

    if (board->enpassant_square != 64) {
        int enpassant_file = board->enpassant_square % BOARD_SIZE;
        if (enpassant_file >= 0 && enpassant_file < BOARD_SIZE) {
            hash ^= zobrist_en_passant[enpassant_file];
        } else {
            fprintf(stderr, "Invalid en passant square index: %d\n", board->enpassant_square);
        }
    }

    if (board->turn == 0) {
        hash ^= zobrist_black_to_move;
    }

    return hash;
}
