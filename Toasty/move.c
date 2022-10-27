#include <string.h>
#include <stdbool.h>
#include "bitboard.h"
#include "board.h"
#include "move.h"
#include "evaluate.h"

int score_move(Board* board, Move* move) {
    Piece src = board->positions[move->from];
    Piece dst = board->positions[move->to];

    int score = 0;
    // Promotions.
    score += PIECE_VALUES[PROMOTED_PIECE(move->flags)] * PROMOTION_BONUS;
    // Piece Square Table evaluation.
    score += PST[src][move->to] - PST[src][move->from];
    // Captures.
    score += (PIECE_VALUES[dst] - PIECE_VALUES[src]) * IS_CAPTURE(move->flags) * CAPTURE_BONUS;

    return score;
}

int extract_moves_pawns(Bitboard board, int8_t offset, Move* moves, int start, Flag flag) {
    while (board != 0) {
        int pos = LSB(board);
        board &= board - 1;
        Move* move = &moves[start++];
        move->to = pos; move->from = pos - offset; move->flags = flag;
    }

    return start;
}

int extract_moves_pawns_promotions(Bitboard board, int8_t offset, Move* moves, int start, Flag flag) {
    while (board != 0) {
        int pos = LSB(board);
        board &= board - 1;
        Move* move;
        move = &moves[start++];
        move->to = pos; move->from = pos - offset; move->flags = ADD_PROMOTED_PIECE(QUEEN) | flag;
        move = &moves[start++];
        move->to = pos; move->from = pos - offset; move->flags = ADD_PROMOTED_PIECE(KNIGHT) | flag;
        move = &moves[start++];
        move->to = pos; move->from = pos - offset; move->flags = ADD_PROMOTED_PIECE(BISHOP) | flag;
        move = &moves[start++];
        move->to = pos; move->from = pos - offset; move->flags = ADD_PROMOTED_PIECE(ROOK) | flag;
    }

    return start;
}

int gen_pawn_pushes(Board* board, Move* moves, int index) {
    Bitboard pawns = get_pieces(board, PAWN, board->active_color);
    Bitboard empty = ~get_all_pieces(board);

    Bitboard single_pushes, double_pushes;
    int8_t offset;

    if (WHITE_TO_MOVE(board)) {
        single_pushes = ((pawns & ~RANK7) << 8) & empty;
        double_pushes = ((single_pushes & RANK3) << 8) & empty;
        offset = 8;
    } else {
        single_pushes = ((pawns & ~RANK2) >> 8) & empty;
        double_pushes = ((single_pushes & RANK6) >> 8) & empty;
        offset = -8;
    }

    index = extract_moves_pawns(single_pushes, offset, moves, index, QUIET);
    index = extract_moves_pawns(double_pushes, offset * 2, moves, index, PAWN_DOUBLE | QUIET);

    return index;
}

int gen_pawn_captures(Board* board, Move* moves, int index) {
    Bitboard pawns = get_pieces(board, PAWN, board->active_color);
    Bitboard enemies = get_pieces_color(board, OPPOSITE(board->active_color));

    if (WHITE_TO_MOVE(board)) {
        Bitboard valid_pawns = pawns & ~RANK7;
        Bitboard capture_left = ((valid_pawns & ~FILEA) << 9) & enemies;
        index = extract_moves_pawns(capture_left, 9, moves, index, CAPTURE);
        Bitboard capture_right = ((valid_pawns & ~FILEH) << 7) & enemies;
        index = extract_moves_pawns(capture_right, 7, moves, index, CAPTURE);
    } else {
        Bitboard valid_pawns = pawns & ~RANK2;
        Bitboard capture_left = ((valid_pawns & ~FILEH) >> 9) & enemies;
        index = extract_moves_pawns(capture_left, -9, moves, index, CAPTURE);
        Bitboard capture_right = ((valid_pawns & ~FILEA) >> 7) & enemies;
        index = extract_moves_pawns(capture_right, -7, moves, index, CAPTURE);
    }

    return index;
}

int gen_pawn_promotions_quiets(Board* board, Move* moves, int index) {
    Bitboard pawns = get_pieces(board, PAWN, board->active_color);
    Bitboard empty = ~get_all_pieces(board);

    Bitboard promotions;
    int8_t offset;

    if (WHITE_TO_MOVE(board)) {
        promotions = ((pawns & RANK7) << 8) & empty;
        offset = 8;
    } else {
        promotions = ((pawns & RANK2) >> 8) & empty;
        offset = -8;
    }

    index = extract_moves_pawns_promotions(promotions, offset, moves, index, QUIET);

    return index;
}

int gen_pawn_promotions_captures(Board* board, Move* moves, int index) {
    Bitboard pawns = get_pieces(board, PAWN, board->active_color);
    Bitboard enemies = get_pieces_color(board, OPPOSITE(board->active_color));

    if (WHITE_TO_MOVE(board)) {
        Bitboard valid_pawns = pawns & RANK7;
        Bitboard capture_left = ((valid_pawns & ~FILEA) << 9) & enemies;
        index = extract_moves_pawns_promotions(capture_left, 9, moves, index, CAPTURE);
        Bitboard capture_right = ((valid_pawns & ~FILEH) << 7) & enemies;
        index = extract_moves_pawns_promotions(capture_right, 7, moves, index, CAPTURE);
    } else {
        Bitboard valid_pawns = pawns & RANK2;
        Bitboard capture_left = ((valid_pawns & ~FILEH) >> 9) & enemies;
        index = extract_moves_pawns_promotions(capture_left, -9, moves, index, CAPTURE);
        Bitboard capture_right = ((valid_pawns & ~FILEA) >> 7) & enemies;
        index = extract_moves_pawns_promotions(capture_right, -7, moves, index, CAPTURE);
    }

    return index;
}

