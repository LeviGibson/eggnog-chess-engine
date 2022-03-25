//
// Created by levigibson on 5/31/21.
//

#include "board.h"

#include <stdio.h>

const char *square_to_coordinates[] = {
        "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
        "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
        "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
        "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
        "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
        "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
        "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
};

const int32_t w_orient[64] = {
        56, 57, 58, 59, 60, 61, 62, 63,
        48, 49, 50, 51, 52, 53, 54, 55,
        40, 41, 42, 43, 44, 45, 46, 47,
        32, 33, 34, 35, 36, 37, 38, 39,
        24, 25, 26, 27, 28, 29, 30, 31,
        16, 17, 18, 19, 20, 21, 22, 23,
        8, 9, 10, 11, 12, 13, 14, 15,
        0, 1, 2, 3, 4, 5, 6, 7,
};

const int32_t b_orient[64] = {
        7, 6, 5, 4, 3, 2, 1, 0,
        15, 14, 13, 12, 11, 10, 9, 8,
        23, 22, 21, 20, 19, 18, 17, 16,
        31, 30, 29, 28, 27, 26, 25, 24,
        39, 38, 37, 36, 35, 34, 33, 32,
        47, 46, 45, 44, 43, 42, 41, 40,
        55, 54, 53, 52, 51, 50, 49, 48,
        63, 62, 61, 60, 59, 58, 57, 56
};

U64 piece_keys[12][64];
U64 castle_keys[16];
U64 enpessent_keys[64];
U64 side_keys[2];
U64 move_keys[4096];

void init_zobrist_keys(){
    for (int32_t piece = p_P; piece <= p_k; piece++){
        for (int32_t square = 0; square < 64; square++){
            piece_keys[piece][square] = get_random_U64();
        }
    }

    for (int32_t i = 0; i < 16; i++) {
        castle_keys[i] = get_random_U64();
    }

    for (int32_t square = 0; square < 64; square++){
        enpessent_keys[square] = get_random_U64();
    }

    for (int32_t square = 0; square < 4096; square++){
        move_keys[square] = get_random_U64();
    }

    side_keys[white] = get_random_U64();
    side_keys[black] = get_random_U64();

}

U64 get_move_key(int32_t move){
    move &= 0b00000000000000000000111111111111;
    assert(move < 4096);
    return move_keys[move];
}

void make_null_move(Board *board) {
    board->ply++;
    board->side ^= 1;
    board->zobrist_key_parts[12] = side_keys[board->side];
    board->current_zobrist_key ^= side_keys[white];
    board->current_zobrist_key ^= side_keys[black];
}

U64 update_zobrist_key(Board *board){

    U64 key = board->current_zobrist_key;

    for (int32_t piece = p_P; piece <= p_k; piece++){
        if (board->bitboards[piece] != board->zobrist_key_bitboards[piece]){
            key ^= board->zobrist_key_parts[piece];

            U64 bitboard = board->bitboards[piece];
            U64 bitboard_key = 0ULL;

            int32_t bb_count = count_bits(bitboard);

            for (int32_t _ = 0; _ < bb_count; _++){
                int32_t square = bsf(bitboard);

                bitboard_key ^= piece_keys[piece][square];

                pop_bit(bitboard, square);
            }

            key ^= bitboard_key;
            board->zobrist_key_parts[piece] = bitboard_key;
            board->zobrist_key_bitboards[piece] = board->bitboards[piece];
        }
    }

    key ^= board->zobrist_key_parts[12] ^ board->zobrist_key_parts[13];

    key ^= side_keys[board->side];
    key ^= castle_keys[board->castle];

    board->zobrist_key_parts[12] = side_keys[board->side];
    board->zobrist_key_parts[13] = castle_keys[board->castle];

    return key;
}

