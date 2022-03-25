//
// Created by levigibson on 3/23/22.
//

#ifndef EGGNOG_CHESS_ENGINE_NNOM_H
#define EGGNOG_CHESS_ENGINE_NNOM_H

#include <stdint.h>

#define IN_SIZE 98304
#define L1_SIZE 512
#define L2_SIZE 384

typedef struct NnomData NnomData;
struct NnomData{
    uint32_t indicies[64];
    uint32_t indexCount;

    int16_t l1[L1_SIZE];
    int32_t l2[L2_SIZE];
};

#include "board.h"

void generate_nnom_indicies(Board *board);
void nnom_propogate_l1(Board *board);
int load_nnom(char *path);

#endif //EGGNOG_CHESS_ENGINE_NNOM_H
