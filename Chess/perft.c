#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "perft.h"
#include "board.h"
#include "move.h"

int main(int argc, char* args[]) {
    init_magic_tables();
    Board board;
    for (int depth = 1; (depth <= args[1][0] - '0') && depth < 10; depth++) {
        board_from_fen(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

        clock_t start = clock();

        uint64_t moves = perft(&board, depth);

        long end = ((clock() - start) * 1000 / CLOCKS_PER_SEC);
        printf("%llu moves at depth %d (%llu ms)\n", moves, depth, end);
    }

    return 0;
}

uint64_t perft(Board* board, int depth) {
    if (depth == 0) return 1ULL;

    Move moves[MAX_MOVES];
    int n_moves = gen_moves(board, moves);

    const Board copy = *board;
    uint64_t nodes = 0;
    for (int i = 0; i < n_moves; i++) {
        make_move(board, &moves[i]);
        nodes += perft(board, depth - 1);
        *board = copy; // Undo move.
    }

    return nodes;
}