U64 generate_zobrist_key(Board *board){

    memcpy(board->zobrist_key_bitboards, board->bitboards, sizeof board->bitboards);

    U64 key = 0ULL;
    for (int32_t piece = p_P; piece <= p_k; piece++){
        U64 bitboard = board->bitboards[piece];
        U64 bitboard_key = 0ULL;
        while (bitboard){
            int32_t square = bsf(bitboard);

            bitboard_key ^= piece_keys[piece][square];

            pop_bit(bitboard, square);
        }

        board->zobrist_key_parts[piece] = bitboard_key;

        key ^= bitboard_key;
    }

    /**if (enpessant != no_sq){
        key ^= enpessent_keys[enpessant];
    }**/

    key ^= side_keys[board->side];
    key ^= castle_keys[board->castle];

    board->zobrist_key_parts[12] = side_keys[board->side];
    board->zobrist_key_parts[13] = castle_keys[board->castle];

    return key;
}

int32_t is_threefold_repetition(Board *board){

    if (board->zobrist_history_length < 8)
        return 0;

    if (board->zobrist_history_length >= 101){
        return 1;
    }

    int32_t repetitionlimit = 2;

    for (int32_t position = 0; position <= board->zobrist_history_length; position++){
        if (position == (board->zobrist_history_search_index + 1))
            repetitionlimit = 1;

        int32_t repetitions = 0;
        for (int32_t compare = position+1; compare <= board->zobrist_history_length; compare++){
            if (board->zobrist_history[compare] == board->zobrist_history[position]){

                repetitions++;

                if (repetitions >= repetitionlimit){
                    return 1;
                }
            }
        }
    }

    return 0;

}

static inline void generate_knight_moves(MoveList *legalMoves, Board *board){
    int32_t ptype = ((board->side == white) ? p_N : p_n);
    U64 bitboard = board->bitboards[ptype];

    U64 friendly_occupanices = board->occupancies[board->side];

    while (bitboard){
        int32_t square = bsf(bitboard);

        U64 bb_moves = knight_mask[square] & (~friendly_occupanices);

        while (bb_moves){
            int32_t target = bsf(bb_moves);

            if ((1ULL << target) & board->occupancies[both]) {
                legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, 0, 0, 1, 0, 0);
                legalMoves->count++;
            }
            else {
                legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, 0, 0, 0, 0, 0);
                legalMoves->count++;
            }

            pop_bit(bb_moves, target);
        }

        pop_bit(bitboard, square);
    }
}

