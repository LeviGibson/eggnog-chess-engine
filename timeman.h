#ifndef MBBCHESS_TIMEMAN_H
#define MBBCHESS_TIMEMAN_H

#include <immintrin.h>

static inline int get_time_ms()
{
  #ifdef WIN64
      return GetTickCount();
  #else
      #include <sys/time.h>
        struct timeval time_value;
        gettimeofday(&time_value, NULL);
      return time_value.tv_sec * 1000 + time_value.tv_usec / 1000;
  #endif
}

int moveTime;

void start_time();
void communicate();

#endif
