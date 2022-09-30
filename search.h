#ifndef MBBCHESS_SEARCH_H
#define MBBCHESS_SEARCH_H

#include "board.h"

#ifndef WASM
void *search_position(void *arg);
#else
int32_t search_position(int32_t depth);
#endif

int32_t stop;

#endif
