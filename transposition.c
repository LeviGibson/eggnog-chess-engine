//
// Created by levigibson on 6/17/21.
//

#include <stdio.h>
#include "transposition.h"
#include "board.h"

HASHE hash_table[tt_size];
int lines[tt_size];
int lineMoveCount = 0;

void reset_hash_table(){
    memset(hash_table, 0, sizeof hash_table);
    memset(lines, 0, sizeof lines);
    lineMoveCount = 0;
}

void recover_line(int depth, HASHE *phashe, Line *pline){
    if (depth <= 1)
        return;

    int* line = phashe->line;

    //things were crashing and this fixed it for some reason idk
    if (*line > 100)
        return;


    memcpy(pline->moves, &line[1], sizeof(int) * (*line));
    pline->length = *line;
}

int ProbeHash(int depth, int alpha, int beta, int *move, Line *pline, Board *board){
    HASHE * phashe = &hash_table[board->current_zobrist_key % tt_size];

    if (phashe->key == board->current_zobrist_key) {
        *move = phashe->best;

        if (phashe->depth >= depth) {

            if (phashe->flags == hashfEXACT) {
                recover_line(depth, phashe, pline);
                return phashe->value;
            }
            if ((phashe->flags == hashfALPHA) && (phashe->value <= alpha)) {
                recover_line(depth, phashe, pline);
                return alpha;
            }
            if ((phashe->flags == hashfBETA) && (phashe->value >= beta)) {
                recover_line(depth, phashe, pline);
                return beta;
            }
        }
    }
    return valUNKNOWN;
}

void RecordHash(int depth, int val, int best, int hashf, Line *pline, Board *board){
    //U64 key = generate_zobrist_key();
    if ((val != 0) && (val < 40000) && (val > -40000)) {
        HASHE *phashe = &hash_table[board->current_zobrist_key % tt_size];
        phashe->key = board->current_zobrist_key;
        phashe->best = best;
        phashe->value = val;
        phashe->flags = hashf;
        phashe->depth = depth;
        phashe->line = NULL;

        if (depth > 1) {

            if (lineMoveCount > (tt_size - 20)){
                printf("info string hash table reset\n");
                reset_hash_table();
            }

            phashe->line = &lines[lineMoveCount];
            lines[lineMoveCount] = pline->length;
            memcpy(&lines[lineMoveCount + 1], &pline->moves, sizeof(int) * pline->length);
            lineMoveCount += pline->length + 1;
        }
    }
}

