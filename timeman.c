#include "timeman.h"
#include "search.h"
#include "board.h"
#include "uci.h"

int startingTime;

void start_time(){
  startingTime = get_time_ms();
}

void communicate(){
  if ((get_time_ms() - startingTime) >= moveTime){
    stop = 1;
  }
}

int choose_movetime(int wtime, int btime){
    if (side == white){
        return wtime / (100-uci_move_sequence_length);
    } else {
        return btime / (100-uci_move_sequence_length);
    }
}