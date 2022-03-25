//
// Created by levigibson on 3/23/22.
//

#include "nnom.h"
#include <stdio.h>
#include <stdalign.h>

alignas(64) int16_t l1_weights[IN_SIZE][L1_SIZE];
alignas(64) int16_t l2_weights[L1_SIZE][L2_SIZE];

alignas(64) int16_t l1_biases[L1_SIZE];
alignas(64) int32_t l2_biases[L2_SIZE];

void nnom_refresh_l1(Board *board){
    NnomData *data = &board->nnom;
    memcpy(&data->l1, l1_biases, sizeof(data->l1));
    generate_nnom_indicies(board);

    for (int32_t i = 0; i < data->indexCount; ++i) {
        uint32_t index = data->indicies[i];
        for (int32_t j = 0; j < L1_SIZE; ++j) {
            data->l1[j] += l1_weights[index][j];
        }
    }

    for (int32_t j = 0; j < L1_SIZE; ++j){
        if (data->l1[j] < 0)
            data->l1[j] = 0;
        data->l1[j] /= 64;
    }

}

int32_t get_nnom_score(int move, Board *board){
    NnomData *data = &board->nnom;
    int orientedPiece = getpiece(move);
    int orientedSquare = gettarget(move);
    if (board->side == black){
        orientedPiece -= 6;
        orientedSquare = w_orient[orientedSquare];
    }
    int32_t moveIndex = (orientedPiece * 64) + orientedSquare;
    int32_t score = l2_biases[moveIndex];

    for (int32_t i = 0; i < L1_SIZE; ++i) {
        score += l2_weights[i][moveIndex] * data->l1[i];
    }

    return score;
}

void nnom_propogate_l2(Board *board){
    NnomData *data = &board->nnom;
    memcpy(&data->l2, l2_biases, sizeof(data->l2));

    for (int32_t i = 0; i < L1_SIZE; ++i) {
        for (int32_t j = 0; j < L2_SIZE; ++j) {
            data->l2[j] += (l2_weights[i][j] * data->l1[i]);
        }
    }
}

int flipPiecePers[12] = {p_p, p_n, p_b, p_r, p_q, p_k, p_P, p_N, p_B, p_R, p_Q, p_K};

void get_index(uint32_t  *i1, uint32_t *i2, int32_t p, int32_t sq, int32_t wk, int32_t bk, int32_t side){
    if (side == black){
        p = flipPiecePers[p];
        sq = w_orient[sq];
    }

    *i1 = wk + (768*p) + (64*sq);
    *i2 = bk + (768*p) + (64*sq) + 49152;
}

void generate_nnom_indicies(Board *board){
    board->nnom.indexCount = 0;

    int32_t wk = bsf(board->bitboards[p_K]);
    int32_t bk = bsf(board->bitboards[p_k]);

    if (board->side == black){
        wk = w_orient[wk];
        bk = w_orient[bk];

        int32_t tmp = wk;
        wk = bk;
        bk = tmp;
    }

    for (int p = 0; p < 12; ++p) {
        U64 bb = board->bitboards[p];
        while (bb){
            int32_t square = bsf(bb);

            uint32_t i1;
            uint32_t i2;

            get_index(&i1, &i2, p, square, wk, bk, board->side);
            board->nnom.indicies[board->nnom.indexCount] = i1;
            board->nnom.indicies[board->nnom.indexCount+1] = i2;

            board->nnom.indexCount += 2;

            pop_bit(bb, square);
        }
    }

}

int load_nnom(char *path){
    __attribute__((unused)) unsigned long tmp;

    FILE *fin = fopen(path, "rb");
    tmp = fread(l1_weights, sizeof(int16_t), IN_SIZE * L1_SIZE, fin);
    tmp = fread(l2_weights, sizeof(int16_t), L1_SIZE * L2_SIZE, fin);

    tmp = fread(l1_biases, sizeof(int16_t), L1_SIZE, fin);
    tmp = fread(l2_biases, sizeof(int16_t), L2_SIZE, fin);

    return 0;
}
