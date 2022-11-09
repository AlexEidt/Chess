#ifndef MOVE_H_
#define MOVE_H_

#include <stdint.h>
#include <stdbool.h>
#include "bitboard.h"
#include "board.h"

// 1st Bit: Is move a capture?
// 2nd Bit: Is move a double pawn push?
// 3rd-5th Bits: Is move a pawn promotion? (Bits contain piece that was promoted).
// 6th Bit: Is move an en passant capture?
// 7th Bit: Is move a castle kingside?
// 8th Bit: Is move a castle queenside?
typedef uint8_t Flag;

#define QUIET 0x0
#define CAPTURE 0x1
#define PAWN_DOUBLE 0x2
#define PROMOTION 0x1c
#define EN_PASSANT 0x20
#define CASTLE_KINGSIDE 0x40
#define CASTLE_QUEENSIDE 0x80
#define CASTLE 0xc0

#define IS_CAPTURE(x) (((x) & CAPTURE) != 0)
#define IS_DOUBLE_PUSH(x) (((x) & PAWN_DOUBLE) != 0)
#define IS_PROMOTION(x) (((x) & PROMOTION) != 0)
#define IS_EN_PASSANT(x) (((x) & EN_PASSANT) != 0)
#define IS_CASTLE_KINGSIDE(x) (((x) & CASTLE_KINGSIDE) != 0)
#define IS_CASTLE_QUEENSIDE(x) (((x) & CASTLE_QUEENSIDE) != 0)
#define IS_CASTLE(x) (((x) & CASTLE) != 0)

#define ADD_PROMOTED_PIECE(x) ((x) << 2)
#define PROMOTED_PIECE(x) (((x) & PROMOTION) >> 2)

#define MAX_MOVES 256

typedef struct {
    uint8_t to, from;
    Flag flags;
} Move;

int score_move(Board* board, Move* move);

int extract_moves_pawns(Bitboard board, int8_t offset, Move* moves, int start, Flag flag);
int extract_moves_pawns_promotions(Bitboard board, int8_t offset, Move* moves, int start, Flag flag);
int extract_moves(Bitboard board, int8_t offset, Move* moves, int start, Flag flag);

int gen_pawn_pushes(Board* board, Move* moves, int index);
int gen_pawn_captures(Board* board, Move* moves, int index);
int gen_pawn_promotions_quiets(Board* board, Move* moves, int index);
int gen_pawn_promotions_captures(Board* board, Move* moves, int index);
int gen_pawn_en_passant(Board* board, Move* moves, int index);

int gen_knight_moves(Board* board, Move* moves, int index, bool captures_only);
int gen_king_moves(Board* board, Move* moves, int index, bool captures_only);

int gen_cardinal_moves(Board* board, Move* moves, int index, bool captures_only);
int gen_intercardinal_moves(Board* board, Move* moves, int index, bool captures_only);

int gen_castle_moves(Board* board, Move* moves, int index);

Bitboard gen_pawn_attacks(Board* board);
Bitboard gen_cardinal_attacks_classical(int position, Bitboard blockers);
Bitboard gen_intercardinal_attacks_classical(int position, Bitboard blockers);
Bitboard gen_cardinal_attacks_magic(int position, Bitboard blockers);
Bitboard gen_intercardinal_attacks_magic(int position, Bitboard blockers);
Bitboard gen_attacks(Board* board);

int gen_moves(Board* board, Move* moves);
int gen_captures(Board* board, Move* moves);

Bitboard gen_checkers(Board* board);
Bitboard gen_pinned(Board* board);
Bitboard gen_pinned_rays(Board* board);
int filter_legal(Board* board, Move* moves, int size);

void make_move(Board* board, Move* move);
void make_move_cheap(Board* board, Move* move);

#endif