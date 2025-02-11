#include "zobrist.h"
#include "bitboard.h"
#include "board.h"
#include <ctype.h>
#include <stdio.h>
#include "move.h"
#include "piece.h"
#include "polyglot.h"
#include "square.h"

uint64_t zobrist_table[PIECE_TYPES][BOARD_SIZE][BOARD_SIZE];
uint64_t zobrist_castling[CASTLING_OPTIONS];
uint64_t zobrist_en_passant[BOARD_SIZE];
uint64_t zobrist_black_to_move;

// https://vigna.di.unimi.it/ftp/papers/xorshift.pdf > page 20
uint64_t rand64()
{
    static uint64_t seed = 270803ULL;

    seed ^= seed >> 12;
    seed ^= seed << 25;
    seed ^= seed >> 27;

    return seed * 2685821657736338717ULL;
}

/*
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
*/

void InitZobrist()
{
    // Initialize zobrist_table
    for (int piece = 0; piece < PIECE_TYPES; piece++) {
        for (int rank = 0; rank < BOARD_SIZE; rank++) {
            for (int file = 0; file < BOARD_SIZE; file++) {
                zobrist_table[piece][rank][file] = rand64();
            }
        }
    }

    // Initialize castling rights
    for (int i = 0; i < CASTLING_OPTIONS; i++) {
        zobrist_castling[i] = rand64();
    }

    // Initialize en passant files
    for (int file = 0; file < BOARD_SIZE; file++) {
        zobrist_en_passant[file] = rand64();
    }

    zobrist_black_to_move = rand64();
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

/*
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
*/

static int remap(int a)
{
    return (a % 6) * 2 + (a / 6);
}

uint64_t CalculateZobristHash(const Board* board)
{
    uint64_t hash = 0;

    for (int pieceIndex = 0; pieceIndex < 12; pieceIndex++) {
        int remappedIndex = remap(pieceIndex);
        uint64_t bitboard = board->bitboards[pieceIndex];
        
        while (bitboard) {
            int square = poplsb(&bitboard);
            
            hash ^= Random64[remappedIndex * 64 + square];  
        }
    }

    if (board->castling_rights & CASTLE_WHITE_KINGSIDE) hash ^= Random64[768];  // White Kingside
    if (board->castling_rights & CASTLE_WHITE_QUEENSIDE) hash ^= Random64[769];  // White Queenside
    if (board->castling_rights & CASTLE_BLACK_KINGSIDE) hash ^= Random64[770];  // Black Kingside
    if (board->castling_rights & CASTLE_BLACK_QUEENSIDE) hash ^= Random64[771];  // Black Queenside

    // TODO: Handle enpassant exclusively when it's possible, not unconditionally
    /*
       If the opponent has performed a double pawn push and there is now a 
       pawn next to it belonging to the player to move then "enpassant" is 
       the entry from RandomEnPassant whose offset is the file of the 
       pushed pawn (counted from 0(=a) to 7(=h)). If this does not apply 
       then enpassant=0.

       Note that this is different from the FEN standard. In the FEN standard 
       the presence of an "en passant target square" after a double pawn push 
       is unconditional.

       Also note that it is irrelevant if the potential en passant capturing 
       move is legal or not (examples where it would not be legal are when 
       the capturing pawn is pinned or when the double pawn push was a 
       discovered check).  
    */

    Square ep = board->enpassant_square;
    if (ep != SQUARE_NONE) {
        Square pawn = ep + ((!board->turn) ? NORTH : SOUTH);
        if (pawn < 0 || pawn >= 64) return 0;

        Square possibleAttackers[] = { pawn + 1, pawn - 1 };
        bool includeEP = false;

        for (size_t i = 0; i < 2; i++) {
            if (possibleAttackers[i] < 0 || possibleAttackers[i] >= 64) continue;

            Piece piece = PieceAt(board, possibleAttackers[i]);
            if (piece.type != ' ' && piece.color == board->turn && tolower(piece.type) == 'p') {
                includeEP = true;
                break;
            }
        }

        if (includeEP) {
            hash ^= Random64[772 + File(ep)];
        }
    }

    if (board->turn == 1) {
        hash ^= Random64[780];
    }

    return hash;
}

