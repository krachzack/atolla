#ifndef TIME_GETTIME_H
#define TIME_GETTIME_H

#ifdef __MACH__
#include "time/mach_gettime.h"
#else
#include <time.h>
#endif

#endif // TIME_GETTIME_H
