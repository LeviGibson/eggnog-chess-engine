//
// Created by levigibson on 6/17/21.
//

#include "transposition.h"
#include "board.h"

HASHE hash_table[tt_size];

void reset_hash_table(){
    memset(hash_table, 0, sizeof hash_table);
}

int ProbeHash(int depth, int alpha, int beta, int *move, Board *board){
    HASHE * phashe = &hash_table[board->current_zobrist_key % tt_size];

    if (phashe->key == board->current_zobrist_key) {
        if (phashe->depth > 2)
            *move = phashe->best;

        if (phashe->depth >= depth) {
            if (phashe->flags == hashfEXACT)
                return phashe->value;

            if ((phashe->flags == hashfALPHA) &&
                (phashe->value <= alpha))

                return alpha;
            if ((phashe->flags == hashfBETA) &&

                (phashe->value >= beta))

                return beta;
        }
    }
    return valUNKNOWN;
}

void RecordHash(int depth, int val, int best, int hashf, Board *board){
    //U64 key = generate_zobrist_key();
    if ((val != 0) && (val < 40000) && (val > -40000)) {
        HASHE *phashe = &hash_table[board->current_zobrist_key % tt_size];
        phashe->key = board->current_zobrist_key;
        phashe->best = best;
        phashe->value = val;
        phashe->flags = hashf;
        phashe->depth = depth;
    }
}

