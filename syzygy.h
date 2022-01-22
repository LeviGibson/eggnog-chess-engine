//
// Created by levigibson on 7/30/21.
//

#ifndef EGGNOG_CHESS_ENGINE_SYZYGY_H
#define EGGNOG_CHESS_ENGINE_SYZYGY_H

#include "board.h"

int get_root_move(Board *board);
unsigned get_wdl(Board *board);

#endif //EGGNOG_CHESS_ENGINE_SYZYGY_H
