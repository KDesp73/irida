#ifndef ENGINE_POLYGLOT_H
#define ENGINE_POLYGLOT_H
#include <stdio.h>
#include <stdint.h>

typedef struct {
    uint64_t zobrist_hash;
    uint16_t move;
    uint16_t weight;
    uint32_t learn;
} PolyglotEntry;

static inline uint16_t LookupBookMove(uint64_t position_hash, const char* book_path)
{
    FILE* book = fopen(book_path, "rb");
    if (!book) return 0;

    PolyglotEntry entry;
    while (fread(&entry, sizeof(PolyglotEntry), 1, book)) {
        if (__builtin_bswap64(entry.zobrist_hash) == position_hash) {
            fclose(book);
            return __builtin_bswap16(entry.move);
        }
    }

    fclose(book);
    return 0; // No book move found
}

#endif // ENGINE_POLYGLOT_H

