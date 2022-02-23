#include "timeman.h"
#include "search.h"
#include "board.h"
#include "uci.h"
#include <math.h>
#include <stdio.h>

int32_t startingTime;

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

int32_t choose_movetime(int32_t i_wtime, int32_t i_btime, int32_t i_winc, int32_t i_binc, int32_t side) {
    float wtime = (float)i_wtime;
    float btime = (float)i_btime;

    float winc = (float)i_winc;
    float binc = (float)i_binc;

    float d = 60;
    float z = 4.6f;
    float t = 1.005f;
    float s = 50;
    float a = -460;

    if (side == white) {
        float time = ((1/(1+powf(t, -((wtime/1000)+a))))*s)-z+(wtime/1000/d);
        time *= 1000;
        return (int32_t )max((time - (float )moveOverhead + (winc / 2)), 75.f);
    } else {
        float time = ((1/(1+powf(t, -((btime/1000)+a))))*s)-z+(btime/1000/d);
        time *= 1000;
        return (int32_t )max((time - (float )moveOverhead + (binc / 2)), 75.f);
    }
}
