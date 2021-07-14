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

    //perft_test(5);
    //parse_go("go depth 9");

    uci_loop();

    return 0;
}