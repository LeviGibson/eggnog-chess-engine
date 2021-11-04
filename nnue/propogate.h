//
// Created by levigibson on 10/3/21.
//

#ifndef HALFKP_PROBE_PROPOGATE_H
#define HALFKP_PROBE_PROPOGATE_H

#include "load.h"

#define CLIPPED_RELU(x) clamp(x, 0, 127)
#define clamp(a, b, c) ((a) < (b) ? (b) : (a) > (c) ? (c) : (a))

typedef struct NNUEDATA {
    int32_t l1[L2SIZE ];
    int32_t l2[L3SIZE ];
    int32_t l3[OUTSIZE];

    int eval;

    int16_t accumulation[2][KPSIZE];

    unsigned activeIndicies[2][32];
    unsigned activeIndexCount[2];
} NnueData;

int nnue_evaluate(NnueData *data);

void nnue_pop_bit(int ptype, int bit);
void nnue_set_bit(int ptype, int bit);

void refresh_accumulator(NnueData *data);

#endif //HALFKP_PROBE_PROPOGATE_H