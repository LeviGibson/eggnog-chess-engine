#ifndef MBBCHESS_TIMEMAN_H
#define MBBCHESS_TIMEMAN_H

#include <stddef.h>
#include "bitboard.h"
#include <immintrin.h>

#ifdef WIN64
#include <sysinfoapi.h>
#else
#include <sys/time.h>
#endif

static inline int32_t get_time_ms() {
#ifdef WIN64
    return GetTickCount();
#else
    struct timeval time_value;
    gettimeofday(&time_value, NULL);
    return time_value.tv_sec * 1000 + time_value.tv_usec / 1000;
#endif
}

int32_t moveTime;

int32_t startingTime;

void start_time();

void communicate();

int32_t choose_movetime(int32_t wtime, int32_t btime, int32_t side);

#endif
