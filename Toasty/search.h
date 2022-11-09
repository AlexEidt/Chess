#ifndef SEARCH_H_
#define SEARCH_H_

#include <stdint.h>
#include <stdbool.h>
#include "board.h"
#include "move.h"
#include "hashmap.h"
#include "tinycthread.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define SEARCH_TIMEOUT 1
#define NUM_CPUS 4

#define INF (1 << 25)

typedef struct {
    Board* board;
    bool* stop;
    HashMap* hashmap;
    int depth, alpha, beta;
    Move* selected;
    mtx_t* mutex;
    int cpu;
} SearchArgs;

static int timer(void* arg);
static void start_timer(bool* stop);

void select_move(Board* board, HashMap* hashmap, Move* move);

int search_moves(Board* board, bool* stop, HashMap* hashmap, int depth, int alpha, int beta, Move* selected);
int alpha_beta(Board* board, bool* stop, HashMap* hashmap, int depth, int ply, int alpha, int beta);
int quiescence(Board* board, int alpha, int beta);

void order_moves(Board* board, Move* moves, int size);

#endif