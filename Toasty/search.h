#ifndef SEARCH_H_
#define SEARCH_H_

#include <stdbool.h>
#include "board.h"
#include "move.h"
#include "hashmap.h"

#define SEARCH_TIMEOUT 3.0

#define INF (1 << 25)
#define MATE (1 << 24)

bool select_move(Board* board, HashMap* hashmap, Move* move);

int iterative_deepening(Board* board, HashMap* hashmap, int min, int max);
int alpha_beta(Board* board, HashMap* hashmap, int depth, int alpha, int beta);
int quiescence(Board* board, HashMap* hashmap, int alpha, int beta);

void order_moves(Board* board, Move* moves, int size);

#endif