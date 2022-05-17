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


typedef struct tagHASHE {
    U64 key;
    unsigned char depth;
    unsigned char flags;
    int32_t value;
    int32_t best[6];
    int32_t *line;
}   HASHE;

typedef struct MoveEval MoveEval;
struct MoveEval{
    int32_t move[6];
    int32_t eval;
};

void RecordHash(int32_t depth, int32_t val, MoveEval *best, int32_t hashf, Line *pline, Board *board);
int32_t ProbeHash(int32_t depth, int32_t alpha, int32_t beta, int32_t *move, Line *pline, Board *board);

void reset_hash_table();
void reinit_transposition();
void transposition_free();

#endif //EGGNOG_CHESS_ENGINE_TRANSPOSITION_H
