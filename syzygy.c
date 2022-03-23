//
// Created by levigibson on 7/30/21.
//

#include "syzygy.h"
#include "bitboard.h"
#include "Fathom/tbprobe.h"


int32_t parse_tb_move(uint32_t from, uint32_t to, uint32_t prom, Board *board){

    MoveList legalMoves;
    memset(&legalMoves, 0, sizeof legalMoves);
    generate_moves(&legalMoves, board);

    for (int32_t i = 0; i < legalMoves.count; i++) {
        if ((getsource(legalMoves.moves[i]) == from) && (gettarget(legalMoves.moves[i]) == to)){
            if ((prom == TB_PROMOTES_QUEEN) && (getpromoted(legalMoves.moves[i]) != p_Q) && (getpromoted(legalMoves.moves[i]) != p_q))
                continue;
            if ((prom == TB_PROMOTES_KNIGHT) && (getpromoted(legalMoves.moves[i]) != p_N) && (getpromoted(legalMoves.moves[i]) != p_n))
                continue;
            if ((prom == TB_PROMOTES_BISHOP) && (getpromoted(legalMoves.moves[i]) != p_B) && (getpromoted(legalMoves.moves[i]) != p_b))
                continue;
            if ((prom == TB_PROMOTES_ROOK) && (getpromoted(legalMoves.moves[i]) != p_R) && (getpromoted(legalMoves.moves[i]) != p_r))
                continue;
            return legalMoves.moves[i];
        }
    }

    return 0;
}

uint32_t get_wdl(Board *board){

    return tb_probe_wdl((board->occupancies[black]), (board->occupancies[white]), (board->bitboards[p_K] | board->bitboards[p_k]),
                        (board->bitboards[p_Q] | board->bitboards[p_q]), (board->bitboards[p_r] | board->bitboards[p_R]),
                        (board->bitboards[p_b] | board->bitboards[p_B]), (board->bitboards[p_N] | board->bitboards[p_n]),
                        (board->bitboards[p_P] | board->bitboards[p_p]), 0, board->castle, 0, board->side);
}

int32_t get_root_move(Board *board){

    uint32_t results[TB_MAX_MOVES];
    memset(results, 0, sizeof results);

    uint32_t tbres = tb_probe_root((board->occupancies[black]), (board->occupancies[white]), (board->bitboards[p_K] | board->bitboards[p_k]),
                                   (board->bitboards[p_Q] | board->bitboards[p_q]), (board->bitboards[p_r] | board->bitboards[p_R]),
                                   (board->bitboards[p_b] | board->bitboards[p_B]), (board->bitboards[p_N] | board->bitboards[p_n]),
                                   (board->bitboards[p_P] | board->bitboards[p_p]), 0, board->castle, 0, (bool)board->side, results);

    return parse_tb_move(TB_GET_FROM(tbres), TB_GET_TO(tbres), TB_GET_PROMOTES(tbres), board);
}
