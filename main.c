#include <stdio.h>
#include <string.h>

#include "bitboard.h"
#include "board.h"
#include "uci.h"
#include "transposition.h"
#include "perft.h"
#include "timeman.h"

int main() {
    init_bitboards();
    init_zobrist_keys();

    parse_fen(start_position);

    uci_loop();

    /**
    int startingtime = get_time_ms();
    parse_go("go depth 10");
    printf("MS: %d\n", get_time_ms() - startingtime);**/

    return 0;
}
