#ifndef ENGINE_MOVE_H
#define ENGINE_MOVE_H

/*--------------------------------------.
| Bitboard move representation.         |
| Moves are 32-bit integers             |
| where bits:                           |
| - 0-5 represent the source            |
| - 6-11 represent the destination      |
| - 12-15 represent the promotion piece |
| - 16-21 represent the flags           |
`--------------------------------------*/

#include "board.h"
#include "square.h"

typedef uint32_t Move;

#define KNIGHT_OFFSETS_COUNT 8
const static int KNIGHT_OFFSETS[] = {
    -17, -15, -10, -6, 6, 10, 15, 17
};
#define BISHOP_OFFSETS_COUNT 4
const static int BISHOP_OFFSETS[] = {
    -9, -7, 7, 9
};
#define ROOK_OFFSETS_COUNT 4
const static int ROOK_OFFSETS[] = {
    -8, -1, 1, 8
};
#define KING_OFFSETS_COUNT 8
const static int KING_OFFSETS[] = {
    -9, -8, -7, -1, 1, 7, 8, 9
};


_Bool MoveIsValid(const Board* board, Move move, Color color);
Move MoveEncode(Square from, Square to, uint8_t promotion, uint8_t flag);
Move MoveEncodeNames(const char* from, const char* to, uint8_t promotion, uint8_t flag);
void MoveDecode(Move move, Square* from, Square* to, uint8_t* promotion, uint8_t* flag);
_Bool MoveMake(Board* board, Move move);
void MoveFreely(Board* board, Move move, Color color);
void MovePrint(Move move);

Square GetFrom(Move move);
Square GetTo(Move move);
uint8_t GetPromotion(Move move);
uint8_t GetFlag(Move move);

// Piece Movement Functions
_Bool PieceCanMove(const Board* board, Square from, Square to, Flag* flag);
_Bool CanMovePawn(const Board* board, Square from, Square to, Color color, Flag* flag);
_Bool CanMoveKnight(const Board* board, Square from, Square to, Color color);
_Bool CanMoveBishop(const Board* board, Square from, Square to, Color color);
_Bool CanMoveRook(const Board* board, Square from, Square to, Color color);
_Bool CanMoveQueen(const Board* board, Square from, Square to, Color color);
_Bool CanMoveKing(const Board* board, Square from, Square to, Color color, Flag* flag);

// Move List Generation
void GenerateLegalMoves(const Board* board, Color color, Move* moves, size_t* move_count);
Bitboard GenerateLegalMovesBitboard(const Board* board, Color color);
void GenerateLegalMovesPiece(const Board* board, Square piece, Move* moves, size_t* move_count);

Bitboard GeneratePawnMoves(const Board* board, Square from, Color color);
Bitboard GenerateKingMoves(const Board* board, Square from, Color color);
Bitboard GenerateQueenMoves(const Board* board, Square from, Color color);
Bitboard GenerateBishopMoves(const Board* board, Square from, Color color);
Bitboard GenerateRookMoves(const Board* board, Square from, Color color);
Bitboard GenerateKnightMoves(const Board* board, Square from, Color color);

void UpdateHalfmove(Board* board, Move move, size_t piece_count_before, size_t piece_count_after, char piece);
uint8_t UpdateCastlingRights(Board* board, Square from);
Square UpdateEnpassantSquare(Board* board, Move move);

void MoveToSquares(Move move, square_t* from, square_t* to, uint8_t* promotion, uint8_t* flags);
Move SquaresToMove(square_t from, square_t to, uint8_t promotion, uint8_t flags);

#endif // ENGINE_MOVE_H
