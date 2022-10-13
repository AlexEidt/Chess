#include <stdint.h>
#include "bitboard.h"

Bitboard clear_bit(Bitboard board, uint8_t index) {
    return board & ~(1 << index);
}

Bitboard add_bit(Bitboard board, uint8_t index) {
    return board | (1 << index);
}