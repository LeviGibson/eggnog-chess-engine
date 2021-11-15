//
// Created by levigibson on 7/30/21.
//

#include "syzygy.h"
#include "bitboard.h"
#include "Fathom/tbprobe.h"


int parse_tb_move(unsigned from, unsigned to, Board *board){

    moveList legalMoves;
    memset(&legalMoves, 0, sizeof legalMoves);
    generate_moves(&legalMoves, board);

    for (int i = 0; i < legalMoves.count; i++) {
        if ((get_move_source(legalMoves.moves[i]) == from) && (get_move_target(legalMoves.moves[i]) == to)){
            return legalMoves.moves[i];
        }
    }

    return 0;
}

unsigned get_wdl(Board *board){

    return tb_probe_wdl((board->occupancies[black]), (board->occupancies[white]), (board->bitboards[K] | board->bitboards[k]),
                        (board->bitboards[Q] | board->bitboards[q]), (board->bitboards[r] | board->bitboards[R]),
                        (board->bitboards[b] | board->bitboards[B]), (board->bitboards[N] | board->bitboards[n]),
                        (board->bitboards[P] | board->bitboards[p]), 0, board->castle, 0, board->side);
}

int get_root_move(Board *board){

    unsigned results[TB_MAX_MOVES];
    memset(results, 0, sizeof results);

    unsigned tbres = tb_probe_root((board->occupancies[black]), (board->occupancies[white]), (board->bitboards[K] | board->bitboards[k]),
                                   (board->bitboards[Q] | board->bitboards[q]), (board->bitboards[r] | board->bitboards[R]),
                                   (board->bitboards[b] | board->bitboards[B]), (board->bitboards[N] | board->bitboards[n]),
                                   (board->bitboards[P] | board->bitboards[p]), 0, board->castle, 0, (bool)board->side, results);

    return parse_tb_move(TB_GET_FROM(tbres), TB_GET_TO(tbres), board);
}
