/**
 * Author: Oleksandr Smorchkov (with help of stackoverflow and other open sources)
 */

#include "stdlib.h"

#ifdef WASM32

static uint32_t alloc_addr;


typedef union FLOAT_BITS
{
	int32_t i;
	float f;
} FLOAT_BITS;


extern uint32_t __heap_base;
void* __get_heap_base__()
{
	return &__heap_base;
}

void* malloc(uint32_t amount)
{
	/* Dummy one-way memory "allocator" */
	uint32_t offset = alloc_addr;
	alloc_addr += amount;
	return (void*)offset;
}

/* Call first before any malloc to "init" internal state */
void free(void *ptr)
{
	/* Dummy dealocator, ignores ptr address and resets all allocated memory,
		alloc_addr starts from __heap_base + 4 to reserve space for NULL address */
	alloc_addr = sizeof(uint32_t) + (uint32_t)__get_heap_base__();
}

/*
 https://docs.oracle.com/cd/E19957-01/806-3568/
 https://codereview.stackexchange.com/a/5562
 */
float sinf(float number)
{
	/* useful to pre-calculate */
	float x2 = number*number;
	float x4 = x2*x2;

	/* 	Calculate the terms
		As long as abs(number) < sqrtf(6), which is 2.45, all terms will be positive.
		Values outside this range should be reduced to [-pi/2, pi/2] anyway for accuracy.
		Some care has to be given to the factorials.
		They can be pre-calculated by the compiler,
		but the value for the higher ones will exceed the storage capacity of int.
		so force the compiler to use unsigned long longs (if available) or doubles. */
	float t1 = number * (1.0f - x2 / (2.0f*3.0f));
	float x5 = number * x4;
	float t2 = x5 * (1.0f - x2 / (6.0f*7.0f)) / 120.0f;
	float x9 = x5 * x4;
	float t3 = x9 * (1.0f - x2 / (10.0f*11.0f)) / 362880.0f;
	float x13 = x9 * x4;
	float t4 = x13 * (1.0f - x2 / (14.0f*15.0f)) / (362880.0f*10.0f*11.0f*12.0f*13.0f);

	/* Sum backwards */
	float result = t4;
	result += t3;
	result += t2;
	result += t1;

	return result;
}

float cosf(float number)
{
	return sinf(1.57079632679f - number);
}

/* http://ilab.usc.edu/wiki/index.php/Fast_Square_Root */
/* https://www.codeproject.com/Articles/69941/Best-Square-Root-Method-Algorithm-Function-Precisi */
float sqrtf(float x)
{
	FLOAT_BITS u;

	if (x < 0.0f) return -NAN;

	u.f = x;
	u.i = (1<<29) + (u.i >> 1) - (1<<22);

	/* Two Babylonian Steps (simplified from:)
	u.x = 0.5f * (u.x + x/u.x);
	u.x = 0.5f * (u.x + x/u.x); */
	u.f =       u.f + x/u.f;
	u.f = 0.25f*u.f + x/u.f;

	return u.f;
}


// https://stackoverflow.com/a/39601967
float ldexpf(float a, int i)
{
	FLOAT_BITS u;
	u.f = a; // scale by 2**i
	int ia = (i << 23) + u.i;
	u.i = ia;
	a = u.f;
	if ((unsigned int)(i + 125) > 250) { // |i| > 125
		i = (i ^ (125 << 23)) - i; // ((i < 0) ? -125 : 125) << 23
		u.i = ia - i; // scale by 2**(+/-125)
		a = u.f;
		u.i = (127 << 23) + i; // scale by 2**(+/-(i%125)) 
		a = a * u.f;
	}
	return a;
}

// originally called "my_expf_unchecked"
// https://stackoverflow.com/a/40519989
float expf(float a)
{
	float f, r;
	int32_t i;

	// exp(a) = exp(i + f); i = rint (a / log(2))
	r = fmaf(0x1.715476p0f, a, 0x1.8p23f) - 0x1.8p23f; // 1.442695, 12582912.0
	f = fmaf(r, -0x1.62e400p-01f, a); // log_2_hi // -6.93145752e-1
	f = fmaf(r, -0x1.7f7d1cp-20f, f); // log_2_lo // -1.42860677e-6
	i = (int32_t)r;
	// approximate r = exp(f) on interval [-log(2)/2,+log(2)/2]
	r =            0x1.694000p-10f; // 1.37805939e-3
	r = fmaf(r, f, 0x1.125edcp-7f); // 8.37312452e-3
	r = fmaf(r, f, 0x1.555b5ap-5f); // 4.16695364e-2
	r = fmaf(r, f, 0x1.555450p-3f); // 1.66664720e-1
	r = fmaf(r, f, 0x1.fffff6p-2f); // 4.99999851e-1
	r = fmaf(r, f, 0x1.000000p+0f); // 1.00000000e+0
	r = fmaf(r, f, 0x1.000000p+0f); // 1.00000000e+0
	// exp(a) = 2**i * exp(f);
	r = ldexpf(r, i);
	return r;
}

/* https://stackoverflow.com/a/39822314 */
float logf(float a)
{
	FLOAT_BITS u;
	float m, r, s, t, i, f;
	int32_t e, tmpi;

	if (0.0f == a) return -INFINITY;
	if (a < 0.0f) return -NAN;

	u.f = a;
	tmpi = u.i;

	e = (tmpi - 0x3f2aaaab) & 0xff800000;

	u.i = tmpi - e;
	m = u.f;
  
	i = (float)e * 1.19209290e-7f; // 0x1.0p-23
	/* m in [2/3, 4/3] */
	f = m - 1.0f;
	s = f * f;
	/* Compute log1p(f) for f in [-1/3, 1/3] */
	r = fmaf(0.230836749f, f, -0.279208571f); // 0x1.d8c0f0p-3, -0x1.1de8dap-2
	t = fmaf(0.331826031f, f, -0.498910338f); // 0x1.53ca34p-2, -0x1.fee25ap-2
	r = fmaf(r, s, t);
	r = fmaf(r, s, f);
	r = fmaf(i, 0.693147182f, r); // 0x1.62e430p-1 // log(2) 
	return r;
}

/* Works well if input x is in range [0..1] */
float powf(float x, float y)
{
	if (x < 0.0f)
	{
		return expf(y * logf(-x)) * (((int32_t)y) & 1 ? -1.0f : 1.0f);
	}
	return expf(y * logf(x));
}

/* https://stackoverflow.com/a/8378022/4458277 */
/*float ceilf(const float number)
{
	FLOAT_BITS u;
	int32_t exponent, fractional_bits;
	uint32_t input, output, integral_mask;
	u.f = number;
	input = u.i
	exponent = ((input >> 23) & 255) - 127;
	if (exponent < 0) return (number > 0);
	// small numbers get rounded to 0 or 1, depending on their sign

	fractional_bits = 23 - exponent;
	if (fractional_bits <= 0) return number;
	// numbers without fractional bits are mapped to themselves

	integral_mask = 0xffffffff << fractional_bits;
	output = input & integral_mask;
	// round the number down by masking out the fractional bits

	u.i = output;
	number = u.f;
	if (number > 0 && output != input) ++number;
	// positive numbers need to be rounded up, not down

	return number;
}*/

#endif /* WASM32 */
