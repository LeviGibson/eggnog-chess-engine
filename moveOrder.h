//
// Created by levigibson on 8/7/21.
//

#ifndef EGGNOG_CHESS_ENGINE_MOVEORDER_H
#define EGGNOG_CHESS_ENGINE_MOVEORDER_H

#include "bitboard.h"

int16_t moveOrderData[15][12][64][14][64];
char moveOrderWorthSearching[15][12][64][14];

int16_t int16bb_min(const int16_t *x);
int16_t int16bb_max(const int16_t *x);
void calculate_ws_table();

void init_move_table();

#endif //EGGNOG_CHESS_ENGINE_MOVEORDER_H
