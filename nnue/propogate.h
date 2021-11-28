//
// Created by levigibson on 10/3/21.
//

#ifndef HALFKP_PROBE_PROPOGATE_H
#define HALFKP_PROBE_PROPOGATE_H

#include <stdint.h>
#include "load.h"

typedef struct NnueData NnueData;

struct NnueData{
    int32_t l1[L2SIZE ];
    int32_t l2[L3SIZE ];
    int32_t l3[OUTSIZE];

    int eval;

    int16_t accumulation[2][KPSIZE];

    unsigned activeIndicies[2][32];
    unsigned activeIndexCount[2];
};

#include "../board.h"

#define CLIPPED_RELU(x) clamp(x, 0, 127)
#define clamp(a, b, c) ((a) < (b) ? (b) : (a) > (c) ? (c) : (a))

int nnue_evaluate(Board *board);

void nnue_pop_bit(int ptype, int bit, Board *board);
void nnue_set_bit(int ptype, int bit, Board *board);

void refresh_accumulator(NnueData *data, Board *board);

#endif //HALFKP_PROBE_PROPOGATE_H
