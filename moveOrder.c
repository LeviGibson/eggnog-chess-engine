//
// Created by levigibson on 8/7/21.
//

#include "moveOrder.h"
#include "moveTable.h"
#include "board.h"

int get_move_score(int f_prevmove, int move){
    return commonMoveTable[get_move_source(f_prevmove)][get_move_target(f_prevmove)][get_move_target(move)];
}