int gen_pawn_en_passant(Board* board, Move* moves, int index) {
    if (board->en_passant == 0) return index;

    Bitboard pawns = get_pieces(board, PAWN, board->active_color);
    Bitboard en_passant = 1ULL << board->en_passant;

    if (WHITE_TO_MOVE(board)) {
        Bitboard capture_left = ((pawns & ~FILEA) << 9) & en_passant;
        index = extract_moves_pawns(capture_left, 9, moves, index, CAPTURE | EN_PASSANT);
        Bitboard capture_right = ((pawns & ~FILEH) << 7) & en_passant;
        index = extract_moves_pawns(capture_right, 7, moves, index, CAPTURE | EN_PASSANT);
    } else {
        Bitboard capture_left = ((pawns & ~FILEH) >> 9) & en_passant;
        index = extract_moves_pawns(capture_left, -9, moves, index, CAPTURE | EN_PASSANT);
        Bitboard capture_right = ((pawns & ~FILEA) >> 7) & en_passant;
        index = extract_moves_pawns(capture_right, -7, moves, index, CAPTURE | EN_PASSANT);
    }

    return index;
}

int extract_moves(Bitboard board, int8_t init, Move* moves, int start, Flag flag) {
    while (board != 0) {
        int pos = LSB(board);
        board &= board - 1;
        Move* move = &moves[start++];
        move->to = pos; move->from = init; move->flags = flag;
    }

    return start;
}

int gen_knight_moves(Board* board, Move* moves, int index, bool captures_only) {
    Bitboard knights = get_pieces(board, KNIGHT, board->active_color);
    Bitboard empty = ~get_all_pieces(board);
    Bitboard enemies = get_pieces_color(board, OPPOSITE(board->active_color));

    while (knights != 0) {
        int pos = LSB(knights);
        knights &= knights - 1;
        if (!captures_only) {
            index = extract_moves(KNIGHT_MOVES[pos] & empty, pos, moves, index, QUIET);
        }
        index = extract_moves(KNIGHT_MOVES[pos] & enemies, pos, moves, index, CAPTURE);
    }

    return index;
}

int gen_king_moves(Board* board, Move* moves, int index, bool captures_only) {
    Bitboard king = get_pieces(board, KING, board->active_color);
    Bitboard empty = ~get_all_pieces(board);
    Bitboard enemies = get_pieces_color(board, OPPOSITE(board->active_color));

    int pos = LSB(king);
    if (!captures_only) {
        index = extract_moves(KING_MOVES[pos] & empty, pos, moves, index, QUIET);
    }
    index = extract_moves(KING_MOVES[pos] & enemies, pos, moves, index, CAPTURE);

    return index;
}

int gen_cardinal_moves(Board* board, Move* moves, int index, bool captures_only) {
    Piece color = board->active_color;
    Bitboard cardinal = get_pieces(board, ROOK, color) | get_pieces(board, QUEEN, color);
    Bitboard us = get_pieces_color(board, color);
    Bitboard enemies = get_pieces_color(board, OPPOSITE(color));
    Bitboard empty = ~get_all_pieces(board);

    while (cardinal != 0) {
        int pos = LSB(cardinal);
        cardinal &= cardinal - 1;

        Bitboard current = 1ULL << pos;
        Bitboard result = 0;

        int ai, qi;
        Bitboard ray, ray_us, ray_enemies;
    
        ray = ROOK_MOVES[pos][NORTH];
        ray_enemies = ray & enemies;
        ray_us = ray & us;
        ai = ray_enemies != 0 ? LSB(ray_enemies) : 64;
        qi = ray_us != 0 ? LSB(ray_us) : 64;
        ray ^= ROOK_MOVES[ai][NORTH] | (current | ROOK_MOVES[qi][NORTH]);
        result |= ray;

        ray = ROOK_MOVES[pos][EAST];
        ray_enemies = ray & enemies;
        ray_us = ray & us;
        ai = ray_enemies != 0 ? MSB(ray_enemies) : 64;
        qi = ray_us != 0 ? MSB(ray_us) : 64;
        ray ^= ROOK_MOVES[ai][EAST] | (current | ROOK_MOVES[qi][EAST]);
        result |= ray;

        ray = ROOK_MOVES[pos][SOUTH];
        ray_enemies = ray & enemies;
        ray_us = ray & us;
        ai = ray_enemies != 0 ? MSB(ray_enemies) : 64;
        qi = ray_us != 0 ? MSB(ray_us) : 64;
        ray ^= ROOK_MOVES[ai][SOUTH] | (current | ROOK_MOVES[qi][SOUTH]);
        result |= ray;

        ray = ROOK_MOVES[pos][WEST];
        ray_enemies = ray & enemies;
        ray_us = ray & us;
        ai = ray_enemies != 0 ? LSB(ray_enemies) : 64;
        qi = ray_us != 0 ? LSB(ray_us) : 64;
        ray ^= ROOK_MOVES[ai][WEST] | (current | ROOK_MOVES[qi][WEST]);
        result |= ray;

        if (!captures_only) {
            index = extract_moves(result & empty, pos, moves, index, QUIET);
        }
        index = extract_moves(result & enemies, pos, moves, index, CAPTURE);
    }

    return index;
}

