#include <stdlib.h>
#include <search.h>
#include <string.h>
#include <limits.h>
#include "search.h"
#include "opening.h"
#include "board.h"
#include "evaluate.h"
#include "move.h"

void select_move(Board* board, Move* move) {
    if (IN_OPENING_BOOK(board)) {
        // If an opening could be found, make that move.
        if (select_opening(board, move)) {
            return;
        }
    }

    Move moves[MAX_MOVES];
    int n_moves = gen_moves(board, moves);

    int scores[MAX_MOVES];

    int max_index = 0;
    int max_value = 0;
    for (int i = 0; i < n_moves; i++) {
        scores[i] = iterative_deepening(board, 1, 10);
        if (scores[i] > max_value) {
            max_value = scores[i];
            max_index = i;
        }
    }

    memcpy(move, &moves[max_index], sizeof(Move));
}

int iterative_deepening(Board* board, int min, int max) {
    int value = 0;

    for (int depth = min; depth < max; depth++) {
        value = mtdf(board, depth, value);
    }

    return value;
}

int mtdf(Board* board, int depth, int value) {
    int upper = INT_MAX;
    int lower = INT_MIN;

    while (lower < upper) {
        int beta = value > lower + 1 ? value : lower + 1;
        value = alpha_beta(board, depth, beta - 1, beta);
        if (value < beta) {
            upper = value;
        } else {
            lower = value;
        }
    }
    
    return value;
}

int alpha_beta(Board* board, int depth, int alpha, int beta) {
    if (depth == 0) {
        return quiescence(board, alpha, beta);
    }

    Move moves[MAX_MOVES];
    int n_moves = gen_moves(board, moves);
    if (n_moves == 0) return -INFINITY;

    order_moves(board, moves, n_moves);
    const Board copy = *board;

    for (int i = 0; i < n_moves; i++) {
        make_move(board, &moves[i]);
        int eval = -alpha_beta(board, depth - 1, -beta, -alpha);
        memcpy(board, &copy, sizeof(Board)); // Undo move.

        if (eval >= beta) return beta;
        if (eval > alpha) alpha = eval;
    }

    return alpha;
}

int quiescence(Board* board, int alpha, int beta) {
    int eval = evaluate(board);
    if (eval >= beta) return beta;
    if (eval > alpha) alpha = eval;

    Move moves[MAX_MOVES];
    int n_moves = gen_captures(board, moves);
    order_moves(board, moves, n_moves);

    const Board copy = *board;

    for (int i = 0; i < n_moves; i++) {
        make_move(board, &moves[i]);
        eval = -quiescence(board, -beta, -alpha);
        memcpy(board, &copy, sizeof(Board)); // Undo move.

        if (eval >= beta) return beta;
        if (eval > alpha) alpha = eval;
    }

    return alpha;
}

void order_moves(Board* board, Move* moves, int size) {
    int scores[MAX_MOVES];

    for (int i = 0; i < size; i++) {
        scores[i] = -score_move(board, &moves[i]);
    }

    // Sort moves based on their scores.
    for (int i = 1; i < size; i++) {
        int j = i;
        while (j > 0 && scores[j - 1] > scores[j]) {
            int temp = scores[j];
            scores[j] = scores[j - 1];
            scores[j - 1] = temp;

            Move temp_move = moves[j];
            moves[j] = moves[j - 1];
            moves[j - 1] = temp_move;
            
            j--;
        }
    }
}