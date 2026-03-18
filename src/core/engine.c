#include "castro.h"
#include "core.h"
#include "tt.h"
#include "version.h"
#include <string.h>

void EngineInit(Engine* engine)
{
    strncpy(engine->name, ENGINE_NAME, sizeof(engine->name));
    strncpy(engine->author, ENGINE_AUTHOR, sizeof(engine->author));

    // TODO: bake masks and magic into castro
    castro_InitMasks();
    castro_InitMagic();
    pesto_init();
    init_mvv_lva();
    tt_init(16);
}
