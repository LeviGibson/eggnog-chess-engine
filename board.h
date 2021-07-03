//
// Created by levigibson on 5/31/21.
//

#ifndef MBBCHESS_BOARD_H
#define MBBCHESS_BOARD_H

#include <string.h>

#include "bitboard.h"

#define start_position "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define kiwipete "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"
#define enpessant_position "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - "
#define tricky_position "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8  "
#define killer_position "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"

enum {P, N, B, R, Q, K, p, n, b, r, q, k};
enum{wk = 1, wq = 2, bk = 4, bq = 8};
enum {all_moves, only_captures};

typedef struct {
    int moves[256];
    U32 count;
} moveList;


//MOVE ENCODING____________
/**

 1 2 4 8 16 32

 Src    dest   pt   prom en cap
 111111 000000 1111 0000 1  0

 **/

#define encode_move(source, target, ptype, prom, enpessant, capture, doublePush, castle) \
    source | ((target) << 6) | ((ptype) << 12) | ((prom) << 16) | ((enpessant) << 20) | ((capture) << 21) | ((doublePush) << 22) | ((castle) << 23)

#define get_move_source(move) \
    (0x3f & move)

#define get_move_target(move) \
    (0xfc0 & move) >> 6

#define get_move_piece(move) \
    (0xf000 & move) >> 12

#define get_move_promoted(move) \
    (0xf0000 & move) >> 16

#define get_move_enpessant(move) \
    get_bit(move, 20)

#define get_move_capture(move) \
    get_bit(move, 21)

#define get_move_double(move) \
    get_bit(move, 22)

#define get_move_castle(move) \
    get_bit(move, 23)

#define copy_board()                                                      \
    U64 bitboards_copy[12], occupancies_copy[3], zobrist_history_copy[101];\
    int side_copy, enpassant_copy, castle_copy, zobrist_history_length_copy;\
    memcpy(zobrist_history_copy, zobrist_history, sizeof zobrist_history_copy);\
    memcpy(bitboards_copy, bitboards, sizeof bitboards_copy);                   \
    memcpy(occupancies_copy, occupancies, sizeof occupancies_copy);             \
    side_copy = side, enpassant_copy = enpessant, castle_copy = castle;    \
    zobrist_history_length_copy = zobrist_history_length                  \

#define take_back()                                                       \
    memcpy(bitboards, bitboards_copy, sizeof bitboards);              \
    memcpy(occupancies, occupancies_copy, sizeof occupancies);        \
    memcpy(zobrist_history, zobrist_history_copy, sizeof zobrist_history); \
    side = side_copy, enpessant = enpassant_copy, castle = castle_copy;   \
    zobrist_history_length = zobrist_history_length_copy                 \

void print_move(int move);

U64 generate_zobrist_key();
void init_zobrist_keys();

int is_threefold_repetition();

U64 zobrist_history[101];
int zobrist_history_length;

U64 bitboards[12];
U64 occupancies[3];

U64 current_zobrist_key;

int enpessant;
int side;
int castle;

void generate_moves(moveList *legalMoves);
int make_move(int move, int flag);

int is_square_attacked(int square, int testingSide);

void print_board();

void parse_fen(char *fen);

#endif //MBBCHESS_BOARD_H
