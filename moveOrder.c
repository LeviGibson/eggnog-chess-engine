//
// Created by levigibson on 8/7/21.
//

#include <stdio.h>
#include "moveOrder.h"
#include "search.h"
#include "Fathom/tbprobe.h"
#include "syzygy.h"

int commonMoveTable[64][64][64];

void init_move_table(){
    //avoid compiler warning
    unsigned long tmp;

    FILE *fd = fopen("moveTable", "rb");
    tmp = fread((void*)&commonMoveTable, 4, 64*64*64, fd);
}

int get_move_score(int f_prevmove, int move){
    return commonMoveTable[getsource(f_prevmove)][gettarget(f_prevmove)][gettarget(move)];
}
//
//int game(){
//    Board b;
//    Board *board = &b;
//    parse_fen(start_position, board);
//    print_board(board);
//
//    while (1){
//        print_board(board);
//        U32 tbres = get_wdl(board);
//        if (tbres != TB_RESULT_FAILED) {
//            if (tbres == 4)
//                return board->side == white ? 1 : -1;
//            if (tbres == 2)
//                return 0;
//            if (tbres == 0)
//                return board->side == white ? -1 : 1;
//        }
//
//        moveList legalMoves;
//        generate_moves(&legalMoves, board);
//        remove_illigal_moves(&legalMoves, board);
//
//        if (legalMoves.count == 0) return board->side == white ? 1 : -1;
//
//        int index = (rand() % legalMoves.count);
//        if (index != 0)
//            index = rand() % index;
//
//        print_move(legalMoves.moves[index]);
//        printf("\n");
//
//        make_move(legalMoves.moves[index], all_moves, 1, board);
//    }
//}
//
//void moveorder_test(int numGames){
//    int score = 0;
//    for (int gameId = 0; gameId < numGames; ++gameId) {
//        score += game();
//    }
//}