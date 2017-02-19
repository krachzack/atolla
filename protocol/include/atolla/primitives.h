/**
 * Contains includes for basic types in atolla, such as uint8_t
 */

#ifndef ATOLLA_PRIMITIVES_H
#define ATOLLA_PRIMITIVES_H

#include "atolla/config.h"

#if defined(HAVE_UINT8_T) && defined(HAVE_UINT16_T)
#include <stdint.h>  // for uint8_t
#else
#error No 8 and 16 bit unsigned integer types available
#endif

#ifdef HAVE_BOOL
#include <stdbool.h> // for bool
#else
#error No boolean type available
#endif

#ifdef HAVE_SIZE_T
#include <stddef.h>  // for size_t
#else
#error No native address offset integer type available
#endif

#endif // ATOLLA_PRIMITIVES_H
