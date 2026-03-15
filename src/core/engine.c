#include "core.h"
#include "tt.h"
#include <string.h>

void EngineInit(Engine* engine, char* name, char* author)
{
    strncpy(engine->name, name, sizeof(engine->name));
    strncpy(engine->author, author, sizeof(engine->author));

    castro_InitMasks();
    pesto_init();
    init_mvv_lva();
    tt_init(16);
}
