#include <string.h>
#include "board.h"

void board_from_fen(Board* board, const char* fen) {
    clear(board);
    int i = 0;

    // Pieces.
    int file = 7;
    int rank = 7;
    while (fen[i] != ' ') {
        int index = rank * 8 + file;
        if (fen[i] == '/') {
            file = 7;
            rank--;
        } else if (fen[i] >= '0' && fen[i] <= '9') {
            file -= fen[i] - '0';
        } else {
            switch (fen[i]) {
                case 'r': add_piece(board, ROOK, BLACK, index); break;
                case 'R': add_piece(board, ROOK, WHITE, index); break;
                case 'n': add_piece(board, KNIGHT, BLACK, index); break;
                case 'N': add_piece(board, KNIGHT, WHITE, index); break;
                case 'b': add_piece(board, BISHOP, BLACK, index); break;
                case 'B': add_piece(board, BISHOP, WHITE, index); break;
                case 'q': add_piece(board, QUEEN, BLACK, index); break;
                case 'Q': add_piece(board, QUEEN, WHITE, index); break;
                case 'k': add_piece(board, KING, BLACK, index); break;
                case 'K': add_piece(board, KING, WHITE, index); break;
                case 'p': add_piece(board, PAWN, BLACK, index); break;
                case 'P': add_piece(board, PAWN, WHITE, index); break;
            }
            file--;
        }
        i++;
    }
    i++;
    // Active color.
    board->active_color = fen[i++] == 'w' ? WHITE : BLACK;
    i++;
    // Castling.
    while (fen[i] != ' ') {
        switch (fen[i++]) {
            case 'k': add_castle_kingside(board, BLACK); break;
            case 'K': add_castle_kingside(board, WHITE); break;
            case 'q': add_castle_queenside(board, BLACK); break;
            case 'Q': add_castle_queenside(board, WHITE); break;
        }
    }
    i++;
    // En passant square.
    if (fen[i] != '-') {
        int file = 8 - (fen[i++] - 'a');
        int rank = fen[i++] - '1';
        board->en_passant = rank * 8 + file;
    } else {
        board->en_passant = 0;
        i++;
    }
}

void board_to_fen(Board* board, char* fen) {
    
}

void switch_ply(Board* board) {
    board->active_color = OPPOSITE(board->active_color);
}

void clear(Board* board) {
    memset(board, 0, sizeof(Board));
}

Piece get_piece(Board* board, uint8_t index) {
    return board->positions[index];
}

Piece get_color(Board* board, uint8_t index) {
    return (board->state[WHITE] & (1ULL << index)) != 0 ? WHITE : BLACK;
}

Bitboard get_pieces(Board* board, Piece piece, Piece color) {
    return board->state[piece] & board->state[color];
}

Bitboard get_pieces_color(Board* board, Piece color) {
    return board->state[color];
}

Bitboard get_all_pieces(Board* board) {
    return board->state[WHITE] | board->state[BLACK];
}

void add_piece(Board* board, Piece piece, Piece color, uint8_t index) {
    board->positions[index] = piece;
    ADD_BIT(board->state[color], index);
    ADD_BIT(board->state[piece], index);
}

void remove_piece(Board* board, Piece piece, Piece color, uint8_t index) {
    board->positions[index] = 0;
    CLEAR_BIT(board->state[color], index);
    CLEAR_BIT(board->state[piece], index);
}

void add_castle_kingside(Board* board, Piece color) {
    board->castle[color & 1] |= 0b01;
}

void add_castle_queenside(Board* board, Piece color) {
    board->castle[color & 1] |= 0b10;
}

void remove_castle_kingside(Board* board, Piece color) {
    board->castle[color & 1] &= 0b10;
}

void remove_castle_queenside(Board* board, Piece color) {
    board->castle[color & 1] &= 0b01;
}

bool can_castle_kingside(Board* board, Piece color) {
    return (board->castle[color & 1] & 0b01) != 0;
}

bool can_castle_queenside(Board* board, Piece color) {
    return (board->castle[color & 1] & 0b10) != 0;
}

bool can_castle(Board* board, Piece color) {
    return board->castle[color & 1] != 0;
}