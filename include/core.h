#ifndef ENGINE_CORE_H
#define ENGINE_CORE_H

#include "moveordering.h"
#include "search.h"
#include "eval.h"

// @module core
// @desc Engine core: Engine struct, init, and global engine instance.

// @const ENGINE_NAME
// @desc Default UCI id before apps/irida.c may override with IRIDA_UCI_NAME.
#define ENGINE_NAME "irida"

// @const ENGINE_AUTHOR
#define ENGINE_AUTHOR "Konstantinos Despoinidis (KDesp73)"

// @const PIECE_VALUES
// @desc Piece values (0–5 = PAWN..KING) for display or tuning.
static const int PIECE_VALUES[6] = {100, 320, 330, 500, 900, 0};

// @struct Engine
// @desc Engine descriptor: name, author, function pointers (search, eval, order), and board.
typedef struct {
    char name[128];
    char author[128];

    SearchFn search; 
    EvalFn eval;
    OrderFn order;

    Board board;
} Engine;

// @function EngineInit
// @desc Initialize engine with name and author; set function pointers and board state.
// @param engine Engine to initialize.
void EngineInit(Engine* engine);

// @var engine
// @desc Global engine instance; expected to be defined in main or startup.
extern Engine engine;

#endif // ENGINE_CORE_H
