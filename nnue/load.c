#include <stdio.h>
#include <string.h>
#include "load.h"

#define TRANSFORMERSTART ((3 * 4) + 181)

alignas(64) int16_t in_weights[INSIZE * KPSIZE ];
alignas(64) int8_t l1_weights[L1SIZE * L2SIZE ];
alignas(64) int8_t l2_weights[L2SIZE * L3SIZE ];
alignas(64) int8_t l3_weights[L3SIZE * OUTSIZE];

alignas(64) int16_t in_biases[KPSIZE ];
alignas(64) int32_t l1_biases[L2SIZE ];
alignas(64) int32_t l2_biases[L3SIZE ];
alignas(64) int32_t l3_biases[OUTSIZE];

void transform_weight_indicies(int8_t arr[], unsigned dims){
    int8_t tmpArr[dims*32];
    memcpy(tmpArr, arr, sizeof tmpArr);

    for (int r = 0; r < 32; ++r) {
        for (int c = 0; c < dims; ++c) {
            arr[(c * 32) + r] = tmpArr[(dims*r) + c];
        }
    }
}

int load_model(const char *path){
    FILE *fin = fopen(path, "rb");

    //FEATURE TRANSFORMER
    fseek(fin, TRANSFORMERSTART, SEEK_SET);
    fread(in_biases, sizeof(int16_t), KPSIZE, fin);
    fread(in_weights, sizeof(int16_t), INSIZE * KPSIZE, fin);

    fseek(fin, 4, SEEK_CUR);

    //Hidden Layer 1
    fread(l1_biases, sizeof (l1_biases[0]), L2SIZE, fin);
    fread(l1_weights, sizeof (l1_weights[0]), L1SIZE * L2SIZE, fin);
    transform_weight_indicies(l1_weights, L1SIZE);

    //Hidden Layer 2
    fread(l2_biases, sizeof (l2_biases[0]), L3SIZE, fin);
    fread(l2_weights, sizeof (l2_weights[0]), L2SIZE * L3SIZE, fin);
    transform_weight_indicies(l2_weights, L2SIZE);

    //Output Layer
    fread(l3_biases, sizeof (l3_biases[0]), OUTSIZE, fin);

    fread(l3_weights, sizeof (l3_weights[0]), L2SIZE * OUTSIZE, fin);

    fclose(fin);

    return 0;
}