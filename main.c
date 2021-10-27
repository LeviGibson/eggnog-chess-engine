#include "bitboard.h"
#include "board.h"
#include "uci.h"
#include "moveOrder.h"

#include "nnue/load.h"

int main() {
    init_bitboards();
    init_zobrist_keys();
    init_move_table();

    load_model("nnue/defualt-nn.nnue");

    parse_fen(start_position);
    uci_loop();

    return 0;
}