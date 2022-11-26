/**
 * Author: Oleksandr Smorchkov
 */

#include "api.h"


PRIVATE(int) reach_half_height;
PRIVATE(uint32_t) opposite;

PRIVATE(INLINE void) flip_t(uint32_t *dest, uint32_t begin, uint32_t end)
{
	uint32_t tmp, i;
	if (0 == reach_half_height)
	{
		// TODO: find a better way to iterate half height of frame
		return;
	}
	--reach_half_height;
	i = opposite;
	for (; begin < end; ++begin, ++i)
	{
		tmp = dest[begin];
		dest[begin] = dest[i];
		dest[i] = tmp;
	}
	opposite -= STRIDE;
}

void flip(uint32_t *layer)
{
	reach_half_height = vertices_get_height() >> 1;
	opposite = INDEX_AT(vertices_get_x(), vertices_get_y() + vertices_get_height() - 1, STRIDE);
	FOR_EACH_LINE_USE_INDEXES({
		flip_t(layer, begin, end);
	});
}
