#ifndef ENGINE_GENERATOR_H
#define  ENGINE_GENERATOR_H

#include "bitboard.h"
#include "board.h"
#include "move.h"

/*** Pseudo-Legal Moves ***/

Moves GeneratePseudoLegalMoves(const Board* board);
Bitboard GeneratePseudoLegalAttacks(const Board* board, Color color);

Bitboard GeneratePawnMoves(const Board* board, Bitboard pieces, Color color);
#define GenerateWhitePawnMoves(board) \
    GeneratePawnMoves(board, (board)->bitboards[INDEX_WHITE_PAWN], COLOR_WHITE)
#define GenerateBlackPawnMoves(board) \
    GeneratePawnMoves(board, (board)->bitboards[INDEX_BLACK_PAWN], COLOR_BLACK)

Bitboard GenerateKnightMoves(const Board* board, Bitboard pieces, Color color);
#define GenerateWhiteKnightMoves(board) \
    GenerateKnightMoves(board, (board)->bitboards[INDEX_WHITE_KNIGHT], COLOR_WHITE)
#define GenerateBlackKnightMoves(board) \
    GenerateKnightMoves(board, (board)->bitboards[INDEX_BLACK_KNIGHT], COLOR_BLACK)

Bitboard GenerateBishopMoves(const Board* board, Bitboard pieces, Color color);
#define GenerateWhiteBishopMoves(board) \
    GenerateBishopMoves(board, (board)->bitboards[INDEX_WHITE_BISHOP], COLOR_WHITE)
#define GenerateBlackBishopMoves(board) \
    GenerateBishopMoves(board, (board)->bitboards[INDEX_BLACK_BISHOP], COLOR_BLACK)

Bitboard GenerateRookMoves(const Board* board, Bitboard pieces, Color color);
#define GenerateWhiteRookMoves(board) \
    GenerateRookMoves(board, (board)->bitboards[INDEX_WHITE_ROOK], COLOR_WHITE)
#define GenerateBlackRookMoves(board) \
    GenerateRookMoves(board, (board)->bitboards[INDEX_BLACK_ROOK], COLOR_BLACK)

Bitboard GenerateQueenMoves(const Board* board, Bitboard pieces, Color color);
#define GenerateWhiteQueenMoves(board) \
    GenerateQueenMoves(board, (board)->bitboards[INDEX_WHITE_QUEEN], COLOR_WHITE)
#define GenerateBlackQueenMoves(board) \
    GenerateQueenMoves(board, (board)->bitboards[INDEX_BLACK_QUEEN], COLOR_BLACK)

Bitboard GenerateKingMoves(const Board* board, Bitboard pieces, Color color);
#define GenerateWhiteKingMoves(board) \
    GenerateLegalKingMoves(board, (board)->bitboards[INDEX_WHITE_KING], COLOR_WHITE)
#define GenerateBlackKingMoves(board) \
    GenerateKingMoves(board, (board)->bitboards[INDEX_BLACK_KING], COLOR_BLACK)

/*** Legal Moves ***/

Moves GenerateLegalMoves(const Board* board);
Bitboard GenerateLegalMovesBitboard(const Board* board);

Bitboard GenerateLegalPawnMoves(const Board* board, Bitboard pieces, Color color);
#define GenerateLegalWhitePawnMoves(board) \
    GenerateLegalPawnMoves(board, (board)->bitboards[INDEX_WHITE_PAWN], COLOR_WHITE)
#define GenerateLegalBlackPawnMoves(board) \
    GenerateLegalPawnMoves(board, (board)->bitboards[INDEX_BLACK_PAWN], COLOR_BLACK)

Bitboard GenerateLegalKnightMoves(const Board* board, Bitboard pieces, Color color);
#define GenerateLegalWhiteKnightMoves(board) \
    GenerateLegalKnightMoves(board, (board)->bitboards[INDEX_WHITE_KNIGHT], COLOR_WHITE)
#define GenerateLegalBlackKnightMoves(board) \
    GenerateLegalKnightMoves(board, (board)->bitboards[INDEX_BLACK_KNIGHT], COLOR_BLACK)

Bitboard GenerateLegalBishopMoves(const Board* board, Bitboard pieces, Color color);
#define GenerateLegalWhiteBishopMoves(board) \
    GenerateLegalBishopMoves(board, (board)->bitboards[INDEX_WHITE_BISHOP], COLOR_WHITE)
#define GenerateLegalBlackBishopMoves(board) \
    GenerateLegalBishopMoves(board, (board)->bitboards[INDEX_BLACK_BISHOP], COLOR_BLACK)

Bitboard GenerateLegalRookMoves(const Board* board, Bitboard pieces, Color color);
#define GenerateLegalWhiteRookMoves(board) \
    GenerateLegalRookMoves(board, (board)->bitboards[INDEX_WHITE_ROOK], COLOR_WHITE)
#define GenerateLegalBlackRookMoves(board) \
    GenerateLegalRookMoves(board, (board)->bitboards[INDEX_BLACK_ROOK], COLOR_BLACK)

Bitboard GenerateLegalQueenMoves(const Board* board, Bitboard pieces, Color color);
#define GenerateLegalWhiteQueenMoves(board) \
    GenerateLegalQueenMoves(board, (board)->bitboards[INDEX_WHITE_QUEEN], COLOR_WHITE)
#define GenerateLegalBlackQueenMoves(board) \
    GenerateLegalQueenMoves(board, (board)->bitboards[INDEX_BLACK_QUEEN], COLOR_BLACK)

Bitboard GenerateLegalKingMoves(const Board* board, Bitboard pieces, Color color);
#define GenerateLegalWhiteKingMoves(board) \
    GenerateLegalLegalKingMoves(board, (board)->bitboards[INDEX_WHITE_KING], COLOR_WHITE)
#define GenerateLegalBlackKingMoves(board) \
    GenerateLegalKingMoves(board, (board)->bitboards[INDEX_BLACK_KING], COLOR_BLACK)

#endif //ENGINE_GENERATOR_H
