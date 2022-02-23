//
// Created by levigibson on 2/18/22.
//

#include <stdio.h>
#include "see.h"

#define hash_size 500000
int32_t hash[hash_size];

int32_t can_piece_move_to(int32_t from, int32_t to, Board *board){
    copy_board();

    int32_t capturedPiece = piece_at(to, board);
    int32_t movedPiece = piece_at(from, board);

    if (capturedPiece != -1) {
        pop_bit(board->bitboards[capturedPiece], to);
    }
    pop_bit(board->bitboards[movedPiece], from);
    set_bit(board->bitboards[movedPiece], to);

    update_occupancies(board);

    int32_t inCheck = is_square_attacked(bsf((board->side == white) ? board->bitboards[K] : board->bitboards[k]), (board->side ^ 1), board);
    take_back();

    if (inCheck) return 0;
    return 1;
}

int32_t get_move_for_see(U64 mask, int32_t square, int32_t piece, Board *board){
    while (mask){
        int32_t bit = bsf(mask);


        if (can_piece_move_to(bit, square, board)){
            return encode_move(bit, square, piece, 0, 0, 1, 0, 0);
        }

        pop_bit(mask, bit);
    }

    return NO_MOVE;
}

int32_t get_smallest_attacker(int32_t square, Board *board){
    if (board->side == white){
        U64 mask = pawn_mask[black][square] & WP;
        int32_t move = get_move_for_see(mask, square, P, board);
        if (move != NO_MOVE) return move;

        mask = knight_mask[square] & WN;
        move = get_move_for_see(mask, square, N, board);
        if (move != NO_MOVE) return move;

        mask = get_bishop_attacks(square, board->occupancies[both]) & WB;
        move = get_move_for_see(mask, square, B, board);
        if (move != NO_MOVE) return move;

        mask = get_rook_attacks(square, board->occupancies[both]) & WR;
        move = get_move_for_see(mask, square, R, board);
        if (move != NO_MOVE) return move;

        mask = get_queen_attacks(square, board->occupancies[both]) & WQ;
        move = get_move_for_see(mask, square, Q, board);
        if (move != NO_MOVE) return move;

        mask = king_mask[square] & WK;
        move = get_move_for_see(mask, square, K, board);
        if (move != NO_MOVE) return move;

        return NO_MOVE;
    } else {
        U64 mask = pawn_mask[white][square] & BP;
        int32_t move = get_move_for_see(mask, square, p, board);
        if (move != NO_MOVE) return move;

        mask = knight_mask[square] & BN;
        move = get_move_for_see(mask, square, n, board);
        if (move != NO_MOVE) return move;

        mask = get_bishop_attacks(square, board->occupancies[both]) & BB;
        move = get_move_for_see(mask, square, b, board);
        if (move != NO_MOVE) return move;

        mask = get_rook_attacks(square, board->occupancies[both]) & BR;
        move = get_move_for_see(mask, square, r, board);
        if (move != NO_MOVE) return move;

        mask = get_queen_attacks(square, board->occupancies[both]) & BQ;
        move = get_move_for_see(mask, square, q, board);
        if (move != NO_MOVE) return move;

        mask = king_mask[square] & BK;
        move = get_move_for_see(mask, square, k, board);
        if (move != NO_MOVE) return move;

        return NO_MOVE;
    }
}

int32_t pieceValues[12] = {100, 300, 300, 500, 1000, 999999, 100, 300, 300, 500, 1000, -999999};

int32_t see(int32_t square, Board *board){
    int32_t value = 0;
    int32_t move = get_smallest_attacker(square, board);

    int32_t *hashptr = &hash[(board->current_zobrist_key ^ get_move_key(move)) % hash_size];
    if (*hashptr != NO_EVAL)
        return *hashptr;

    if (move != NO_MOVE){
        copy_board();

        int32_t pieceCapturedValue = pieceValues[piece_at(square, board)];
        int32_t success = make_move(move, all_moves, 0, board);
        if (!success){
            *hashptr = 0;
            return 0;
        }
        value = max(0, pieceCapturedValue - see(square, board));

        take_back();
    }

    *hashptr = value;
    return value;
}

int32_t seeCapture(int32_t move, Board *board){
    if (getenpessant(move))
        return 0;

    copy_board();
    board->nnueUpdate = 0;

    int32_t capturedPieceValue = pieceValues[piece_at(gettarget(move), board)];
    int32_t success = make_move(move, all_moves, 0, board);
    if (!success){
        take_back();
        return 0;
    }
    int32_t seeval = capturedPieceValue - see(gettarget(move), board);

    take_back();
    return seeval;
}

void init_see(){
    for (int32_t i = 0; i < hash_size; ++i) {
        hash[i] = NO_EVAL;
    }
}
