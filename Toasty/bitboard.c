#include "bitboard.h"
#include "move.h"

Bitboard get_blocker(Bitboard mask, int square) {
    Bitboard blockers = 0ULL;
    int bits = COUNT(mask);
    for (int i = 0; i < bits; i++) {
        int pos = LSB(mask);
        mask &= mask - 1;
        if (square & (1 << i)) {
            blockers |= (1ULL << pos);
        }
    }
    return blockers;
}

void init_magic_tables() {
    init_rook_table();
    init_bishop_table();
}

void init_rook_table() {
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < (1 << ROOK_OFFSET[i]); j++) {
            Bitboard blockers = get_blocker(ROOK_BLOCKER_MASK[i], j);
            Bitboard key = (blockers * ROOK_MAGIC[i]) >> (64 - ROOK_OFFSET[i]);
            ROOK_TABLE[i][key] = gen_cardinal_attacks_classical(i, blockers);
        }
    }
}

void init_bishop_table() {
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < (1 << BISHOP_OFFSET[i]); j++) {
            Bitboard blockers = get_blocker(BISHOP_BLOCKER_MASK[i], j);
            Bitboard key = (blockers * BISHOP_MAGIC[i]) >> (64 - BISHOP_OFFSET[i]);
            BISHOP_TABLE[i][key] = gen_intercardinal_attacks_classical(i, blockers);
        }
    }
}

// All possible king moves for each square.
const Bitboard KING_MOVES[64] = {
    0x302ULL, 0x705ULL, 0xe0aULL, 0x1c14ULL,
    0x3828ULL, 0x7050ULL, 0xe0a0ULL, 0xc040ULL,
    0x30203ULL, 0x70507ULL, 0xe0a0eULL, 0x1c141cULL,
    0x382838ULL, 0x705070ULL, 0xe0a0e0ULL, 0xc040c0ULL,
    0x3020300ULL, 0x7050700ULL, 0xe0a0e00ULL, 0x1c141c00ULL,
    0x38283800ULL, 0x70507000ULL, 0xe0a0e000ULL, 0xc040c000ULL,
    0x302030000ULL, 0x705070000ULL, 0xe0a0e0000ULL, 0x1c141c0000ULL,
    0x3828380000ULL, 0x7050700000ULL, 0xe0a0e00000ULL, 0xc040c00000ULL,
    0x30203000000ULL, 0x70507000000ULL, 0xe0a0e000000ULL, 0x1c141c000000ULL,
    0x382838000000ULL, 0x705070000000ULL, 0xe0a0e0000000ULL, 0xc040c0000000ULL,
    0x3020300000000ULL, 0x7050700000000ULL, 0xe0a0e00000000ULL, 0x1c141c00000000ULL,
    0x38283800000000ULL, 0x70507000000000ULL, 0xe0a0e000000000ULL, 0xc040c000000000ULL,
    0x302030000000000ULL, 0x705070000000000ULL, 0xe0a0e0000000000ULL, 0x1c141c0000000000ULL,
    0x3828380000000000ULL, 0x7050700000000000ULL, 0xe0a0e00000000000ULL, 0xc040c00000000000ULL,
    0x203000000000000ULL, 0x507000000000000ULL, 0xa0e000000000000ULL, 0x141c000000000000ULL,
    0x2838000000000000ULL, 0x5070000000000000ULL, 0xa0e0000000000000ULL, 0x40c0000000000000ULL
};

