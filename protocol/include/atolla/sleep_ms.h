#ifndef ATOLLA_SLEEP_MS_H
#define ATOLLA_SLEEP_MS_H

#include "atolla/config.h"

#if defined(HAVE_POSIX_SLEEP)
    #include <unistd.h>
    #define sleep_ms(ms) (usleep((ms) * 1000))
#elif defined(HAVE_WINDOWS_SLEEP)
    #include <windows.h>
    #define sleep_ms(ms) (Sleep((ms)))
#else
    #error "No sleep function available"
#endif

#endif // ATOLLA_SLEEP_MS_H
