#include <unistd.h>
#include <time.h>
#include <stdbool.h>

#include "rttimer.h"


static bool STOP = false;


static long Sleep_For(struct timespec start, struct timespec end, long interval_us);


void Start_Job(void (*func_ptr)(void), long interval_us) 
{
    long sleep;
    struct timespec start, end;
    STOP = false;
    while (1)
    {
        clock_gettime(CLOCK_MONOTONIC, &start);
        func_ptr();
        clock_gettime(CLOCK_MONOTONIC, &end);

        sleep = Sleep_For(start, end, interval_us);

        usleep(sleep);

        if (STOP) break;
    }
    
}


void Stop_Job(void)
{
    STOP = true;
}


static long Sleep_For(struct timespec start, struct timespec end, long interval_us)
{
    long time_taken, sleep;
    time_taken = (end.tv_sec - start.tv_sec) * 1000000 +
                 (end.tv_nsec - start.tv_nsec) / 1000;
    

    sleep = interval_us - time_taken - 50;
    sleep = sleep < 0 ? 0 : sleep;
    
    return sleep;    
}
