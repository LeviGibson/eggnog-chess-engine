#include <stdio.h>
#include <string.h>

#include "transposition.h"

#define hash_size 1000000

entry transposition_table[hash_size];

void reset_transposition_table(){
  memset(&transposition_table, 0, sizeof transposition_table);
}

void write_hash(U64 key, int score, int flag, int depth, Line *line, int best_move){
  int index = key % hash_size;
  if (depth > transposition_table[index].depth){
    entry e;
    e.key = key;
    e.flag = flag;
    e.score = score;
    e.depth = depth;
    e.move = best_move;

    transposition_table[index] = e;
    memcpy(&transposition_table[index].line, line, sizeof(*line));
  }
}

int hash_lookup(U64 key, int beta, int depth, Line *pline){
  entry *found_entry = &transposition_table[key%hash_size];
  if ((found_entry->key == 0) || (found_entry->key != key) || (found_entry->depth < depth))
    return no_entry;

  if (found_entry->flag == hash_flag_alpha){

    //principle variation update :)
    pline->moves[0] = found_entry->move;
    memcpy(pline->moves + 1, found_entry->line.moves, found_entry->line.length * 4);
    pline->length = found_entry->line.length + 1;

    return found_entry->score;
  }

  if (found_entry->flag == hash_flag_beta){
    return found_entry->score;
  }

  return no_entry;
}
