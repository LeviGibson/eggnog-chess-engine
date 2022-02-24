#ifndef MBBCHESS_SEARCH_H
#define MBBCHESS_SEARCH_H

#define MAX_PLY 64
#include "board.h"

void search_init();
void *search_position(void *arg);
int32_t stop;

#endif
