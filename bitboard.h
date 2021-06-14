//
// Created by levigibson on 5/29/21.
//

#ifndef MBBCHESS_BITBOARD_H
#define MBBCHESS_BITBOARD_H

#define U64 unsigned long long

#include <assert.h>
#include <stdlib.h>

//ENUMS
enum {white, black, both};

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

//BIT MACROS_____________
#define get_bit(bitboard, square) ((bitboard) & (1ULL << (square))) ? 1 : 0
#define set_bit(bitboard, square) ((bitboard) |= (1ULL << (square)))
#define pop_bit(bitboard, square) if (get_bit((bitboard), (square))) {((bitboard) ^= (1ULL << (square)));}


//count bits in U64
static inline int count_bits(U64 bitboard){

    int count = 0;
    while (bitboard){
        count++;
        bitboard &= bitboard-1;
    }
    return count;

}

static inline int get_ls1b_index(U64 x){
  assert (x != 0);
   asm ("bsfq %0, %0" : "=r" (x) : "0" (x));
   return (int) x;
}

static inline U64 get_random_U64(){
  return (U64)rand() | ((U64)rand() << 32);
}

void print_bitboard(U64 bitboard);

U64 knight_mask[64];
U64 pawn_mask[2][64];
U64 king_mask[64];

U64 bishop_relevant_occupancies[64];
U64 rook_relevant_occupancies[64];

U64 set_occupancy(int index, U64 mask);

U64 get_rook_attacks(int square, U64 occupancies);

U64 get_bishop_attacks(int square, U64 occupancies);

void init_bitboards();



#endif //MBBCHESS_BITBOARD_H
