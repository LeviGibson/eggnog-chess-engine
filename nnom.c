//
// Created by levigibson on 3/23/22.
//

#include "nnom.h"
#include <stdio.h>
#include <stdalign.h>

#define IN_SIZE 98304
#define L1_SIZE 512
#define L2_SIZE 384

int16_t l1_weights[IN_SIZE][L1_SIZE];
int16_t l2_weights[L1_SIZE][L2_SIZE];

int16_t l1_biases[L1_SIZE];
int16_t l2_biases[L2_SIZE];

//void nnom_propogate_l1(NnomData *data){
//
//}

void get_index(int32_t  *i1, int32_t *i2, int32_t p, int32_t sq, int32_t wk, int32_t bk, int32_t side){
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

            int32_t i1;
            int32_t i2;

            get_index(&i1, &i2, p, square, wk, bk, board->side);
            board->nnom.indicies[board->nnom.indexCount] = i1;
            board->nnom.indicies[board->nnom.indexCount+1] = i2;
            board->nnom.indexCount += 2;

            printf("%d\n", i1);
            printf("%d\n", i2);

            pop_bit(bb, square);
        }
    }
}

int load_nnom(char *path){
    __attribute__((unused)) unsigned long tmp;

    FILE *fin = fopen(path, "rb");
    tmp = fread(l1_weights, sizeof(int16_t), IN_SIZE * L1_SIZE, fin);
    tmp = fread(l2_weights, sizeof(int16_t), L1_SIZE * L2_SIZE, fin);

    tmp = fread(l2_weights, sizeof(int16_t), L1_SIZE * L2_SIZE, fin);
    tmp = fread(l2_weights, sizeof(int16_t), L1_SIZE * L2_SIZE, fin);


    return 0;
}
