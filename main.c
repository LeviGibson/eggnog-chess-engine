#include <stdio.h>
#include <string.h>

#include "bitboard.h"
#include "board.h"
#include "uci.h"
#include "perft.h"
#include "Fathom/tbprobe.h"
#include "syzygy.h"

int main() {
    init_bitboards();
    init_zobrist_keys();
    tb_init("/home/levigibson/Documents/static/arenalinux_64bit_3.10beta/TB/syzygy");

    parse_fen(kiwipete);

    uci_loop();
    //parse_go("go depth 9");

    return 0;
}