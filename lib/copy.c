/**
 * Author: Oleksandr Smorchkov
 */

#include "api.h"


PRIVATE(void) copy_t(uint32_t *begin, const uint32_t *end, const uint32_t *src, const int ignore_compose_op)
{
	if (1 == ignore_compose_op || COPY == compose_get_operation())
	{
		copy_in_range(begin, end, src);
	}
	else
	{
		compose(begin, end, src);
	}
}


void copy_in_range(uint32_t *begin, const uint32_t *end, const uint32_t *src)
{
	while (begin < end) *begin++ = *src++;
}

const int copy(uint32_t *dest, const uint32_t *src, const int ignore_compose_op)
{
	if (dest == src)
	{
		return 0;
	}
	
	FOR_EACH_LINE_USE_INDEXES({
		copy_t(dest + begin, dest + end, src + begin, ignore_compose_op);
	});
	return 1;
}
