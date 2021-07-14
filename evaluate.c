#include "evaluate.h"
#include "board.h"
#include "bitboard.h"
#include <stdio.h>

const int pst[6][64] = {
        {0,  0,  0,  0,  0,  0,  0,  0,
                50, 50, 50, 50, 50, 50, 50, 50,
                10, 10, 20, 30, 30, 20, 10, 10,
                5,  5, 10, 25, 25, 10,  5,  5,
                0,  0,  0, 20, 20,  0,  0,  0,
                5, -5,-10,  0,  0,-10, -5,  5,
                5, 10, 10,-20,-20, 10, 10,  5,
                0,  0,  0,  0,  0,  0,  0,  0},
        {-50,-40,-30,-30,-30,-30,-40,-50,
                -40,-20,  0,  0,  0,  0,-20,-40,
                -30,  0, 10, 15, 15, 10,  0,-30,
                -30,  5, 15, 20, 20, 15,  5,-30,
                -30,  0, 15, 20, 20, 15,  0,-30,
                -30,  5, 10, 15, 15, 10,  5,-30,
                -40,-20,  0,  5,  5,  0,-20,-40,
                -50,-40,-30,-30,-30,-30,-40,-50},
        {-20,-10,-10,-10,-10,-10,-10,-20,
                -10,  0,  0,  0,  0,  0,  0,-10,
                -10,  0,  5, 10, 10,  5,  0,-10,
                -10,  5,  5, 10, 10,  5,  5,-10,
                -10,  0, 10, 10, 10, 10,  0,-10,
                -10, 10, 10, 10, 10, 10, 10,-10,
                -10,  5,  0,  0,  0,  0,  5,-10,
                -20,-10,-10,-10,-10,-10,-10,-20},
        {0,  0,  0,  0,  0,  0,  0,  0,
                5, 10, 10, 10, 10, 10, 10,  5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                0,  0,  0,  5,  5,  0,  0,  0},
        {-20,-10,-10, -5, -5,-10,-10,-20,
                -10,  0,  0,  0,  0,  0,  0,-10,
                -10,  0,  5,  5,  5,  5,  0,-10,
                -5,  0,  5,  5,  5,  5,  0, -5,
                0,  0,  5,  5,  5,  5,  0, -5,
                -10,  5,  5,  5,  5,  5,  0,-10,
                -10,  0,  5,  0,  0,  0,  0,-10,
                -20,-10,-10, -5, -5,-10,-10,-20},
        {-30,-40,-40,-50,-50,-40,-40,-30,
                -30,-40,-40,-50,-50,-40,-40,-30,
                -30,-40,-40,-50,-50,-40,-40,-30,
                -30,-40,-40,-50,-50,-40,-40,-30,
                -20,-30,-30,-40,-40,-30,-30,-20,
                -10,-20,-20,-20,-20,-20,-20,-10,
                20, 20,  0,  0,  0,  0, 20, 20,
                20, 30, 10,  0,  0, 10, 30, 20}

};

const int scores[] = {100, 300, 325, 500, 900, 0, -100, -300, -325, -500, -900, 0};

int evaluate(){

    int eval = 0;

    for (int piece = P; piece <= K; piece++){
        U64 bitboard = bitboards[piece];

        int bb_count = count_bits(bitboard);
        eval += (scores[piece] * bb_count);

        for (int _ = 0; _ < bb_count; _++) {
            int target = bsf(bitboard);

            eval += pst[piece][target];

            if (piece == B){
                eval += count_bits(get_bishop_attacks(target, occupancies[white] | bitboards[p])) * 10;
            } else if (piece == R){
                eval += count_bits(get_rook_attacks(target, occupancies[white] | bitboards[p])) * 10;
            }

            pop_bit(bitboard, target);
        }
    }

    for (int piece = p; piece <= k; piece++){
        U64 bitboard = bitboards[piece];

        int bb_count = count_bits(bitboard);
        eval += (scores[piece] * bb_count);

        for (int _ = 0; _ < bb_count; _++){

            int target = bsf(bitboard);

            eval -= pst[piece-6][target ^ 56];

            if (piece == b){
                eval -= count_bits(get_bishop_attacks(target, occupancies[black] | bitboards[P])) * 10;
            } else if (piece == r){
                eval -= count_bits(get_rook_attacks(target, occupancies[black] | bitboards[P])) * 10;
            }

            pop_bit(bitboard, target);
        }
    }

    return (side == white) ? eval : -eval;
}
