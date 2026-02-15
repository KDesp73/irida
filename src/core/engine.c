#include "core.h"
#include <string.h>

void EngineInit(Engine* engine, char* name, char* author)
{
    strncpy(engine->name, name, sizeof(engine->name));
    strncpy(engine->author, author, sizeof(engine->author));
}
