#include "history.h"
#include "hashing.h"
#include "move.h"
#include "zobrist.h"

bool HistoryAdd(History* history, const Board* board, uint32_t move)
{
    history->moves[history->count++] = MakeUndo(board, move);
    return UpdateHashTable(&history->positions, CalculateZobristHash(board));
}

void HistoryRemove(History* history)
{
    history->count--;

    for(size_t i = 0; i < history->positions.count; i++){
        if(history->positions.last_added == history->positions.entries[i].hash){
            history->positions.entries[i].count--;
        }
    }
}
