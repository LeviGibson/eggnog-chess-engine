//
// Created by levigibson on 6/2/21.
//

#include <stdio.h>
#include "perft.h"
#include "timeman.h"

long perft_nodes = 0;

static inline void perft_driver(int32_t depth, Board *board){

  if (depth == 0){
    //print_board();
    perft_nodes++;
    return;
  }

  MoveList legalMoves;
  legalMoves.count = 0;

  generate_moves(&legalMoves, board);

  copy_board();
  for (int32_t moveId = 0; moveId < legalMoves.count; moveId++){
    if (make_move(legalMoves.moves[moveId], all_moves, 0, board)){

      perft_driver(depth-1, board);

      take_back();

    }
  }

}

void perft_test(int32_t depth, Board *board){

  int32_t startingTime = get_time_ms();

  MoveList legalMoves;
  legalMoves.count = 0;

  generate_moves(&legalMoves, board);

  copy_board();

  int32_t allNodes = 0;

  for (int32_t moveId = 0; moveId < legalMoves.count; moveId++) {
    if (make_move(legalMoves.moves[moveId], all_moves, 0, board)){
      perft_nodes = 0;
      perft_driver(depth-1, board);

      print_move(legalMoves.moves[moveId]);
      printf(": %ld\n", perft_nodes);

      allNodes += perft_nodes;

      take_back();
    }
  }

  printf("\nnodes : %d\n", allNodes);
  printf("finished in %d ms\n\n", get_time_ms() - startingTime);

}
