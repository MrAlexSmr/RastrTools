/**
 * Author: Oleksandr Smorchkov
 *
 * Custom standart library implementation (polyfill) for WebAssembly
 * Implemented only needed/required methods/types
 */

#ifndef LIB_H
#define LIB_H

#include "config.h"

#ifdef WASM32

#ifdef NULL
#undef NULL
#endif /* NULL */

#ifdef INFINITY
#undef INFINITY
#endif /* INFINITY */

#ifdef NAN
#undef NAN
#endif /* NAN */

#define NULL __get_heap_base__()
#define INFINITY (1.0f / 0.0f)
#define NAN (INFINITY / INFINITY)

/* Full version may be token here: https://git.musl-libc.org/cgit/musl/tree/src/math/fmaf.c */
#define fmaf(x, y, z) ((x) * (y) + (z))

typedef char int8_t;
typedef unsigned char uint8_t;

typedef short int16_t;
typedef unsigned short uint16_t;

typedef int int32_t;
typedef unsigned int uint32_t;

typedef long long int64_t;
typedef unsigned long long uint64_t;


EXTERN void* ATTR __get_heap_base__();


EXTERN void* ATTR malloc(uint32_t amount);
EXTERN void ATTR free(void *ptr);


EXTERN float ATTR sinf(float number);
EXTERN float ATTR cosf(float number);
EXTERN float ATTR sqrtf(float number);
EXTERN float ATTR ldexpf(float a, int i);
EXTERN float ATTR expf(float a);
EXTERN float ATTR logf(float a);
EXTERN float ATTR powf(float x, float y);
/*EXTERN float ATTR ceilf(const float number);*/


INLINE const float fabs(const float number)
{
	/* Ignores NaN and negative sero */
	return (number < 0.0f ? -number : number);
}

INLINE const int32_t abs(const int32_t number)
{
	return (number < 0 ? -number : number);
}

#else /* WASM32 */

/* Usual envirment, (not tested) */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <float.h>
#include <math.h>

#endif /* WASM32 */


#endif /*LIB_H*/
