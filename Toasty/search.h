#ifndef SEARCH_H_
#define SEARCH_H_

#include "board.h"
#include "move.h"

#define INFINITY (1 << 25)

void select_move(Board* board, Move* move);

int iterative_deepening(Board* board, int min, int max);
int mtdf(Board* board, int depth, int value);
int alpha_beta(Board* board, int depth, int alpha, int beta);
int quiescence(Board* board, int alpha, int beta);

void order_moves(Board* board, Move* moves, int size);

#endif