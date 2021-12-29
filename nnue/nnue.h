//
// Created by levigibson on 10/3/21.
//

#ifndef HALFKP_PROBE_PROPOGATE_H
#define HALFKP_PROBE_PROPOGATE_H

#include <stdint.h>

#define INSIZE  41024
#define KPSIZE  256
#define L1SIZE  512
#define L2SIZE  32
#define L3SIZE  32
#define OUTSIZE 1

#include <stdint.h>
#include <stdalign.h>

alignas(64) int16_t in_weights[INSIZE * KPSIZE ];
alignas(64) int8_t l1_weights[L1SIZE * L2SIZE ];
alignas(64) int8_t l2_weights[L2SIZE * L3SIZE ];
alignas(64) int8_t l3_weights[L3SIZE * OUTSIZE];

alignas(64) int16_t in_biases[KPSIZE ];
alignas(64) int32_t l1_biases[L2SIZE ];
alignas(64) int32_t l2_biases[L3SIZE ];
alignas(64) int32_t l3_biases[OUTSIZE];

int load_model(const char *path);

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
