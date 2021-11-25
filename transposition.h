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
#define tt_size 5000000ULL

typedef struct tagHASHE {
    U64 key;
    int depth;
    int flags;
    int value;
    int best;
    int *line;
}   HASHE;

void RecordHash(int depth, int val, int best, int hashf, Line *pline, Board *board);
int ProbeHash(int depth, int alpha, int beta, int *move, Line *pline, Board *board);

void reset_hash_table();

#endif //EGGNOG_CHESS_ENGINE_TRANSPOSITION_H
