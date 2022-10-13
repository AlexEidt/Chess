#ifndef BOARD_H_
#define BOARD_H_

typedef uint8_t Piece;

#define PAWN 0b001
#define KNIGHT 0b010
#define BISHOP 0b100
#define ROOK 0b101
#define QUEEN 0b110
#define KING 0b011
#define BLACK 0b111
#define WHITE 0b000

#define OPPOSITE(x) ((x) ^ BLACK)
#define WHITE_TO_MOVE(x) ((x->active_color) == 0)
#define IS_SLIDING(x) ((x) & 0b100 != 0)

#define FEN_PIECES 0
#define FEN_ACTIVE_COLOR 1
#define FEN_CASTLING 2
#define FEN_EN_PASSANT 3
#define FEN_HALFMOVE 4
#define FEN_FULLMOVE 5

typedef struct {
    Piece active_color;
    uint8_t en_passant;
    uint8_t castle[2]; // First bit for Kingside, Second for Queenside.
    Bitboard state[8];
    uint8_t positions[64]; // Stores locations of pieces.
} Board;

void from_fen(Board* board, const char* fen);
void to_fen(Board* board, char* fen);

void switch_ply(Board* board);

void clear(Board* board);

Piece get_piece(Board* board, uint8_t index);
Piece get_color(Board* board, uint8_t index);

Bitboard get_pieces(Board* board, Piece piece, Piece color);
Bitboard get_pieces_color(Board* board, Piece color);
Bitboard get_all_pieces(Board* board);

void add_piece(Board* board, Piece piece, Piece color, uint8_t index);
void remove_piece(Board* board, Piece piece, Piece color, uint8_t index);

void make_move(Board* board, Move* move);
void undo_move(Board* board, Move* move);

void add_castle_kingside(Board* board, Piece color);
void add_castle_queenside(Board* board, Piece color);
void remove_castle_kingside(Board* board, Piece color);
void remove_castle_queenside(Board* board, Piece color);
bool can_castle_kingside(Board* board, Piece color);
bool can_castle_queenside(Board* board, Piece color);
bool can_castle(Board* board, Piece color);

#endif