// All possible knight moves for each square.
const Bitboard KNIGHT_MOVES[64] = {
    0x20400ULL, 0x50800ULL, 0xa1100ULL, 0x142200ULL,
    0x284400ULL, 0x508800ULL, 0xa01000ULL, 0x402000ULL,
    0x2040004ULL, 0x5080008ULL, 0xa110011ULL, 0x14220022ULL,
    0x28440044ULL, 0x50880088ULL, 0xa0100010ULL, 0x40200020ULL,
    0x204000402ULL, 0x508000805ULL, 0xa1100110aULL, 0x1422002214ULL,
    0x2844004428ULL, 0x5088008850ULL, 0xa0100010a0ULL, 0x4020002040ULL,
    0x20400040200ULL, 0x50800080500ULL, 0xa1100110a00ULL, 0x142200221400ULL,
    0x284400442800ULL, 0x508800885000ULL, 0xa0100010a000ULL, 0x402000204000ULL,
    0x2040004020000ULL, 0x5080008050000ULL, 0xa1100110a0000ULL, 0x14220022140000ULL,
    0x28440044280000ULL, 0x50880088500000ULL, 0xa0100010a00000ULL, 0x40200020400000ULL,
    0x204000402000000ULL, 0x508000805000000ULL, 0xa1100110a000000ULL, 0x1422002214000000ULL,
    0x2844004428000000ULL, 0x5088008850000000ULL, 0xa0100010a0000000ULL, 0x4020002040000000ULL,
    0x400040200000000ULL, 0x800080500000000ULL, 0x1100110a00000000ULL, 0x2200221400000000ULL,
    0x4400442800000000ULL, 0x8800885000000000ULL, 0x100010a000000000ULL, 0x2000204000000000ULL,
    0x4020000000000ULL, 0x8050000000000ULL, 0x110a0000000000ULL, 0x22140000000000ULL,
    0x44280000000000ULL, 0x88500000000000ULL, 0x10a00000000000ULL, 0x20400000000000ULL
};

