#ifndef PERFT_H_
#define PERFT_H_

#include <stdint.h>
#include "board.h"

uint64_t perft(int depth);
uint64_t helper(Board* board, int depth);

#endif