#ifndef MBBCHESS_SEARCH_H
#define MBBCHESS_SEARCH_H



#define max_ply 64
#define U16 uint16_t

typedef struct tagLINE {
   int length;              // Number of moves in the line.
   int moves[64];  // The line.

}   Line;

void search_position(int depth);
int stop;

#endif