const Bitboard ROOK_MOVES[65][4] = { // South, West, North, East
    {0x0ULL, 0xfeULL, 0x101010101010100ULL, 0x0ULL},
    {0x0ULL, 0xfcULL, 0x202020202020200ULL, 0x1ULL},
    {0x0ULL, 0xf8ULL, 0x404040404040400ULL, 0x3ULL},
    {0x0ULL, 0xf0ULL, 0x808080808080800ULL, 0x7ULL},
    {0x0ULL, 0xe0ULL, 0x1010101010101000ULL, 0xfULL},
    {0x0ULL, 0xc0ULL, 0x2020202020202000ULL, 0x1fULL},
    {0x0ULL, 0x80ULL, 0x4040404040404000ULL, 0x3fULL},
    {0x0ULL, 0x0ULL, 0x8080808080808000ULL, 0x7fULL},
    {0x1ULL, 0xfe00ULL, 0x101010101010000ULL, 0x0ULL},
    {0x2ULL, 0xfc00ULL, 0x202020202020000ULL, 0x100ULL},
    {0x4ULL, 0xf800ULL, 0x404040404040000ULL, 0x300ULL},
    {0x8ULL, 0xf000ULL, 0x808080808080000ULL, 0x700ULL},
    {0x10ULL, 0xe000ULL, 0x1010101010100000ULL, 0xf00ULL},
    {0x20ULL, 0xc000ULL, 0x2020202020200000ULL, 0x1f00ULL},
    {0x40ULL, 0x8000ULL, 0x4040404040400000ULL, 0x3f00ULL},
    {0x80ULL, 0x0ULL, 0x8080808080800000ULL, 0x7f00ULL},
    {0x101ULL, 0xfe0000ULL, 0x101010101000000ULL, 0x0ULL},
    {0x202ULL, 0xfc0000ULL, 0x202020202000000ULL, 0x10000ULL},
    {0x404ULL, 0xf80000ULL, 0x404040404000000ULL, 0x30000ULL},
    {0x808ULL, 0xf00000ULL, 0x808080808000000ULL, 0x70000ULL},
    {0x1010ULL, 0xe00000ULL, 0x1010101010000000ULL, 0xf0000ULL},
    {0x2020ULL, 0xc00000ULL, 0x2020202020000000ULL, 0x1f0000ULL},
    {0x4040ULL, 0x800000ULL, 0x4040404040000000ULL, 0x3f0000ULL},
    {0x8080ULL, 0x0ULL, 0x8080808080000000ULL, 0x7f0000ULL},
    {0x10101ULL, 0xfe000000ULL, 0x101010100000000ULL, 0x0ULL},
    {0x20202ULL, 0xfc000000ULL, 0x202020200000000ULL, 0x1000000ULL},
    {0x40404ULL, 0xf8000000ULL, 0x404040400000000ULL, 0x3000000ULL},
    {0x80808ULL, 0xf0000000ULL, 0x808080800000000ULL, 0x7000000ULL},
    {0x101010ULL, 0xe0000000ULL, 0x1010101000000000ULL, 0xf000000ULL},
    {0x202020ULL, 0xc0000000ULL, 0x2020202000000000ULL, 0x1f000000ULL},
    {0x404040ULL, 0x80000000ULL, 0x4040404000000000ULL, 0x3f000000ULL},
    {0x808080ULL, 0x0ULL, 0x8080808000000000ULL, 0x7f000000ULL},
    {0x1010101ULL, 0xfe00000000ULL, 0x101010000000000ULL, 0x0ULL},
    {0x2020202ULL, 0xfc00000000ULL, 0x202020000000000ULL, 0x100000000ULL},
    {0x4040404ULL, 0xf800000000ULL, 0x404040000000000ULL, 0x300000000ULL},
    {0x8080808ULL, 0xf000000000ULL, 0x808080000000000ULL, 0x700000000ULL},
    {0x10101010ULL, 0xe000000000ULL, 0x1010100000000000ULL, 0xf00000000ULL},
    {0x20202020ULL, 0xc000000000ULL, 0x2020200000000000ULL, 0x1f00000000ULL},
    {0x40404040ULL, 0x8000000000ULL, 0x4040400000000000ULL, 0x3f00000000ULL},
    {0x80808080ULL, 0x0ULL, 0x8080800000000000ULL, 0x7f00000000ULL},
    {0x101010101ULL, 0xfe0000000000ULL, 0x101000000000000ULL, 0x0ULL},
    {0x202020202ULL, 0xfc0000000000ULL, 0x202000000000000ULL, 0x10000000000ULL},
    {0x404040404ULL, 0xf80000000000ULL, 0x404000000000000ULL, 0x30000000000ULL},
    {0x808080808ULL, 0xf00000000000ULL, 0x808000000000000ULL, 0x70000000000ULL},
    {0x1010101010ULL, 0xe00000000000ULL, 0x1010000000000000ULL, 0xf0000000000ULL},
    {0x2020202020ULL, 0xc00000000000ULL, 0x2020000000000000ULL, 0x1f0000000000ULL},
    {0x4040404040ULL, 0x800000000000ULL, 0x4040000000000000ULL, 0x3f0000000000ULL},
    {0x8080808080ULL, 0x0ULL, 0x8080000000000000ULL, 0x7f0000000000ULL},
    {0x10101010101ULL, 0xfe000000000000ULL, 0x100000000000000ULL, 0x0ULL},
    {0x20202020202ULL, 0xfc000000000000ULL, 0x200000000000000ULL, 0x1000000000000ULL},
    {0x40404040404ULL, 0xf8000000000000ULL, 0x400000000000000ULL, 0x3000000000000ULL},
    {0x80808080808ULL, 0xf0000000000000ULL, 0x800000000000000ULL, 0x7000000000000ULL},
    {0x101010101010ULL, 0xe0000000000000ULL, 0x1000000000000000ULL, 0xf000000000000ULL},
    {0x202020202020ULL, 0xc0000000000000ULL, 0x2000000000000000ULL, 0x1f000000000000ULL},
    {0x404040404040ULL, 0x80000000000000ULL, 0x4000000000000000ULL, 0x3f000000000000ULL},
    {0x808080808080ULL, 0x0ULL, 0x8000000000000000ULL, 0x7f000000000000ULL},
    {0x1010101010101ULL, 0xfe00000000000000ULL, 0x0ULL, 0x0ULL},
    {0x2020202020202ULL, 0xfc00000000000000ULL, 0x0ULL, 0x100000000000000ULL},
    {0x4040404040404ULL, 0xf800000000000000ULL, 0x0ULL, 0x300000000000000ULL},
    {0x8080808080808ULL, 0xf000000000000000ULL, 0x0ULL, 0x700000000000000ULL},
    {0x10101010101010ULL, 0xe000000000000000ULL, 0x0ULL, 0xf00000000000000ULL},
    {0x20202020202020ULL, 0xc000000000000000ULL, 0x0ULL, 0x1f00000000000000ULL},
    {0x40404040404040ULL, 0x8000000000000000ULL, 0x0ULL, 0x3f00000000000000ULL},
    {0x80808080808080ULL, 0x0ULL, 0x0ULL, 0x7f00000000000000ULL},
    {0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL}
};