static inline void generate_pawn_moves(MoveList *legalMoves, Board *board){
    int32_t ptype = ((board->side == white) ? p_P : p_p);
    U64 bitboard = board->bitboards[ptype];

    U64 enemy_occupanices = (board->side == white) ? board->occupancies[black] : board->occupancies[white];
    int32_t enpoffset = board->side == white ? 8 : -8;

    //this deals with a weird bug in Arena GUI
    if (board->enpessant != no_sq && (enemy_occupanices & (1ULL << (board->enpessant+enpoffset))))
        enemy_occupanices |= 1ULL << board->enpessant;

    while (bitboard){
        int32_t square = bsf(bitboard);

        //Pawn Captures
        U64 bb_moves = pawn_mask[board->side][square] & enemy_occupanices;

        while (bb_moves){
            int32_t target = bsf(bb_moves);

            //Enpessant
            if (target == board->enpessant) {
                legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, 0, 1, 1, 0, 0);
                legalMoves->count++;
            }
            else {
                if ((target <= 7) || (target >= 56)) {
                    if (board->side == white) {
                        legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, p_Q, 0, 1, 0, 0);
                        legalMoves->count++;
                        legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, p_B, 0, 1, 0, 0);
                        legalMoves->count++;
                        legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, p_R, 0, 1, 0, 0);
                        legalMoves->count++;
                        legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, p_N, 0, 1, 0, 0);
                        legalMoves->count++;
                    } else {
                        legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, p_q, 0, 1, 0, 0);
                        legalMoves->count++;
                        legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, p_b, 0, 1, 0, 0);
                        legalMoves->count++;
                        legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, p_r, 0, 1, 0, 0);
                        legalMoves->count++;
                        legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, p_n, 0, 1, 0, 0);
                        legalMoves->count++;
                    }

                } else{
                    legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, 0, 0, 1, 0, 0);
                    legalMoves->count++;
                }
            }

            pop_bit(bb_moves, target);
        }

        //Pawn quiet moves
        if (board->side == white){
            if (!(board->occupancies[both] & 1ULL << (square-8))) {

                if (square <= 15) {
                    legalMoves->moves[legalMoves->count] = encode_move(square, (square-8), ptype, p_Q, 0, 0, 0, 0);
                    legalMoves->count++;
                    legalMoves->moves[legalMoves->count] = encode_move(square, (square-8), ptype, p_R, 0, 0, 0, 0);
                    legalMoves->count++;
                    legalMoves->moves[legalMoves->count] = encode_move(square, (square-8), ptype, p_B, 0, 0, 0, 0);
                    legalMoves->count++;
                    legalMoves->moves[legalMoves->count] = encode_move(square, (square-8), ptype, p_N, 0, 0, 0, 0);
                    legalMoves->count++;
                }
                else {
                    legalMoves->moves[legalMoves->count] = encode_move(square, (square-8), ptype, 0, 0, 0, 0, 0);
                    legalMoves->count++;
                }

                //double pawn pushes
                if (square >= 48) {
                    if (!(board->occupancies[both] & 1ULL << (square-16))){
                        legalMoves->moves[legalMoves->count] = encode_move(square, (square-16), ptype, 0, 0, 0, 1, 0);
                        legalMoves->count++;
                    }
                }
            }
        } else {
            if (!(board->occupancies[both] & 1ULL << (square+8))) {

                if (square >= 48) {
                    legalMoves->moves[legalMoves->count] = encode_move(square, (square+8), ptype, p_q, 0, 0, 0, 0);
                    legalMoves->count++;
                    legalMoves->moves[legalMoves->count] = encode_move(square, (square+8), ptype, p_r, 0, 0, 0, 0);
                    legalMoves->count++;
                    legalMoves->moves[legalMoves->count] = encode_move(square, (square+8), ptype, p_b, 0, 0, 0, 0);
                    legalMoves->count++;
                    legalMoves->moves[legalMoves->count] = encode_move(square, (square+8), ptype, p_n, 0, 0, 0, 0);
                    legalMoves->count++;
                }
                else {
                    legalMoves->moves[legalMoves->count] = encode_move(square, (square+8), ptype, 0, 0, 0, 0, 0);
                    legalMoves->count++;
                }

                //double pawn pushes
                if (square <= 15) {
                    if (!(board->occupancies[both] & 1ULL << (square+16))){
                        legalMoves->moves[legalMoves->count] = encode_move(square, (square+16), ptype, 0, 0, 0, 1, 0);
                        legalMoves->count++;
                    }
                }
            }
        }

        pop_bit(bitboard, square);
    }
}

static inline void generate_bishop_moves(MoveList *legalMoves, Board *board){
    int32_t ptype = ((board->side == white) ? p_B : p_b);
    U64 bitboard = board->bitboards[ptype];

    U64 friendly_occupanices = board->occupancies[board->side];

    while (bitboard){
        int32_t square = bsf(bitboard);

        U64 bb_moves = get_bishop_attacks(square, board->occupancies[both]) & (~friendly_occupanices);

        while (bb_moves){
            int32_t target = bsf(bb_moves);

            if ((1ULL << target) & board->occupancies[both]) {
                legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, 0, 0, 1, 0, 0);
                legalMoves->count++;
            } else {
                legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, 0, 0, 0, 0, 0);
                legalMoves->count++;
            }

            pop_bit(bb_moves, target);
        }

        pop_bit(bitboard, square);
    }
}

static inline void generate_rook_moves(MoveList *legalMoves, Board *board){
    int32_t ptype = ((board->side == white) ? p_R : p_r);
    U64 bitboard = board->bitboards[ptype];

    U64 friendly_occupanices = board->occupancies[board->side];

    while (bitboard){
        int32_t square = bsf(bitboard);

        U64 bb_moves = get_rook_attacks(square, board->occupancies[both]) & (~friendly_occupanices);

        while (bb_moves){
            int32_t target = bsf(bb_moves);

            if ((1ULL << target) & board->occupancies[both]) {
                legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, 0, 0, 1, 0, 0);
                legalMoves->count++;
            }
            else {
                legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, 0, 0, 0, 0, 0);
                legalMoves->count++;
            }

            pop_bit(bb_moves, target);
        }

        pop_bit(bitboard, square);
    }
}

