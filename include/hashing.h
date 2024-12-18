#ifndef ENGINE_HASHING_H
#define ENGINE_HASHING_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint64_t hash;
    int count;
} HashEntry;

typedef struct {
    uint64_t last_added;
    HashEntry* entries;
    size_t count;
} HashTable;

void InitHashTable(HashTable* table, const char* starting_fen);
void InitHashTableHash(HashTable* table, uint64_t starting_hash);
_Bool UpdateHashTable(HashTable* table, uint64_t hash);
void FreeHashTable(HashTable* table);


#endif // ENGINE_HASHING_H