const Bitboard BISHOP_MOVES[65][4] = { // South East, South West, North West, North East
    {0x0ULL, 0x0ULL, 0x8040201008040200ULL, 0x0ULL},
    {0x0ULL, 0x0ULL, 0x80402010080400ULL, 0x100ULL},
    {0x0ULL, 0x0ULL, 0x804020100800ULL, 0x10200ULL},
    {0x0ULL, 0x0ULL, 0x8040201000ULL, 0x1020400ULL},
    {0x0ULL, 0x0ULL, 0x80402000ULL, 0x102040800ULL},
    {0x0ULL, 0x0ULL, 0x804000ULL, 0x10204081000ULL},
    {0x0ULL, 0x0ULL, 0x8000ULL, 0x1020408102000ULL},
    {0x0ULL, 0x0ULL, 0x0ULL, 0x102040810204000ULL},
    {0x0ULL, 0x2ULL, 0x4020100804020000ULL, 0x0ULL},
    {0x1ULL, 0x4ULL, 0x8040201008040000ULL, 0x10000ULL},
    {0x2ULL, 0x8ULL, 0x80402010080000ULL, 0x1020000ULL},
    {0x4ULL, 0x10ULL, 0x804020100000ULL, 0x102040000ULL},
    {0x8ULL, 0x20ULL, 0x8040200000ULL, 0x10204080000ULL},
    {0x10ULL, 0x40ULL, 0x80400000ULL, 0x1020408100000ULL},
    {0x20ULL, 0x80ULL, 0x800000ULL, 0x102040810200000ULL},
    {0x40ULL, 0x0ULL, 0x0ULL, 0x204081020400000ULL},
    {0x0ULL, 0x204ULL, 0x2010080402000000ULL, 0x0ULL},
    {0x100ULL, 0x408ULL, 0x4020100804000000ULL, 0x1000000ULL},
    {0x201ULL, 0x810ULL, 0x8040201008000000ULL, 0x102000000ULL},
    {0x402ULL, 0x1020ULL, 0x80402010000000ULL, 0x10204000000ULL},
    {0x804ULL, 0x2040ULL, 0x804020000000ULL, 0x1020408000000ULL},
    {0x1008ULL, 0x4080ULL, 0x8040000000ULL, 0x102040810000000ULL},
    {0x2010ULL, 0x8000ULL, 0x80000000ULL, 0x204081020000000ULL},
    {0x4020ULL, 0x0ULL, 0x0ULL, 0x408102040000000ULL},
    {0x0ULL, 0x20408ULL, 0x1008040200000000ULL, 0x0ULL},
    {0x10000ULL, 0x40810ULL, 0x2010080400000000ULL, 0x100000000ULL},
    {0x20100ULL, 0x81020ULL, 0x4020100800000000ULL, 0x10200000000ULL},
    {0x40201ULL, 0x102040ULL, 0x8040201000000000ULL, 0x1020400000000ULL},
    {0x80402ULL, 0x204080ULL, 0x80402000000000ULL, 0x102040800000000ULL},
    {0x100804ULL, 0x408000ULL, 0x804000000000ULL, 0x204081000000000ULL},
    {0x201008ULL, 0x800000ULL, 0x8000000000ULL, 0x408102000000000ULL},
    {0x402010ULL, 0x0ULL, 0x0ULL, 0x810204000000000ULL},
    {0x0ULL, 0x2040810ULL, 0x804020000000000ULL, 0x0ULL},
    {0x1000000ULL, 0x4081020ULL, 0x1008040000000000ULL, 0x10000000000ULL},
    {0x2010000ULL, 0x8102040ULL, 0x2010080000000000ULL, 0x1020000000000ULL},
    {0x4020100ULL, 0x10204080ULL, 0x4020100000000000ULL, 0x102040000000000ULL},
    {0x8040201ULL, 0x20408000ULL, 0x8040200000000000ULL, 0x204080000000000ULL},
    {0x10080402ULL, 0x40800000ULL, 0x80400000000000ULL, 0x408100000000000ULL},
    {0x20100804ULL, 0x80000000ULL, 0x800000000000ULL, 0x810200000000000ULL},
    {0x40201008ULL, 0x0ULL, 0x0ULL, 0x1020400000000000ULL},
    {0x0ULL, 0x204081020ULL, 0x402000000000000ULL, 0x0ULL},
    {0x100000000ULL, 0x408102040ULL, 0x804000000000000ULL, 0x1000000000000ULL},
    {0x201000000ULL, 0x810204080ULL, 0x1008000000000000ULL, 0x102000000000000ULL},
    {0x402010000ULL, 0x1020408000ULL, 0x2010000000000000ULL, 0x204000000000000ULL},
    {0x804020100ULL, 0x2040800000ULL, 0x4020000000000000ULL, 0x408000000000000ULL},
    {0x1008040201ULL, 0x4080000000ULL, 0x8040000000000000ULL, 0x810000000000000ULL},
    {0x2010080402ULL, 0x8000000000ULL, 0x80000000000000ULL, 0x1020000000000000ULL},
    {0x4020100804ULL, 0x0ULL, 0x0ULL, 0x2040000000000000ULL},
    {0x0ULL, 0x20408102040ULL, 0x200000000000000ULL, 0x0ULL},
    {0x10000000000ULL, 0x40810204080ULL, 0x400000000000000ULL, 0x100000000000000ULL},
    {0x20100000000ULL, 0x81020408000ULL, 0x800000000000000ULL, 0x200000000000000ULL},
    {0x40201000000ULL, 0x102040800000ULL, 0x1000000000000000ULL, 0x400000000000000ULL},
    {0x80402010000ULL, 0x204080000000ULL, 0x2000000000000000ULL, 0x800000000000000ULL},
    {0x100804020100ULL, 0x408000000000ULL, 0x4000000000000000ULL, 0x1000000000000000ULL},
    {0x201008040201ULL, 0x800000000000ULL, 0x8000000000000000ULL, 0x2000000000000000ULL},
    {0x402010080402ULL, 0x0ULL, 0x0ULL, 0x4000000000000000ULL},
    {0x0ULL, 0x2040810204080ULL, 0x0ULL, 0x0ULL},
    {0x1000000000000ULL, 0x4081020408000ULL, 0x0ULL, 0x0ULL},
    {0x2010000000000ULL, 0x8102040800000ULL, 0x0ULL, 0x0ULL},
    {0x4020100000000ULL, 0x10204080000000ULL, 0x0ULL, 0x0ULL},
    {0x8040201000000ULL, 0x20408000000000ULL, 0x0ULL, 0x0ULL},
    {0x10080402010000ULL, 0x40800000000000ULL, 0x0ULL, 0x0ULL},
    {0x20100804020100ULL, 0x80000000000000ULL, 0x0ULL, 0x0ULL},
    {0x40201008040201ULL, 0x0ULL, 0x0ULL, 0x0ULL},
    {0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL}
};

