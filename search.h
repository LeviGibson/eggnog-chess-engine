#ifndef MBBCHESS_SEARCH_H
#define MBBCHESS_SEARCH_H

#define max_ply 64
#include "board.h"

void *search_position(void *arg);
int stop;

#endif
