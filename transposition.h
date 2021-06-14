#ifndef MBBCHESS_TRANSPOSITION_H
#define MBBCHESS_TRANSPOSITION_H

#include "bitboard.h"
#include "search.h"

//FLAG ENUMS
enum {hash_flag_alpha, hash_flag_beta, no_entry};

typedef struct HASHENTRY {
  U64 key;
  int score;
  int flag;
  int depth;
  int move;
  Line line;
} entry;

void reset_transposition_table();
void write_hash(U64 key, int score, int flag, int depth, Line *line, int best_move);
int hash_lookup(U64 key, int beta, int depth, Line *pline);

#endif
