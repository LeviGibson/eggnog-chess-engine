//
// Created by levigibson on 3/23/22.
//

#ifndef EGGNOG_CHESS_ENGINE_NNOM_H
#define EGGNOG_CHESS_ENGINE_NNOM_H

#include <stdint.h>

typedef struct NnomData NnomData;
struct NnomData{
    uint32_t indicies[32];
    uint32_t indexCount;
};

#include "board.h"

void generate_nnom_indicies(Board *board);
int load_nnom(char *path);

#endif //EGGNOG_CHESS_ENGINE_NNOM_H
