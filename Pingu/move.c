#include "move.h"

int extract_moves_pawns(Bitboard board, int8_t offset, Move* moves, int start, Flag flag) {
    while (board != 0) {
        int pos = LSB(board);
        CLEAR_BIT(board, pos);
        Move* move = &moves[start++];
        move->to = pos; move->from = pos - offset; move->flags = flag;
    }

    return start;
}

int extract_moves_pawns_promotions(Bitboard board, int8_t offset, Move* moves, int start, Flag flag) {
    while (board != 0) {
        int pos = LSB(board);
        CLEAR_BIT(board, pos);
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
        CLEAR_BIT(board, pos);
        Move* move = &moves[start++];
        move->to = pos; move->from = init; move->flags = flag;
    }

    return start;
}

int gen_knight_moves(Board* board, Move* moves, int index) {
    Bitboard knights = get_pieces(board, KNIGHT, board->active_color);
    Bitboard empty = ~get_all_pieces(board);
    Bitboard enemies = get_pieces_color(board, OPPOSITE(board->active_color));

    while (knights != 0) {
        int pos = LSB(knights);
        CLEAR_BIT(knights, pos);
        index = extract_moves(knight_moves[pos] & empty, pos, moves, index, QUIET);
        index = extract_moves(knight_moves[pos] & enemies, pos, moves, index, CAPTURE);
    }

    return index;
}

int gen_king_moves(Board* board, Move* moves, int index) {
    Bitboard king = get_pieces(board, KING, board->active_color);
    Bitboard empty = ~get_all_pieces(board);
    Bitboard enemies = get_pieces_color(board, OPPOSITE(board->active_color));

    int pos = LSB(king);
    index = extract_moves(king_moves[pos] & empty, pos, moves, index, QUIET);
    index = extract_moves(king_moves[pos] & enemies, pos, moves, index, CAPTURE);

    return index;
}

int gen_rook_moves(Board* board, Move* moves, int index) {
    return gen_cardinal_moves(board, moves, index, ROOK);
}

int gen_bishop_moves(Board* board, Move* moves, int index) {
    return gen_intercardinal_moves(board, moves, index, BISHOP);
}

int gen_queen_moves(Board* board, Move* moves, int index) {
    index = gen_cardinal_moves(board, moves, index, QUEEN);
    index = gen_intercardinal_moves(board, moves, index, QUEEN);
    return index;
}

