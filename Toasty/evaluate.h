#ifndef EVALUATE_H_
#define EVALUATE_H_

#include "board.h"

int evaluate(Board* board);

int evaluate_pawns(Board* board, Piece color);
int piece_square_eval(Board* board);
int mop_up_eval(Board* board);

#endif