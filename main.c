#include <stdio.h>
#include <string.h>

#include "bitboard.h"
#include "board.h"
#include "uci.h"
#include "transposition.h"

int main() {
    init_bitboards();
    init_zobrist_keys();

    parse_fen(kiwipete);

    uci_loop();

    return 0;
}
