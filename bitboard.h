//
// Created by levigibson on 5/29/21.
//

#ifndef MBBCHESS_BITBOARD_H
#define MBBCHESS_BITBOARD_H

#define U64 uint64_t
#define U32 uint32_t

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

//ENUMS
enum {white, black, both};
enum {p_P, p_N, p_B, p_R, p_Q, p_K, p_p, p_n, p_b, p_r, p_q, p_k};

enum {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1, no_sq
};

#define NO_EVAL (-124)
int32_t tt_size;
int32_t tt_linesize;

//BIT MACROS_____________
#ifndef RELEASE

#define get_bit(bitboard, square)((bitboard) & (1ULL << (square)))

#define set_bit(bitboard, square)\
    assert((get_bit(bitboard, square) == 0ULL));\
    (bitboard) |= (1ULL << (square))

#define pop_bit(bitboard, square)\
    assert(get_bit(bitboard, square));  \
    (bitboard) ^= (1ULL << (square))

#else

#define get_bit(bitboard, square)((bitboard) & (1ULL << (square)))

#define set_bit(bitboard, square)\
    (bitboard) |= (1ULL << (square))

#define pop_bit(bitboard, square)\
    (bitboard) ^= (1ULL << (square))

#endif

//count bits in U64
#define count_bits(bitboard) __builtin_popcountll(bitboard)

//get least significant bit in U64
#define bsf(x) __builtin_ctzll(x)

static inline U64 get_random_U64(){
  return (U64)rand() | ((U64)rand() << 32);
}

void print_bitboard(U64 bitboard);

U64 bishop_relevant_occupancies[64];
U64 rook_relevant_occupancies[64];

U64 set_occupancy(int32_t index, U64 mask);

void init_bitboards();

const extern U64 knight_mask[64];
const extern U64 pawn_mask[2][64];
const extern U64 king_mask[64];

const extern int32_t rook_relevant_occupancy_count[64];
const extern int32_t bishop_relevant_occupancy_count[64];

U64 rook_attacks[64][4096];
U64 bishop_attacks[64][512];

const extern U64 rook_magic_numbers[64];
const extern U64 bishop_magic_numbers[64];

#define get_rook_attacks(square, occupancies) rook_attacks[square][((rook_magic_numbers[square] * ((occupancies) & rook_relevant_occupancies[square])) >> (64-rook_relevant_occupancy_count[square]))]
#define get_bishop_attacks(square, occupancies) bishop_attacks[square][((bishop_magic_numbers[square] * ((occupancies) & bishop_relevant_occupancies[square])) >> (64-bishop_relevant_occupancy_count[square]))]

#define get_queen_attacks(square, occupancies) (get_rook_attacks((square), (occupancies)) | get_bishop_attacks((square), (occupancies)))

#endif //MBBCHESS_BITBOARD_H
