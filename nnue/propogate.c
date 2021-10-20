//
// Created by levigibson on 10/3/21.
//

#include <string.h>
#include <stdio.h>
#include "../board.h"

#include "propogate.h"
#include <immintrin.h>

int NnuePtypes[12] = {6, 5, 4, 3, 2, K, 12, 11, 10, 9, 8, k};

const static int w_orient[64] = {
        56, 57, 58, 59, 60, 61, 62, 63,
        48, 49, 50, 51, 52, 53, 54, 55,
        40, 41, 42, 43, 44, 45, 46, 47,
        32, 33, 34, 35, 36, 37, 38, 39,
        24, 25, 26, 27, 28, 29, 30, 31,
        16, 17, 18, 19, 20, 21, 22, 23,
        8, 9, 10, 11, 12, 13, 14, 15,
        0, 1, 2, 3, 4, 5, 6, 7,
};

const static int b_orient[64] = {
        7, 6, 5, 4, 3, 2, 1, 0,
        15, 14, 13, 12, 11, 10, 9, 8,
        23, 22, 21, 20, 19, 18, 17, 16,
        31, 30, 29, 28, 27, 26, 25, 24,
        39, 38, 37, 36, 35, 34, 33, 32,
        47, 46, 45, 44, 43, 42, 41, 40,
        55, 54, 53, 52, 51, 50, 49, 48,
        63, 62, 61, 60, 59, 58, 57, 56
};

enum {
    PS_W_PAWN = 1,
    PS_B_PAWN = 1 * 64 + 1,
    PS_W_KNIGHT = 2 * 64 + 1,
    PS_B_KNIGHT = 3 * 64 + 1,
    PS_W_BISHOP = 4 * 64 + 1,
    PS_B_BISHOP = 5 * 64 + 1,
    PS_W_ROOK = 6 * 64 + 1,
    PS_B_ROOK = 7 * 64 + 1,
    PS_W_QUEEN = 8 * 64 + 1,
    PS_B_QUEEN = 9 * 64 + 1,
    PS_END = 10 * 64 + 1
};

uint32_t PieceToIndex[2][14] = {
        {0, 0, PS_W_QUEEN, PS_W_ROOK, PS_W_BISHOP, PS_W_KNIGHT, PS_W_PAWN,
                0, PS_B_QUEEN, PS_B_ROOK, PS_B_BISHOP, PS_B_KNIGHT, PS_B_PAWN, 0},
        {0, 0, PS_B_QUEEN, PS_B_ROOK, PS_B_BISHOP, PS_B_KNIGHT, PS_B_PAWN,
                0, PS_W_QUEEN, PS_W_ROOK, PS_W_BISHOP, PS_W_KNIGHT, PS_W_PAWN, 0}
};


int orient(int c, int s) {
    return s ^ (c == white ? 0x00 : 0x3f);
}

int make_index(int c, int s, int pc, int ksq) {
    return orient(c, s) + PieceToIndex[c][pc] + PS_END * ksq;
}

static void append_index(int c, int index, NnueData *data) {
    data->activeIndicies[c][data->activeIndexCount[c]++] = index;
}

void append_active_indicies(NnueData *data) {
    data->activeIndexCount[black] = 0;
    data->activeIndexCount[white] = 0;

    int w_ksq = w_orient[bsf(bitboards[K])];
    int b_ksq = b_orient[bsf(bitboards[k])];

    for (unsigned ptype = P; ptype < k; ++ptype) {
        if (ptype == K)
            continue;

        U64 bitboard = bitboards[ptype];
        while (bitboard) {
            int bit = bsf(bitboard);
            int sq = w_orient[bit];
            int pc = NnuePtypes[ptype];

            append_index(white, make_index(white, sq, pc, w_ksq), data);
            append_index(black, make_index(black, sq, pc, b_ksq), data);

            pop_bit(bitboard, bit);
        }
    }
}

void add_index(NnueData *data, unsigned int index, unsigned int c) {
    unsigned offset = 256 * index;

    for (unsigned j = 0; j < 256; j++)
        data->accumulation[c][j] += in_weights[offset + j];
}

void subtract_index(NnueData *data, unsigned index, unsigned c) {
    unsigned offset = 256 * index;

    for (unsigned j = 0; j < 256; j++)
        data->accumulation[c][j] -= in_weights[offset + j];
}

void refresh_accumulator(NnueData *data) {
    append_active_indicies(data);

    for (unsigned int c = 0; c < 2; c++) {
        memcpy(data->accumulation[c], in_biases, 256 * sizeof(int16_t));

        for (size_t k = 0; k < data->activeIndexCount[c]; k++) {
            unsigned index = data->activeIndicies[c][k];
            add_index(data, index, c);
        }
    }
}

void propogate_l1(NnueData *data) {



    int16_t *accum = (int16_t *) &data->accumulation;
    memcpy(data->l1, l1_biases, sizeof l1_biases);
    memcpy(data->l2, l2_biases, sizeof l2_biases);
    memcpy(data->l3, l3_biases, sizeof l3_biases);

    for (int i = 0; i < 512; ++i) {
        for (int j = 0; j < 32; ++j) {

            int offset = 32 * i;

            data->l1[j] += CLIPPED_RELU(accum[i]) * l1_weights[offset + j];
        }
    }

    for (int i = 0; i < 32; ++i) {
        data->l1[i] = clamp(data->l1[i] / 64, 0, 127);
    }

    for (int o = 0; o < 32; ++o) {
        for (int d = 0; d < 32; ++d) {
            int offset = 32 * o;

            data->l2[d] += data->l1[o] * l2_weights[offset + d];
        }
    }

    for (int i = 0; i < 32; ++i) {
        data->l2[i] = clamp(data->l2[i] / 64, 0, 127);
    }

    for (int i = 0; i < 32; ++i) {
        data->l3[0] += data->l2[i] * l3_weights[i];
    }
}

int nnue_evaluate(NnueData *data) {

    propogate_l1(data);
    data->eval = (side == white) ? data->l3[0] : -data->l3[0];

    return data->eval;
}

//int bit = bsf(bitboard);
//int sq = w_orient[bit];
//int pc = NnuePtypes[ptype];
//
//append_index(white, make_index(white, sq, pc, w_ksq), data);
//append_index(black, make_index(black, sq, pc, b_ksq), data);

//void accumulator_relu(NnueData *data){
//    for (unsigned int c = 0; c < 2; c++) {
//        for (int i = 0; i < 256; i++) {
//            data->accumulation[c][i] = CLIPPED_RELU(data->accumulation[c][i]);
//        }
//    }
//}

void nnue_pop_bit(int ptype, int bit){

    pop_bit(bitboards[ptype], bit);

    int w_ksq = w_orient[bsf(bitboards[K])];
    int b_ksq = b_orient[bsf(bitboards[k])];

    int sq = w_orient[bit];
    int pc = NnuePtypes[ptype];

    int wi = make_index(white, sq, pc, w_ksq);
    int bi = make_index(black, sq, pc, b_ksq);

    subtract_index(&currentNnue, wi, white);
    subtract_index(&currentNnue, bi, black);
}

void nnue_set_bit(int ptype, int bit){

    set_bit(bitboards[ptype], bit);

    int w_ksq = w_orient[bsf(bitboards[K])];
    int b_ksq = b_orient[bsf(bitboards[k])];

    int sq = w_orient[bit];
    int pc = NnuePtypes[ptype];

    int wi = make_index(white, sq, pc, w_ksq);
    int bi = make_index(black, sq, pc, b_ksq);

    add_index(&currentNnue, wi, white);
    add_index(&currentNnue, bi, black);
}
