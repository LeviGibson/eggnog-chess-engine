//
// Created by levigibson on 9/24/21.
//

#ifndef PROBE_PROPOGATE_H
#define PROBE_PROPOGATE_H

#define RELU(x) ((x) > 0) ? x : 0

float nnue_evaluate(unsigned long long bitboards[]);

#endif //PROBE_PROPOGATE_H
