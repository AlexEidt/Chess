#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "board.h"
#include "move.h"

int main()
{
    FILE* file = fopen("fens.txt", "r");
    FILE* boards = fopen("board.txt", "w");
    FILE* moves = fopen("moves.txt", "w");
    char line[1024];

    Board board;

    int i = 0;

    while (fgets(line, sizeof(line), file)) {
        if (i % 2 == 0) {
            board_from_fen(&board, line);
            fprintf(boards, "%lluULL,\n", hash(&board));
        } else {
            int file1 = line[0] - 'a';
            int rank1 = line[1] - '1';
            int file2 = line[2] - 'a';
            int rank2 = line[3] - '1';
            int src = rank1 * 8 + (7 - file1);
            int dst = rank2 * 8 + (7 - file2);
            uint16_t flags = 0;
            int abs = __builtin_abs(src - dst);
            if (board.positions[src] == PAWN && abs == 16) {
                flags |= PAWN_DOUBLE;
            }
            if (board.positions[dst] != 0) {
                flags |= CAPTURE;
            }
            if (board.positions[src] == KING && abs == 2) {
                if (src - dst < 0) flags |= CASTLE_QUEENSIDE;
                if (src - dst > 0) flags |= CASTLE_KINGSIDE;
            }
            if (board.positions[src] == PAWN && (abs == 7 || abs == 9) && board.positions[dst] == 0) {
                flags |= EN_PASSANT;
            }
            switch (line[4]) {
                case 'n': flags |= ADD_PROMOTED_PIECE(KNIGHT); break;
                case 'b': flags |= ADD_PROMOTED_PIECE(BISHOP); break;
                case 'r': flags |= ADD_PROMOTED_PIECE(ROOK); break;
                case 'q': flags |= ADD_PROMOTED_PIECE(QUEEN); break;
            }
            fprintf(moves, "{%d, %d, 0x%0x},\n", dst, src, flags);
        }
        i++;
    }

    fclose(file);
    fclose(boards);
    fclose(moves);

    return 0;
}
