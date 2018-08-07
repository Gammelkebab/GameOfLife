#ifndef timing_h
#define timing_h

#include <time.h>
#include <sys/time.h>
#include <stdio.h>

inline void start_timer(struct timeval *begin)
{
    gettimeofday(begin, NULL);
}

inline void print_time_since(struct timeval *begin)
{
    struct timeval end;

    gettimeofday(&end, NULL);
    double elapsed = (end.tv_sec - begin->tv_sec) + ((end.tv_usec - begin->tv_usec) / 1000000.0);
    printf("Elapsed: %.5fs\n", elapsed);
}

#endif