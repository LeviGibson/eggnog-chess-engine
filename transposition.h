//
// Created by levigibson on 6/17/21.
//

#ifndef EGGNOG_CHESS_ENGINE_TRANSPOSITION_H
#define EGGNOG_CHESS_ENGINE_TRANSPOSITION_H

#include "bitboard.h"

#define hashfEXACT   0
#define hashfALPHA   1
#define hashfBETA    2
#define valUNKNOWN -1
#define tt_size 131072ULL

typedef struct tagHASHE {
    U64 key;
    int depth;
    int flags;
    int value;
    int best;
}   HASHE;

void RecordHash(int depth, int val, int hashf);
int ProbeHash(int depth, int alpha, int beta);

#endif //EGGNOG_CHESS_ENGINE_TRANSPOSITION_H
