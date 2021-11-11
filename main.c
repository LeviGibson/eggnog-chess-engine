#include "bitboard.h"
#include "board.h"
#include "uci.h"
#include "moveOrder.h"

#include "nnue/load.h"

int main() {
    init_bitboards();
    init_zobrist_keys();
    init_move_table();

    load_model("nn-eba324f53044.nnue");

    parse_fen(start_position);
    uci_loop();

    return 0;
}