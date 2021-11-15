#include "timeman.h"
#include "search.h"
#include "board.h"
#include "uci.h"

int startingTime;

void start_time() {
    startingTime = get_time_ms();
}

void communicate() {
    if (!dynamicTimeManagment) {
        if ((get_time_ms() - startingTime) >= moveTime) {
            stop = 1;
        }
    }
}

int choose_movetime(int wtime, int btime, int side) {

    if (side == white) {
        return wtime / 15;
    } else {
        return btime / 15;
    }
}