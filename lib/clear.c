/**
 * Author: Oleksandr Smorchkov
 *
 * Force-fill color into the layer.
 */

#include "api.h"


void clear_in_range(uint32_t *begin, const uint32_t *end, const uint32_t value)
{
	while (begin < end) *begin++ = value;
}

void clear(uint32_t *layer, const uint32_t color)
{
	FOR_EACH_LINE_USE_INDEXES({
		clear_in_range(layer + begin, layer + end, color);
	});
}
