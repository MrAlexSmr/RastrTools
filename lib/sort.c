/**
 * Author: Oleksandr Smorchkov
 */

#include "api.h"


/* Insertion sort implementation */
#define SORT(T, dest, length)						\
	void sort_##T(T *dest, const uint32_t length) {	\
		uint32_t i, j, prev;						\
		for (i = 1; i < length;)					\
			for (j = i++; j > 0; --j) {				\
				prev = j - 1;						\
				T tmp = dest[j];					\
				if (tmp < dest[prev]) {				\
					dest[j] = dest[prev];			\
					dest[prev] = tmp;				\
				} else break;						\
			}										\
	}


SORT(int16_t, dest, length)
SORT(uint32_t, dest, length)

const uint32_t lower_bound(
	const uint32_t *src, const uint32_t length, const uint32_t key)
{
	uint32_t b, e, m;
	b = 0;
	e = length;
	while (b < e)
	{
		m = b + ((e - b) >> 1);
		if (src[m] < key)
		{
			b = m + 1;
		}
		else
		{
			e = m;
		}
	}
	return e;
}
