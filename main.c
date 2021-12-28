#include <stdio.h>
#include "bitboard.h"
#include "board.h"
#include "uci.h"
#include "moveOrder.h"
#include "transposition.h"

#include "nnue/load.h"

/*
 position startpos moves d2d4 d7d5 g1f3 g8f6 c2c4 e7e6 c4d5 e6d5 c1g5 c7c6 b1c3
 */

int main(int argc, char* argv[]) {
    change_to_correct_directory(argv[0]);

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