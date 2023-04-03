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
    score += PIECE_VALUES[PROMOTED_PIECE(move->flags)];
    // Captures.
    score += (PIECE_VALUES[dst] * CAPTURE_BONUS - PIECE_VALUES[src]) * IS_CAPTURE(move->flags);

    if (src != PAWN) {
        switch_ply(board);
        Bitboard attacks = gen_attacks(board);
        switch_ply(board);
        // Promote moving away from a piece currently attacked.
        if (((1ULL << move->from) & attacks) != 0) {
            score += PIECE_VALUES[src];
        }
        // Penalize moving to an attacked spot.
        if (((1ULL << move->to) & attacks) != 0) {
            score -= CAPTURE_BONUS * PIECE_VALUES[src];
        }
    }

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

        Bitboard attacks = gen_cardinal_attacks_magic(pos, ~empty);

        if (!captures_only) {
            index = extract_moves(attacks & empty, pos, moves, index, QUIET);
        }
        index = extract_moves(attacks & enemies, pos, moves, index, CAPTURE);
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

        Bitboard attacks = gen_intercardinal_attacks_magic(pos, ~empty);

        if (!captures_only) {
            index = extract_moves(attacks & empty, pos, moves, index, QUIET);
        }
        index = extract_moves(attacks & enemies, pos, moves, index, CAPTURE);
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

        Bitboard us = get_pieces_color(board, board->active_color);
        Bitboard enemies = get_pieces_color(board, OPPOSITE(board->active_color));

        if (can_castle_kingside(board, board->active_color)) {
            if ((CASTLING[color][KINGSIDE_PATH] & (attacks | us | enemies)) == 0) {
                Move* move = &moves[index++];
                move->to = CASTLING[color][KING_DST_KINGSIDE];
                move->from = CASTLING[color][KING_POSITION];
                move->flags = CASTLE_KINGSIDE;
            }
        }
        if (can_castle_queenside(board, board->active_color)) {
            if ((CASTLING[color][QUEENSIDE_PATH] & (attacks | us | enemies)) == 0 &&
                (CASTLING[color][QUEENSIDE_PATH_TO_ROOK] & (us | enemies)) == 0) {
                Move* move = &moves[index++];
                move->to = CASTLING[color][KING_DST_QUEENSIDE];
                move->from = CASTLING[color][KING_POSITION];
                move->flags = CASTLE_QUEENSIDE;
            }
        }
    }

    return index;
}

Bitboard gen_pawn_attacks(Board* board) {
    Bitboard attacks = 0;
    Bitboard pawns = get_pieces(board, PAWN, board->active_color);
    if (WHITE_TO_MOVE(board)) {
        attacks |= (pawns & ~FILEA) << 9;
        attacks |= (pawns & ~FILEH) << 7;
    } else {
        attacks |= (pawns & ~FILEH) >> 9;
        attacks |= (pawns & ~FILEA) >> 7;
    }
    return attacks;
}

Bitboard gen_cardinal_attacks_classical(int position, Bitboard blockers) {
    Bitboard attacks = 0;
    Bitboard ray_blockers;

    attacks |= ROOK_MOVES[position][NORTH];
    ray_blockers = ROOK_MOVES[position][NORTH] & blockers;
    attacks &= ~ROOK_MOVES[ray_blockers != 0 ? LSB(ray_blockers) : 64][NORTH];

    attacks |= ROOK_MOVES[position][EAST];
    ray_blockers = ROOK_MOVES[position][EAST] & blockers;
    attacks &= ~ROOK_MOVES[ray_blockers != 0 ? MSB(ray_blockers) : 64][EAST];

    attacks |= ROOK_MOVES[position][SOUTH];
    ray_blockers = ROOK_MOVES[position][SOUTH] & blockers;
    attacks &= ~ROOK_MOVES[ray_blockers != 0 ? MSB(ray_blockers) : 64][SOUTH];

    attacks |= ROOK_MOVES[position][WEST];
    ray_blockers = ROOK_MOVES[position][WEST] & blockers;
    attacks &= ~ROOK_MOVES[ray_blockers != 0 ? LSB(ray_blockers) : 64][WEST];

    return attacks;
}

Bitboard gen_intercardinal_attacks_classical(int position, Bitboard blockers) {
    Bitboard attacks = 0;
    Bitboard ray_blockers;
    
    attacks |= BISHOP_MOVES[position][SOUTHEAST];
    ray_blockers = BISHOP_MOVES[position][SOUTHEAST] & blockers;
    attacks &= ~BISHOP_MOVES[ray_blockers != 0 ? MSB(ray_blockers) : 64][SOUTHEAST];

    attacks |= BISHOP_MOVES[position][SOUTHWEST];
    ray_blockers = BISHOP_MOVES[position][SOUTHWEST] & blockers;
    attacks &= ~BISHOP_MOVES[ray_blockers != 0 ? MSB(ray_blockers) : 64][SOUTHWEST];

    attacks |= BISHOP_MOVES[position][NORTHEAST];
    ray_blockers = BISHOP_MOVES[position][NORTHEAST] & blockers;
    attacks &= ~BISHOP_MOVES[ray_blockers != 0 ? LSB(ray_blockers) : 64][NORTHEAST];

    attacks |= BISHOP_MOVES[position][NORTHWEST];
    ray_blockers = BISHOP_MOVES[position][NORTHWEST] & blockers;
    attacks &= ~BISHOP_MOVES[ray_blockers != 0 ? LSB(ray_blockers) : 64][NORTHWEST];

    return attacks;
}

