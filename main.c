#include "bitboard.h"
#include "board.h"
#include "uci.h"
#include "moveOrder.h"

#include "nnue/load.h"

int main() {
    init_bitboards();
    init_zobrist_keys();
    init_move_table();

    load_model("defualt-nn.nnue");

    parse_fen(start_position);
    uci_loop();
    //parse_go("go depth 9");

    return 0;
}