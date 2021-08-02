#include <stdio.h>
#include "evaluate.h"
#include "board.h"
#include "bitboard.h"

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

const unsigned squareToFile[64] = {
        0, 1, 2, 3, 4, 5, 6, 7,
        0, 1, 2, 3, 4, 5, 6, 7,
        0, 1, 2, 3, 4, 5, 6, 7,
        0, 1, 2, 3, 4, 5, 6, 7,
        0, 1, 2, 3, 4, 5, 6, 7,
        0, 1, 2, 3, 4, 5, 6, 7,
        0, 1, 2, 3, 4, 5, 6, 7,
        0, 1, 2, 3, 4, 5, 6, 7
};

const U64 adjacentFiles[8] = {144680345676153346ULL,
                             361700864190383365ULL,
                             723401728380766730ULL,
                             1446803456761533460ULL,
                             2893606913523066920ULL,
                             5787213827046133840ULL,
                             11574427654092267680ULL,
                             4629771061636907072ULL};

const int scores[] = {100, 270, 300, 500, 900, 0, -100, -300, -300, -500, -900, 0};

int evaluate_pawn_structure(){

    int eval = 0;

    U64 bitboard = bitboards[P];
    int bb_count = count_bits(bitboard);
    for (int i = 0; i < bb_count; i++){
        int square = bsf(bitboard);

        if (!(adjacentFiles[squareToFile[square]] & bitboards[P]))
            eval -= 6;

        pop_bit(bitboard, square);
    }

    bitboard = bitboards[p];
    bb_count = count_bits(bitboard);
    for (int i = 0; i < bb_count; i++){
        int square = bsf(bitboard);

        if (!(adjacentFiles[squareToFile[square]] & bitboards[p]))
            eval += 6;

        pop_bit(bitboard, square);
    }

    return eval;
}

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

    if (abs(eval) < 300) {
        eval += evaluate_pawn_structure() * 6;
    }

    return (side == white) ? eval : -eval;
}
