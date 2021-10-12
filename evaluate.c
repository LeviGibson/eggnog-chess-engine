#include "evaluate.h"
#include "board.h"
#include "bitboard.h"
#include "nnue/propogate.h"

int evaluate(NnueData *data){

    int nnue_eval = nnue_evaluate(data);
    return (side == white) ? nnue_eval : -nnue_eval;

}