static inline void generate_queen_moves(MoveList *legalMoves, Board *board){
    int32_t ptype = ((board->side == white) ? p_Q : p_q);
    U64 bitboard = board->bitboards[ptype];

    U64 friendly_occupanices = board->occupancies[board->side];

    while (bitboard){
        int32_t square = bsf(bitboard);

        U64 bb_moves = (get_bishop_attacks(square, board->occupancies[both]) | get_rook_attacks(square, board->occupancies[both])) & (~friendly_occupanices);

        while (bb_moves){
            int32_t target = bsf(bb_moves);

            if ((1ULL << target) & board->occupancies[both]) {
                legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, 0, 0, 1, 0, 0);
                legalMoves->count++;
            } else {
                legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, 0, 0, 0, 0, 0);
                legalMoves->count++;
            }

            pop_bit(bb_moves, target);
        }

        pop_bit(bitboard, square);
    }
}

static inline void generate_king_moves(MoveList *legalMoves, Board *board){
    int32_t ptype = ((board->side == white) ? p_K : p_k);
    U64 bitboard = board->bitboards[ptype];

    U64 friendly_occupanices = board->occupancies[board->side];

    if (bitboard == 0){
        print_board(board);
    }

    int32_t square = bsf(bitboard);

    U64 bb_moves = king_mask[square] & (~friendly_occupanices);

    while (bb_moves){
        int32_t target = bsf(bb_moves);

        if ((1ULL << target) & board->occupancies[both]) {
            //source, target, ptype, prom, enpessant, capture
            legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, 0, 0, 1, 0, 0);
            legalMoves->count++;
        }
        else{
            legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, 0, 0, 0, 0, 0);
            legalMoves->count++;

        }

        pop_bit(bb_moves, target);
    }

    //castling
    if (board->side == white){
        if (board->castle & wk){
            //0x6000000000000000ULL
            //(1ULL << f1) | (1ULL << g1)))
            if (!(board->occupancies[both] & 0x6000000000000000ULL)){
                if (!(is_square_attacked(f1, black, board)) && !(is_square_attacked(g1, black, board)) && !(is_square_attacked(e1, black, board))){
                    legalMoves->moves[legalMoves->count] = encode_move(square, 62, ptype, 0, 0, 0, 0, 1);
                    legalMoves->count++;
                }
            }
        }
        if (board->castle & (wq)){
            //0xe00000000000000ULL
            //((1ULL << d1) | (1ULL << c1) | (1ULL << b1))
            if (!(board->occupancies[both] & 0xe00000000000000ULL)){
                if (!(is_square_attacked(c1, black, board)) && !(is_square_attacked(d1, black, board)) && !(is_square_attacked(e1, black, board))){
                    legalMoves->moves[legalMoves->count] = encode_move(square, 58, ptype, 0, 0, 0, 0, 1);
                    legalMoves->count++;
                }
            }
        }
    }else{
        if (board->castle & bk){
            //0x60ULL
            //((1ULL << f8) | (1ULL << g8))
            if (!(board->occupancies[both] & 0x60ULL)){
                if (!(is_square_attacked(f8, white, board)) && !(is_square_attacked(g8, white, board)) && !(is_square_attacked(e8, white, board))){
                    legalMoves->moves[legalMoves->count] = encode_move(square, 6, ptype, 0, 0, 0, 0, 1);
                    legalMoves->count++;
                }
            }
        }
        if (board->castle & (bq)){
            //0xeULL
            //((1ULL << d8) | (1ULL << c8) | (1ULL << b8))
            if (!(board->occupancies[both] & 0xeULL)){
                if (!(is_square_attacked(c8, white, board)) && !(is_square_attacked(d8, white, board)) && !(is_square_attacked(e8, white, board))){
                    legalMoves->moves[legalMoves->count] = encode_move(square, 2, ptype, 0, 0, 0, 0, 1);
                    legalMoves->count++;
                }
            }
        }
    }
}

