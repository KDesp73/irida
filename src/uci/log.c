#include "tt.h"
#include "uci.h"

void irida_UciSearchReport(int depth, int bestScore, uint64_t timeMs, const char* pvBuf)
{
    const int mateThreshold = MATE_SCORE - MAX_PLY;
    uint64_t total_nodes = g_searchStats.nodes + g_searchStats.qnodes;
    uint64_t nps = timeMs > 0 ? (total_nodes * 1000) / timeMs : 0;
    
    int hashfull = irida_TTHashfull(); 

    irida_UciStdoutLock();
    printf("info depth %d seldepth %d score ", depth, g_searchStats.selDepth);

    if (bestScore > mateThreshold) {
        // Convert ply-to-mate to moves-to-mate
        printf("mate %d ", (MATE_SCORE - bestScore + 1) / 2);
    } else if (bestScore < -mateThreshold) {
        printf("mate %d ", -(MATE_SCORE + bestScore + 1) / 2);
    } else {
        printf("cp %d ", bestScore);
    }

    printf("nodes %llu nps %llu hashfull %d tbhits %llu time %llu pv %s\n",
           (unsigned long long)total_nodes, (unsigned long long)nps, hashfull,
           (unsigned long long)g_searchStats.tbHits, (unsigned long long)timeMs, pvBuf);
    
    fflush(stdout);
    irida_UciStdoutUnlock();
}
