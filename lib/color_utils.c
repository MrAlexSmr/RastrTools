/**
 * Author: Oleksandr Smorchkov
 */

#include "api.h"


PRIVATE(const uint32_t) line_colors_count(
	const uint32_t *b, const uint32_t *e, uint32_t *set, uint32_t set_size)
{
	uint32_t value;
	while (b < e)
	{
		value = *b++;
		if (lower_bound(set, set_size, value) == set_size)
		{
			/* no need to check out of bouns, since extra_memory is same as max possible layer: */
			set[set_size++] = value;
			sort_uint32_t(set, set_size);
		}
	}
	return set_size;
}


/* returns count of all found unique colors of given layer */
const uint32_t colors_get_count(uint32_t *layer)
{
	uint32_t *set, count;
	set = (uint32_t*)layers_get_extra();
	count = 0;
	FOR_EACH_LINE_USE_INDEXES({
		count = line_colors_count(layer + begin, layer + end, set, count);
	});
	return count;
}

const uint32_t color_get_at(
	const uint32_t *layer,
	const uint32_t x, const uint32_t y,
	const uint32_t bad_color)
{
	const uint32_t index = INDEX_AT(x, y, STRIDE);

	if (clip_indexes_get_nearest_pair_index(index) != -1)
	{
		return layer[index];
	}

	return bad_color;
}