void update_occupancies(Board *board){

    memset(board->occupancies, 0ULL, sizeof(board->occupancies));

    for (int32_t piece = p_P; piece <= p_K; piece++){
        board->occupancies[white] |= board->bitboards[piece];
    }

    for (int32_t piece = p_p; piece <= p_k; piece++){
        board->occupancies[black] |= board->bitboards[piece];
    }

    board->occupancies[both] = (board->occupancies[black] | board->occupancies[white]);
}

void generate_moves(MoveList *legalMoves, Board *board){
    legalMoves->count = 0;

    generate_knight_moves(legalMoves, board);
    generate_pawn_moves(legalMoves, board);
    generate_bishop_moves(legalMoves, board);
    generate_rook_moves(legalMoves, board);
    generate_queen_moves(legalMoves, board);
    generate_king_moves(legalMoves, board);
}

const int32_t castling_right_table[64] = {
        7, 15, 15, 15,  3, 15, 15, 11,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        13, 15, 15, 15, 12, 15, 15, 14
};

void refresh_weak_squares(Board *board){
    U64 *wb = &board->unprotectedPieces[white];
    U64 *bb = &board->unprotectedPieces[black];

    *wb = 0ULL;
    *bb = 0ULL;

    for (int32_t pt = 0; pt < 12; ++pt) {

        U64 bitboard = board->bitboards[pt];
        if (!bitboard) continue;

        int32_t bitcount = count_bits(bitboard);

        for (int32_t i = 0; i < bitcount; ++i) {
            int32_t square = bsf(bitboard);

            if (pt == p_P)
                *wb |= pawn_mask[white][square];
            if (pt == p_N)
                *wb |= knight_mask[square];
            if (pt == p_B)
                *wb |= get_bishop_attacks(square, board->occupancies[both]);
            if (pt == p_R)
                *wb |= get_rook_attacks(square, board->occupancies[both]);
            if (pt == p_K)
                *wb |= king_mask[square];
            if (pt == p_Q)
                *wb |= get_bishop_attacks(square, board->occupancies[both]) | get_rook_attacks(square, board->occupancies[both]);
            if (pt == p_p)
                *bb |= pawn_mask[black][square];
            if (pt == p_n)
                *bb |= knight_mask[square];
            if (pt == p_b)
                *bb |= get_bishop_attacks(square, board->occupancies[both]);
            if (pt == p_r)
                *bb |= get_rook_attacks(square, board->occupancies[both]);
            if (pt == p_k)
                *bb |= king_mask[square];
            if (pt == p_q)
                *bb |= get_rook_attacks(square, board->occupancies[both]) | get_bishop_attacks(square, board->occupancies[both]);

            pop_bit(bitboard, square);
        }
    }

    *wb = (~*wb) & board->occupancies[white];
    *bb = (~*bb) & board->occupancies[black];
}

