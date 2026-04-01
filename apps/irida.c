#include "castro.h"
#include "core.h"
#include "eval.h"
#include "moveordering.h"
#include "search.h"
#include "uci.h"
#include <string.h>

#include "shared.h"

/* UCI "id name" / arena headers. Override when A/B testing builds (same VERSION):
 *   make irida-no-pvs   (see targets.mk), or
 *   cc ... -DIRIDA_UCI_NAME=\"irida-no-pvs\" apps/irida.c ...
 */
#ifndef IRIDA_UCI_NAME
#define IRIDA_UCI_NAME "irida"
#endif

int main(void)
{
    EngineInit(&engine);
    strncpy(engine.name, IRIDA_UCI_NAME, sizeof(engine.name) - 1);
    engine.name[sizeof(engine.name) - 1] = '\0';
    engine.eval = evaluation;
    engine.search = search;
    engine.order = order_moves;

    UciMain();

    castro_BoardFree(&engine.board);
}
