#include <stdlib.h>
#include <string.h>
#include "opening.h"


bool select_opening(Board* board, Move* move) {
    int possible[1024];

    Move moves[256];
    int n_moves = gen_moves(board, moves);

    uint64_t board_hash = hash(board);

    const int openings_size = sizeof(openings) / sizeof(Opening);
    const int possible_size = sizeof(possible) / sizeof(int);

    int n_openings = 0;
    for (int i = 0; i < openings_size && n_openings < possible_size; i++) {
        Opening* opening = &openings[i];
        if (opening->hash == board_hash) {
            Move* move = &opening->move;
            // Check if the given move is valid in case of hash collision.
            for (int j = 0; j < n_moves; j++) {
                Move* check = &moves[j];
                if (check->to == move->to && check->from == move->from && check->flags == move->flags) {
                    possible[n_openings++] = i;
                    break;
                }
            }
        }
    }

    if (n_openings == 0) return false;

    int total = 0;
    for (int i = 0; i < n_openings; i++) {
        total += openings[possible[i]].count;
    }

    // Weighted random selection based on the number of times
    // the opening appears in the database.
    int selection = rand() % total;
    total = 0;
    for (int i = 0; i < n_openings; i++) {
        total += openings[possible[i]].count;
        if (total >= selection) {
            *((uint32_t*) move) = *((uint32_t*) &openings[possible[i]].move);
            // memcpy(move, &openings[possible[i]].move, sizeof(Move));
            return true;
        }
    }

    return false;
}