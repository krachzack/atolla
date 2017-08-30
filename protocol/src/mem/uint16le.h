#ifndef MSG_MEM_UINT16LE_H
#define MSG_MEM_UINT16LE_H

#ifndef HAVE_ARDUINO_WIFI_UDP
// Don't use config file when compiling for arduino
#include "atolla/config.h"
#endif

#include "atolla/primitives.h"

#define IS_BIG_ENDIAN (*(uint16_t *)"\0\xff" < 0x100)

// Set either NATIVE_BIG_ENDIAN or NATIVE_LITTLE_ENDIAN if you are sure,
// otherwise the code will autodetect

#ifdef NATIVE_BIG_ENDIAN
    // REVIEW what if input is wider than 16bit? does
    #define mem_uint16le_to(in)   ((uint16_t) (((in) << 8) | (((in) & 0xFFFF) >> 8)) )
    #define mem_uint16le_from(in) ((uint16_t) (((in) << 8) | (((in) & 0xFFFF) >> 8)) )

#elif NATIVE_LITTLE_ENDIAN
    #define mem_uint16le_to(in)   ((uint16_t) (in))
    #define mem_uint16le_from(in) ((uint16_t) (in))
#else
    #define mem_uint16le_to(in)  ( (IS_BIG_ENDIAN) ? ((uint16_t) (((in) << 8) | (((in) & 0xFFFF) >> 8)) ) : (in) )
    #define mem_uint16le_from(in)  ( (IS_BIG_ENDIAN) ? ((uint16_t) (((in) << 8) | (((in) & 0xFFFF) >> 8)) ) : (in) )
#endif

#endif // MSG_MEM_UINT16LE_H
