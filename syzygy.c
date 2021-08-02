//
// Created by levigibson on 7/30/21.
//

#include "syzygy.h"
#include "bitboard.h"
#include "board.h"
#include "Fathom/tbprobe.h"

int tbInitilised = 0;

int parse_tb_move(unsigned from, unsigned to){

    moveList legalMoves;
    memset(&legalMoves, 0, sizeof legalMoves);
    generate_moves(&legalMoves);

    for (int i = 0; i < legalMoves.count; i++) {
        if ((get_move_source(legalMoves.moves[i]) == from) && (get_move_target(legalMoves.moves[i]) == to)){
            return legalMoves.moves[i];
        }
    }

    return 0;
}

unsigned get_wdl(){

    return tb_probe_wdl((occupancies[black]), (occupancies[white]), (bitboards[K] | bitboards[k]),
                        (bitboards[Q] | bitboards[q]), (bitboards[r] | bitboards[R]),
                        (bitboards[b] | bitboards[B]), (bitboards[N] | bitboards[n]),
                        (bitboards[P] | bitboards[p]), 0, castle, 0, side);
}

int get_root_move(){

    unsigned results[TB_MAX_MOVES];
    memset(results, 0, sizeof results);

    unsigned tbres = tb_probe_root((occupancies[black]), (occupancies[white]), (bitboards[K] | bitboards[k]),
                                   (bitboards[Q] | bitboards[q]), (bitboards[r] | bitboards[R]),
                                   (bitboards[b] | bitboards[B]), (bitboards[N] | bitboards[n]),
                                   (bitboards[P] | bitboards[p]), 0, castle, 0, (bool)side, results);

    return parse_tb_move(TB_GET_FROM(tbres), TB_GET_TO(tbres));
}