const Bitboard ROOK_MAGIC[64] = {
    0xa8002c000108020ULL, 0x6c00049b0002001ULL, 0x100200010090040ULL, 0x2480041000800801ULL,
    0x280028004000800ULL, 0x900410008040022ULL, 0x280020001001080ULL, 0x2880002041000080ULL,
    0xa000800080400034ULL, 0x4808020004000ULL, 0x2290802004801000ULL, 0x411000d00100020ULL,
    0x402800800040080ULL, 0xb000401004208ULL, 0x2409000100040200ULL, 0x1002100004082ULL,
    0x22878001e24000ULL, 0x1090810021004010ULL, 0x801030040200012ULL, 0x500808008001000ULL,
    0xa08018014000880ULL, 0x8000808004000200ULL, 0x201008080010200ULL, 0x801020000441091ULL,
    0x800080204005ULL, 0x1040200040100048ULL, 0x120200402082ULL, 0xd14880480100080ULL,
    0x12040280080080ULL, 0x100040080020080ULL, 0x9020010080800200ULL, 0x813241200148449ULL,
    0x491604001800080ULL, 0x100401000402001ULL, 0x4820010021001040ULL, 0x400402202000812ULL,
    0x209009005000802ULL, 0x810800601800400ULL, 0x4301083214000150ULL, 0x204026458e001401ULL,
    0x40204000808000ULL, 0x8001008040010020ULL, 0x8410820820420010ULL, 0x1003001000090020ULL,
    0x804040008008080ULL, 0x12000810020004ULL, 0x1000100200040208ULL, 0x430000a044020001ULL,
    0x280009023410300ULL, 0xe0100040002240ULL, 0x200100401700ULL, 0x2244100408008080ULL,
    0x8000400801980ULL, 0x2000810040200ULL, 0x8010100228810400ULL, 0x2000009044210200ULL,
    0x4080008040102101ULL, 0x40002080411d01ULL, 0x2005524060000901ULL, 0x502001008400422ULL,
    0x489a000810200402ULL, 0x1004400080a13ULL, 0x4000011008020084ULL, 0x26002114058042ULL
};

