#include <stdint.h>
#include <stdbool.h>
#include "board.h"
#include "bitboard.h"
#include "move.h"

void from_fen(Board* board, const char* fen) {
    clear(board);
    int i = 0;
    int field = 0;
    while (fen[i] != '\0') {
        if (fen[i] == ' ') {
            i++;
            field++;
            continue;
        }

        switch (field) {
            case FEN_PIECES:
                int file = 0;
                int rank = 0;
                while (fen[i] != ' ') {
                    int index = rank * 8 + file;
                    if (fen[i] == '/') {
                        file = 8;
                        rank++;
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
                break;
            case FEN_ACTIVE_COLOR:
                board->active_color = fen[i++] == 'w' ? WHITE : BLACK;
                break;
            case FEN_CASTLING:
                while (fen[i] != ' ') {
                    switch (fen[i++]) {
                        case 'k': add_castle_kingside(board, BLACK); break;
                        case 'K': add_castle_kingside(board, WHITE); break;
                        case 'q': add_castle_queenside(board, BLACK); break;
                        case 'Q': add_castle_queenside(board, WHITE); break;
                    }
                }
                break;
            case FEN_EN_PASSANT:
                if (fen[i] != '-') {
                    int file = 8 - (fen[i++] - 'a');
                    int rank = fen[i++] - '1';
                    board->en_passant = rank * 8 + file;
                }
                break;
            case FEN_HALFMOVE:
                break;
            case FEN_FULLMOVE:
                break;
        }
        i++;
    }
}

void to_fen(Board* board, char* fen) {
    
}

void switch_ply(Board* board) {
    board->active_color = OPPOSITE(board->active_color);
}

void clear(Board* board) {
    board->active_color = 0;
    board->en_passant = 0;
    board->castle[0] = 0;
    board->castle[1] = 0;
    for (int i = 0; i < 8; i++) board->state[i] = 0;
    for (int i = 0; i < 64; i++) board->positions[i] = 0;
}

Piece get_piece(Board* board, uint8_t index) {
    return board->positions[index];
}

Piece get_color(Board* board, uint8_t index) {
    return (1 << index) & board->state[WHITE] != 0 ? WHITE : BLACK;
}

Bitboard get_pieces(Board* board, Piece piece, Piece color) {
    return board->state[piece] & board->state[color];
}

Bitboard get_pieces_color(Board* board, Piece color) {
    return board->state[color];
}

Bitboard get_all_pieces(Board* board) {
    return board->state[WHITE] & board->state[BLACK];
}

void add_piece(Board* board, Piece piece, Piece color, uint8_t index) {
    board->positions[index] = piece;
    board->state[color] = add_bit(board->state[color], index);
    board->state[piece] = add_bit(board->state[piece], index);
}

void remove_piece(Board* board, Piece piece, Piece color, uint8_t index) {
    board->positions[index] = 0;
    board->state[color] = clear_bit(board->state[color], index);
    board->state[piece] = clear_bit(board->state[piece], index);
}

void make_move(Board* board, Move* move) {
    uint8_t src = move->from;
    uint8_t dst = move->to;

    Piece src_piece = board->positions[src];
    Piece dst_piece = board->positions[dst];

    Flag flags = move->flags;

    Piece active = board->active_color;
    Piece inactive = OPPOSITE(active);

    // If King moved and it wasn't a castle and we still could have castled, then the player can no longer castle.
    if (src_piece == KING && !IS_CASTLE(flags) && can_castle(board, active)) {
        remove_castle_kingside(board, active);
        remove_castle_queenside(board, active);
        move->flags |= CASTLING_LOST;
    }

    // If any of the rooks moved from one of the corners, then the player can no longer castle on that side.
    if (src_piece == ROOK) {
        if (src == H1 || src == H8) {
            remove_castle_kingside(board, active);
            move->flags |= CASTLING_LOST_KINGSIDE;
        } else if (src == A1 || src == A8) {
            remove_castle_queenside(board, active);
            move->flags |= CASTLING_LOST_QUEENSIDE;
        }
    }

    // Set the previous en passant square in the flag.
    move->flags |= SET_EN_PASSANT(board->en_passant);

    if (IS_CAPTURE(flags)) {
        // Add Captured Piece to Move flag.
        move->flags |= SET_CAPTURED_PIECE(dst_piece);

        // If a rook is captured, then remove the other players ability to castle on that side.
        if (dst_piece == ROOK) {
            if (dst == H1 || dst == H8) {
                remove_castle_kingside(board, inactive);
                move->flags |= CASTLING_LOST_KINGSIDE;
            } else if (dst == A1 || dst == A8) {
                remove_castle_queenside(board, inactive);
                move->flags |= CASTLING_LOST_QUEENSIDE;
            }
        }

        // board->positions[src] == PAWN && get_en_passant(board, OPPOSITE(board->active_color)) == dst
        if (IS_EN_PASSANT(flags)) {
            int8_t offset = WHITE_TO_MOVE(board) ? -8 : 8;
            remove_piece(board, PAWN, inactive, dst + offset);
            // Remove the en passant square once the en passant capture happens.
            board->en_passant = 0;
        } else {
            remove_piece(board, dst_piece, inactive, dst);
        }
    }

    remove_piece(board, src_piece, active, src);

    if (IS_PROMOTION(flags)) {
        // If the flag is a promotion, the top 3 bits contain the piece promoted.
        add_piece(board, PROMOTED_PIECE(flags), active, dst);
    } else {
        add_piece(board, src_piece, active, dst);
    }

    if (IS_DOUBLE_PUSH(flags)) {
        int8_t offset = WHITE_TO_MOVE(board) ? -8 : 8;
        board->en_passant = dst + offset;
    } else {
        // If the right to capture en passant is not exercised immediately, it is subsequently lost.
        board->en_passant = 0;
    }

    // If the move was a castle, move the rook to the corresponding position.
    if (IS_CASTLE_KINGSIDE(flags)) {
        move->flags |= CASTLING_LOST_KINGSIDE;
        if (WHITE_TO_MOVE(board)) {
            remove_piece(board, ROOK, active, H1);
            add_piece(board, ROOK, active, F1);
        } else {
            remove_piece(board, ROOK, active, H8);
            add_piece(board, ROOK, active, F8);
        }
        remove_castle_kingside(board, active);
    } else if (IS_CASTLE_QUEENSIDE(flags)) {
        move->flags |= CASTLING_LOST_QUEENSIDE;
        if (WHITE_TO_MOVE(board)) {
            remove_piece(board, ROOK, active, A1);
            add_piece(board, ROOK, active, D1);
        } else {
            remove_piece(board, ROOK, active, A8);
            add_piece(board, ROOK, active, D8);
        }
        remove_castle_queenside(board, active);
    }
}

void undo_move(Board* board, Move* move) {
    uint8_t src = move->from;
    uint8_t dst = move->to;

    Piece src_piece = board->positions[src];
    Piece dst_piece = board->positions[dst];

    Flag flags = move->flags;

    Piece active = board->active_color;
    Piece inactive = OPPOSITE(active);

    remove_piece(board, dst_piece, active, dst);
    if (IS_CAPTURE(flags)) {
        if (IS_EN_PASSANT(flags)) {
            int8_t offset = WHITE_TO_MOVE(board) ? -8 : 8;
            add_piece(board, PAWN, inactive, dst + offset);
        } else {
            add_piece(board, EXTRACT_CAPTURED_PIECE(flags), inactive, dst);
        }
        // Remove Captured Piece from Move flag.
        move->flags &= ~SET_CAPTURED_PIECE(dst_piece);
    }
    
    if (IS_PROMOTION(flags)) {
        add_piece(board, PAWN, active, src);
    } else {
        add_piece(board, dst_piece, active, src);
    }

    uint8_t offset = WHITE_TO_MOVE(board) ? 40 : 16;
    uint8_t en_passant = EXTRACT_EN_PASSANT(flags) + offset;
    board->en_passant = en_passant;
    // Remove the previous en passant square in the flag.
    move->flags &= ~SET_EN_PASSANT(en_passant);

    if (IS_CASTLING_LOST_KINGSIDE(flags)) {
        add_castle_kingside(board, IS_CAPTURE(flags) ? inactive : active);
        move->flags &= ~CASTLING_LOST_KINGSIDE;
    }

    if (IS_CASTLING_LOST_QUEENSIDE(flags)) {
        add_castle_queenside(board, IS_CAPTURE(flags) ? inactive : active);
        move->flags &= ~CASTLING_LOST_QUEENSIDE;
    }

    if (IS_CASTLE_KINGSIDE(flags)) {
        if (WHITE_TO_MOVE(board)) {
            remove_piece(board, ROOK, active, F1);
            add_piece(board, ROOK, active, H1);
        } else {
            remove_piece(board, ROOK, inactive, F8);
            add_piece(board, ROOK, inactive, H8);
        }
    } else if (IS_CASTLE_QUEENSIDE(flags)) {
        if (WHITE_TO_MOVE(board)) {
            remove_piece(board, ROOK, active, D1);
            add_piece(board, ROOK, active, A1);
        } else {
            remove_piece(board, ROOK, inactive, D8);
            add_piece(board, ROOK, inactive, A8);
        }
    }
}

void add_castle_kingside(Board* board, Piece color) {
    board->castle[color >> 2] |= 0b01;
}

void add_castle_queenside(Board* board, Piece color) {
    board->castle[color >> 2] |= 0b10;
}

void remove_castle_kingside(Board* board, Piece color) {
    board->castle[color >> 2] &= 0b10;
}

void remove_castle_queenside(Board* board, Piece color) {
    board->castle[color >> 2] &= 0b01;
}

bool can_castle_kingside(Board* board, Piece color) {
    return board->castle[color >> 2] & 0b01 != 0;
}

bool can_castle_queenside(Board* board, Piece color) {
    return board->castle[color >> 2] & 0b10 != 0;
}

bool can_castle(Board* board, Piece color) {
    return board->castle[color >> 2] != 0;
}