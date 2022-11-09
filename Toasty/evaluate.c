#include "evaluate.h"
#include "bitboard.h"
#include "board.h"
#include "move.h"

int evaluate(Board* board) {
    Piece active = board->active_color;
    Piece inactive = OPPOSITE(board->active_color);

    int material = material_eval(board, active) - material_eval(board, inactive);
    int pawns = pawn_structure_eval(board, inactive) - pawn_structure_eval(board, active);
    // score += mop_up_eval(board, material, active);
    int development = piece_square_eval(board, active) - piece_square_eval(board, inactive);
    // score += king_safety_eval(board, active);

    // score -= piece_square_eval(board, inactive);
    // score -= king_safety_eval(board, inactive);

    int score = material + 50 * pawns + 50 * development;

    return active == WHITE ? score : -score;
}

int material_eval(Board* board, Piece color) {
    int score = 0;

    score += COUNT(get_pieces(board, PAWN, color)) * PAWN_VALUE;
    score += COUNT(get_pieces(board, KNIGHT, color)) * KNIGHT_VALUE;
    score += COUNT(get_pieces(board, ROOK, color)) * ROOK_VALUE;
    score += COUNT(get_pieces(board, BISHOP, color)) * BISHOP_VALUE;
    score += COUNT(get_pieces(board, QUEEN, color)) * QUEEN_VALUE;

    // int n_bishops = COUNT(get_pieces(board, BISHOP, color));
    // score += n_bishops * BISHOP_VALUE;
    // // If the player still has both of their bishops, they get a bonus.
    // score += (n_bishops == 2) * BISHOP_BONUS;

    return score;
}

int pawn_structure_eval(Board* board, Piece color) {
    Bitboard pawns = get_pieces(board, PAWN, color);

    int files[10] = {
        0,
        COUNT(pawns & FILEA),
        COUNT(pawns & FILEB),
        COUNT(pawns & FILEC),
        COUNT(pawns & FILED),
        COUNT(pawns & FILEE),
        COUNT(pawns & FILEF),
        COUNT(pawns & FILEG),
        COUNT(pawns & FILEH),
        0
    };

    int stacked = 0;
    int isolated = 0;
    for (int i = 1; i < sizeof(files) / sizeof(int) - 1; i++) {
        stacked += files[i] > 1;
        isolated += (files[i - 1] == 0) & (files[i + 1] == 0);
    }

    return stacked + isolated;
}

int piece_square_eval(Board* board, Piece color) {
    int score = 0;
    for (int i = 0; i < 64; i++) {
        Piece current = get_color(board, i);
        if (current == color) {
            score += pst(board->positions[i], color, i);
        }
    }
    return score * PIECE_SQUARE_BONUS;
}

int pst(Piece piece, Piece color, int index) {
    int si = color == WHITE ? index : 63 - index;
    return PST[piece][si];
}

int king_safety_eval(Board* board, Piece color) {
    Bitboard us = get_pieces_color(board, color);
    Bitboard enemies = get_pieces_color(board, OPPOSITE(color));

    int pos = LSB(get_pieces(board, KING, color));
    // Find the difference in the number of friendly/enemy pieces in the kings quadrant.
    int qdiff = COUNT(QUADRANT[pos] & us) - COUNT(QUADRANT[pos] & enemies);

    return qdiff * KING_QUADRANT_BONUS;
}

int mop_up_eval(Board* board, int material, Piece color) {
    int our_king = LSB(get_pieces(board, KING, color));
    int enemy_king = LSB(get_pieces(board, KING, OPPOSITE(color)));
    // Prefer when opponent king is forced into corners.
    int score = 0;
    score += (KING_ENDGAME_PST[our_king] - KING_ENDGAME_PST[enemy_king]) * KING_CORNER_BONUS;

    // Minimize the distance between kings.
    int rankDistance = ABS((enemy_king - our_king) / 8);
    int fileDistance = ABS((enemy_king & 7) - (our_king & 7));
    score += (14 - (rankDistance + fileDistance)) * KING_DISTANCE_BONUS;

    double endgame = (double) (TOTAL_VALUE - material) / (double) TOTAL_VALUE;

    return (int)((double) score * endgame);
}

const int PIECE_VALUES[7] = {
    0,
    PAWN_VALUE,
    KNIGHT_VALUE,
    KING_VALUE,
    BISHOP_VALUE,
    ROOK_VALUE,
    QUEEN_VALUE
};

// Piece Square Tables.
const int PST[7][64] = {
    { // Empty
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    },
    { // Pawns
        0,  0,  0,  0,  0,  0,  0,  0,
        50, 50, 50, 50, 50, 50, 50, 50,
        10, 10, 20, 30, 30, 20, 10, 10,
        5,  5, 10, 25, 25, 10,  5,  5,
        0,  0,  0, 20, 20,  0,  0,  0,
        5, -5,-10,  0,  0,-10, -5,  5,
        5, 10, 10,-20,-20, 10, 10,  5,
        0,  0,  0,  0,  0,  0,  0,  0
    },
    { // Knights
        -50,-40,-30,-30,-30,-30,-40,-50,
        -40,-20,  0,  0,  0,  0,-20,-40,
        -30,  0, 10, 15, 15, 10,  0,-30,
        -30,  5, 15, 20, 20, 15,  5,-30,
        -30,  0, 15, 20, 20, 15,  0,-30,
        -30,  5, 10, 15, 15, 10,  5,-30,
        -40,-20,  0,  5,  5,  0,-20,-40,
        -50,-40,-30,-30,-30,-30,-40,-50,
    },
    { // King Middle Game
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -20,-30,-30,-40,-40,-30,-30,-20,
        -10,-20,-20,-20,-20,-20,-20,-10,
        20, 20,  0,  0,  0,  0, 20, 20,
        20, 30, 10,  0,  0, 10, 30, 20
    },
    { // Bishops
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -20,-10,-10,-10,-10,-10,-10,-20,
    },
    { // Rooks
        0,  0,  0,  0,  0,  0,  0,  0,
        5, 10, 10, 10, 10, 10, 10,  5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        0,  0,  0,  5,  5,  0,  0,  0
    },
    { // Queens
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5,  5,  5,  5,  0,-10,
        -5,  0,  5,  5,  5,  5,  0, -5,
        -5,  0,  5,  5,  5,  5,  0, -5,
        -10,  5,  5,  5,  5,  5,  0,-10,
        -10,  0,  5,  0,  0,  0,  0,-10,
        -20,-10,-10, -5, -5,-10,-10,-20
    }
};

const int KING_ENDGAME_PST[64] = {
    -50,-40,-30,-20,-20,-30,-40,-50,
    -30,-20,-10,  0,  0,-10,-20,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-30,  0,  0,  0,  0,-30,-30,
    -50,-30,-30,-30,-30,-30,-30,-50
};

const Bitboard QUADRANT[64] = {
    Q1, Q1, Q1, Q1, Q2, Q2, Q2, Q2,
    Q1, Q1, Q1, Q1, Q2, Q2, Q2, Q2,
    Q1, Q1, Q1, Q1, Q2, Q2, Q2, Q2,
    Q1, Q1, Q1, Q1, Q2, Q2, Q2, Q2,
    Q3, Q3, Q3, Q3, Q4, Q4, Q4, Q4,
    Q3, Q3, Q3, Q3, Q4, Q4, Q4, Q4,
    Q3, Q3, Q3, Q3, Q4, Q4, Q4, Q4,
    Q3, Q3, Q3, Q3, Q4, Q4, Q4, Q4
};