int gen_intercardinal_moves(Board* board, Move* moves, int index, bool captures_only) {
    Piece color = board->active_color;
    Bitboard intercardinal = get_pieces(board, BISHOP, color) | get_pieces(board, QUEEN, color);
    Bitboard us = get_pieces_color(board, color);
    Bitboard enemies = get_pieces_color(board, OPPOSITE(color));
    Bitboard empty = ~get_all_pieces(board);

    while (intercardinal != 0) {
        int pos = LSB(intercardinal);
        intercardinal &= intercardinal - 1;

        Bitboard current = 1ULL << pos;
        Bitboard result = 0;

        int ai, qi;
        Bitboard ray, ray_enemies, ray_us;

        ray = BISHOP_MOVES[pos][SOUTHEAST];
        ray_enemies = ray & enemies;
        ray_us = ray & us;
        ai = ray_enemies != 0 ? MSB(ray_enemies) : 64;
        qi = ray_us != 0 ? MSB(ray_us) : 64;
        ray ^= BISHOP_MOVES[ai][SOUTHEAST] | (current | BISHOP_MOVES[qi][SOUTHEAST]);
        result |= ray;

        ray = BISHOP_MOVES[pos][SOUTHWEST];
        ray_enemies = ray & enemies;
        ray_us = ray & us;
        ai = ray_enemies != 0 ? MSB(ray_enemies) : 64;
        qi = ray_us != 0 ? MSB(ray_us) : 64;
        ray ^= BISHOP_MOVES[ai][SOUTHWEST] | (current | BISHOP_MOVES[qi][SOUTHWEST]);
        result |= ray;

        ray = BISHOP_MOVES[pos][NORTHEAST];
        ray_enemies = ray & enemies;
        ray_us = ray & us;
        ai = ray_enemies != 0 ? LSB(ray_enemies) : 64;
        qi = ray_us != 0 ? LSB(ray_us) : 64;
        ray ^= BISHOP_MOVES[ai][NORTHEAST] | (current | BISHOP_MOVES[qi][NORTHEAST]);
        result |= ray;

        ray = BISHOP_MOVES[pos][NORTHWEST];
        ray_enemies = ray & enemies;
        ray_us = ray & us;
        ai = ray_enemies != 0 ? LSB(ray_enemies) : 64;
        qi = ray_us != 0 ? LSB(ray_us) : 64;
        ray ^= BISHOP_MOVES[ai][NORTHWEST] | (current | BISHOP_MOVES[qi][NORTHWEST]);
        result |= ray;

        if (!captures_only) {
            index = extract_moves(result & empty, pos, moves, index, QUIET);
        }
        index = extract_moves(result & enemies, pos, moves, index, CAPTURE);
    }

    return index;
}

int gen_castle_moves(Board* board, Move* moves, int index) {
    switch_ply(board);
    Bitboard attacks = gen_attacks(board);
    switch_ply(board);

    Bitboard king = get_pieces(board, KING, board->active_color);
    if ((king & attacks) == 0) { // If king is not in check.
        uint8_t color = board->active_color & 1; // Maps White to 0, Black to 1.
        Bitboard kingside = CASTLING[color][KINGSIDE_PATH];
        Bitboard queenside = CASTLING[color][QUEENSIDE_PATH];
        Bitboard queenside_path_rook = CASTLING[color][QUEENSIDE_PATH_TO_ROOK];
        Bitboard king_pos = CASTLING[color][KING_POSITION];
        Bitboard king_dst_kingside = CASTLING[color][KING_DST_KINGSIDE];
        Bitboard king_dst_queenside = CASTLING[color][KING_DST_QUEENSIDE];

        Bitboard us = get_pieces_color(board, board->active_color);
        Bitboard enemies = get_pieces_color(board, OPPOSITE(board->active_color));

        if (can_castle_kingside(board, board->active_color)) {
            if ((kingside & (attacks | us | enemies)) == 0) {
                Move* move = &moves[index++];
                move->to = king_dst_kingside; move->from = king_pos; move->flags = CASTLE_KINGSIDE;
            }
        }
        if (can_castle_queenside(board, board->active_color)) {
            if ((queenside & (attacks | us | enemies)) == 0 && (queenside_path_rook & (us | enemies)) == 0) {
                Move* move = &moves[index++];
                move->to = king_dst_queenside; move->from = king_pos; move->flags = CASTLE_QUEENSIDE;
            }
        }
    }

    return index;
}

Bitboard gen_cardinal_attacks(Board* board) {
    Bitboard attacks = 0;

    Piece color = board->active_color;
    Bitboard cardinal = get_pieces(board, ROOK, color) | get_pieces(board, QUEEN, color);
    Bitboard us = get_pieces_color(board, color);
    Bitboard enemies = get_pieces_color(board, OPPOSITE(color));

    while (cardinal != 0) {
        int pos = LSB(cardinal);
        cardinal &= cardinal - 1;

        Bitboard current = 1ULL << pos;

        int ai, qi;
        Bitboard ray, ray_enemies, ray_us;
    
        ray = ROOK_MOVES[pos][NORTH];
        ray_enemies = ray & enemies;
        ray_us = ray & us;
        ai = ray_enemies != 0 ? LSB(ray_enemies) : 64;
        qi = ray_us != 0 ? LSB(ray_us) : 64;
        ray ^= ROOK_MOVES[ai][NORTH] | (current | ROOK_MOVES[qi][NORTH]);
        attacks |= ray;

        ray = ROOK_MOVES[pos][EAST];
        ray_enemies = ray & enemies;
        ray_us = ray & us;
        ai = ray_enemies != 0 ? MSB(ray_enemies) : 64;
        qi = ray_us != 0 ? MSB(ray_us) : 64;
        ray ^= ROOK_MOVES[ai][EAST] | (current | ROOK_MOVES[qi][EAST]);
        attacks |= ray;

        ray = ROOK_MOVES[pos][SOUTH];
        ray_enemies = ray & enemies;
        ray_us = ray & us;
        ai = ray_enemies != 0 ? MSB(ray_enemies) : 64;
        qi = ray_us != 0 ? MSB(ray_us) : 64;
        ray ^= ROOK_MOVES[ai][SOUTH] | (current | ROOK_MOVES[qi][SOUTH]);
        attacks |= ray;

        ray = ROOK_MOVES[pos][WEST];
        ray_enemies = ray & enemies;
        ray_us = ray & us;
        ai = ray_enemies != 0 ? LSB(ray_enemies) : 64;
        qi = ray_us != 0 ? LSB(ray_us) : 64;
        ray ^= ROOK_MOVES[ai][WEST] | (current | ROOK_MOVES[qi][WEST]);
        attacks |= ray;
    }

    return attacks;
}

