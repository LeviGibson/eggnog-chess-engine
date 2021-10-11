#include <stdio.h>
#include "bitboard.h"
#include "board.h"
#include "uci.h"

#include "halfkp-probe/load.h"
#include "halfkp-probe/propogate.h"

int main() {
    init_bitboards();
    init_zobrist_keys();

    load_model("halfkp-probe/bbcnn.nnue");

    parse_fen(start_position);

    parse_go("go movetime 5000");
    //uci_loop();

    return 0;
}