int gen_castle_moves(Board* board, Move* moves, int index) {
    switch_ply(board);
    Bitboard attacks = gen_attacks(board);
    switch_ply(board);

    Bitboard king = get_pieces(board, KING, board->active_color);
    if ((king & attacks) == 0) { // If king is not in check.
        uint8_t color = board->active_color & 1; // Maps White to 0, Black to 1.
        Bitboard kingside = castling[color][KINGSIDE_PATH];
        Bitboard queenside = castling[color][QUEENSIDE_PATH];
        Bitboard queenside_path_rook = castling[color][QUEENSIDE_PATH_TO_ROOK];
        Bitboard king_pos = castling[color][KING_POSITION];
        Bitboard king_dst_kingside = castling[color][KING_DST_KINGSIDE];
        Bitboard king_dst_queenside = castling[color][KING_DST_QUEENSIDE];

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

int gen_cardinal_moves(Board* board, Move* moves, int index, Piece piece) {
    Piece color = board->active_color;
    Bitboard cardinal = get_pieces(board, piece, color);
    Bitboard us = get_pieces_color(board, color);
    Bitboard enemies = get_pieces_color(board, OPPOSITE(color));
    Bitboard empty = ~get_all_pieces(board);

    while (cardinal != 0) {
        int pos = LSB(cardinal);
        CLEAR_BIT(cardinal, pos);

        int ai, qi;
        Bitboard ray, ray_us, ray_enemies;
    
        ray = rook_moves[pos][NORTH];
        ray_enemies = ray & enemies;
        ray_us = ray & us;
        ai = ray_enemies != 0 ? LSB(ray_enemies) : 64;
        qi = ray_us != 0 ? LSB(ray_us) : 64;
        ray ^= rook_moves[ai][NORTH] | rook_moves_inc[qi][NORTH];
        index = extract_moves(ray & enemies, pos, moves, index, CAPTURE);
        index = extract_moves(ray & empty, pos, moves, index, QUIET);

        ray = rook_moves[pos][EAST];
        ray_enemies = ray & enemies;
        ray_us = ray & us;
        ai = ray_enemies != 0 ? MSB(ray_enemies) : 64;
        qi = ray_us != 0 ? MSB(ray_us) : 64;
        ray ^= rook_moves[ai][EAST] | rook_moves_inc[qi][EAST];
        index = extract_moves(ray & enemies, pos, moves, index, CAPTURE);
        index = extract_moves(ray & empty, pos, moves, index, QUIET);

        ray = rook_moves[pos][SOUTH];
        ray_enemies = ray & enemies;
        ray_us = ray & us;
        ai = ray_enemies != 0 ? MSB(ray_enemies) : 64;
        qi = ray_us != 0 ? MSB(ray_us) : 64;
        ray ^= rook_moves[ai][SOUTH] | rook_moves_inc[qi][SOUTH];
        index = extract_moves(ray & enemies, pos, moves, index, CAPTURE);
        index = extract_moves(ray & empty, pos, moves, index, QUIET);

        ray = rook_moves[pos][WEST];
        ray_enemies = ray & enemies;
        ray_us = ray & us;
        ai = ray_enemies != 0 ? LSB(ray_enemies) : 64;
        qi = ray_us != 0 ? LSB(ray_us) : 64;
        ray ^= rook_moves[ai][WEST] | rook_moves_inc[qi][WEST];
        index = extract_moves(ray & enemies, pos, moves, index, CAPTURE);
        index = extract_moves(ray & empty, pos, moves, index, QUIET);
    }

    return index;
}

int gen_intercardinal_moves(Board* board, Move* moves, int index, Piece piece) {
    Piece color = board->active_color;
    Bitboard intercardinal = get_pieces(board, piece, color);
    Bitboard us = get_pieces_color(board, color);
    Bitboard enemies = get_pieces_color(board, OPPOSITE(color));
    Bitboard empty = ~get_all_pieces(board);

    while (intercardinal != 0) {
        int pos = LSB(intercardinal);
        CLEAR_BIT(intercardinal, pos);

        int ai, qi;
        Bitboard ray, ray_enemies, ray_us;

        ray = bishop_moves[pos][SOUTHEAST];
        ray_enemies = ray & enemies;
        ray_us = ray & us;
        ai = ray_enemies != 0 ? MSB(ray_enemies) : 64;
        qi = ray_us != 0 ? MSB(ray_us) : 64;
        ray ^= bishop_moves[ai][SOUTHEAST] | bishop_moves_inc[qi][SOUTHEAST];
        index = extract_moves(ray & enemies, pos, moves, index, CAPTURE);
        index = extract_moves(ray & empty, pos, moves, index, QUIET);

        ray = bishop_moves[pos][SOUTHWEST];
        ray_enemies = ray & enemies;
        ray_us = ray & us;
        ai = ray_enemies != 0 ? MSB(ray_enemies) : 64;
        qi = ray_us != 0 ? MSB(ray_us) : 64;
        ray ^= bishop_moves[ai][SOUTHWEST] | bishop_moves_inc[qi][SOUTHWEST];
        index = extract_moves(ray & enemies, pos, moves, index, CAPTURE);
        index = extract_moves(ray & empty, pos, moves, index, QUIET);

        ray = bishop_moves[pos][NORTHEAST];
        ray_enemies = ray & enemies;
        ray_us = ray & us;
        ai = ray_enemies != 0 ? LSB(ray_enemies) : 64;
        qi = ray_us != 0 ? LSB(ray_us) : 64;
        ray ^= bishop_moves[ai][NORTHEAST] | bishop_moves_inc[qi][NORTHEAST];
        index = extract_moves(ray & enemies, pos, moves, index, CAPTURE);
        index = extract_moves(ray & empty, pos, moves, index, QUIET);

        ray = bishop_moves[pos][NORTHWEST];
        ray_enemies = ray & enemies;
        ray_us = ray & us;
        ai = ray_enemies != 0 ? LSB(ray_enemies) : 64;
        qi = ray_us != 0 ? LSB(ray_us) : 64;
        ray ^= bishop_moves[ai][NORTHWEST] | bishop_moves_inc[qi][NORTHWEST];
        index = extract_moves(ray & enemies, pos, moves, index, CAPTURE);
        index = extract_moves(ray & empty, pos, moves, index, QUIET);
    }

    return index;
}

Bitboard gen_cardinal_attacks(Board* board, Piece piece) {
    Bitboard attacks = 0;

    Piece color = board->active_color;
    Bitboard cardinal = get_pieces(board, piece, color);
    Bitboard us = get_pieces_color(board, color);
    Bitboard enemies = get_pieces_color(board, OPPOSITE(color));

    while (cardinal != 0) {
        int pos = LSB(cardinal);
        CLEAR_BIT(cardinal, pos);

        int ai, qi;
        Bitboard ray, ray_enemies, ray_us;
    
        ray = rook_moves[pos][NORTH];
        ray_enemies = ray & enemies;
        ray_us = ray & us;
        ai = ray_enemies != 0 ? LSB(ray_enemies) : 64;
        qi = ray_us != 0 ? LSB(ray_us) : 64;
        ray ^= rook_moves[ai][NORTH] | rook_moves_inc[qi][NORTH];
        attacks |= ray;

        ray = rook_moves[pos][EAST];
        ray_enemies = ray & enemies;
        ray_us = ray & us;
        ai = ray_enemies != 0 ? MSB(ray_enemies) : 64;
        qi = ray_us != 0 ? MSB(ray_us) : 64;
        ray ^= rook_moves[ai][EAST] | rook_moves_inc[qi][EAST];
        attacks |= ray;

        ray = rook_moves[pos][SOUTH];
        ray_enemies = ray & enemies;
        ray_us = ray & us;
        ai = ray_enemies != 0 ? MSB(ray_enemies) : 64;
        qi = ray_us != 0 ? MSB(ray_us) : 64;
        ray ^= rook_moves[ai][SOUTH] | rook_moves_inc[qi][SOUTH];
        attacks |= ray;

        ray = rook_moves[pos][WEST];
        ray_enemies = ray & enemies;
        ray_us = ray & us;
        ai = ray_enemies != 0 ? LSB(ray_enemies) : 64;
        qi = ray_us != 0 ? LSB(ray_us) : 64;
        ray ^= rook_moves[ai][WEST] | rook_moves_inc[qi][WEST];
        attacks |= ray;
    }

    return attacks;
}

Bitboard gen_intercardinal_attacks(Board* board, Piece piece) {
    Bitboard attacks = 0;

    Piece color = board->active_color;
    Bitboard intercardinal = get_pieces(board, piece, color);
    Bitboard us = get_pieces_color(board, color);
    Bitboard enemies = get_pieces_color(board, OPPOSITE(color));

    while (intercardinal != 0) {
        int pos = LSB(intercardinal);
        CLEAR_BIT(intercardinal, pos);

        int ai, qi;
        Bitboard ray, ray_enemies, ray_us;
    
        ray = bishop_moves[pos][SOUTHEAST];
        ray_enemies = ray & enemies;
        ray_us = ray & us;
        ai = ray_enemies != 0 ? MSB(ray_enemies) : 64;
        qi = ray_us != 0 ? MSB(ray_us) : 64;
        ray ^= bishop_moves[ai][SOUTHEAST] | bishop_moves_inc[qi][SOUTHEAST];
        attacks |= ray;

        ray = bishop_moves[pos][SOUTHWEST];
        ray_enemies = ray & enemies;
        ray_us = ray & us;
        ai = ray_enemies != 0 ? MSB(ray_enemies) : 64;
        qi = ray_us != 0 ? MSB(ray_us) : 64;
        ray ^= bishop_moves[ai][SOUTHWEST] | bishop_moves_inc[qi][SOUTHWEST];
        attacks |= ray;

        ray = bishop_moves[pos][NORTHEAST];
        ray_enemies = ray & enemies;
        ray_us = ray & us;
        ai = ray_enemies != 0 ? LSB(ray_enemies) : 64;
        qi = ray_us != 0 ? LSB(ray_us) : 64;
        ray ^= bishop_moves[ai][NORTHEAST] | bishop_moves_inc[qi][NORTHEAST];
        attacks |= ray;

        ray = bishop_moves[pos][NORTHWEST];
        ray_enemies = ray & enemies;
        ray_us = ray & us;
        ai = ray_enemies != 0 ? LSB(ray_enemies) : 64;
        qi = ray_us != 0 ? LSB(ray_us) : 64;
        ray ^= bishop_moves[ai][NORTHWEST] | bishop_moves_inc[qi][NORTHWEST];
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
        CLEAR_BIT(knights, pos);
        attacks |= knight_moves[pos];
    }

    // King Attacks.
    Bitboard king = get_pieces(board, KING, active);
    attacks |= king_moves[LSB(king)];

    // Sliding Attacks.
    attacks |= gen_cardinal_attacks(board, ROOK);
    attacks |= gen_intercardinal_attacks(board, BISHOP);
    attacks |= gen_cardinal_attacks(board, QUEEN);
    attacks |= gen_intercardinal_attacks(board, QUEEN);
    
    return attacks;
}

int gen_moves(Board* board, Move* moves) {
    int index = 0;

    index = gen_pawn_pushes(board, moves, index);
    index = gen_pawn_captures(board, moves, index);
    index = gen_pawn_promotions_quiets(board, moves, index);
    index = gen_pawn_promotions_captures(board, moves, index);
    index = gen_pawn_en_passant(board, moves, index);

    index = gen_knight_moves(board, moves, index);
    index = gen_king_moves(board, moves, index);
    index = gen_rook_moves(board, moves, index);
    index = gen_bishop_moves(board, moves, index);
    index = gen_queen_moves(board, moves, index);

    index = gen_castle_moves(board, moves, index);

    const Board copy = *board;

    int size = 0;
    for (int i = 0; i < index; i++) {
        const Move* move = &moves[i];
        make_move_cheap(board, move);
        Bitboard king = get_pieces(board, KING, board->active_color);
        switch_ply(board);
        Bitboard attacks = gen_attacks(board);
        // If king is not in check after making the move, then it is legal.
        if ((king & attacks) == 0) {
            moves[size++] = *move;
        }
        *board = copy; // Undo Move.
    }

    return size;
}

void make_move(Board* board, Move* move) {
    uint8_t src = move->from;
    uint8_t dst = move->to;

    Piece src_piece = board->positions[src];
    Piece dst_piece = board->positions[dst];

    Flag flags = move->flags;

    Piece active = board->active_color;
    Piece inactive = OPPOSITE(active);

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
    if (IS_CASTLE(flags)) {
        if (IS_CASTLE_KINGSIDE(flags)) {
            if (WHITE_TO_MOVE(board)) {
                remove_piece(board, ROOK, active, H1);
                add_piece(board, ROOK, active, F1);
            } else {
                remove_piece(board, ROOK, active, H8);
                add_piece(board, ROOK, active, F8);
            }
        } else if (IS_CASTLE_QUEENSIDE(flags)) {
            if (WHITE_TO_MOVE(board)) {
                remove_piece(board, ROOK, active, A1);
                add_piece(board, ROOK, active, D1);
            } else {
                remove_piece(board, ROOK, active, A8);
                add_piece(board, ROOK, active, D8);
            }
        }
    }
}
