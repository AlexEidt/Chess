#include "search.h"
#include "opening.h"

void select_move(Board* board, Move* move) {
    if (IN_OPENING_BOOK(board)) {
        // If an opening could be found, make that move.
        if (select_opening(board, move)) {
            return;
        }
    }

    // Search.
}