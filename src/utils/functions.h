/** @file functions.h
 *  @brief Various functions
 *
 *  @author Tobias Waurick
 */
#ifndef KERNELMODULE_PCM3060_UTILS_FUNCTIONS_H
#define KERNELMODULE_PCM3060_UTILS_FUNCTIONS_H

/* MATH */
#define HALF(x) ((x)/(2))

/* OPERATORS */
#define CYCLE_SHIFT(x, shift) (x << shift) | (x >> (sizeof(x)*8 - shift))

/* UNITS */
#define HZ_TO_NS(f) ((1000000000)/(f))

#endif // !KERNELMODULE_PCM3060_UTILS_FUNCTIONS_H