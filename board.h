//
// Created by levigibson on 5/31/21.
//

#ifndef MBBCHESS_BOARD_H
#define MBBCHESS_BOARD_H

typedef struct Board Board;

#include <string.h>
#include "bitboard.h"
#include "nnue/propogate.h"


#define start_position "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define kiwipete "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"
#define enpessant_position "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - "
#define tricky_position "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8  "
#define killer_position "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"

enum{wk = 1, wq = 2, bk = 4, bq = 8};
enum {all_moves, only_captures};

#define WK board->bitboards[K]
#define WN board->bitboards[N]
#define WB board->bitboards[B]
#define WR board->bitboards[R]
#define WQ board->bitboards[Q]
#define WP board->bitboards[P]

#define BK board->bitboards[k]
#define BN board->bitboards[n]
#define BB board->bitboards[b]
#define BR board->bitboards[r]
#define BQ board->bitboards[q]
#define BP board->bitboards[p]

typedef struct {
    int moves[256];
    U32 count;
} moveList;

typedef struct tagLINE {
    int length;              // Number of moves in the line.
    int moves[64];  // The line.

}   Line;

struct Board{
    U64 zobrist_history[101];
    int zobrist_history_length;
    int zobrist_history_search_index;

    U64 zobrist_key_parts[15];
    U64 zobrist_key_bitboards[12];

    U64 bitboards[12];
    U64 occupancies[3];
    U64 weaksquares[2];

    U64 current_zobrist_key;

    int nnueUpdate;
    NnueData currentNnue;

    int ply;

    int enpessant;
    int side;
    int castle;
    int prevmove;
    int searchColor;
    Line pv_line;

    int searchDepth;

    int helperThread;
};

//MOVE ENCODING____________
/**
 Moves are encoded in integers like this

 1 2 4 8 16 32

 Src    dest   pt   prom en cap
 111111 000000 1111 0000 1  0

 and can be accessed with macros below

    get_move_source(move)
    get_move_target(move)
    get_move_piece(move)
    get_move_promoted(move)
    get_move_enpessant(move)
    get_move_capture(move) Note : this acts like a boolean
    get_move_double(move) double pawn pushes
    get_move_castle(move)
 **/

#define encode_move(source, target, ptype, prom, enpessant, capture, doublePush, castle) \
    ((source) | ((target) << 6) | ((ptype) << 12) | ((prom) << 16) | ((enpessant) << 20) | ((capture) << 21) | ((doublePush) << 22) | ((castle) << 23))

#define getsource(move) \
    (0x3f & (move))

#define gettarget(move) \
    ((0xfc0 & (move)) >> 6)

#define getpiece(move) \
    ((0xf000 & (move)) >> 12)

#define getpromoted(move) \
    ((0xf0000 & (move)) >> 16)

#define getenpessant(move) \
    get_bit(move, 20)

#define getcapture(move) \
    (2097152 & (move))

#define getdouble(move) \
    get_bit(move, 22)

#define getcastle(move) \
    get_bit(move, 23)

#define copy_board() \
    Board board_copy; \
    memcpy(&board_copy, board, sizeof(Board))

#define take_back() \
    memcpy(board, &board_copy, sizeof(Board))

//#define copy_board()                                                      \
//    U64 bitboards_copy[12], occupancies_copy[3], zobrist_history_copy[zobrist_history_length], current_zobrist_key_copy, zobrist_key_parts_copy[15], zobrist_key_bitboards_copy[12];\
//    int side_copy, enpassant_copy, castle_copy, zobrist_history_length_copy, prevmove_copy;                                                                                         \
//                                                                          \
//    int16_t accumulation_copy[2][KPSIZE];                               \
//    memcpy(accumulation_copy, currentNnue.accumulation, sizeof accumulation_copy); \
//                                                                          \
//    memcpy(zobrist_history_copy, zobrist_history, sizeof zobrist_history_copy);\
//    memcpy(bitboards_copy, bitboards, sizeof bitboards_copy);                   \
//    memcpy(occupancies_copy, occupancies, sizeof occupancies_copy);       \
//    memcpy(zobrist_key_parts_copy, zobrist_key_parts, sizeof zobrist_key_parts); \
//    memcpy(zobrist_key_bitboards_copy, zobrist_key_bitboards, sizeof zobrist_key_bitboards);  \
//                                                                          \
//    side_copy = side, enpassant_copy = enpessant, castle_copy = castle;    \
//    zobrist_history_length_copy = zobrist_history_length;                 \
//    prevmove_copy = prevmove;\
//    current_zobrist_key_copy = current_zobrist_key                          \
//
//#define take_back() \
//    memcpy(currentNnue.accumulation, accumulation_copy, sizeof accumulation_copy); \
//                    \
//    memcpy(bitboards, bitboards_copy, sizeof bitboards);              \
//    memcpy(occupancies, occupancies_copy, sizeof occupancies);        \
//    memcpy(zobrist_history, zobrist_history_copy, sizeof zobrist_history);\
//                                                                          \
//    memcpy(zobrist_key_parts, zobrist_key_parts_copy, sizeof zobrist_key_parts); \
//    memcpy(zobrist_key_bitboards, zobrist_key_bitboards_copy, sizeof zobrist_key_bitboards); \
//                                                                          \
//    side = side_copy, enpessant = enpassant_copy, castle = castle_copy;   \
//    current_zobrist_key = current_zobrist_key_copy;                       \
//    prevmove = prevmove_copy;\
//    zobrist_history_length = zobrist_history_length_copy                 \

void remove_illigal_moves(moveList *moves, Board *board);
void print_move(int move);

U64 generate_zobrist_key(Board *board);
U64 update_zobrist_key(Board *board);
void init_zobrist_keys();

int is_threefold_repetition(Board *board);

void generate_moves(moveList *legalMoves, Board *board);
int make_move(int move, int flag, int notquinode, Board *board);

int is_square_attacked(int square, int testingSide, Board *board);
int is_move_direct_check(int move, Board *board);

void print_board(Board *board);
void print_fen(Board *board);

void parse_fen(char *fen, Board *board);

#endif //MBBCHESS_BOARD_H

