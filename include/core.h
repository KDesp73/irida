#ifndef ENGINE_CORE_H
#define ENGINE_CORE_H

#include "search.h"
#include "eval.h"

typedef struct {
    char name[128];
    char author[128];

    SearchFn search; 
    EvalFn eval;

    Board board;
} Engine;

void EngineInit(Engine* engine, char* name, char* author);

// Expected to be defined elsewhere
extern Engine engine;

#endif // ENGINE_CORE_H
