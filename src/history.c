#include "history.h"
#include "hashing.h"
#include "move.h"
#include "square.h"
#include "zobrist.h"
#include <stdio.h>

bool HistoryAddUndo(History* history, const Board* board, uint32_t move)
{
    if (history->count >= MAX_MOVES) return false;

    history->moves[history->count++] = MakeUndo(board, move);

    return true;
}

void HistoryRemove(History* history)
{
    if (history->count == 0) return;
    history->count--;

    for (size_t i = 0; i < history->positions.count; i++) {
        if (history->positions.last_added == history->positions.entries[i].hash) {
            if (history->positions.entries[i].count > 0) {
                history->positions.entries[i].count--;
            }
            break;
        }
    }
}

void UndoPrint(Undo undo)
{
    MovePrint(undo.move);
    printf("Castling: %d\n", undo.castling);
    printf("Halfmove: %zu\n", undo.fiftyMove);
    if(undo.enpassant == 64)
        printf("Enpassant: -\n");
    else
        SQUARE_PRINT(undo.enpassant);
}
