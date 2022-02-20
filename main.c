#include <stdio.h>
#include "bitboard.h"
#include "board.h"
#include "uci.h"
#include "moveOrder.h"
#include "transposition.h"
#include "nnue/nnue.h"

int main(int argc, char* argv[]) {
    change_to_correct_directory(argv[0]);

    init_bitboards();
    init_zobrist_keys();
    init_move_table();
    init_transposition();

    load_model("nn-eba324f53044.nnue");
    parse_fen(kiwipete, &UciBoard);

    uci_loop();

    transposition_free();

    return 0;
}