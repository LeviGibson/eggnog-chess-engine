#include <stdio.h>
#include <string.h>

#include "bitboard.h"
#include "board.h"
#include "uci.h"

U64 file(int f){
    U64 bb = 0ULL;
    for (int j = 0; j < 8; ++j) {
        bb ^= 1ULL << ((8*j) + f);
    }
    return bb;
}

int main() {
    init_bitboards();
    init_zobrist_keys();

    parse_fen(kiwipete);

    uci_loop();
    return 0;
}