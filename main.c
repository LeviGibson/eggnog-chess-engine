#include <stdio.h>
#include <string.h>

#include "bitboard.h"
#include "board.h"
#include "uci.h"
#include "perft.h"

int main() {
    init_bitboards();
    init_zobrist_keys();

    parse_fen(start_position);

    //perft_test(4);
    //parse_go("go depth 8");

    uci_loop();

    return 0;
}