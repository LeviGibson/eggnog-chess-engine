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

    float d = 70;
    float z = 1.3f;
    float t = 1.007f;
    float s = 43;
    float a = -500;

    if (side == white) {
        float time = ((1/(1+powf(t, -((wtime/1000)+a))))*s)-z+(wtime/1000/d);
        time *= 1000;
        return (int )max((time - (float )moveOverhead + (winc / 2)), 75.f);
    } else {
        float time = ((1/(1+powf(t, -((btime/1000)+a))))*s)-z+(btime/1000/d);
        time *= 1000;
        return (int )max((time - (float )moveOverhead + (binc / 2)), 75.f);
    }
}
