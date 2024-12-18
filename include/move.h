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


typedef enum {
    FLAG_NORMAL = 0,
    FLAG_CASTLING,
    FLAG_ENPASSANT,
    FLAG_PAWN_DOUBLE_MOVE,
    FLAG_PROMOTION,
    FLAG_PROMOTION_WITH_CAPTURE,
} Flag;

typedef enum {
    PROMOTION_NONE = 0,
    PROMOTION_KNIGHT,
    PROMOTION_BISHOP,
    PROMOTION_ROOK,
    PROMOTION_QUEEN,
} Promotion;

enum {
    CASTLE_WHITE_KINGSIDE = 0b0001,
    CASTLE_WHITE_QUEENSIDE = 0b0010,
    CASTLE_BLACK_KINGSIDE = 0b0100,
    CASTLE_BLACK_QUEENSIDE = 0b1000,
};

typedef uint32_t Move;

#define MOVES_CAPACITY 256
typedef struct {
    Move list[MOVES_CAPACITY];
    size_t count;
} Moves;
void MovesAppend(Moves* moves, Move move);
void MovesAppendList(Moves* dest, Moves src);

Undo MakeUndo(const Board* board, Move move);

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

// Adds src, dst, promotion and flag withing the current scope
#define MOVE_DECODE(move) \
    Square src, dst; \
    uint8_t promotion, flag; \
    MoveDecode(move, &src, &dst, &promotion, &flag)


_Bool MoveIsValid(const Board* board, Move move, Color color);
Move MoveEncode(Square from, Square to, uint8_t promotion, uint8_t flag);
Move MoveEncodeNames(const char* from, const char* to, uint8_t promotion, uint8_t flag);
void MoveDecode(Move move, Square* from, Square* to, uint8_t* promotion, uint8_t* flag);

void MoveSetFlag(Move* move, Flag flag);
void MoveSetPromotion(Move* move, Promotion promotion);

Bitboard DoMove(Bitboard* current, Move move);
Bitboard UndoMove(Bitboard* current, Move move);

void MakeMove(Board* board, Move move);
void UnmakeMove(Board* board);
bool Castle(Board* board, Move move);
bool Enpassant(Board* board, Move move);

_Bool MoveMake(Board* board, Move move);
void MoveFreely(Board* board, Move move, Color color);
void MovePrint(Move move);
void MoveToString(Move move, char* buffer);

Square GetFrom(Move move);
Square GetTo(Move move);
uint8_t GetPromotion(Move move);
uint8_t GetFlag(Move move);


void UpdateHalfmove(Board* board, Move move, size_t piece_count_before, size_t piece_count_after, char piece);
uint8_t UpdateCastlingRights(Board* board, Square from, Square to);
Square UpdateEnpassantSquare(Board* board, Move move);

Moves BitboardToMoves(Bitboard bitboard, Square from);

void BoardPrintMove(const Board* board, Move move);

#endif // ENGINE_MOVE_H
