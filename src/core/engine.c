/*
 * Theory: Engine descriptor.
 *
 * The engine is a single global struct holding name, author, and function
 * pointers for search, evaluation, and move ordering. EngineInit only sets
 * the name and author; the callers (main) assign the actual search/eval/order
 * implementations. This allows swapping in different eval (e.g. NNUE vs PeSTO)
 * and search/ordering without changing the UCI/CLI layer.
 */
#include "core.h"
#include <string.h>

void EngineInit(Engine* engine, char* name, char* author)
{
    strncpy(engine->name, name, sizeof(engine->name));
    strncpy(engine->author, author, sizeof(engine->author));
}
