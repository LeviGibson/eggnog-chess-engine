#include <stdio.h>
#include "bitboard.h"
#include "board.h"
#include "uci.h"
#include "transposition.h"
#include "nnue.h"
#include "see.h"
#include "nnom.h"

int main(int argc, char* argv[]) {
    change_to_correct_directory(argv[0]);

    init_bitboards();
    init_zobrist_keys();
    reinit_transposition();
    init_see();

    load_model("nn-eba324f53044.nnue");
    load_nnom("network.nnom");
    parse_fen(start_position, &UciBoard);

    uci_loop();

    transposition_free();

    return 0;
}