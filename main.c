
#include "bitboard.h"
#include "board.h"
#include "uci.h"

int main() {
    init_bitboards();
    init_zobrist_keys();

    parse_fen(start_position);

    //parse_go("go depth 10");
    uci_loop();

    return 0;
}