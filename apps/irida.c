#include "castro.h"
#include "core.h"
#include "eval.h"
#include "moveordering.h"
#include "nnue.h"
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
    irida_EngineInit(&engine);
    strncpy(engine.name, IRIDA_UCI_NAME, sizeof(engine.name) - 1);
    engine.name[sizeof(engine.name) - 1] = '\0';
    engine.eval = irida_EvalNNUE;
    engine.search = irida_Search;
    engine.order = irida_OrderMoves;

    irida_UciMain();

    castro_BoardFree(&engine.board);
}