const Bitboard BISHOP_MAGIC[64] = {
    0x89a1121896040240ULL, 0x2004844802002010ULL, 0x2068080051921000ULL, 0x62880a0220200808ULL,
    0x4042004000000ULL, 0x100822020200011ULL, 0xc00444222012000aULL, 0x28808801216001ULL,
    0x400492088408100ULL, 0x201c401040c0084ULL, 0x840800910a0010ULL, 0x82080240060ULL,
    0x2000840504006000ULL, 0x30010c4108405004ULL, 0x1008005410080802ULL, 0x8144042209100900ULL,
    0x208081020014400ULL, 0x4800201208ca00ULL, 0xf18140408012008ULL, 0x1004002802102001ULL,
    0x841000820080811ULL, 0x40200200a42008ULL, 0x800054042000ULL, 0x88010400410c9000ULL,
    0x520040470104290ULL, 0x1004040051500081ULL, 0x2002081833080021ULL, 0x400c00c010142ULL,
    0x941408200c002000ULL, 0x658810000806011ULL,0x188071040440a00ULL, 0x4800404002011c00ULL,
    0x104442040404200ULL, 0x511080202091021ULL, 0x4022401120400ULL, 0x80c0040400080120ULL,
    0x8040010040820802ULL, 0x480810700020090ULL, 0x102008e00040242ULL, 0x809005202050100ULL,
    0x8002024220104080ULL, 0x431008804142000ULL, 0x19001802081400ULL, 0x200014208040080ULL,
    0x3308082008200100ULL, 0x41010500040c020ULL, 0x4012020c04210308ULL, 0x208220a202004080ULL,
    0x111040120082000ULL, 0x6803040141280a00ULL, 0x2101004202410000ULL, 0x8200000041108022ULL,
    0x21082088000ULL, 0x2410204010040ULL, 0x40100400809000ULL, 0x822088220820214ULL,
    0x40808090012004ULL, 0x910224040218c9ULL, 0x402814422015008ULL, 0x90014004842410ULL,
    0x1000042304105ULL, 0x10008830412a00ULL, 0x2520081090008908ULL, 0x40102000a0a60140ULL
};

const int ROOK_OFFSET[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12
};

const int BISHOP_OFFSET[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6
};

