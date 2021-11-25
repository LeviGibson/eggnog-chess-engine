#include "bitboard.h"
#include "board.h"
#include "uci.h"
#include "moveOrder.h"
#include "transposition.h"
#include <pthread.h>
#include <stdio.h>

#include "nnue/load.h"

int main() {
    init_bitboards();
    init_zobrist_keys();
    init_move_table();
    init_transposition();

    load_model("nn-eba324f53044.nnue");
    parse_fen(start_position, &UciBoard);

    uci_loop();

    transposition_free();

    return 0;
}