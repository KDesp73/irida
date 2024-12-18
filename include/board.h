#ifndef ENGINE_BOARD_H
#define ENGINE_BOARD_H


/*--------------------------------------------.
| Bitboard representation of the chess board. |
`--------------------------------------------*/

#include "hashing.h"

#include "square.h"
#include <stddef.h>
#include <stdint.h>
#include "bitboard.h"


#define BOARD_SIZE 8
#define PIECE_TYPES 12
typedef struct {
    Bitboard bitboards[PIECE_TYPES];
    Square enpassant_square;
    bool turn;
    uint8_t castling_rights;
    size_t halfmove;
    size_t fullmove;
    HashTable history;
} Board;

#define PIECES "pnbrqkPNBRQK"
enum {
    INDEX_BLACK_PAWN,
    INDEX_BLACK_KNIGHT,
    INDEX_BLACK_BISHOP,
    INDEX_BLACK_ROOK,
    INDEX_BLACK_QUEEN,
    INDEX_BLACK_KING,
    INDEX_WHITE_PAWN,
    INDEX_WHITE_KNIGHT,
    INDEX_WHITE_BISHOP,
    INDEX_WHITE_ROOK,
    INDEX_WHITE_QUEEN,
    INDEX_WHITE_KING,
};

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
    PROMOTION_QUEEN,
    PROMOTION_ROOK,
    PROMOTION_BISHOP,
    PROMOTION_KNIGHT
} Promotion;

typedef enum {
    COLOR_NONE = -1,
    COLOR_BLACK = 0,
    COLOR_WHITE,
} Color;


char PromotionToChar(uint8_t promotion);
uint8_t CharToPromotion(char promotion);

void BoardInitFen(Board* board, const char* fen);
void BoardFree(Board* board);

Bitboard GetWhite(const Board* board);
Bitboard GetBlack(const Board* board);
Bitboard GetEnemyColor(const Board *board, Color color);
Bitboard GetEnemy(const Board* board);
Bitboard GetEmpty(const Board* board);

int HasCastlingRights(const Board* board, uint8_t castling_rights);
void RevokeCastlingRights(Board* board, uint8_t castling_rights);

bool IsSquareAttacked(const Board* board, Square square, Color color);
bool IsSquareEmpty(const Board* board, Square square);
bool IsSquareOccupiedBy(const Board* board, Square square, Color color);

size_t NumberOfPieces(const Board* board, Color color);

bool IsCheckmate(Board* board);
bool IsStalemate(Board* board);
bool IsInsufficientMaterial(Board* board);
bool IsThreefoldRepetition(Board* board);
bool IsInCheck(const Board* board);


void BoardPrintSquares(const Board* board, Square* squares, size_t count);
void BoardPrintBitboard(const Board* board, Bitboard highlight);
void BoardPrint(const Board* board, Square first, ...);
void BoardPrintBitboards(Board board);

Board BoardCopy(const Board* board);

#endif // ENGINE_BOARD_H
