#include "castro.h"
#include "core.h"
#include "eval.h"
#include "moveordering.h"
#include "search.h"
#include "uci.h"

#include "shared.h"

int main(void)
{
    EngineInit(&engine);
    engine.eval = nnue_eval;
    engine.search = search;
    engine.order = order_moves;

    UciMain();

    castro_BoardFree(&engine.board);
}
