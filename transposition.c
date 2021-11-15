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

//position startpos moves e2e4 b8c6 d2d4 e7e6 g1f3 d7d5 b1d2 g8e7 f1e2 d5e4 d2e4 e7f5 e2b5 f8b4 c2c3 b4e7 e1g1 e8g8 d1c2 f5d6 b5c6 b7c6 f3e5 d8e8 c3c4 d6e4 c2e4 c8b7 c1f4 e7f6 a1d1 a8b8 e5g4 f6d8 e4e3 h7h5 g4e5 d8f6 f1e1 e8d8 e3a3 a7a6 a3c5 f6e7 c5a5 b8c8 a5c3 h5h4 c3f3 d8e8 c4c5 c8b8 e5c4 e8d7 c4e5 d7e8 e5c4 e8d7 f4e5 b8d8 a2a3 f8e8 e5g7 d7d5 f3g4 d5g5 g4g5 e7g5 g7e5 d8d7 e1e4 g8h7 h2h3 e8d8 c4a5 b7a8 a5c4 a8b7 c4d2 f7f5 e4e1 h7g8 d2c4 d7f7 d1d3 g5f6 d3b3 b7a8 b3d3 a8b7 d3b3 b7a8 b3b4 f5f4 e1e4 f6g5 c4d2 g8h7 d2f3 h7h6 b4b3 d8f8 g1h1 a6a5 f3g5 h6g5 h1g1 g5h6 b3f3 h6g5 g2g3 f4g3 f3f7 f8f7 f2g3 h4g3 e5g3 f7d7 e4e6 d7d4 g3c7 d4d1 g1f2 d1d2 f2g1 d2d1 g1g2 d1d2 g2h1 d2b2 c7a5 a8b7 a5b4 b7c8 e6e3 c8f5 h1g1 b2b1 g1g2 b1b2 g2g1 g5h6 b4c3 b2b1 g1g2 b1c1 g2h2 c1c2 h2g1 c2c1 g1g2 c1c2 g2g3 h6h7 h3h4 f5g6 e3e7 h7g8 c3d4 g6f5 e7e5 f5d7 e5g5 g8f8 g5g7 d7f5 g7c7 f5e4 h4h5 c2d2 c7c8 f8f7 c8c7 f7g8 d4e5 d2a2 c7c8 g8f7 c8a8 e4f5 e5d4 a2d2 d4e3 d2e2 a8a7 f7g8 a7a8 g8f7 a8a7 f7g8 e3d4 e2d2 a7a8 g8f7 d4f2 d2d3 g3f4 f7g7 a8a7 g7g8 h5h6 f5d7 f2e3 d7f5 f4f3 d3c3 f3f2 f5e4 a7e7 e4f5 a3a4 c3c4 e7g7 g8h8 a4a5 c4a4 g7f7 f5e4 f7e7 e4d5 e7e8 h8h7 e8e7 h7h8 a5a6 d5b3 a6a7 a4a2 f2g3 b3c2 e7e8 h8h7 a7a8q a2a8 e8a8 c2f5 a8a7 h7g8 a7g7 g8f8 e3d4 f8e8 h6h7 f5h7 g7h7 e8d8 d4e5 d8e8 e5d6

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
