#ifndef MOVE_H_
#define MOVE_H_

#include <stdint.h>
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

#define SOUTH 0
#define WEST 1
#define NORTH 2
#define EAST 3

#define SOUTHEAST 0
#define SOUTHWEST 1
#define NORTHWEST 2
#define NORTHEAST 3

#define KINGSIDE_PATH 0
#define QUEENSIDE_PATH 1
#define QUEENSIDE_PATH_TO_ROOK 2
#define KING_POSITION 3
#define KING_DST_KINGSIDE 4
#define KING_DST_QUEENSIDE 5

typedef struct {
    uint8_t to, from;
    Flag flags;
} Move;

extern const Bitboard KING_MOVES[64];
extern const Bitboard KNIGHT_MOVES[64];
extern const Bitboard ROOK_MOVES[65][4];
extern const Bitboard BISHOP_MOVES[65][4];
extern const Bitboard CASTLING[2][6];

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
int gen_rook_moves(Board* board, Move* moves, int index, bool captures_only);
int gen_bishop_moves(Board* board, Move* moves, int index, bool captures_only);
int gen_queen_moves(Board* board, Move* moves, int index, bool captures_only);

int gen_castle_moves(Board* board, Move* moves, int index);

int gen_cardinal_moves(Board* board, Move* moves, int index, Piece piece, bool captures_only);
int gen_intercardinal_moves(Board* board, Move* moves, int index, Piece piece, bool captures_only);

Bitboard gen_cardinal_attacks(Board* board, Piece piece);
Bitboard gen_intercardinal_attacks(Board* board, Piece piece);
Bitboard gen_attacks(Board* board);

int gen_moves(Board* board, Move* moves);
int gen_captures(Board* board, Move* moves);
int filter_legal(Board* board, Move* moves, int size);

void make_move(Board* board, Move* move);
void make_move_cheap(Board* board, Move* move);

#endif