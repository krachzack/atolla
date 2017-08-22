#ifndef MSG_MEM_UINT16LE_H
#define MSG_MEM_UINT16LE_H

#include "atolla/config.h"
#include "atolla/primitives.h"

#ifdef NATIVE_BIG_ENDIAN
    // REVIEW what if input is wider than 16bit? does
    #define mem_uint16le_to(in)   ((uint16_t) (((in) << 8) | (((in) & 0xFFFF) >> 8)) )
    #define mem_uint16le_from(in) ((uint16_t) (((in) << 8) | (((in) & 0xFFFF) >> 8)) )
#else
    #define mem_uint16le_to(in)   ((uint16_t) (in))
    #define mem_uint16le_from(in) ((uint16_t) (in))
#endif

#endif // MSG_MEM_UINT16LE_H
