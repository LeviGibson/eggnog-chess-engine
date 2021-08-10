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

const U64 pastPawnMasks[2][64] = {
        {0ULL,
                0ULL,
                0ULL,
                0ULL,
                0ULL,
                0ULL,
                0ULL,
                0ULL,
                2ULL,
                6ULL,
                14ULL,
                28ULL,
                56ULL,
                112ULL,
                224ULL,
                192ULL,
                770ULL,
                1798ULL,
                3598ULL,
                7196ULL,
                14392ULL,
                28784ULL,
                57568ULL,
                49344ULL,
                197378ULL,
                460550ULL,
                921102ULL,
                1842204ULL,
                3684408ULL,
                7368816ULL,
                14737632ULL,
                12632256ULL,
                50529026ULL,
                117901062ULL,
                235802126ULL,
                471604252ULL,
                943208504ULL,
                1886417008ULL,
                3772834016ULL,
                3233857728ULL,
                12935430914ULL,
                30182672134ULL,
                60365344270ULL,
                120730688540ULL,
                241461377080ULL,
                482922754160ULL,
                965845508320ULL,
                827867578560ULL,
                3311470314242ULL,
                7726764066566ULL,
                15453528133134ULL,
                30907056266268ULL,
                61814112532536ULL,
                123628225065072ULL,
                247256450130144ULL,
                211934100111552ULL,
                847736400446210ULL,
                1978051601041158ULL,
                3956103202082318ULL,
                7912206404164636ULL,
                15824412808329272ULL,
                31648825616658544ULL,
                63297651233317088ULL,
                54255129628557504ULL,},
                {
            217020518514230016ULL,
            506381209866536704ULL,
            1012762419733073408ULL,
            2025524839466146816ULL,
            4051049678932293632ULL,
            8102099357864587264ULL,
            16204198715729174528ULL,
            13889313184910721024ULL,
            217020518514229248ULL,
            506381209866534912ULL,
            1012762419733069824ULL,
            2025524839466139648ULL,
            4051049678932279296ULL,
            8102099357864558592ULL,
            16204198715729117184ULL,
            13889313184910671872ULL,
            217020518514032640ULL,
            506381209866076160ULL,
            1012762419732152320ULL,
            2025524839464304640ULL,
            4051049678928609280ULL,
            8102099357857218560ULL,
            16204198715714437120ULL,
            13889313184898088960ULL,
            217020518463700992ULL,
            506381209748635648ULL,
            1012762419497271296ULL,
            2025524838994542592ULL,
            4051049677989085184ULL,
            8102099355978170368ULL,
            16204198711956340736ULL,
            13889313181676863488ULL,
            217020505578799104ULL,
            506381179683864576ULL,
            1012762359367729152ULL,
            2025524718735458304ULL,
            4051049437470916608ULL,
            8102098874941833216ULL,
            16204197749883666432ULL,
            13889312357043142656ULL,
            217017207043915776ULL,
            506373483102470144ULL,
            1012746966204940288ULL,
            2025493932409880576ULL,
            4050987864819761152ULL,
            8101975729639522304ULL,
            16203951459279044608ULL,
            13889101250810609664ULL,
            216172782113783808ULL,
            504403158265495552ULL,
            1008806316530991104ULL,
            2017612633061982208ULL,
            4035225266123964416ULL,
            8070450532247928832ULL,
            16140901064495857664ULL,
            13835058055282163712ULL,
            0ULL,
            0ULL,
            0ULL,
            0ULL,
            0ULL,
            0ULL,
            0ULL,
            0ULL,
            }
};

const int scores[] = {100, 270, 300, 500, 900, 0, -100, -300, -300, -500, -900, 0};

int evaluate_pawn_structure(){

    int eval = 0;

    U64 bitboard = bitboards[P];
    int bb_count = count_bits(bitboard);

    for (int i = 0; i < bb_count; i++){
        int square = bsf(bitboard);

        if (!(pastPawnMasks[white][square] & bitboards[p]))
            eval += 100;
        if (!(adjacentFiles[squareToFile[square]] & bitboards[P]))
            eval -= 30;

        pop_bit(bitboard, square);
    }

    bitboard = bitboards[p];
    bb_count = count_bits(bitboard);
    for (int i = 0; i < bb_count; i++){
        int square = bsf(bitboard);

        if (!(pastPawnMasks[black][square] & bitboards[P]))
            eval -= 100;
        if (!(adjacentFiles[squareToFile[square]] & bitboards[p]))
            eval += 30;

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
                eval += count_bits(get_bishop_attacks(target, occupancies[white] | bitboards[p]) & 0xffffffffffffffULL) * 10;
            } else if (piece == R){
                eval += count_bits(get_rook_attacks(target, occupancies[white] | bitboards[p]) & 0xffffffffffffff00ULL) * 10;
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

    /**if (abs(eval) < 300) {
        eval += evaluate_pawn_structure();
    }**/

    return (side == white) ? eval : -eval;
}
