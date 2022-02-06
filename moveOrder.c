//
// Created by levigibson on 8/7/21.
//

#include <stdio.h>
#include "moveOrder.h"
#include "search.h"
#include "moveOrder/moveOrderData.h"

float floatbb_min(const float *x){
    float min = 10000000;
    for (int i = 0; i < 64; ++i) {
        if (x[i] < min)
            min = x[i];
    }

    return min;
}

float floatbb_max(const float *x){
    float max = -10000000;
    for (int i = 0; i < 64; ++i) {
        if (x[i] > max)
            max = x[i];
    }

    return max;
}

void calculate_ws_table(){
    memset(moveOrderWorthSearching, 0, sizeof(moveOrderWorthSearching));

    for (int nump = 0; nump < 15; ++nump) {
        for (int piece = 0; piece < 12; ++piece) {
            for (int square = 0; square < 64; ++square) {
                for (int bb = 0; bb < 14; ++bb) {
                    float range = floatbb_max(moveOrderData[nump][piece][square][bb]) - floatbb_min(moveOrderData[nump][piece][square][bb]);
                    if (range > 1700) {
                        moveOrderWorthSearching[nump][piece][square][bb] = 1;
                    }
                }
            }
        }
    }
}

void init_move_table(){
    FILE *file = fopen("moveOrderData.bin", "r");
    fread((void*)&moveOrderData[0][0][0][0][0], sizeof(float ), 15*12*64*14*64, file);

    calculate_ws_table();
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