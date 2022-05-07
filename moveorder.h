//
// Created by levigibson on 5/7/22.
//

#ifndef EGGNOG_CHESS_ENGINE_MOVEORDER_H
#define EGGNOG_CHESS_ENGINE_MOVEORDER_H

#include "board.h"

int32_t historyCount;
int32_t killer_moves[MAX_PLY][2];
float history_moves[12][64][64];
void sort_moves(MoveList *move_list, int32_t *hashmove, Thread *thread);

#endif //EGGNOG_CHESS_ENGINE_MOVEORDER_H
