//
// Created by levigibson on 8/7/21.
//

#include <stdio.h>
#include "moveOrder.h"
#include "board.h"

int commonMoveTable[64][64][64];

void init_move_table(){

    FILE *fd = fopen("moveTable", "rb");
    fread((void*)&commonMoveTable, 4, 64*64*64, fd);
}

int get_move_score(int f_prevmove, int move){
    return commonMoveTable[get_move_source(f_prevmove)][get_move_target(f_prevmove)][get_move_target(move)];
}