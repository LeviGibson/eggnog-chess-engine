//
// Created by levigibson on 6/2/21.
//

#include "perft.h"
#include <stdio.h>
#include "board.h"
#include "timeman.h"

long perft_nodes = 0;

static inline void perft_driver(int depth){

  if (depth == 0){
    //print_board();
    perft_nodes++;
    return;
  }

  moveList legalMoves;
  legalMoves.count = 0;

  generate_moves(&legalMoves);

  copy_board();
  for (int moveId = 0; moveId < legalMoves.count; moveId++){
    if (make_move(legalMoves.moves[moveId], all_moves, 0)){

      perft_driver(depth-1);

      take_back();

    }
  }

}

void perft_test(int depth){

  int startingTime = get_time_ms();

  moveList legalMoves;
  legalMoves.count = 0;

  generate_moves(&legalMoves);

  copy_board();

  int allNodes = 0;

  for (int moveId = 0; moveId < legalMoves.count; moveId++) {
    if (make_move(legalMoves.moves[moveId], all_moves, 0)){
      perft_nodes = 0;
      perft_driver(depth-1);

      print_move(legalMoves.moves[moveId]);
      printf(": %ld\n", perft_nodes);

      allNodes += perft_nodes;

      take_back();
    }
  }

  printf("\nnodes : %d\n", allNodes);
  printf("finished in %d ms\n\n", get_time_ms() - startingTime);

}
