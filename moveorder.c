//
// Created by levigibson on 5/7/22.
//

#include <stdio.h>
#include "moveorder.h"
#include "see.h"

static inline void swap(int* a, int* b) {
    int32_t t = *a;
    *a = *b;
    *b = t;
}

void insertion_sort(MoveList *movearr){
    int32_t i = 1;
    while (i < movearr->count){
        int32_t j = i;
        while (j > 0 && movearr->scores[j-1] < movearr->scores[j]) {
            swap(&movearr->scores[j], &movearr->scores[j - 1]);
            swap(&movearr->moves[j], &movearr->moves[j - 1]);
            j--;
        }
        i++;
    }
}

const int32_t mvv_lva[12][12] = {
        15, 25, 35, 45, 55, 65,  15, 25, 35, 45, 55, 65,
        14, 24, 34, 44, 54, 64,  14, 24, 34, 44, 54, 64,
        13, 23, 33, 43, 53, 63,  13, 23, 33, 43, 53, 63,
        12, 22, 32, 42, 52, 62,  12, 22, 32, 42, 52, 62,
        11, 21, 31, 41, 51, 61,  11, 21, 31, 41, 51, 61,
        10, 20, 30, 40, 50, 60,  10, 20, 30, 40, 50, 60,

        15, 25, 35, 45, 55, 65,  15, 25, 35, 45, 55, 65,
        14, 24, 34, 44, 54, 64,  14, 24, 34, 44, 54, 64,
        13, 23, 33, 43, 53, 63,  13, 23, 33, 43, 53, 63,
        12, 22, 32, 42, 52, 62,  12, 22, 32, 42, 52, 62,
        11, 21, 31, 41, 51, 61,  11, 21, 31, 41, 51, 61,
        10, 20, 30, 40, 50, 60,  10, 20, 30, 40, 50, 60
};

//Board is included in Thread *thread
//returns the score of a move, integer.
//int32_t *hashmove is  the best move stored in the hash table (from a previous depth)
int32_t score_move(int32_t move, const int32_t *hashmove, Thread *thread){
    Board *board = &thread->board;

    if (thread->found_pv){
        if (move == board->prevPv.moves[board->ply]){
            thread->found_pv = 0;
            return 200000;
        }
    }

    for (int32_t i = 0; i < 6; i++) {
        if (hashmove[i] == -15 || hashmove[i] == 0)
            break;
        if (hashmove[i] == move) {
            return 100000 - i;
        }
    }

    if (getcapture(move)){


        int32_t start_piece, end_piece;
        int32_t target_piece = p_P;

        int32_t target_square = gettarget(move);

        if (board->side == white) {start_piece = p_p; end_piece = p_k;}
        else{start_piece = p_P; end_piece = p_K;}

        for (int32_t bb_piece = start_piece; bb_piece < end_piece; bb_piece++){
            if (get_bit(board->bitboards[bb_piece], target_square)){
                target_piece = bb_piece;
                break;
            }
        }

        if (board->side == white){
            if ((getpiece(move) != p_P) && (pawn_mask[white][gettarget(move)] & BP))
                return 10000;
        } else {
            if ((getpiece(move) != p_p) && (pawn_mask[black][gettarget(move)] & WP))
                return 10000;
        }

        int32_t val = seeCapture(move, board);
        if (board->quinode)
            return val + mvv_lva[getpiece(move)][target_piece];
        return val + mvv_lva[getpiece(move)][target_piece] + 10000;

    } else {
        if (move == killer_moves[board->ply][0]){
            return(8000);
        }
        if (move == killer_moves[board->ply][1]){
            return(7000);
        }

        if (board->quinode){ return 0; }

        int32_t score = get_nnom_score(move, board);

        if (historyCount > 0) {
            float historyscore = (history_moves[getpiece(move)][getsource(move)][gettarget(move)] / (float) historyCount) * 1400.f;
            score += ((int32_t )historyscore) * 10;
        }

        return (int)score;
    }
}

//sorts a list of moves
//int32_t *hashmove is the move stored in the transposition table from a previous depth
//pretty much ignore this function it's very boring. The main function is the function above (score_move)
void sort_moves(MoveList *move_list, int32_t *hashmove, Thread *thread){

    for (int32_t i = 0; i < move_list->count; i++) {
        move_list->scores[i] = score_move(move_list->moves[i], hashmove, thread);
    }

    insertion_sort(move_list);

//    if (thread->board.quinode) return;
//    print_fen(&thread->board);
//    printf("\n");
//     for (int32_t i = 0; i < move_list->count; i++) {
//         print_move(move_list->moves[i]);
//         printf(" : %d\n", move_list->scores[i]);
//     }
//
//     printf("\n");
}
