#include <stdio.h>
#include "bitboard.h"
#include "board.h"
#include "uci.h"
#include "moveOrder.h"
#include "transposition.h"
#include "nnue.h"
#include "see.h"
#include "nnom.h"

int main(int argc, char* argv[]) {
    change_to_correct_directory(argv[0]);

    init_bitboards();
    init_zobrist_keys();
    init_move_table();
    reinit_transposition();
    init_see();
    search_init();

    load_model("nn-eba324f53044.nnue");
    load_nnom("network.nnom");
    parse_fen(start_position, &UciBoard);

    generate_nnom_indicies(&UciBoard);
    nnom_propogate_l1(&UciBoard);

    uci_loop();

    transposition_free();

    return 0;
}