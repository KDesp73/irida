#ifndef ENGINE_TESTS_H
#define ENGINE_TESTS_H
#include "square.h"

#include "test_piece_at.h"
int test_piece_at(const char* fen, Square square, char expected);

#include "test_name_to_square.h"
int test_name_to_square(const char name[3], int expected);

#include "test_square_to_name.h"
int test_square_to_name(Square square, const char* expected);

#endif // ENGINE_TESTS_H
