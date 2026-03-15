#include "castro.h"
#include "IncludeOnly/logging.h"
#include "core.h"
#include "eval.h"
#include "moveordering.h"
#include "nnue.h"
#include "search.h"
#include "uci.h"
#include <stdint.h>

Engine engine = {0};
UciState uci_state = {0};
SearchConfig g_searchConfig = {
    .maxDepth = 8,
    .timeLimitMs = 0 // Infinite
};
SearchStats g_searchStats = {0};

void run_benchmark(SearchFn search, EvalFn eval, OrderFn order) {
    const char* test_positions[] = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", // Startpos
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", // Kiwipete (High complexity)
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", // Endgame / Zugzwang test
    };

    uint64_t total_nodes = 0;
    double total_time = 0;

    for (int i = 0; i < 3; i++) {
        Board board;
        castro_BoardInitFen(&board, test_positions[i]);
        
        search(&board, eval, order, &g_searchConfig); 
        
        total_nodes += (g_searchStats.nodes + g_searchStats.qnodes);
        total_time += search_elapsed_ms();
    }

    printf("Benchmark Results:\n");
    printf("Total Nodes: %lu\n", total_nodes);
    printf("NPS: %.0f\n", (total_nodes / (total_time / 1000.0)));
}

int main(int argc, char** argv)
{
    if(!nnue_load(NNUE_DEFAULT_PATH)){
        ERRO("Could not load nnue %s\n", NNUE_DEFAULT_PATH);
        return 1;
    }
    EngineInit(&engine);

    run_benchmark(negamax_id_ab, nnue_eval, order_moves);
    run_benchmark(negamax_id_ab_q_mo, nnue_eval, order_moves);
    run_benchmark(negamax_id_ab_q_mo_tt, nnue_eval, order_moves);
    run_benchmark(negamax_id_ab_q_mo_tt_nmp, nnue_eval, order_moves);

    return 0;
}
