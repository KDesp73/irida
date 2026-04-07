#include "castro.h"
#include "core.h"
#include "tt.h"
#include "version.h"
#include <stdlib.h>
#include <string.h>

void irida_EngineInit(Engine* engine)
{
    strncpy(engine->name, ENGINE_NAME, sizeof(engine->name));
    strncpy(engine->author, ENGINE_AUTHOR, sizeof(engine->author));

    /* Before castro_InitMagic: it uses rand(). Fixed seed ⇒ reproducible runs. */
    srand(0xC6134A75u);

    // TODO: bake masks and magic into castro
    castro_InitMasks();
    castro_InitMagic();
    irida_pesto_init();
    irida_init_mvv_lva();
    irida_ordering_reset();
    irida_tt_init(16);
}
