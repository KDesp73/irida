#ifndef ENGINE_CORE_H
#define ENGINE_CORE_H

#include "moveordering.h"
#include "search.h"
#include "eval.h"

// Piece values (0–5 = PAWN..KING)
static const int PIECE_VALUES[6] = {100, 320, 330, 500, 900, 0};

typedef struct {
    char name[128];
    char author[128];

    SearchFn search; 
    EvalFn eval;
    OrderFn order;

    Board board;
} Engine;

void EngineInit(Engine* engine, char* name, char* author);

// Expected to be defined elsewhere
extern Engine engine;

#endif // ENGINE_CORE_H
