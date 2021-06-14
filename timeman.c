#include "timeman.h"
#include "search.h"


int startingTime;

void start_time(){
  startingTime = get_time_ms();
}

void communicate(){
  if ((get_time_ms() - startingTime) >= moveTime){
    stop = 1;
  }
}
