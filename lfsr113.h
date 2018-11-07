#ifndef lfsr113_h
#define lfsr113_h

#include <stdint.h>

/**
 * Pseudo-random number generator (RNG).  Returns a value in [0,UINT32_MAX].
 */
uint32_t lfsr113(void);

#endif