int32_t make_move(int32_t move, int32_t flag, int32_t notquinode, Board *board){

    if (flag == all_moves){
        copy_board();

        board->ply++;

        int32_t ptype = getpiece(move);
        int32_t source = getsource(move);
        int32_t target = gettarget(move);
        int32_t capture = getcapture(move);

        //if move is a king move, refresh the accumulator;
        if (ptype == p_K || ptype == p_k){
            pop_bit(board->bitboards[ptype], source);
            set_bit(board->bitboards[ptype], target);
            refresh_accumulator(&board->currentNnue, board);
        } else {
            nnue_pop_bit(ptype, source, board);
            nnue_set_bit(ptype, target, board);
        }

        if (capture){
            if (getenpessant(move)){
                if (board->side == white){
                    nnue_pop_bit(p_p, target + 8, board);
                } else {
                    nnue_pop_bit(p_P, target - 8, board);
                }
            } else {
                if (board->side == white){
                    for (int32_t piece = p_p; piece <= p_k; piece++) {
                        if (get_bit(board->bitboards[piece], target)) {

                            nnue_pop_bit(piece, target, board);

                            break;
                        }
                    }
                } else {
                    for (int32_t piece = p_P; piece <= p_K; piece++) {
                        if (get_bit(board->bitboards[piece], target)) {

                            nnue_pop_bit(piece, target, board);

                            break;
                        }
                    }
                }
            }
        }

        if (getdouble(move)){
            board->enpessant = (board->side == white) ? (target + 8) : (target - 8);
        } else {
            board->enpessant = no_sq;
        }

        if (getcastle(move)){
            if (board->side == white){
                if (target == 62){
                    nnue_pop_bit(p_R, 63, board);
                    nnue_set_bit(p_R, 61, board);
                } else if (target == 58){
                    nnue_pop_bit(p_R, 56, board);
                    nnue_set_bit(p_R, 59, board);
                }
            } else {
                if (target == 6){
                    nnue_pop_bit(p_r, 7, board);
                    nnue_set_bit(p_r, 5, board);
                } else if (target == 2){
                    nnue_pop_bit(p_r, 0, board);
                    nnue_set_bit(p_r, 3, board);
                }
            }
        }

        int32_t promoted = getpromoted(move);
        if (promoted){
            nnue_pop_bit(ptype, target, board);
            nnue_set_bit(promoted, target, board);
        }

        update_occupancies(board);

        if (board->side == white){
            if (is_square_attacked(bsf(board->bitboards[p_K]), black, board)){
                take_back();
                return 0;
            }
        } else {
            if (is_square_attacked(bsf(board->bitboards[p_k]), white, board)){
                take_back();
                return 0;
            }
        }

        board->castle &= castling_right_table[source];
        board->castle &= castling_right_table[target];

        board->side ^= 1;

        if ((ptype == p_P) || (ptype == p_p) || (board_copy.castle != board->castle) || capture){
            memset(board->zobrist_history, 0, sizeof(board->zobrist_history));
            board->zobrist_history_length = 0;
        }

        //UPDATE ZOBRIST HISTORY
        board->current_zobrist_key = update_zobrist_key(board);
        board->zobrist_history[board->zobrist_history_length] = board->current_zobrist_key;
        board->zobrist_history_length++;
    } else {
        if (getcapture(move) || is_move_direct_check(move, board) || getpromoted(move)){
            return make_move(move, all_moves, notquinode, board);
        } else {
            return 0;
        }
    }

    if (notquinode)
        refresh_weak_squares(board);

    board->prevmove = move;

    return 1;
}

void generate_only_legal_moves(MoveList *moves, Board *board){
    memset(moves, 0, sizeof(MoveList));
    MoveList tmp;
    memset(&tmp, 0, sizeof (tmp));
    generate_moves(&tmp, board);

    copy_board();
    for (int32_t i = 0; i < tmp.count; ++i) {
        if (make_move(tmp.moves[i], all_moves, 1, board)) {
            moves->moves[moves->count] = tmp.moves[i];
            moves->count++;
        }
        take_back();
    }
}

int32_t piece_at(int32_t square, Board *board){
    for (int32_t i = 0; i < 12; ++i) {
        if (board->bitboards[i] & 1ULL << square)
            return i;
    }

    return -1;
}

//Simple and fast function to judge if a move is a check.
//Does not work with discovered checks or castle-checks

int32_t is_move_direct_check(int32_t move, Board *board) {
    int32_t ptype = getpiece(move);
    U64 kingbb = board->side == white ? board->bitboards[p_k] : board->bitboards[p_K];
    int32_t kingsq = bsf(kingbb);
    int32_t target = gettarget(move);
    U64 targetBb = 1ULL << target;

    if (ptype == p_N || ptype == p_n){
        if (knight_mask[target] & kingbb)
            return 1;
        else
            return 0;
    }

    if (ptype == p_P){
        if (pawn_mask[white][target] & kingbb)
            return 1;
        else
            return 0;
    }

    if (ptype == p_p){
        if (pawn_mask[black][target] & kingbb)
            return 1;
        else
            return 0;
    }

    if (ptype == p_B || ptype == p_b){
        if (get_bishop_attacks(kingsq, board->occupancies[both]) & targetBb)
            return 1;
        else
            return 0;
    }

    if (ptype == p_R || ptype == p_r){
        if (get_rook_attacks(kingsq, board->occupancies[both]) & targetBb)
            return 1;
        else
            return 0;
    }

    if (ptype == p_Q || ptype == p_q){
        if (get_bishop_attacks(kingsq, board->occupancies[both]) & targetBb)
            return 1;
        if (get_rook_attacks(kingsq, board->occupancies[both]) & targetBb)
            return 1;

        return 0;
    }

    return 0;
}