Bitboard gen_cardinal_attacks_magic(int position, Bitboard blockers) {
    Bitboard key = (blockers & ROOK_BLOCKER_MASK[position]) * ROOK_MAGIC[position];
    key >>= (64 - ROOK_OFFSET[position]);
    return ROOK_TABLE[position][key];
}

Bitboard gen_intercardinal_attacks_magic(int position, Bitboard blockers) {
    Bitboard key = (blockers & BISHOP_BLOCKER_MASK[position]) * BISHOP_MAGIC[position];
    key >>= (64 - BISHOP_OFFSET[position]);
    return BISHOP_TABLE[position][key];
}

Bitboard gen_attacks(Board* board) {
    Bitboard attacks = 0;

    Piece active = board->active_color;
    Piece inactive = OPPOSITE(active);
    Bitboard empty = ~get_all_pieces(board);

    // Pawn Attacks.
    attacks |= gen_pawn_attacks(board);

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

    Bitboard rooks = get_pieces(board, ROOK, active);
    Bitboard bishops = get_pieces(board, BISHOP, active);
    Bitboard queens = get_pieces(board, QUEEN, active);
    // Sliding Attacks.
    Bitboard sliding_cardinal = rooks | queens;
    while (sliding_cardinal != 0) {
        int pos = LSB(sliding_cardinal);
        sliding_cardinal &= sliding_cardinal - 1;
        attacks |= gen_cardinal_attacks_magic(pos, ~empty);
    }
    Bitboard sliding_intercardinal = bishops | queens;
    while (sliding_intercardinal != 0) {
        int pos = LSB(sliding_intercardinal);
        sliding_intercardinal &= sliding_intercardinal - 1;
        attacks |= gen_intercardinal_attacks_magic(pos, ~empty);
    }

    return attacks;
}

int gen_moves(Board* board, Move* moves) {
    int index = 0;

    index = gen_king_moves(board, moves, index, false);

    index = gen_pawn_promotions_quiets(board, moves, index);
    index = gen_pawn_promotions_captures(board, moves, index);
    index = gen_pawn_captures(board, moves, index);

    index = gen_knight_moves(board, moves, index, false);
    index = gen_cardinal_moves(board, moves, index, false);
    index = gen_intercardinal_moves(board, moves, index, false);

    index = gen_pawn_pushes(board, moves, index);
    index = gen_pawn_en_passant(board, moves, index);

    index = filter_legal(board, moves, index);

    // gen_castle_moves generates legal castling moves only.
    if (can_castle_color(board, board->active_color)) {
        index = gen_castle_moves(board, moves, index);
    }

    return index;
}

int gen_captures(Board* board, Move* moves) {
    int index = 0;

    index = gen_king_moves(board, moves, index, true);

    index = gen_pawn_promotions_captures(board, moves, index);
    index = gen_pawn_captures(board, moves, index);

    index = gen_knight_moves(board, moves, index, true);
    index = gen_cardinal_moves(board, moves, index, true);
    index = gen_intercardinal_moves(board, moves, index, true);

    index = gen_pawn_en_passant(board, moves, index);

    return filter_legal(board, moves, index);
}

Bitboard gen_checkers(Board* board, int position) {
    Piece active = board->active_color;
    Piece inactive = OPPOSITE(active);

    Bitboard checks = 0;

    Bitboard piece = 1ULL << position;
    checks |= KING_MOVES[position] & get_pieces(board, KING, inactive);

    // Knight Checks.
    checks |= KNIGHT_MOVES[position] & get_pieces(board, KNIGHT, inactive);

    // Pawn Checks.
    Bitboard pawns = get_pieces(board, PAWN, inactive);
    if (WHITE_TO_MOVE(board)) {
        checks |= ((piece & ~FILEA) << 9) & pawns;
        checks |= ((piece & ~FILEH) << 7) & pawns;
    } else {
        checks |= ((piece & ~FILEH) >> 9) & pawns;
        checks |= ((piece & ~FILEA) >> 7) & pawns;
    }

    // Sliding Checks.
    Bitboard rooks = get_pieces(board, ROOK, inactive);
    Bitboard bishops = get_pieces(board, BISHOP, inactive);
    Bitboard queens = get_pieces(board, QUEEN, inactive);
    Bitboard all = get_all_pieces(board);
    checks |= gen_cardinal_attacks_magic(position, all) & (rooks | queens);
    checks |= gen_intercardinal_attacks_magic(position, all) & (bishops | queens);

    return checks;
}

int filter_legal(Board* board, Move* moves, int size) {
    const Board copy = *board;
    int king_pos = LSB(get_pieces(board, KING, board->active_color));

    int n_legal = 0;
    for (int i = 0; i < size; i++) {
        Move* move = &moves[i];
        int pos = board->positions[move->from] == KING ? move->to : king_pos;
        make_move_cheap(board, move);
        // If king is not in check after making the move, then it is legal.
        if (gen_checkers(board, pos) == 0) {
            moves[n_legal++] = *move;
        }
        *board = copy; // Undo move.
    }

    return n_legal;
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
