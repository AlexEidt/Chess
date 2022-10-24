#include "evaluate.h"
#include "bitboard.h"

int evaluate(Board* board) {
    // Material Count
    // Two Pawns in the same file
    // Piece square tables

    // Mop up Evaluation for Endgame
}

int evaluate_pawns(Board* board, Piece color) {
    Bitboard pawns = get_pieces(board, PAWN, color);
    int score = 0;
}

int piece_square_eval(Board* board) {

}

int mop_up_eval(Board* board) {

}