Bitboard gen_intercardinal_attacks(Board* board) {
    Bitboard attacks = 0;

    Piece color = board->active_color;
    Bitboard intercardinal = get_pieces(board, BISHOP, color) | get_pieces(board, QUEEN, color);
    Bitboard us = get_pieces_color(board, color);
    Bitboard enemies = get_pieces_color(board, OPPOSITE(color));

    while (intercardinal != 0) {
        int pos = LSB(intercardinal);
        intercardinal &= intercardinal - 1;

        Bitboard current = 1ULL << pos;

        int ai, qi;
        Bitboard ray, ray_enemies, ray_us;
    
        ray = BISHOP_MOVES[pos][SOUTHEAST];
        ray_enemies = ray & enemies;
        ray_us = ray & us;
        ai = ray_enemies != 0 ? MSB(ray_enemies) : 64;
        qi = ray_us != 0 ? MSB(ray_us) : 64;
        ray ^= BISHOP_MOVES[ai][SOUTHEAST] | (current | BISHOP_MOVES[qi][SOUTHEAST]);
        attacks |= ray;

        ray = BISHOP_MOVES[pos][SOUTHWEST];
        ray_enemies = ray & enemies;
        ray_us = ray & us;
        ai = ray_enemies != 0 ? MSB(ray_enemies) : 64;
        qi = ray_us != 0 ? MSB(ray_us) : 64;
        ray ^= BISHOP_MOVES[ai][SOUTHWEST] | (current | BISHOP_MOVES[qi][SOUTHWEST]);
        attacks |= ray;

        ray = BISHOP_MOVES[pos][NORTHEAST];
        ray_enemies = ray & enemies;
        ray_us = ray & us;
        ai = ray_enemies != 0 ? LSB(ray_enemies) : 64;
        qi = ray_us != 0 ? LSB(ray_us) : 64;
        ray ^= BISHOP_MOVES[ai][NORTHEAST] | (current | BISHOP_MOVES[qi][NORTHEAST]);
        attacks |= ray;

        ray = BISHOP_MOVES[pos][NORTHWEST];
        ray_enemies = ray & enemies;
        ray_us = ray & us;
        ai = ray_enemies != 0 ? LSB(ray_enemies) : 64;
        qi = ray_us != 0 ? LSB(ray_us) : 64;
        ray ^= BISHOP_MOVES[ai][NORTHWEST] | (current | BISHOP_MOVES[qi][NORTHWEST]);
        attacks |= ray;
    }

    return attacks;
}

Bitboard gen_attacks(Board* board) {
    Bitboard attacks = 0;

    Piece active = board->active_color;
    Piece inactive = OPPOSITE(active);
    Bitboard empty = ~get_all_pieces(board);

    // Pawn Attacks.
    Bitboard pawns = get_pieces(board, PAWN, active);
    if (WHITE_TO_MOVE(board)) {
        attacks |= (pawns & ~FILEA) << 9;
        attacks |= (pawns & ~FILEH) << 7;
    } else {
        attacks |= (pawns & ~FILEH) >> 9;
        attacks |= (pawns & ~FILEA) >> 7;
    }

    // Knight Attacks.
    Bitboard knights = get_pieces(board, KNIGHT, active);
    while (knights != 0) {
        int pos = LSB(knights);
        knights &= knights - 1;
        attacks |= KNIGHT_MOVES[pos];
    }

    // King Attacks.
    Bitboard king = get_pieces(board, KING, active);
    attacks |= KING_MOVES[LSB(king)];

    // Sliding Attacks.
    attacks |= gen_cardinal_attacks(board);
    attacks |= gen_intercardinal_attacks(board);
    
    return attacks;
}

int gen_moves(Board* board, Move* moves) {
    int index = 0;

    index = gen_pawn_promotions_quiets(board, moves, index);
    index = gen_pawn_promotions_captures(board, moves, index);
    index = gen_pawn_captures(board, moves, index);

    index = gen_knight_moves(board, moves, index, false);
    index = gen_cardinal_moves(board, moves, index, false);
    index = gen_intercardinal_moves(board, moves, index, false);
    index = gen_king_moves(board, moves, index, false);

    index = gen_pawn_pushes(board, moves, index);
    index = gen_pawn_en_passant(board, moves, index);

    if (can_castle_color(board, board->active_color)) {
        index = gen_castle_moves(board, moves, index);
    }

    return filter_legal(board, moves, index);
}

int gen_captures(Board* board, Move* moves) {
    int index = 0;

    index = gen_pawn_promotions_captures(board, moves, index);
    index = gen_pawn_captures(board, moves, index);

    index = gen_knight_moves(board, moves, index, true);
    index = gen_cardinal_moves(board, moves, index, true);
    index = gen_intercardinal_moves(board, moves, index, true);
    index = gen_king_moves(board, moves, index, true);

    index = gen_pawn_en_passant(board, moves, index);

    return filter_legal(board, moves, index);
}

int filter_legal(Board* board, Move* moves, int size) {
    const Board copy = *board;

    int n_legal = 0;
    for (int i = 0; i < size; i++) {
        Move* move = &moves[i];
        make_move_cheap(board, move);
        Bitboard king = get_pieces(board, KING, board->active_color);
        switch_ply(board);
        Bitboard attacks = gen_attacks(board);
        // If king is not in check after making the move, then it is legal.
        if ((king & attacks) == 0) {
            moves[n_legal++] = *move;
        }
        memcpy(board, &copy, sizeof(Board)); // Undo move.
    }

    return n_legal;
}

