#ifndef ENGINE_RESULT_H
#define ENGINE_RESULT_H

#include "board.h"
#include <stdbool.h>

typedef enum {
    RESULT_NONE = 0,
    RESULT_WHITE_WON,
    RESULT_BLACK_WON,
    RESULT_STALEMATE,
    RESULT_DRAW_BY_REPETITION,
    RESULT_DRAW_DUE_TO_INSUFFICIENT_MATERIAL,
    RESULT_DRAW_DUE_TO_50_MOVE_RULE,
    RESULT_COUNT
} Result ;

static const char result_score[][8] = {
    "*",
    "1-0",
    "0-1",
    "1/2-1/2",
    "1/2-1/2",
    "1/2-1/2",
    "1/2-1/2"
};
static const char result_message[][256] = {
    "No result yet",
    "White won",
    "Black won",
    "Stalemate",
    "Draw by repetition",
    "Draw due to insufficient material",
    "Draw due to 50 move rule"
};

Result IsResult(Board* board);
bool IsCheckmate(const Board* board);
bool IsStalemate(const Board* board);
bool IsInsufficientMaterial(const Board* board);
bool IsThreefoldRepetition(Board* board);

#endif // ENGINE_RESULT_H
