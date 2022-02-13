#include "timeman.h"
#include "search.h"
#include "board.h"
#include "uci.h"
#include <math.h>
#include <stdio.h>

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

int choose_movetime(int i_wtime, int i_btime, int i_winc, int i_binc, int side) {
    float wtime = (float)i_wtime;
    float btime = (float)i_btime;

    float winc = (float)i_winc;
    float binc = (float)i_binc;

    if (side == white) {
        float time = (powf((wtime/1000), 1.2f) * 1000)/30;
        return (int )(max(time, 15) - (float )moveOverhead + (winc / 2));
    } else {
        float time = (powf((btime/1000), 1.2f) * 1000)/30;
        return (int )(max(time, 15) - (float )moveOverhead + (binc / 2));
    }
}