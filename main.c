#include "bitboard.h"
#include "board.h"
#include "uci.h"

#include "nnue/probe/load.h"
#include "nnue/probe/propogate.h"

int main() {
    init_bitboards();
    init_zobrist_keys();

    parse_fen(start_position);

    load_model("nnue/probe/model");

    uci_loop();

    return 0;
}