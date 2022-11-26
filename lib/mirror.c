/**
 * Author: Oleksandr Smorchkov
 */

#include "api.h"


PRIVATE(INLINE void) swap(uint32_t *begin, uint32_t *end)
{
	uint32_t tmp;
	while (begin < end)
	{
		tmp = *begin;
		*begin++ = *(--end);
		*end = tmp;
	}
}

void mirror(uint32_t *layer)
{
	FOR_EACH_LINE_USE_INDEXES({
		swap(layer + begin, layer + end);
	});
}
