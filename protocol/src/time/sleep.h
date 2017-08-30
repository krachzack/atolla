#ifndef TIME_SLEEP_H
#define TIME_SLEEP_H

#ifndef HAVE_ARDUINO_SLEEP
// Don't use config file when compiling for arduino
#include "atolla/config.h"
#endif

#if defined(HAVE_POSIX_SLEEP)
    #include <unistd.h>
    #define time_sleep(ms) (usleep((ms) * 1000))
#elif defined(HAVE_WINDOWS_SLEEP)
    #include <windows.h>
    #define time_sleep(ms) (Sleep((ms)))
#else
    #include <Arduino.h>
    #define time_sleep(ms) (delay((ms))
#endif

#endif // TIME_SLEEP_H
