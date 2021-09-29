//
// Created by levigibson on 9/19/21.
//

#include "load.h"

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

/**
ALL WEIGHT SHAPES

(768, 768)
(768,)
(256, 768)
(256,)
(32, 256)
(32,)
(1, 32)
(1,)
**/

float l1w[768][768];
float l1b[768];
float l2w[256][768];
float l2b[256];
float l3w[32][256];
float l3b[32];
float l4w[1][32];
float l4b[1];

static inline void changeEndianness32(float *val) {
    int *i_val = (int*) val;

    *i_val = (*i_val << 24 |
    ((*i_val << 8) & 0x00ff0000) |
    ((*i_val >>  8) & 0x0000ff00) |
    ((*i_val >> 24) & 0x000000ff));
}

void load_weights(float *buffer){
    memcpy(l1w, buffer, sizeof l1w);
    buffer += sizeof l1w / 4;

    memcpy(l1b, buffer, sizeof l1b);
    buffer += sizeof l1b / 4;

    memcpy(l2w, buffer, sizeof l2w);
    buffer += sizeof l2w / 4;

    memcpy(l2b, buffer, sizeof l2b);
    buffer += sizeof l2b / 4;

    memcpy(l3w, buffer, sizeof l3w);
    buffer += sizeof l3w / 4;

    memcpy(l3b, buffer, sizeof l3b);
    buffer += sizeof l3b / 4;

    memcpy(l4w, buffer, sizeof l4w);
    buffer += sizeof l4w / 4;

    memcpy(l4b, buffer, sizeof l4b);
}

int load_model(char *path){
    FILE *ptr = fopen(path,"rb");
    int size = 3182852;
    float buffer[size/4];

    fread(&buffer, size/4, 4, ptr);

    for (int i = 0; i < size / 4; i++){
        changeEndianness32(&buffer[i]);
    }

    load_weights(buffer);

    printf("NNUE Loaded!\n");

    return 1;
}