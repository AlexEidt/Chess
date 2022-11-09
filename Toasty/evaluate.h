#ifndef EVALUATE_H_
#define EVALUATE_H_

#include "bitboard.h"
#include "board.h"

#define ABS(x) (__builtin_abs(x))

#define PAWN_VALUE 100
#define KNIGHT_VALUE 350
#define BISHOP_VALUE 350
#define ROOK_VALUE 525
#define QUEEN_VALUE 1000
#define KING_VALUE 220

#define TOTAL_VALUE 4250

#define BISHOP_BONUS (BISHOP_VALUE / 2)
#define KING_QUADRANT_BONUS 4
#define KING_CORNER_BONUS 4
#define KING_DISTANCE_BONUS 8
#define PIECE_SQUARE_BONUS 8
#define STACKED_PAWN_BONUS 2
#define ISOLATED_PAWN_BONUS 4

#define PROMOTION_BONUS 32768
#define CAPTURE_BONUS 8
#define PAWN_ATTACK_PENALTY 4096

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