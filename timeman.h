#ifndef MBBCHESS_TIMEMAN_H
#define MBBCHESS_TIMEMAN_H

#include <stddef.h>
#include <immintrin.h>

#ifndef WIN64
#include <sys/time.h>
#endif

static inline int get_time_ms()
{
  #ifdef WIN64
      return GetTickCount();
  #else
        struct timeval time_value;
        gettimeofday(&time_value, NULL);
      return time_value.tv_sec * 1000 + time_value.tv_usec / 1000;
  #endif
}

int moveTime;

int startingTime;

void start_time();
void communicate();

int choose_movetime(int wtime, int btime);

#endif
