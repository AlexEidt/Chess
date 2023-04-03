# Chess

A Chess Engine with a GUI.

<p align="center">
    <img src="https://github.com/AlexEidt/docs/blob/master/Toasty/Toasty-Chess.gif" alt="Chess Engine Demo" />
</p>

## Features

* Bitboard Board Representation
* Magic Bitboard Sliding Move Generation
* Opening Book based on ~8000 games
* Move Searching using Minimax with Alpha-Beta pruning, MTDF, Null Move Pruning, Move Ordering, Quiescence Search, Memoization, and Iterative Deepening

## Usage

Below is an example showing usage of the chess engine without a GUI.

```C
#include "Chess/bitboard.h"
#include "Chess/board.h"
#include "Chess/move.h"
#include "Chess/search.h"
#include "Chess/hashmap.h"

int main() {
    init_magic_tables();
    Board board;
    board_from_fen(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    HashMap* hashmap = hashmap_alloc(20);

    Move selected;
    Move moves[MAX_MOVES];

    while (1) {
        int n_moves = gen_moves(&board, moves); // Generate all possible moves.
        Move move = moves[...]; // Select a move.
        make_move(&board, &move); // Make the move on the board.

        // Then the AI selects a move. If it could not, then you've either won or the game is
        // in stalemate.
        if (!select_move(&board, hashmap, &selected)) {
            if (is_in_check(&board)) {
                // Checkmate.
            } else {
                // Stalemate.
            }
            break;
        }
        make_move(&board, &selected);
    }

    hashmap_free(hashmap);
    
    return 0;
}
```

## Compilation

Currently only configured for Windows. Compilation commands available in `Makefile`.

```bash
# Chess GUI
make chess

# Perft Tests
make perft
perft <depth>
```

## Resources

The following resources/projects were very helpful in the creation of this Chess Engine.

* [Blog Post by Josh Ervin on Move Generation](https://www.josherv.in/2021/03/19/chess-1/)
* [Blog Post by Rhys Rustad-Elliott on Magic Bitboards](https://rhysre.net/fast-chess-move-generation-with-magic-bitboards.html)
* [Blog Post by Markus Boeck about Chess Engines](https://markus7800.github.io/blog/AI/chess_engine.html)
* [Michael Fogleman's MisterQueen Chess Engine](https://github.com/fogleman/MisterQueen)