#ifndef ENGINE_HISTORY_H
#define ENGINE_HISTORY_H

#include "hashing.h"
#include "square.h"
#include <stdint.h>


#define MAX_MOVES 2 * 1024
typedef struct {
    uint32_t move;
    uint8_t castling;
    Square enpassant;
    size_t fiftyMove;
    char captured;
} Undo;

#define NULL_UNDO (Undo){.move = NULL_MOVE}

void UndoPrint(Undo undo);

typedef struct {
    HashTable positions;
    Undo moves[MAX_MOVES];
    size_t count;
} History;

void HistoryRemove(History* history);
Undo HistoryGetLast(History history);

#endif // ENGINE_HISTORY_H

