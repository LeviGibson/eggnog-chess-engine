
#include "bitboard.h"
#include "board.h"
#include "uci.h"

int main() {
    init_bitboards();
    init_zobrist_keys();

    uci_loop();

    return 0;
}