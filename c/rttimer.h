#ifndef RT_TIMER
#define RT_TIMER


/**
 * @brief This function repeatedly calls a specified function
 *        at a specified time interval.
 * 
 * @param job: A function to be called.
 * 
 * @param interval_us: Timer interval in microseconds.
 */
void Start_Job(void (*job)(void), long interval_us);


/**
 * @brief Stops the timer.
 */
void Stop_Job(void);


#endif // RT_TIMER