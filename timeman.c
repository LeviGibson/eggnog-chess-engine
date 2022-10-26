#include "timeman.h"
#include "search.h"
#include "board.h"
#include "uci.h"
#include <math.h>
#include <stdio.h>

int32_t startingTime;
int32_t useNodes = 0;

void start_time() {
    startingTime = get_time_ms();
}

//this function decides if the search should stop
//it is called continuously during the search (every 2048 nodes).
void communicate(int64_t nodes) {
    //variable UseNodes is set to 1 every time `go nodes x` is called
    if (useNodes){
        assert(!dynamicTimeManagment);
        if (nodes >= moveTime){
            printf("%d : %d\n", nodes, moveTime);
            stop = 1;
            useNodes = 0;
        }
    } else if (!dynamicTimeManagment) {
        if ((get_time_ms() - startingTime) >= moveTime) {
            //variable stop stops the search
            stop = 1;
        }
    }
}

//in a game with a chess clock, decide how much time to spend
//this time can increase or decrease during the search depending on the difficulty of the position.
int32_t choose_movetime(int32_t i_wtime, int32_t i_btime, int32_t side) {
    float wtime = (float)i_wtime;
    float btime = (float)i_btime;

    float d = 60;
    float z = 4.58f;
    float t = 1.005f;
    float s = 50;
    float a = -460;

    assert(side == white || side == black);
    assert(i_wtime || i_btime);

    if (side == white) {
        //put it into desmos and you'll get it
        float time = ((1/(1+powf(t, -((wtime/1000)+a))))*s)-z+(wtime/1000/d);
        time *= 1000;
        return (int32_t )max((time - (float )moveOverhead), 75.f);
    } else {
        float time = ((1/(1+powf(t, -((btime/1000)+a))))*s)-z+(btime/1000/d);
        time *= 1000;
        return (int32_t )max((time - (float )moveOverhead), 75.f);
    }
}
