#include "time/now.h"
#include "time/gettime.h"

int time_now()
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    return ts.tv_sec * 1000 +
           ts.tv_nsec / 1000000;
}