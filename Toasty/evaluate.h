#ifndef EVALUATE_H_
#define EVALUATE_H_

#include "bitboard.h"
#include "board.h"

#define ABS(x) (__builtin_abs(x))

#define PAWN_VALUE 100
#define KNIGHT_VALUE 300
#define BISHOP_VALUE 300
#define ROOK_VALUE 500
#define QUEEN_VALUE 900
#define KING_VALUE 200

#define TOTAL_VALUE (PAWN_VALUE + KNIGHT_VALUE + BISHOP_VALUE + ROOK_VALUE + QUEEN_VALUE + KING_VALUE)
#define KING_DISTANCE_BONUS 8
#define KING_CORNER_BONUS 4
#define KING_QUADRANT_BONUS 8

#define CAPTURE_BONUS 10
#define PAWN_ATTACK_PENALTY 256
#define QUEEN_CAPTURE_BONUS 1000000

#define PAWN_STRUCTURE_BONUS 5
#define DEVELOPMENT_BONUS 10
#define KING_SAFETY_BONUS 5

#define CHECKMATE 131072

extern const int PIECE_VALUES[7];
extern const int PST[7][64];
extern const int KING_ENDGAME_PST[64];
extern const Bitboard QUADRANT[64];

int evaluate(Board* board);

int material_eval(Board* board, Piece color);
int pawn_structure_eval(Board* board, Piece color);
int piece_square_eval(Board* board, Piece color);
int pst(Piece piece, Piece color, int index);
int king_safety_eval(Board* board, Piece color);
int mop_up_eval(Board* board, int material, Piece color);

#endif