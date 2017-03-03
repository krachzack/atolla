#ifndef TIME_SLEEP_H
#define TIME_SLEEP_H

#include "atolla/config.h"

#if defined(HAVE_POSIX_SLEEP)
    #include <unistd.h>
    #define time_sleep(ms) (usleep((ms) * 1000))
#elif defined(HAVE_WINDOWS_SLEEP)
    #include <windows.h>
    #define time_sleep(ms) (Sleep((ms)))
#else
    #error "No sleep function available"
#endif

#endif // TIME_SLEEP_H
