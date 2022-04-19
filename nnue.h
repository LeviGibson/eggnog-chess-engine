//
// Created by levigibson on 10/3/21.
//

#ifndef HALFKP_PROBE_PROPOGATE_H
#define HALFKP_PROBE_PROPOGATE_H


#define NNUE_INSIZE  384
#define NNUE_KPSIZE  128
#define NNUE_L1SIZE  256
#define NNUE_L2SIZE  32
#define NNUE_L3SIZE  32
#define NNUE_L4SIZE 1

#include "bitboard.h"
#include <stdalign.h>

int32_t load_nnue(const char *path);

typedef struct NnueData NnueData;

struct NnueData{
    int32_t l2[NNUE_L2SIZE ];
    int32_t l3[NNUE_L3SIZE ];
    int32_t l4[NNUE_L4SIZE];

    int32_t eval;

    int16_t accumulation[2][NNUE_KPSIZE];
    int16_t l1[NNUE_L1SIZE];

    uint32_t activeIndicies[2][16];
    uint32_t activeIndexCount[2];
};

#include "board.h"

#define CLIPPED_RELU(x) clamp(x, 0, 127)
#define clamp(a, b, c) ((a) < (b) ? (b) : (a) > (c) ? (c) : (a))

int32_t nnue_evaluate(Board *board);

void nnue_pop_bit(int32_t ptype, int32_t bit, Board *board);
void nnue_set_bit(int32_t ptype, int32_t bit, Board *board);

void refresh_accumulator(NnueData *data, Board *board);

#endif //HALFKP_PROBE_PROPOGATE_H