int32_t is_square_attacked(int32_t square, int32_t testingSide, Board *board){

    if ((testingSide == white) && (pawn_mask[black][square] & board->bitboards[p_P])) return 1;
    if ((testingSide == black) && (pawn_mask[white][square] & board->bitboards[p_p])) return 1;

    if (knight_mask[square] & ((testingSide == white ? board->bitboards[p_N] : board->bitboards[p_n]))) return 1;

    if (get_bishop_attacks(square, board->occupancies[both]) & ((testingSide == white ? board->bitboards[p_B] : board->bitboards[p_b]) | (testingSide == white ? board->bitboards[p_Q] : board->bitboards[p_q]))) return 1;
    if (get_rook_attacks(square, board->occupancies[both]) & ((testingSide == white ? board->bitboards[p_R] : board->bitboards[p_r]) | (testingSide == white ? board->bitboards[p_Q] : board->bitboards[p_q]))) return 1;

    if (king_mask[square] & ((testingSide == white ? board->bitboards[p_K] : board->bitboards[p_k]))) return 1;

    return 0;
}

//
//int32_t piece_values[12] = {10, 30, 32, 50, 100, 0, -10, -30, -32, -50, -100, 0};
//
//int32_t get_smallest_attacker(int32_t square, int32_t testingSide){
//    if ((testingSide == white) && (pawn_mask[black][square] & bitboards[P])) return P;
//
//    if ((testingSide == black) && (pawn_mask[white][square] & bitboards[p])) return p;
//
//    if (knight_mask[square] & ((testingSide == white) ? bitboards[N] : bitboards[n])) return testingSide == white ? N : n;
//
//    if (get_bishop_attacks(testingSide, occupancies[both]) & ((testingSide == white) ? bitboards[B] : bitboards[b])) testingSide == white ? B : b;
//
//    if (get_rook_attacks(testingSide, occupancies[both]) & ((testingSide == white) ? bitboards[R] : bitboards[r])) testingSide == white ? R : r;
//
//    if (get_queen_attacks(testingSide, occupancies[both]) & ((testingSide == white) ? bitboards[Q] : bitboards[q])) testingSide == white ? Q : q;
//
//    if (king_mask[square] & ((testingSide == white) ? bitboards[K] : bitboards[k])) testingSide == white ? K : k;
//
//    return -1;
//}
//
//int32_t static_exchange_evaluation(int32_t move) {
//    int32_t exchange_eval = 0;
//
//    int32_t target = get_move_target(move);
//    int32_t cap = piece_at(target);
//    exchange_eval -= piece_values[cap];
//
//    if (get_smallest_attacker(target, side^1) != -1)
//        exchange_eval -= piece_values[get_move_piece(move)];
//
//    return side == white ? exchange_eval + 2000 : (-exchange_eval) + 2000;
//}

int32_t char_pieces[] = {
        ['P'] = p_P,
        ['N'] = p_N,
        ['B'] = p_B,
        ['R'] = p_R,
        ['Q'] = p_Q,
        ['K'] = p_K,
        ['p'] = p_p,
        ['n'] = p_n,

        ['b'] = p_b,
        ['r'] = p_r,
        ['q'] = p_q,
        ['k'] = p_k
};
char piece_symbols[12] = {
        'P', 'N', 'B', 'R', 'Q', 'K', 'p', 'n', 'b', 'r', 'q', 'k'
};

