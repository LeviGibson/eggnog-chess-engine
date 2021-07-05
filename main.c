#include <stdio.h>
#include <string.h>

#include "bitboard.h"
#include "board.h"
#include "uci.h"
#include "transposition.h"
#include "perft.h"
#include "timeman.h"

int main() {
    init_bitboards();
    init_zobrist_keys();

    parse_fen(start_position);

    uci_loop();

    return 0;
}