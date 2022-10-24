#include <stdio.h>
#include "perft.h"
#include "move.h"

int main() {
    uint64_t expected[] = {1, 20, 400, 8902, 197281, 4865609, 119060324};
    for (int i = 0; i < sizeof(expected) / sizeof(uint64_t); i++) {
        uint64_t actual = perft(i);
        printf("Depth: %d, Expected: %d, Actual: %d\n", i, expected[i], actual);
    }
}

uint64_t perft(int depth) {
    Board* board;
    board_from_fen(board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    return helper(board, depth);
}

uint64_t helper(Board* board, int depth) {
    if (depth == 0) return 1ULL;

    Move moves[MAX_MOVES];
    int n_moves = gen_moves(board, moves);

    Board copy = *board;
    uint64_t nodes = 0;
    for (int i = 0; i < n_moves; i++) {
        make_move(board, &moves[i]);
        nodes += helper(board, depth - 1);
        *board = copy; // Undo move.
    }

    return nodes;
}