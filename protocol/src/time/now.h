#ifndef TIME_NOW_H
#define TIME_NOW_H

/**
 * Gets the current time as the difference in milliseconds between the instant
 * of calling this function and the instant of a fixed but arbitrary origin time,
 * which may even be negative.
 */
int time_now();

#endif // TIME_NOW_H
