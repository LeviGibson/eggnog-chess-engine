//
// Created by levigibson on 3/23/22.
//

#include "nnom.h"
#include <stdio.h>
#include <stdalign.h>

int16_t l1_weights[IN_SIZE][L1_SIZE];
int16_t l2_weights[L1_SIZE][L2_SIZE];

int16_t l1_biases[L1_SIZE];
int16_t l2_biases[L2_SIZE];

void nnom_propogate_l1(NnomData *data){
    memcpy(&data->l1, l1_biases, sizeof(data->l1));
    memcpy(&data->l2, l2_biases, sizeof(data->l2));

    for (int32_t i = 0; i < data->indexCount; ++i) {
        uint32_t index = data->indicies[i];
        for (int32_t j = 0; j < L1_SIZE; ++j) {
            data->l1[j] += l1_weights[index][j];
        }
    }

    for (int32_t j = 0; j < L1_SIZE; ++j){
        if (data->l1[j] < 0)
            data->l1[j] = 0;
    }

    for (int32_t i = 0; i < L1_SIZE; ++i) {
        for (int32_t j = 0; j < L2_SIZE; ++j) {
            data->l2[j] += l2_weights[i][j] * data->l1[i];
        }
    }
}

void get_index(uint32_t  *i1, uint32_t *i2, int32_t p, int32_t sq, int32_t wk, int32_t bk, int32_t side){
    *i1 = wk + (768*p) + (64*sq);
    *i2 = bk + (768*p) + (64*sq) + 49152;
}

void generate_nnom_indicies(Board *board){
    board->nnom.indexCount = 0;

    int32_t wk = bsf(board->bitboards[p_K]);
    int32_t bk = bsf(board->bitboards[p_k]);

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
