/**
 * Author: Oleksandr Smorchkov
 */

#include "api.h"


PRIVATE(void) replace_t(
	uint32_t *begin, const uint32_t *end, const uint32_t dest, const uint32_t src)
{
	while (begin < end)
	{
		if (*begin == dest) *begin = src;
		++begin;
	}
}


const int replace(
	uint32_t *layer,
	const uint32_t dest, const uint32_t src)
{
	if (src == dest)
	{
		/* Same color values, no reason to continue */
		return 0;
	}

	FOR_EACH_LINE_USE_INDEXES({
		replace_t(layer + begin, layer + end, dest, src);
	});
	return 1;
}
