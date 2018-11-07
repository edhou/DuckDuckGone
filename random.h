/**
 * Provides a method to simulate the random arrival of events that have
 * average arrival rate.  Since Cortex-M3 doesn't support floating-point
 * arithmetic, next arrival times are represented in 32-bit fixed-point
 * with 16 bits before the decimal point.
 *
 * Andrew Morton, 2018
 */
#ifndef random_h
#define random_h

#include <stdint.h>
#include "lfsr113.h"

/**
 * Generate seconds to next event in fixed-point representation.
 */
uint32_t next_event(void);

#endif
