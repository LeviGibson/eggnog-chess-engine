//
// Created by levigibson on 5/31/21.
//

#include "board.h"
#include "nnue/propogate.h"

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

U64 piece_keys[12][64];
U64 castle_keys[16];
U64 enpessent_keys[64];
U64 side_keys[2];

void init_zobrist_keys(){
    for (int piece = P; piece <= k; piece++){
        for (int square = 0; square < 64; square++){
            piece_keys[piece][square] = get_random_U64();
        }
    }

    for (int i = 0; i < 16; i++) {
        castle_keys[i] = get_random_U64();
    }

    for (int square = 0; square < 64; square++){
        enpessent_keys[square] = get_random_U64();
    }

    side_keys[white] = get_random_U64();
    side_keys[black] = get_random_U64();

}

U64 update_zobrist_key(Board *board){

    U64 key = board->current_zobrist_key;

    for (int piece = P; piece <= k; piece++){
        if (board->bitboards[piece] != board->zobrist_key_bitboards[piece]){
            key ^= board->zobrist_key_parts[piece];

            U64 bitboard = board->bitboards[piece];
            U64 bitboard_key = 0ULL;

            int bb_count = count_bits(bitboard);

            for (int _ = 0; _ < bb_count; _++){
                int square = bsf(bitboard);

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
    //memcpy(&zobrist_key_bitboards, &bitboards, sizeof bitboards);

    return key;
}

U64 generate_zobrist_key(Board *board){

    memcpy(board->zobrist_key_bitboards, board->bitboards, sizeof board->bitboards);

    U64 key = 0ULL;
    for (int piece = P; piece <= k; piece++){
        U64 bitboard = board->bitboards[piece];
        U64 bitboard_key = 0ULL;
        while (bitboard){
            int square = bsf(bitboard);

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

int is_threefold_repetition(Board *board){

    if (board->zobrist_history_length < 8)
        return 0;

    if (board->zobrist_history_length >= 100){
        return 1;
    }

    for (int position = 0; position <= board->zobrist_history_length; position++){
        int repetitions = 0;
        for (int compare = position+1; compare <= board->zobrist_history_length; compare++){
            if (board->zobrist_history[compare] == board->zobrist_history[position]){

                repetitions++;

                if (repetitions >= 2){
                    return 1;
                }
            }
        }
    }

    return 0;

}

static inline void generate_knight_moves(moveList *legalMoves, Board *board){
    int ptype = ((board->side == white) ? N : n);
    U64 bitboard = board->bitboards[ptype];

    U64 friendly_occupanices = board->occupancies[board->side];

    while (bitboard){
        int square = bsf(bitboard);

        U64 bb_moves = knight_mask[square] & (~friendly_occupanices);

        while (bb_moves){
            int target = bsf(bb_moves);

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

static inline void generate_pawn_moves(moveList *legalMoves, Board *board){
    int ptype = ((board->side == white) ? P : p);
    U64 bitboard = board->bitboards[ptype];

    U64 enemy_occupanices = (board->side == white) ? board->occupancies[black] : board->occupancies[white];

    if (board->enpessant != no_sq)
        enemy_occupanices |= 1ULL << board->enpessant;

    while (bitboard){
        int square = bsf(bitboard);

        //Pawn Captures
        U64 bb_moves = pawn_mask[board->side][square] & enemy_occupanices;

        while (bb_moves){
            int target = bsf(bb_moves);

            //Enpessant
            if (target == board->enpessant) {
                legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, 0, 1, 1, 0, 0);
                legalMoves->count++;
            }
            else {
                if ((target <= 7) || (target >= 56)) {
                    if (board->side == white) {
                        legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, Q, 0, 1, 0, 0);
                        legalMoves->count++;
                        legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, B, 0, 1, 0, 0);
                        legalMoves->count++;
                        legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, R, 0, 1, 0, 0);
                        legalMoves->count++;
                        legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, N, 0, 1, 0, 0);
                        legalMoves->count++;
                    } else {
                        legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, q, 0, 1, 0, 0);
                        legalMoves->count++;
                        legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, b, 0, 1, 0, 0);
                        legalMoves->count++;
                        legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, r, 0, 1, 0, 0);
                        legalMoves->count++;
                        legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, n, 0, 1, 0, 0);
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
                    legalMoves->moves[legalMoves->count] = encode_move(square, (square-8), ptype, Q, 0, 0, 0, 0);
                    legalMoves->count++;
                    legalMoves->moves[legalMoves->count] = encode_move(square, (square-8), ptype, R, 0, 0, 0, 0);
                    legalMoves->count++;
                    legalMoves->moves[legalMoves->count] = encode_move(square, (square-8), ptype, B, 0, 0, 0, 0);
                    legalMoves->count++;
                    legalMoves->moves[legalMoves->count] = encode_move(square, (square-8), ptype, N, 0, 0, 0, 0);
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
                    legalMoves->moves[legalMoves->count] = encode_move(square, (square+8), ptype, q, 0, 0, 0, 0);
                    legalMoves->count++;
                    legalMoves->moves[legalMoves->count] = encode_move(square, (square+8), ptype, r, 0, 0, 0, 0);
                    legalMoves->count++;
                    legalMoves->moves[legalMoves->count] = encode_move(square, (square+8), ptype, b, 0, 0, 0, 0);
                    legalMoves->count++;
                    legalMoves->moves[legalMoves->count] = encode_move(square, (square+8), ptype, n, 0, 0, 0, 0);
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

static inline void generate_bishop_moves(moveList *legalMoves, Board *board){
    int ptype = ((board->side == white) ? B : b);
    U64 bitboard = board->bitboards[ptype];

    U64 friendly_occupanices = board->occupancies[board->side];

    while (bitboard){
        int square = bsf(bitboard);

        U64 bb_moves = get_bishop_attacks(square, board->occupancies[both]) & (~friendly_occupanices);

        while (bb_moves){
            int target = bsf(bb_moves);

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

static inline void generate_rook_moves(moveList *legalMoves, Board *board){
    int ptype = ((board->side == white) ? R : r);
    U64 bitboard = board->bitboards[ptype];

    U64 friendly_occupanices = board->occupancies[board->side];

    while (bitboard){
        int square = bsf(bitboard);

        U64 bb_moves = get_rook_attacks(square, board->occupancies[both]) & (~friendly_occupanices);

        while (bb_moves){
            int target = bsf(bb_moves);

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

static inline void generate_queen_moves(moveList *legalMoves, Board *board){
    int ptype = ((board->side == white) ? Q : q);
    U64 bitboard = board->bitboards[ptype];

    U64 friendly_occupanices = board->occupancies[board->side];

    while (bitboard){
        int square = bsf(bitboard);

        U64 bb_moves = (get_bishop_attacks(square, board->occupancies[both]) | get_rook_attacks(square, board->occupancies[both])) & (~friendly_occupanices);

        while (bb_moves){
            int target = bsf(bb_moves);

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

static inline void generate_king_moves(moveList *legalMoves, Board *board){
    int ptype = ((board->side == white) ? K : k);
    U64 bitboard = board->bitboards[ptype];

    U64 friendly_occupanices = board->occupancies[board->side];

    if (bitboard == 0){
        print_board(board);
    }

    int square = bsf(bitboard);

    U64 bb_moves = king_mask[square] & (~friendly_occupanices);

    while (bb_moves){
        int target = bsf(bb_moves);

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

static inline void update_occupancies(Board *board){

    memset(board->occupancies, 0ULL, sizeof(board->occupancies));

    for (int piece = P; piece <= K; piece++){
        board->occupancies[white] |= board->bitboards[piece];
    }

    for (int piece = p; piece <= k; piece++){
        board->occupancies[black] |= board->bitboards[piece];
    }

    board->occupancies[both] = (board->occupancies[black] | board->occupancies[white]);
}

void generate_moves(moveList *legalMoves, Board *board){
    legalMoves->count = 0;

    generate_knight_moves(legalMoves, board);
    generate_pawn_moves(legalMoves, board);
    generate_bishop_moves(legalMoves, board);
    generate_rook_moves(legalMoves, board);
    generate_queen_moves(legalMoves, board);
    generate_king_moves(legalMoves, board);
}

const int castling_right_table[64] = {
        7, 15, 15, 15,  3, 15, 15, 11,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        13, 15, 15, 15, 12, 15, 15, 14
};

int make_move(int move, int flag, int zobristUpdate, Board *board){

    if (flag == all_moves){
        copy_board();

        int ptype = getpiece(move);
        int source = getsource(move);
        int target = gettarget(move);
        int capture = getcapture(move);

        //if move is a king move, refresh the accumulator;
        if (ptype == K || ptype == k){
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
                    nnue_pop_bit(p, target+8, board);
                } else {
                    nnue_pop_bit(P, target-8, board);
                }
            } else {
                if (board->side == white){
                    for (int piece = p; piece <= k; piece++) {
                        if (get_bit(board->bitboards[piece], target)) {

                            nnue_pop_bit(piece, target, board);

                            break;
                        }
                    }
                } else {
                    for (int piece = P; piece <= K; piece++) {
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
                    nnue_pop_bit(R, 63, board);
                    nnue_set_bit(R, 61, board);
                } else if (target == 58){
                    nnue_pop_bit(R, 56, board);
                    nnue_set_bit(R, 59, board);
                }
            } else {
                if (target == 6){
                    nnue_pop_bit(r, 7, board);
                    nnue_set_bit(r, 5, board);
                } else if (target == 2){
                    nnue_pop_bit(r, 0, board);
                    nnue_set_bit(r, 3, board);
                }
            }
        }

        int promoted = getpromoted(move);
        if (promoted){
            nnue_pop_bit(ptype, target, board);
            nnue_set_bit(promoted, target, board);
        }

        update_occupancies(board);

        if (board->side == white){
            if (is_square_attacked(bsf(board->bitboards[K]), black, board)){
                take_back();
                return 0;
            }
        } else {
            if (is_square_attacked(bsf(board->bitboards[k]), white, board)){
                take_back();
                return 0;
            }
        }

        board->castle &= castling_right_table[source];
        board->castle &= castling_right_table[target];

        board->side ^= 1;

        if ((ptype == P) || (ptype == p) || (board_copy.castle != board->castle) || capture){
            memset(board->zobrist_history, 0, sizeof(board->zobrist_history));
            board->zobrist_history_length = 0;
        }

        //UPDATE ZOBRIST HISTORY
        if (zobristUpdate) {
            board->current_zobrist_key = update_zobrist_key(board);
            board->zobrist_history[board->zobrist_history_length] = board->current_zobrist_key;
            board->zobrist_history_length++;
        } else
            board->current_zobrist_key = 0ULL;
    } else {
        if (getcapture(move) || is_move_direct_check(move, board)){
            return make_move(move, all_moves, zobristUpdate, board);
        } else {
            return 0;
        }

    }

    board->prevmove = move;

    return 1;
}

int piece_at(int square, Board *board){
    for (int i = 0; i < 12; ++i) {
        if (board->bitboards[i] & 1ULL << square)
            return i;
    }

    return -1;
}

//Simple and fast function to judge if a move is a check.
//Does not work with discovered checks or castle-checks

int is_move_direct_check(int move, Board *board) {
    int ptype = getpiece(move);
    U64 kingbb = board->side == white ? board->bitboards[k] : board->bitboards[K];
    int kingsq = bsf(kingbb);
    int target = gettarget(move);
    U64 targetBb = 1ULL << target;

    if (ptype == N || ptype == n){
        if (knight_mask[target] & kingbb)
            return 1;
        else
            return 0;
    }

    if (ptype == P){
        if (pawn_mask[white][target] & kingbb)
            return 1;
        else
            return 0;
    }

    if (ptype == p){
        if (pawn_mask[black][target] & kingbb)
            return 1;
        else
            return 0;
    }

    if (ptype == B || ptype == b){
        if (get_bishop_attacks(kingsq, board->occupancies[both]) & targetBb)
            return 1;
        else
            return 0;
    }

    if (ptype == R || ptype == r){
        if (get_rook_attacks(kingsq, board->occupancies[both]) & targetBb)
            return 1;
        else
            return 0;
    }

    if (ptype == Q || ptype == q){
        if (get_bishop_attacks(kingsq, board->occupancies[both]) & targetBb)
            return 1;
        if (get_rook_attacks(kingsq, board->occupancies[both]) & targetBb)
            return 1;

        return 0;
    }

    return 0;
}

int is_square_attacked(int square, int testingSide, Board *board){

    if ((testingSide == white) && (pawn_mask[black][square] & board->bitboards[P])) return 1;
    if ((testingSide == black) && (pawn_mask[white][square] & board->bitboards[p])) return 1;

    if (knight_mask[square] & ((testingSide == white ? board->bitboards[N] : board->bitboards[n]))) return 1;

    if (get_bishop_attacks(square, board->occupancies[both]) & ((testingSide == white ? board->bitboards[B] : board->bitboards[b]) | (testingSide == white ? board->bitboards[Q] : board->bitboards[q]))) return 1;
    if (get_rook_attacks(square, board->occupancies[both]) & ((testingSide == white ? board->bitboards[R] : board->bitboards[r]) | (testingSide == white ? board->bitboards[Q] : board->bitboards[q]))) return 1;

    if (king_mask[square] & ((testingSide == white ? board->bitboards[K] : board->bitboards[k]))) return 1;

    return 0;
}

//
//int piece_values[12] = {10, 30, 32, 50, 100, 0, -10, -30, -32, -50, -100, 0};
//
//int get_smallest_attacker(int square, int testingSide){
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
//int static_exchange_evaluation(int move) {
//    int exchange_eval = 0;
//
//    int target = get_move_target(move);
//    int cap = piece_at(target);
//    exchange_eval -= piece_values[cap];
//
//    if (get_smallest_attacker(target, side^1) != -1)
//        exchange_eval -= piece_values[get_move_piece(move)];
//
//    return side == white ? exchange_eval + 2000 : (-exchange_eval) + 2000;
//}

int char_pieces[] = {
        ['P'] = P,
        ['N'] = N,
        ['B'] = B,
        ['R'] = R,
        ['Q'] = Q,
        ['K'] = K,
        ['p'] = p,
        ['n'] = n,

        ['b'] = b,
        ['r'] = r,
        ['q'] = q,
        ['k'] = k
};
char piece_symbols[12] = {
        'P', 'N', 'B', 'R', 'Q', 'K', 'p', 'n', 'b', 'r', 'q', 'k'
};

void print_move(int move){
    printf("%s%s", square_to_coordinates[getsource(move)], square_to_coordinates[gettarget(move)]);
    if ((getpromoted(move) == Q) | (getpromoted(move) == q)){
        printf("q");
    }
    if ((getpromoted(move) == B) | (getpromoted(move) == b)){
        printf("b");
    }
    if ((getpromoted(move) == N) | (getpromoted(move) == n)){
        printf("n");
    }
    if ((getpromoted(move) == R) | (getpromoted(move) == r)){
        printf("r");
    }
}

void print_board(Board *board){
    int rank = 8;
    for (int square = 0; square < 64; square++){

        if (!(square % 8)) {
            printf("\n");
            printf("%d | ", rank);
            rank--;
        }

        int found_piece = 0;

        for (int piece = P; piece <= k; piece++){
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
    for (int r = 0; r < 8; ++r) {
        int esq = 0;
        for (int f = 0; f < 8; ++f) {

            int p = piece_at((r * 8) + f, board);
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

    printf(board->side == white ? " w" : " b");
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

    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            int square = rank * 8 + file;

            if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z'))
            {
                int piece = char_pieces[*fen];

                set_bit(board->bitboards[piece], square);

                fen++;
            }

            if (*fen >= '0' && *fen <= '9')
            {
                int offset = *fen - '0';

                int piece = -1;

                for (int bb_piece = P; bb_piece <= k; bb_piece++)
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
        int file = fen[0] - 'a';
        int rank = 8 - (fen[1] - '0');

        board->enpessant = rank * 8 + file;
    }

    else
        board->enpessant = no_sq;

    for (int piece = P; piece <= K; piece++)
        board->occupancies[white] |= board->bitboards[piece];

    for (int piece = p; piece <= k; piece++)
        board->occupancies[black] |= board->bitboards[piece];

    update_occupancies(board);
    board->current_zobrist_key = generate_zobrist_key(board);
    board->helperThread = 0;
    board->nnueUpdate = 1;

    refresh_accumulator(&board->currentNnue, board);
}
