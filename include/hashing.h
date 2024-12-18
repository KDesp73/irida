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
    HashEntry* entries;
    size_t size;
    size_t capacity;
} HashTable;

void InitHashTable(HashTable* table, size_t capacity, const char* starting_fen);
void InitHashTableHash(HashTable* table, size_t capacity, uint64_t starting_hash);
_Bool UpdateHashTable(HashTable* table, uint64_t hash);
void FreeHashTable(HashTable* table);


#endif // ENGINE_HASHING_H
