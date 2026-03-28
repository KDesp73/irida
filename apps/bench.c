#include "castro.h"
#include "IncludeOnly/logging.h"
#include "core.h"
#include "eval.h"
#include "moveordering.h"
#include "nnue.h"
#include "search.h"
#include "tt.h"
#include "uci.h"
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "IncludeOnly/kv.h"
#include <inttypes.h>

/* --- Style Definitions --- */
#define C_RESET  "\033[0m"
#define C_BOLD   "\033[1m"
#define C_TITLE  "\033[38;5;111m"  /* Arctic Blue */
#define C_SUB    "\033[38;5;244m"  /* Muted Grey */
#define C_NUM    "\033[38;5;80m"   /* Glacier Teal */
#define C_TIME   "\033[38;5;251m"  /* Snow White */
#define C_RATE   "\033[38;5;215m"  /* Aurora Orange */

#include "shared.h"

static double now_s(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

static void print_banner(const char* search, const char* eval, int depth) {
    printf("\n %s%sCASTRO ENGINE BENCH%s %s[%s + %s]%s\n", 
        C_BOLD, C_TITLE, C_RESET, C_SUB, search, eval, C_RESET);
    printf(" %sTarget Depth: %s%d%s\n", C_SUB, C_NUM, depth, C_RESET);
    printf(" %s──────────────────────────────────────────────────────────────────%s\n", C_SUB, C_RESET);
    printf(" %-24s %14s %12s %12s\n", "Position", "Nodes", "Time", "NPS");
}

void run_benchmark(SearchFn search, EvalFn eval, OrderFn order) {
    const char* test_positions[] = {
        STARTING_FEN,
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
    };
    const char* pos_names[] = { "Starting Pos", "Kiwipete", "Endgame" };

    uint64_t total_nodes = 0;
    double total_time = 0;

    print_banner(kv_get("search", "default"), kv_get("eval", "default"), g_searchConfig.maxDepth);

    for (int i = 0; i < 3; i++) {
        Board board;
        castro_BoardInitFen(&board, test_positions[i]);
        
        tt_clear(); 
        g_searchStats.nodes = 0;
        g_searchStats.qnodes = 0;

        double t0 = now_s();
        search(&board, eval, order, &g_searchConfig); 
        double elapsed = now_s() - t0;
        
        uint64_t n = g_searchStats.nodes + g_searchStats.qnodes;
        double nps = elapsed > 0 ? (double)n / elapsed : 0;

        printf(" %s›%s %-22s %s%14" PRIu64 "%s %s%11.3fs%s %s%12.0f%s\n",
            C_SUB, C_RESET, pos_names[i],
            C_NUM, n, C_RESET,
            C_TIME, elapsed, C_RESET,
            C_RATE, nps, C_RESET);

        total_nodes += n;
        total_time += elapsed;
        castro_BoardFree(&board);
    }

    printf(" %s──────────────────────────────────────────────────────────────────%s\n", C_SUB, C_RESET);
    printf(" %s%sOVERALL%s %21s%s%14" PRIu64 "%s %s%11.3fs%s %s%12.0f%s\n\n",
        C_BOLD, C_TITLE, C_RESET, "",
        C_NUM, total_nodes, C_RESET,
        C_TIME, total_time, C_RESET,
        C_RATE, total_time > 0 ? (total_nodes / total_time) : 0, C_RESET);
}

int main(int argc, char** argv)
{
    EngineInit(&engine);

    kv_parse(argc, argv);
    const char* search = kv_get("search", "id_ab_q_mo_tt_nmp");
    const char* eval = kv_get("eval", "handcrafted");
    int depth = strtol(kv_get("depth", "6"), NULL, 10);

    SearchFn searchfn = NULL;
    EvalFn evalfn = NULL;

    for(size_t i = 0; i < sizeof(search_variants) / sizeof(search_variants[0]); ++i) {
        if(strcmp(search, search_variants[i].name))
            searchfn = search_variants[i].fn;
    }

    if(!strcmp("nnue", eval)) {
        if(!nnue_load(NNUE_DEFAULT_PATH)){
            ERRO("Could not load nnue %s\n", NNUE_DEFAULT_PATH);
            return 1;
        }
        evalfn = nnue_eval;
    } else if(!strcmp("handcrafted", eval)) evalfn = evaluation;
    else if(!strcmp("material", eval)) evalfn = material_eval;
    else evalfn = evaluation;

    engine.search = searchfn;
    engine.eval = evalfn;
    engine.order = order_moves;
    g_searchConfig.maxDepth = depth;

    run_benchmark(searchfn, evalfn, order_moves);

    return 0;
}