void print_move(int32_t move){
    printf("%s%s", square_to_coordinates[getsource(move)], square_to_coordinates[gettarget(move)]);
    if ((getpromoted(move) == p_Q) | (getpromoted(move) == p_q)){
        printf("q");
    }
    if ((getpromoted(move) == p_B) | (getpromoted(move) == p_b)){
        printf("b");
    }
    if ((getpromoted(move) == p_N) | (getpromoted(move) == p_n)){
        printf("n");
    }
    if ((getpromoted(move) == p_R) | (getpromoted(move) == p_r)){
        printf("r");
    }
}

void print_board(Board *board){
    int32_t rank = 8;
    for (int32_t square = 0; square < 64; square++){

        if (!(square % 8)) {
            printf("\n");
            printf("%d | ", rank);
            rank--;
        }

        int32_t found_piece = 0;

        for (int32_t piece = p_P; piece <= p_k; piece++){
            if (get_bit(board->bitboards[piece], square)){
                printf("%c ", piece_symbols[piece]);
                found_piece = 1;
                break;
            }
        }

        if (!found_piece)
            printf(". ");
    }
    printf("\n    _______________\n");
    printf("    A B C D E F G H\n\n");
}



void print_fen(Board *board){
    for (int32_t r = 0; r < 8; ++r) {
        int32_t esq = 0;
        for (int32_t f = 0; f < 8; ++f) {

            int32_t p = piece_at((r * 8) + f, board);
            if (p == -1) { esq++; }
            else {
                if (esq > 0) {
                    printf("%d", esq);
                    esq = 0;
                }
                printf("%c", piece_symbols[p]);
            }
        }
        if (esq > 0)
            printf("%d", esq);
        if (r != 7)
            printf("/");
    }

    printf(board->side == white ? " w\n" : " b\n");
}

void parse_fen(char *fen, Board *board)
{
    board->zobrist_history_length = 0;

    memset(board->bitboards, 0ULL, sizeof(board->bitboards));

    memset(board->occupancies, 0ULL, sizeof(board->occupancies));

    memset(board->zobrist_history, 0, sizeof(board->zobrist_history));
    board->zobrist_history_length = 0;

    board->side = 0;
    board->enpessant = no_sq;
    board->castle = 0;

    for (int32_t rank = 0; rank < 8; rank++)
    {
        for (int32_t file = 0; file < 8; file++)
        {
            int32_t square = rank * 8 + file;

            if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z'))
            {
                int32_t piece = char_pieces[*fen];

                set_bit(board->bitboards[piece], square);

                fen++;
            }

            if (*fen >= '0' && *fen <= '9')
            {
                int32_t offset = *fen - '0';

                int32_t piece = -1;

                for (int32_t bb_piece = p_P; bb_piece <= p_k; bb_piece++)
                {
                    if (get_bit(board->bitboards[bb_piece], square))
                        piece = bb_piece;
                }
                if (piece == -1)
                    file--;

                file += offset;

                fen++;
            }

            if (*fen == '/')
                fen++;
        }
    }
    fen++;

    (*fen == 'w') ? (board->side = white) : (board->side = black);

    fen += 2;

    while (*fen != ' ')
    {
        switch (*fen)
        {
            case 'K': board->castle |= wk; break;
            case 'Q': board->castle |= wq; break;
            case 'k': board->castle |= bk; break;
            case 'q': board->castle |= bq; break;
            case '-': break;
        }

        fen++;
    }

    fen++;

    if (*fen != '-')
    {
        int32_t file = fen[0] - 'a';
        int32_t rank = 8 - (fen[1] - '0');

        board->enpessant = rank * 8 + file;
    }

    else
        board->enpessant = no_sq;

    for (int32_t piece = p_P; piece <= p_K; piece++)
        board->occupancies[white] |= board->bitboards[piece];

    for (int32_t piece = p_p; piece <= p_k; piece++)
        board->occupancies[black] |= board->bitboards[piece];

    update_occupancies(board);
    board->current_zobrist_key = generate_zobrist_key(board);
    board->helperThread = 0;
    board->nnueUpdate = 1;
    board->depthAdjuster = 0;
    board->kpExtended = 0;

    refresh_accumulator(&board->currentNnue, board);
    refresh_weak_squares(board);
}
