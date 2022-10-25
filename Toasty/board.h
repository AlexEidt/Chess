#ifndef BOARD_H_
#define BOARD_H_

#include <stdint.h>
#include <stdbool.h>
#include "bitboard.h"

typedef uint8_t Piece;

#define EMPTY 0b000
#define PAWN 0b001
#define KNIGHT 0b010
#define BISHOP 0b100
#define ROOK 0b101
#define QUEEN 0b110
#define KING 0b011
#define BLACK 0b111
#define WHITE 0b000
#define DRAW 0b001

#define IS_SLIDING(x) ((x) & 0b100 != 0)

#define OPPOSITE(x) ((x) ^ BLACK)
#define WHITE_TO_MOVE(x) (((x)->active_color) == 0)
#define IN_OPENING_BOOK(x) (((x)->full_moves) < 5)

typedef struct {
    Piece positions[64]; // Stores locations of pieces.
    Bitboard state[8]; // One bitboard for each piece type and color.
    Piece active_color;
    uint8_t en_passant;
    uint8_t castle[2]; // First bit for Kingside, Second for Queenside.
    uint8_t half_moves;
    uint8_t full_moves;
} Board;

void board_from_fen(Board* board, const char* fen);
void board_to_fen(Board* board, char* fen);

void switch_ply(Board* board);

void clear(Board* board);
uint64_t hash(Board* board);

Piece get_piece(Board* board, uint8_t index);
Piece get_color(Board* board, uint8_t index);
Bitboard get_pieces(Board* board, Piece piece, Piece color);
Bitboard get_pieces_color(Board* board, Piece color);
Bitboard get_all_pieces(Board* board);

void add_piece(Board* board, Piece piece, Piece color, uint8_t index);
void remove_piece(Board* board, Piece piece, Piece color, uint8_t index);

void add_castle_kingside(Board* board, Piece color);
void add_castle_queenside(Board* board, Piece color);
void remove_castle_kingside(Board* board, Piece color);
void remove_castle_queenside(Board* board, Piece color);
bool can_castle_kingside(Board* board, Piece color);
bool can_castle_queenside(Board* board, Piece color);
bool can_castle_color(Board* board, Piece color);
bool can_castle(Board* board);

#endif