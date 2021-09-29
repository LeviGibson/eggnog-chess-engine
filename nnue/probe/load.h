//
// Created by levigibson on 9/19/21.
//

#ifndef PROBE_LOAD_H
#define PROBE_LOAD_H

float l1w[768][768];
float l1b[768];
float l2w[256][768];
float l2b[256];
float l3w[32][256];
float l3b[32];
float l4w[1][32];
float l4b[1];

int load_model(char *path);

#endif //PROBE_LOAD_H
