//
// Created by levigibson on 3/23/22.
//

#ifndef EGGNOG_CHESS_ENGINE_NNOM_H
#define EGGNOG_CHESS_ENGINE_NNOM_H

#include <stdint.h>

#define IN_SIZE (12*64)
#define L1_SIZE 512
#define L2_SIZE 384

typedef struct NnomData NnomData;
struct NnomData{
    uint32_t indicies[2][64];
    uint32_t indexCount;

    //there are two copies of the first layer because of the two perspectives
    //(white and black)
    int16_t l1[2][L1_SIZE];
};

#include "board.h"

void nnom_set_bit(int32_t ptype, int32_t bit, Board *board);
void nnom_pop_bit(int32_t ptype, int32_t bit, Board *board);
int32_t get_nnom_score(int move, Board *board);
void generate_nnom_indicies(Board *board);
void nnom_refresh_l1(Board *board);
int load_nnom(char *path);

#endif //EGGNOG_CHESS_ENGINE_NNOM_H
