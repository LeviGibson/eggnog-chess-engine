//
// Created by levigibson on 6/17/21.
//

#include "transposition.h"
#include "board.h"

HASHE hash_table[tt_size];

void reset_hash_table(){
    memset(hash_table, 0, sizeof hash_table);
}

int ProbeHash(int depth, int alpha, int beta){
    HASHE * phashe = &hash_table[current_zobrist_key % tt_size];

    if (phashe->key == current_zobrist_key) {
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

void RecordHash(int depth, int val, int hashf){
    U64 key = generate_zobrist_key();
    HASHE * phashe = &hash_table[key % tt_size];
    phashe->key = key;
    //phashe->best = best_move;
    phashe->value = val;
    phashe->flags = hashf;
    phashe->depth = depth;
}
