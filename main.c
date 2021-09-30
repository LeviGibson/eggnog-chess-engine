#include <stdio.h>
#include "bitboard.h"
#include "board.h"
#include "uci.h"

#include "nnue/probe/load.h"
#include "nnue/probe/propogate.h"

int main() {
    init_bitboards();
    init_zobrist_keys();

    parse_fen(tricky_position);
    load_model("nnue/probe/model");

    parse_go("go depth 2");

    //uci_loop();

    return 0;
}