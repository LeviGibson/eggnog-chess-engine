#ifndef MBBCHESS_SEARCH_H
#define MBBCHESS_SEARCH_H

#define max_ply 64

//macro "max" is defined in minwindef.h when compiling with mingw
#ifndef max
#define max(x, y) (y) > (x) ? (y) : (x)
#endif

typedef struct tagLINE {
   int length;              // Number of moves in the line.
   int moves[64];  // The line.

}   Line;

void *search_position(void *arg);
int stop;

#endif
