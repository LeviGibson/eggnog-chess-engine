#include <stdio.h>
#include "bitboard.h"
#include "board.h"
#include "uci.h"
#include "transposition.h"
#include "nnue.h"
#include "see.h"
#include "nnom.h"
#include "timeman.h"

int main(int argc, char* argv[]) {
    #ifndef WASM
    change_to_correct_directory(argv[0]);
    #endif

    init_bitboards();
    init_zobrist_keys();
    reinit_transposition();
    init_see();

    load_nnue("./bin/network.nnue");
    load_nnom("./bin/network.nnom");
    parse_fen(start_position, &UciBoard);

    printf("%d\n", nnue_evaluate(&UciBoard));
    
    transposition_free();

    return 0;
}