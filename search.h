#ifndef MBBCHESS_SEARCH_H
#define MBBCHESS_SEARCH_H

#define max_ply 64
#include "board.h"

//macro "max" is defined in minwindef.h when compiling with mingw
#ifndef max
#define max(x, y) ((y) > (x) ? (y) : (x))
#endif

void *search_position(void *arg);
int stop;

#endif