const Bitboard ROOK_BLOCKER_MASK[64] = {
    0x101010101017eULL, 0x202020202027cULL, 0x404040404047aULL, 0x8080808080876ULL,
    0x1010101010106eULL, 0x2020202020205eULL, 0x4040404040403eULL, 0x8080808080807eULL,
    0x1010101017e00ULL, 0x2020202027c00ULL, 0x4040404047a00ULL, 0x8080808087600ULL,
    0x10101010106e00ULL, 0x20202020205e00ULL, 0x40404040403e00ULL, 0x80808080807e00ULL,
    0x10101017e0100ULL, 0x20202027c0200ULL, 0x40404047a0400ULL, 0x8080808760800ULL,
    0x101010106e1000ULL, 0x202020205e2000ULL, 0x404040403e4000ULL, 0x808080807e8000ULL,
    0x101017e010100ULL, 0x202027c020200ULL, 0x404047a040400ULL, 0x8080876080800ULL,
    0x1010106e101000ULL, 0x2020205e202000ULL, 0x4040403e404000ULL, 0x8080807e808000ULL,
    0x1017e01010100ULL, 0x2027c02020200ULL, 0x4047a04040400ULL, 0x8087608080800ULL,
    0x10106e10101000ULL, 0x20205e20202000ULL, 0x40403e40404000ULL, 0x80807e80808000ULL,
    0x17e0101010100ULL, 0x27c0202020200ULL, 0x47a0404040400ULL, 0x8760808080800ULL,
    0x106e1010101000ULL, 0x205e2020202000ULL, 0x403e4040404000ULL, 0x807e8080808000ULL,
    0x7e010101010100ULL, 0x7c020202020200ULL, 0x7a040404040400ULL, 0x76080808080800ULL,
    0x6e101010101000ULL, 0x5e202020202000ULL, 0x3e404040404000ULL, 0x7e808080808000ULL,
    0x7e01010101010100ULL, 0x7c02020202020200ULL, 0x7a04040404040400ULL, 0x7608080808080800ULL,
    0x6e10101010101000ULL, 0x5e20202020202000ULL, 0x3e40404040404000ULL, 0x7e80808080808000ULL
};

const Bitboard BISHOP_BLOCKER_MASK[64] = {
    0x40201008040200ULL, 0x402010080400ULL, 0x4020100a00ULL, 0x40221400ULL,
    0x2442800ULL, 0x204085000ULL, 0x20408102000ULL, 0x2040810204000ULL,
    0x20100804020000ULL, 0x40201008040000ULL, 0x4020100a0000ULL, 0x4022140000ULL,
    0x244280000ULL, 0x20408500000ULL, 0x2040810200000ULL, 0x4081020400000ULL,
    0x10080402000200ULL, 0x20100804000400ULL, 0x4020100a000a00ULL, 0x402214001400ULL,
    0x24428002800ULL, 0x2040850005000ULL, 0x4081020002000ULL, 0x8102040004000ULL,
    0x8040200020400ULL, 0x10080400040800ULL, 0x20100a000a1000ULL, 0x40221400142200ULL,
    0x2442800284400ULL, 0x4085000500800ULL, 0x8102000201000ULL, 0x10204000402000ULL,
    0x4020002040800ULL, 0x8040004081000ULL, 0x100a000a102000ULL, 0x22140014224000ULL,
    0x44280028440200ULL, 0x8500050080400ULL, 0x10200020100800ULL, 0x20400040201000ULL,
    0x2000204081000ULL, 0x4000408102000ULL, 0xa000a10204000ULL, 0x14001422400000ULL,
    0x28002844020000ULL, 0x50005008040200ULL, 0x20002010080400ULL, 0x40004020100800ULL,
    0x20408102000ULL, 0x40810204000ULL, 0xa1020400000ULL, 0x142240000000ULL,
    0x284402000000ULL, 0x500804020000ULL, 0x201008040200ULL, 0x402010080400ULL,
    0x2040810204000ULL, 0x4081020400000ULL, 0xa102040000000ULL, 0x14224000000000ULL,
    0x28440200000000ULL, 0x50080402000000ULL, 0x20100804020000ULL, 0x40201008040200ULL
};

Bitboard ROOK_TABLE[64][4096];
Bitboard BISHOP_TABLE[64][512];

// Each 6-tuple represents:
// 1. Kingside path (White: E1-G1, Black: E8-G8)
// 2. Queenside path (White: E1-C1, Black: E8-C8)
// 3. Queenside path to rook (White: E1-B1, Black: E8-B8)
// 4. King Position (White: E1, Black: E8)
// 5. King Destination Kingside (White: G1, Black: G8)
// 6. King Destination Queenside (White: C1, Black: C8)
const Bitboard CASTLING[2][6] = {
    {0x6ULL, 0x30ULL, 0x70ULL, E1, G1, C1}, // White
    {0x600000000000000ULL, 0x3000000000000000ULL, 0x7000000000000000ULL, E8, G8, C8} // Black
};
