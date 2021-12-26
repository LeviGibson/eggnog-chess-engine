//
// Created by levigibson on 6/17/21.
//

#ifndef EGGNOG_CHESS_ENGINE_TRANSPOSITION_H
#define EGGNOG_CHESS_ENGINE_TRANSPOSITION_H

#include "bitboard.h"
#include "board.h"
#include "search.h"

#define hashfEXACT   0
#define hashfALPHA   1
#define hashfBETA    2
#define valUNKNOWN -1
#define NO_EVAL 9999999
#define tt_size 2000000ULL
#define tt_linesize (tt_size * 6)

typedef struct tagHASHE {
    U64 key;
    unsigned char depth;
    unsigned char flags;
    int value;
    int best[4];
    int staticeval;
    int *line;
}   HASHE;

void RecordHash(int depth, int val, int best, int hashf, int staticeval, Line *pline, Board *board);
int ProbeHash(int depth, int alpha, int beta, int *move, int *staticeval, Line *pline, Board *board);

void reset_hash_table();
void init_transposition();
void transposition_free();

#endif //EGGNOG_CHESS_ENGINE_TRANSPOSITION_H
