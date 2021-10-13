#ifndef LOAD_H
#define LOAD_H

#define INSIZE  41024
#define KPSIZE  256
#define L1SIZE  512
#define L2SIZE  32
#define L3SIZE  32
#define OUTSIZE 1

#include <stdint-gcc.h>
#include <stdalign.h>

//int16_t in_weights[INSIZE * KPSIZE ];
//int8_t  l1_weights[L1SIZE * L2SIZE ];
//int8_t  l2_weights[L2SIZE * L3SIZE ];
//int8_t  l3_weights[L3SIZE * OUTSIZE];
//
//int16_t in_biases[KPSIZE ];
//int32_t l1_biases[L2SIZE ];
//int32_t  l2_biases[L3SIZE ];
//int32_t  l3_biases[OUTSIZE];

alignas(64) int16_t in_weights[INSIZE * KPSIZE ];
alignas(64) int8_t l1_weights[L1SIZE * L2SIZE ];
alignas(64) int8_t l2_weights[L2SIZE * L3SIZE ];
alignas(64) int8_t l3_weights[L3SIZE * OUTSIZE];

alignas(64) int16_t in_biases[KPSIZE ];
alignas(64) int32_t l1_biases[L2SIZE ];
alignas(64) int32_t l2_biases[L3SIZE ];
alignas(64) int32_t l3_biases[OUTSIZE];

int load_model(const char *path);

#endif