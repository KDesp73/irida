#ifndef ENGINE_BOARD_H
#define ENGINE_BOARD_H


/*--------------------------------------------.
| Bitboard representation of the chess board. |
`--------------------------------------------*/

#ifdef CHESS_DEVELOPMENT
    #error "Build the chess library in release mode"
#endif


#include "square.h"
#include <chess/ui.h>
#include <chess/hashing.h>
#include <chess/zobrist.h>
#include <stddef.h>
#include <stdint.h>

typedef uint64_t Bitboard;

typedef struct {
    Bitboard bitboards[PIECE_TYPES];
    Square enpassant_square;
    state_t state;
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
void BoardPrint(Board board);
void BoardPrintBitboards(Board board);
void BitboardPrint(Bitboard bitboard);
void PiecesPrint(const Board* board, int index);

Bitboard GetWhite(const Board* board);
Bitboard GetBlack(const Board* board);

void Uint32Print(uint32_t value);
void Uint64Print(uint64_t value);

static inline _Bool IsSquareAttacked(Board board, Square square, Color color);
static inline _Bool IsKingInCheck(Board board, Color color);
static inline _Bool IsSquareEmpty(const Board* board, Square square);
static inline _Bool IsSquareOccupiedBy(const Board* board, Square square, Color color);

size_t NumberOfPieces(const Board* board, Color color);

_Bool IsCheckmate(Board* board);
_Bool IsStalemate(Board* board);
_Bool IsInsufficientMaterial(Board* board);
_Bool IsThreefoldRepetition(Board* board);

Square* SquareIsAttackedBy(const Board* board, Square square, int attacked_by, size_t* count);
Square* SquareIsAccessibleBy(const Board* board, Square square, char piece, size_t* count);
Square* AttackPathToKing(Board* board, Square king, Square attacker, size_t* path_count);

void TuiBoardPrintSquares(const Board* board, ui_config_t config, Square* squares, size_t count);
void TuiBoardPrint(const Board* board, ui_config_t config, Square first, ...);

void BoardToBoardT(const Board* board, board_t* board_t);

#endif // ENGINE_BOARD_H
