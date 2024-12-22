#ifndef ENGINE_NOTATION_H
#define ENGINE_NOTATION_H

/*--------------------------------.
| Contains Fen Import and Export  |
| Pgn Import and Export are       |
| already implemented in libchess |
`-------------------------------*/

#include "move.h"

void FenImport(Board* board, const char* fen);
void FenExport(const Board* board, char fen[]);

#define MAX_HEADER_LENGTH 256

typedef enum {
    RESULT_NONE,
    RESULT_WHITE_WON,
    RESULT_BLACK_WON,
    RESULT_STALEMATE,
    RESULT_DRAW_BY_REPETITION,
    RESULT_DRAW_DUE_TO_INSUFFICIENT_MATERIAL,
    RESULT_DRAW_DUE_TO_50_MOVE_RULE
} result_t ;

#define SCORE_COUNT 7
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

typedef struct {
    char move[16];  // For example, "e4", "Nf3"
} SanMove;

typedef struct {
    char event[MAX_HEADER_LENGTH];
    char site[MAX_HEADER_LENGTH];
    char date[MAX_HEADER_LENGTH];
    char white[MAX_HEADER_LENGTH];
    char black[MAX_HEADER_LENGTH];
    char result[MAX_HEADER_LENGTH];
    char fen[MAX_HEADER_LENGTH];
    SanMove moves[MAX_MOVES];
    size_t move_count;
} Game;

bool move_name(const Board* board, Game* game, const char* move_str);
#define MOVE(board, game, move) \
        move_name(board, game, move)

void GameInit(Game* game, 
    const char* event,
    const char* site,
    const char* white,
    const char* black,
    const char* fen
);

void GameRun(Game game);
void GamePrint(Game game);
void GameAddMove(Game* game, SanMove move);
void GameSetEvent(Game* game, const char* event);
void GameSetSite(Game* game, const char* site);
void GameSetDate(Game* game, const char* date);
void GameSetWhite(Game* game, const char* white);
void GameSetBlack(Game* game, const char* black);
void GameSetFen(Game* game, const char* fen);
void GameSetResult(Game* game, const char* result);

void PgnImport(Game* game, const char* pgn);
void PgnExport(Game* game, char* pgn);
void PgnExportFile(Game* game, const char* path);

void Notate(Board* board, Move move, SanMove* san);
Move SanToMove(Board* board, SanMove san);


#endif // ENGINE_NOTATION_H
