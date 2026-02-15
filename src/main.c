#include "castro.h"
#include "core.h"
#include "eval.h"
#include "search.h"
#include "uci.h"
#include "cli.h"

Engine engine;
UciState uci_state;

int main(int argc, char** argv)
{
    castro_InitMasks();

    EngineInit(&engine, "chess-engine", "KDesp73");
    engine.eval = pesto_eval;
    engine.search = alpha_beta_search;

    if(argc > 1) return CliMain(argc, argv);

    UciMain();

    castro_BoardFree(&engine.board);
}
