#include "time/now.h"

#ifdef HAVE_ARDUINO_SLEEP
#include <Arduino.h>
#else
#include "time/gettime.h"
#endif

unsigned int time_now()
{
#ifdef HAVE_ARDUINO_SLEEP
    return millis();
#else
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    return ts.tv_sec * 1000 +
           ts.tv_nsec / 1000000;
#endif
}