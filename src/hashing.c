#include "hashing.h"
#include "notation.h"
#include "zobrist.h"
#include <io/logging.h>
#include <stdio.h>
#include <stdlib.h>

void InitHashTableHash(HashTable* table, size_t capacity, uint64_t starting_hash)
{
    if (!table) {
        ERRO("hash_table_t pointer is NULL.\n");
        exit(EXIT_FAILURE);
    }

    table->entries = calloc(capacity, sizeof(hash_entry_t));
    if (!table->entries) {
        ERRO("Memory allocation failed for hash_table entries.\n");
        exit(EXIT_FAILURE);
    }

    table->size = 0;
    table->capacity = capacity;

    // Adding starting position
    UpdateHashTable(table, starting_hash);
}

void InitHashTable(HashTable* table, size_t capacity, const char* starting_fen)
{
    if (!table) {
        ERRO("hash_table_t pointer is NULL.\n");
        exit(EXIT_FAILURE);
    }

    table->entries = calloc(capacity, sizeof(hash_entry_t));
    if (!table->entries) {
        ERRO("Memory allocation failed for hash_table entries.\n");
        exit(EXIT_FAILURE);
    }

    table->size = 0;
    table->capacity = capacity;

    // Adding starting position
    Board board;
    FenImport(&board, (starting_fen) ? starting_fen : STARTING_FEN);
    UpdateHashTable(table, CalculateZobristHash(&board));
}

void FreeHashTable(HashTable* table)
{
    free(table->entries);
}

_Bool UpdateHashTable(HashTable* table, uint64_t hash)
{
    for (size_t i = 0; i < table->size; i++) {
        if (table->entries[i].hash == hash) {
            table->entries[i].count++;
            return table->entries[i].count >= 3;
        }
    }

    if (table->size < table->capacity) {
        table->entries[table->size].hash = hash;
        table->entries[table->size].count = 1;
        table->size++;
    }

    return 0;
}
