#include <stdlib.h>
#include <search.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <time.h>
#include "search.h"
#include "opening.h"
#include "board.h"
#include "evaluate.h"
#include "move.h"
#include "hashmap.h"

#include <stdio.h>

bool select_move(Board* board, HashMap* hashmap, Move* move) {
    Move moves[MAX_MOVES];
    int n_moves = gen_moves(board, moves);

    if (n_moves == 0) {
        return false;
    }

    if (IN_OPENING_BOOK(board)) {
        // If an opening could be found, make that move.
        if (select_opening(board, moves, n_moves, move)) {
            return true;
        }
    }

    order_moves(board, moves, n_moves);

    hashmap_clear(hashmap);

    int max_index = 0;
    int max_value = INT_MIN;
    int value;
    const Board copy = *board;
    for (int i = 0; i < n_moves; i++) {
        make_move(board, &moves[i]);
        value = iterative_deepening(board, hashmap, 1, 150);
        memcpy(board, &copy, sizeof(Board)); // Undo move.
        if (value > max_value) {
            max_value = value;
            max_index = i;
        }
    }

    if (max_value != -INF && max_value != MATE) {
        memcpy(move, &moves[max_index], sizeof(Move));
        return true;
    }
    return false;
}

int iterative_deepening(Board* board, HashMap* hashmap, int min, int max) {
    clock_t begin = clock();

    int value = INT_MIN;
    for (int depth = min; depth <= max; depth++) {
        value = alpha_beta(board, hashmap, depth, INT_MIN, INT_MAX);
        clock_t end = clock();
        double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        if (time_spent > SEARCH_TIMEOUT) break;
    }

    return value;
}

int alpha_beta(Board* board, HashMap* hashmap, int depth, int alpha, int beta) {
    uint64_t board_hash = hash(board);
    int score;
    if (hashmap_get(hashmap, board_hash, depth, &score)) {
        return score;
    }

    if (depth == 0) {
        return quiescence(board, hashmap, alpha, beta);
    }

    Move moves[MAX_MOVES];
    int n_moves = gen_moves(board, moves);
    if (n_moves == 0) {
        if (is_in_check(board)) {
            hashmap_set(hashmap, board_hash, -INF, depth);
            return -INF;
        }
        hashmap_set(hashmap, board_hash, -MATE, depth);
        return 0;
    }

    order_moves(board, moves, n_moves);
    const Board copy = *board;

    for (int i = 0; i < n_moves; i++) {
        make_move(board, &moves[i]);
        int eval = -alpha_beta(board, hashmap, depth - 1, -beta, -alpha);
        memcpy(board, &copy, sizeof(Board)); // Undo move.

        if (eval >= beta) {
            hashmap_set(hashmap, board_hash, beta, depth);
            return beta;
        }
        if (eval > alpha) alpha = eval;
    }

    hashmap_set(hashmap, board_hash, alpha, depth);

    return alpha;
}

int quiescence(Board* board, HashMap* hashmap, int alpha, int beta) {
    int eval = evaluate(board);
    if (eval >= beta) return beta;
    if (eval > alpha) alpha = eval;

    Move moves[MAX_MOVES];
    int n_moves = gen_captures(board, moves);
    order_moves(board, moves, n_moves);

    const Board copy = *board;

    for (int i = 0; i < n_moves; i++) {
        make_move(board, &moves[i]);
        eval = -quiescence(board, hashmap, -beta, -alpha);
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