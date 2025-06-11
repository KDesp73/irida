#ifndef TESTS_H
#define TESTS_H

#include "test_evaluation_mirroring.h"
int test_evaluation_mirroring(char* white_fen);

#include "test_evaluation_known_position.h"
int test_evaluation_known_position(char* postition, int eval);

#include "test_evaluation_gradient.h"
int test_evaluation_gradient(char* worse, char* better);


#endif // TESTS_H
