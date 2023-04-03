#ifndef OPENING_H_
#define OPENING_H_

#include <stdint.h>
#include "board.h"
#include "move.h"

typedef struct {
    uint64_t hash;
    int count;
    Move move;
} Opening;

extern const Opening openings[];

bool select_opening(Board* board, Move* move);

#endif