bool is_in_check(Board* board) {
    Bitboard king = get_pieces(board, KING, board->active_color);
    Bitboard attacks = gen_attacks(board);

    return (king & attacks) != 0;
}

void make_move(Board* board, Move* move) {
    uint8_t src = move->from;
    uint8_t dst = move->to;

    Piece src_piece = board->positions[src];
    Piece dst_piece = board->positions[dst];

    Flag flags = move->flags;

    Piece active = board->active_color;
    Piece inactive = OPPOSITE(active);

    if (src_piece == PAWN || IS_CAPTURE(flags)) {
        board->half_moves = 0;
    } else {
        board->half_moves++;
    }

    if (active == BLACK) {
        board->full_moves++;
    }

    // If King moved and it wasn't a castle, then the player can no longer castle.
    if (src_piece == KING && !IS_CASTLE(flags)) {
        remove_castle_kingside(board, active);
        remove_castle_queenside(board, active);
    }

    // If any of the rooks moved from one of the corners, then the player can no longer castle on that side.
    if (src_piece == ROOK) {
        if (src == H1 || src == H8) {
            remove_castle_kingside(board, active);
        } else if (src == A1 || src == A8) {
            remove_castle_queenside(board, active);
        }
    }

    if (IS_CAPTURE(flags)) {
        // If a rook is captured, then remove the other players ability to castle on that side.
        if (dst_piece == ROOK) {
            if (dst == H1 || dst == H8) {
                remove_castle_kingside(board, inactive);
            } else if (dst == A1 || dst == A8) {
                remove_castle_queenside(board, inactive);
            }
        }

        if (IS_EN_PASSANT(flags)) {
            int8_t offset = WHITE_TO_MOVE(board) ? -8 : 8;
            remove_piece(board, PAWN, inactive, dst + offset);
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
        // If an en passant capture happens, then it is lost as well.
        board->en_passant = 0;
    }

    // If the move was a castle, move the rook to the corresponding position.
    if (IS_CASTLE(flags)) {
        if (IS_CASTLE_KINGSIDE(flags)) {
            if (WHITE_TO_MOVE(board)) {
                remove_piece(board, ROOK, active, H1);
                add_piece(board, ROOK, active, F1);
            } else {
                remove_piece(board, ROOK, active, H8);
                add_piece(board, ROOK, active, F8);
            }
            remove_castle_kingside(board, active);
        } else if (IS_CASTLE_QUEENSIDE(flags)) {
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

    board->active_color = inactive;
}

void make_move_cheap(Board* board, Move* move) {
    uint8_t src = move->from;
    uint8_t dst = move->to;

    Piece src_piece = board->positions[src];
    Piece dst_piece = board->positions[dst];

    Flag flags = move->flags;

    Piece active = board->active_color;
    Piece inactive = OPPOSITE(active);

    if (IS_CAPTURE(flags)) {
        if (IS_EN_PASSANT(flags)) {
            int8_t offset = WHITE_TO_MOVE(board) ? -8 : 8;
            remove_piece(board, PAWN, inactive, dst + offset);
        } else {
            remove_piece(board, dst_piece, inactive, dst);
        }
    }

    remove_piece(board, src_piece, active, src);

    add_piece(board, IS_PROMOTION(flags) ? PROMOTED_PIECE(flags) : src_piece, active, dst);

    // If the move was a castle, move the rook to the corresponding position.
    if (IS_CASTLE(flags)) {
        bool is_castle_kingside = IS_CASTLE_KINGSIDE(flags);
        if (WHITE_TO_MOVE(board)) {
            remove_piece(board, ROOK, active, is_castle_kingside ? H1 : A1);
            add_piece(board, ROOK, active, is_castle_kingside ? F1 : D1);
        } else {
            remove_piece(board, ROOK, active, is_castle_kingside ? H8 : A8);
            add_piece(board, ROOK, active, is_castle_kingside ? F8 : D8);
        }
    }
}

// All possible king moves for each square.
const Bitboard KING_MOVES[64] = {
    0x302ULL, 0x705ULL, 0xe0aULL, 0x1c14ULL, 0x3828ULL, 0x7050ULL, 0xe0a0ULL, 0xc040ULL,
    0x30203ULL, 0x70507ULL, 0xe0a0eULL, 0x1c141cULL, 0x382838ULL, 0x705070ULL, 0xe0a0e0ULL, 0xc040c0ULL,
    0x3020300ULL, 0x7050700ULL, 0xe0a0e00ULL, 0x1c141c00ULL, 0x38283800ULL, 0x70507000ULL, 0xe0a0e000ULL, 0xc040c000ULL,
    0x302030000ULL, 0x705070000ULL, 0xe0a0e0000ULL, 0x1c141c0000ULL, 0x3828380000ULL, 0x7050700000ULL, 0xe0a0e00000ULL, 0xc040c00000ULL,
    0x30203000000ULL, 0x70507000000ULL, 0xe0a0e000000ULL, 0x1c141c000000ULL, 0x382838000000ULL, 0x705070000000ULL, 0xe0a0e0000000ULL, 0xc040c0000000ULL,
    0x3020300000000ULL, 0x7050700000000ULL, 0xe0a0e00000000ULL, 0x1c141c00000000ULL, 0x38283800000000ULL, 0x70507000000000ULL, 0xe0a0e000000000ULL, 0xc040c000000000ULL,
    0x302030000000000ULL, 0x705070000000000ULL, 0xe0a0e0000000000ULL, 0x1c141c0000000000ULL, 0x3828380000000000ULL, 0x7050700000000000ULL, 0xe0a0e00000000000ULL, 0xc040c00000000000ULL,
    0x203000000000000ULL, 0x507000000000000ULL, 0xa0e000000000000ULL, 0x141c000000000000ULL, 0x2838000000000000ULL, 0x5070000000000000ULL, 0xa0e0000000000000ULL, 0x40c0000000000000ULL
};

// All possible knight moves for each square.
const Bitboard KNIGHT_MOVES[64] = {
    0x20400ULL, 0x50800ULL, 0xa1100ULL, 0x142200ULL, 0x284400ULL, 0x508800ULL, 0xa01000ULL, 0x402000ULL,
    0x2040004ULL, 0x5080008ULL, 0xa110011ULL, 0x14220022ULL, 0x28440044ULL, 0x50880088ULL, 0xa0100010ULL, 0x40200020ULL,
    0x204000402ULL, 0x508000805ULL, 0xa1100110aULL, 0x1422002214ULL, 0x2844004428ULL, 0x5088008850ULL, 0xa0100010a0ULL, 0x4020002040ULL,
    0x20400040200ULL, 0x50800080500ULL, 0xa1100110a00ULL, 0x142200221400ULL, 0x284400442800ULL, 0x508800885000ULL, 0xa0100010a000ULL, 0x402000204000ULL,
    0x2040004020000ULL, 0x5080008050000ULL, 0xa1100110a0000ULL, 0x14220022140000ULL, 0x28440044280000ULL, 0x50880088500000ULL, 0xa0100010a00000ULL, 0x40200020400000ULL,
    0x204000402000000ULL, 0x508000805000000ULL, 0xa1100110a000000ULL, 0x1422002214000000ULL, 0x2844004428000000ULL, 0x5088008850000000ULL, 0xa0100010a0000000ULL, 0x4020002040000000ULL,
    0x400040200000000ULL, 0x800080500000000ULL, 0x1100110a00000000ULL, 0x2200221400000000ULL, 0x4400442800000000ULL, 0x8800885000000000ULL, 0x100010a000000000ULL, 0x2000204000000000ULL,
    0x4020000000000ULL, 0x8050000000000ULL, 0x110a0000000000ULL, 0x22140000000000ULL, 0x44280000000000ULL, 0x88500000000000ULL, 0x10a00000000000ULL, 0x20400000000000ULL
};

const Bitboard ROOK_MOVES[65][4] = { // South, West, North, East
    {0x0ULL, 0xfeULL, 0x101010101010100ULL, 0x0ULL},
    {0x0ULL, 0xfcULL, 0x202020202020200ULL, 0x1ULL},
    {0x0ULL, 0xf8ULL, 0x404040404040400ULL, 0x3ULL},
    {0x0ULL, 0xf0ULL, 0x808080808080800ULL, 0x7ULL},
    {0x0ULL, 0xe0ULL, 0x1010101010101000ULL, 0xfULL},
    {0x0ULL, 0xc0ULL, 0x2020202020202000ULL, 0x1fULL},
    {0x0ULL, 0x80ULL, 0x4040404040404000ULL, 0x3fULL},
    {0x0ULL, 0x0ULL, 0x8080808080808000ULL, 0x7fULL},
    {0x1ULL, 0xfe00ULL, 0x101010101010000ULL, 0x0ULL},
    {0x2ULL, 0xfc00ULL, 0x202020202020000ULL, 0x100ULL},
    {0x4ULL, 0xf800ULL, 0x404040404040000ULL, 0x300ULL},
    {0x8ULL, 0xf000ULL, 0x808080808080000ULL, 0x700ULL},
    {0x10ULL, 0xe000ULL, 0x1010101010100000ULL, 0xf00ULL},
    {0x20ULL, 0xc000ULL, 0x2020202020200000ULL, 0x1f00ULL},
    {0x40ULL, 0x8000ULL, 0x4040404040400000ULL, 0x3f00ULL},
    {0x80ULL, 0x0ULL, 0x8080808080800000ULL, 0x7f00ULL},
    {0x101ULL, 0xfe0000ULL, 0x101010101000000ULL, 0x0ULL},
    {0x202ULL, 0xfc0000ULL, 0x202020202000000ULL, 0x10000ULL},
    {0x404ULL, 0xf80000ULL, 0x404040404000000ULL, 0x30000ULL},
    {0x808ULL, 0xf00000ULL, 0x808080808000000ULL, 0x70000ULL},
    {0x1010ULL, 0xe00000ULL, 0x1010101010000000ULL, 0xf0000ULL},
    {0x2020ULL, 0xc00000ULL, 0x2020202020000000ULL, 0x1f0000ULL},
    {0x4040ULL, 0x800000ULL, 0x4040404040000000ULL, 0x3f0000ULL},
    {0x8080ULL, 0x0ULL, 0x8080808080000000ULL, 0x7f0000ULL},
    {0x10101ULL, 0xfe000000ULL, 0x101010100000000ULL, 0x0ULL},
    {0x20202ULL, 0xfc000000ULL, 0x202020200000000ULL, 0x1000000ULL},
    {0x40404ULL, 0xf8000000ULL, 0x404040400000000ULL, 0x3000000ULL},
    {0x80808ULL, 0xf0000000ULL, 0x808080800000000ULL, 0x7000000ULL},
    {0x101010ULL, 0xe0000000ULL, 0x1010101000000000ULL, 0xf000000ULL},
    {0x202020ULL, 0xc0000000ULL, 0x2020202000000000ULL, 0x1f000000ULL},
    {0x404040ULL, 0x80000000ULL, 0x4040404000000000ULL, 0x3f000000ULL},
    {0x808080ULL, 0x0ULL, 0x8080808000000000ULL, 0x7f000000ULL},
    {0x1010101ULL, 0xfe00000000ULL, 0x101010000000000ULL, 0x0ULL},
    {0x2020202ULL, 0xfc00000000ULL, 0x202020000000000ULL, 0x100000000ULL},
    {0x4040404ULL, 0xf800000000ULL, 0x404040000000000ULL, 0x300000000ULL},
    {0x8080808ULL, 0xf000000000ULL, 0x808080000000000ULL, 0x700000000ULL},
    {0x10101010ULL, 0xe000000000ULL, 0x1010100000000000ULL, 0xf00000000ULL},
    {0x20202020ULL, 0xc000000000ULL, 0x2020200000000000ULL, 0x1f00000000ULL},
    {0x40404040ULL, 0x8000000000ULL, 0x4040400000000000ULL, 0x3f00000000ULL},
    {0x80808080ULL, 0x0ULL, 0x8080800000000000ULL, 0x7f00000000ULL},
    {0x101010101ULL, 0xfe0000000000ULL, 0x101000000000000ULL, 0x0ULL},
    {0x202020202ULL, 0xfc0000000000ULL, 0x202000000000000ULL, 0x10000000000ULL},
    {0x404040404ULL, 0xf80000000000ULL, 0x404000000000000ULL, 0x30000000000ULL},
    {0x808080808ULL, 0xf00000000000ULL, 0x808000000000000ULL, 0x70000000000ULL},
    {0x1010101010ULL, 0xe00000000000ULL, 0x1010000000000000ULL, 0xf0000000000ULL},
    {0x2020202020ULL, 0xc00000000000ULL, 0x2020000000000000ULL, 0x1f0000000000ULL},
    {0x4040404040ULL, 0x800000000000ULL, 0x4040000000000000ULL, 0x3f0000000000ULL},
    {0x8080808080ULL, 0x0ULL, 0x8080000000000000ULL, 0x7f0000000000ULL},
    {0x10101010101ULL, 0xfe000000000000ULL, 0x100000000000000ULL, 0x0ULL},
    {0x20202020202ULL, 0xfc000000000000ULL, 0x200000000000000ULL, 0x1000000000000ULL},
    {0x40404040404ULL, 0xf8000000000000ULL, 0x400000000000000ULL, 0x3000000000000ULL},
    {0x80808080808ULL, 0xf0000000000000ULL, 0x800000000000000ULL, 0x7000000000000ULL},
    {0x101010101010ULL, 0xe0000000000000ULL, 0x1000000000000000ULL, 0xf000000000000ULL},
    {0x202020202020ULL, 0xc0000000000000ULL, 0x2000000000000000ULL, 0x1f000000000000ULL},
    {0x404040404040ULL, 0x80000000000000ULL, 0x4000000000000000ULL, 0x3f000000000000ULL},
    {0x808080808080ULL, 0x0ULL, 0x8000000000000000ULL, 0x7f000000000000ULL},
    {0x1010101010101ULL, 0xfe00000000000000ULL, 0x0ULL, 0x0ULL},
    {0x2020202020202ULL, 0xfc00000000000000ULL, 0x0ULL, 0x100000000000000ULL},
    {0x4040404040404ULL, 0xf800000000000000ULL, 0x0ULL, 0x300000000000000ULL},
    {0x8080808080808ULL, 0xf000000000000000ULL, 0x0ULL, 0x700000000000000ULL},
    {0x10101010101010ULL, 0xe000000000000000ULL, 0x0ULL, 0xf00000000000000ULL},
    {0x20202020202020ULL, 0xc000000000000000ULL, 0x0ULL, 0x1f00000000000000ULL},
    {0x40404040404040ULL, 0x8000000000000000ULL, 0x0ULL, 0x3f00000000000000ULL},
    {0x80808080808080ULL, 0x0ULL, 0x0ULL, 0x7f00000000000000ULL},
    {0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL}
};

const Bitboard BISHOP_MOVES[65][4] = { // South East, South West, North West, North East
    {0x0ULL, 0x0ULL, 0x8040201008040200ULL, 0x0ULL},
    {0x0ULL, 0x0ULL, 0x80402010080400ULL, 0x100ULL},
    {0x0ULL, 0x0ULL, 0x804020100800ULL, 0x10200ULL},
    {0x0ULL, 0x0ULL, 0x8040201000ULL, 0x1020400ULL},
    {0x0ULL, 0x0ULL, 0x80402000ULL, 0x102040800ULL},
    {0x0ULL, 0x0ULL, 0x804000ULL, 0x10204081000ULL},
    {0x0ULL, 0x0ULL, 0x8000ULL, 0x1020408102000ULL},
    {0x0ULL, 0x0ULL, 0x0ULL, 0x102040810204000ULL},
    {0x0ULL, 0x2ULL, 0x4020100804020000ULL, 0x0ULL},
    {0x1ULL, 0x4ULL, 0x8040201008040000ULL, 0x10000ULL},
    {0x2ULL, 0x8ULL, 0x80402010080000ULL, 0x1020000ULL},
    {0x4ULL, 0x10ULL, 0x804020100000ULL, 0x102040000ULL},
    {0x8ULL, 0x20ULL, 0x8040200000ULL, 0x10204080000ULL},
    {0x10ULL, 0x40ULL, 0x80400000ULL, 0x1020408100000ULL},
    {0x20ULL, 0x80ULL, 0x800000ULL, 0x102040810200000ULL},
    {0x40ULL, 0x0ULL, 0x0ULL, 0x204081020400000ULL},
    {0x0ULL, 0x204ULL, 0x2010080402000000ULL, 0x0ULL},
    {0x100ULL, 0x408ULL, 0x4020100804000000ULL, 0x1000000ULL},
    {0x201ULL, 0x810ULL, 0x8040201008000000ULL, 0x102000000ULL},
    {0x402ULL, 0x1020ULL, 0x80402010000000ULL, 0x10204000000ULL},
    {0x804ULL, 0x2040ULL, 0x804020000000ULL, 0x1020408000000ULL},
    {0x1008ULL, 0x4080ULL, 0x8040000000ULL, 0x102040810000000ULL},
    {0x2010ULL, 0x8000ULL, 0x80000000ULL, 0x204081020000000ULL},
    {0x4020ULL, 0x0ULL, 0x0ULL, 0x408102040000000ULL},
    {0x0ULL, 0x20408ULL, 0x1008040200000000ULL, 0x0ULL},
    {0x10000ULL, 0x40810ULL, 0x2010080400000000ULL, 0x100000000ULL},
    {0x20100ULL, 0x81020ULL, 0x4020100800000000ULL, 0x10200000000ULL},
    {0x40201ULL, 0x102040ULL, 0x8040201000000000ULL, 0x1020400000000ULL},
    {0x80402ULL, 0x204080ULL, 0x80402000000000ULL, 0x102040800000000ULL},
    {0x100804ULL, 0x408000ULL, 0x804000000000ULL, 0x204081000000000ULL},
    {0x201008ULL, 0x800000ULL, 0x8000000000ULL, 0x408102000000000ULL},
    {0x402010ULL, 0x0ULL, 0x0ULL, 0x810204000000000ULL},
    {0x0ULL, 0x2040810ULL, 0x804020000000000ULL, 0x0ULL},
    {0x1000000ULL, 0x4081020ULL, 0x1008040000000000ULL, 0x10000000000ULL},
    {0x2010000ULL, 0x8102040ULL, 0x2010080000000000ULL, 0x1020000000000ULL},
    {0x4020100ULL, 0x10204080ULL, 0x4020100000000000ULL, 0x102040000000000ULL},
    {0x8040201ULL, 0x20408000ULL, 0x8040200000000000ULL, 0x204080000000000ULL},
    {0x10080402ULL, 0x40800000ULL, 0x80400000000000ULL, 0x408100000000000ULL},
    {0x20100804ULL, 0x80000000ULL, 0x800000000000ULL, 0x810200000000000ULL},
    {0x40201008ULL, 0x0ULL, 0x0ULL, 0x1020400000000000ULL},
    {0x0ULL, 0x204081020ULL, 0x402000000000000ULL, 0x0ULL},
    {0x100000000ULL, 0x408102040ULL, 0x804000000000000ULL, 0x1000000000000ULL},
    {0x201000000ULL, 0x810204080ULL, 0x1008000000000000ULL, 0x102000000000000ULL},
    {0x402010000ULL, 0x1020408000ULL, 0x2010000000000000ULL, 0x204000000000000ULL},
    {0x804020100ULL, 0x2040800000ULL, 0x4020000000000000ULL, 0x408000000000000ULL},
    {0x1008040201ULL, 0x4080000000ULL, 0x8040000000000000ULL, 0x810000000000000ULL},
    {0x2010080402ULL, 0x8000000000ULL, 0x80000000000000ULL, 0x1020000000000000ULL},
    {0x4020100804ULL, 0x0ULL, 0x0ULL, 0x2040000000000000ULL},
    {0x0ULL, 0x20408102040ULL, 0x200000000000000ULL, 0x0ULL},
    {0x10000000000ULL, 0x40810204080ULL, 0x400000000000000ULL, 0x100000000000000ULL},
    {0x20100000000ULL, 0x81020408000ULL, 0x800000000000000ULL, 0x200000000000000ULL},
    {0x40201000000ULL, 0x102040800000ULL, 0x1000000000000000ULL, 0x400000000000000ULL},
    {0x80402010000ULL, 0x204080000000ULL, 0x2000000000000000ULL, 0x800000000000000ULL},
    {0x100804020100ULL, 0x408000000000ULL, 0x4000000000000000ULL, 0x1000000000000000ULL},
    {0x201008040201ULL, 0x800000000000ULL, 0x8000000000000000ULL, 0x2000000000000000ULL},
    {0x402010080402ULL, 0x0ULL, 0x0ULL, 0x4000000000000000ULL},
    {0x0ULL, 0x2040810204080ULL, 0x0ULL, 0x0ULL},
    {0x1000000000000ULL, 0x4081020408000ULL, 0x0ULL, 0x0ULL},
    {0x2010000000000ULL, 0x8102040800000ULL, 0x0ULL, 0x0ULL},
    {0x4020100000000ULL, 0x10204080000000ULL, 0x0ULL, 0x0ULL},
    {0x8040201000000ULL, 0x20408000000000ULL, 0x0ULL, 0x0ULL},
    {0x10080402010000ULL, 0x40800000000000ULL, 0x0ULL, 0x0ULL},
    {0x20100804020100ULL, 0x80000000000000ULL, 0x0ULL, 0x0ULL},
    {0x40201008040201ULL, 0x0ULL, 0x0ULL, 0x0ULL},
    {0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL}
};

// Each 6-tuple represents:
// 1. Kingside path (White: E1-G1, Black: E8-G8)
// 2. Queenside path (White: E1-C1, Black: E8-C8)
// 3. Queenside path to rook (White: E1-B1, Black: E8-B8)
// 4. King Position (White: E1, Black: E8)
// 5. King Destination Kingside (White: G1, Black: G8)
// 6. King Destination Queenside (White: C1, Black: C8)
const Bitboard CASTLING[2][6] = {
    {0x6ULL, 0x30ULL, 0x70ULL, E1, G1, C1}, // White
    {0x600000000000000ULL, 0x3000000000000000ULL, 0x7000000000000000ULL, E8, G8, C8} // Black
};
