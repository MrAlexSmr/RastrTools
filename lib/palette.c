/**
 * Author: Oleksandr Smorchkov
 */

#include "api.h"


PRIVATE(uint32_t*) palette;
PRIVATE(uint32_t) palette_size;
PRIVATE(uint32_t) palette_max_size;

PRIVATE(void) palette_apply_t(uint32_t *begin, const uint32_t *end, const int skipAlpha)
{
	uint32_t token;
	while (begin < end)
	{
		token = *begin;
		*begin++ = palette_find_nearest_color(token, skipAlpha);
	}
}

void palette_alloc()
{
	palette_max_size = 256;
	palette = (uint32_t*)malloc(palette_max_size * sizeof(uint32_t));
	(void)palette_resize(0);
}

uint32_t *palette_get()
{
	return palette;
}

const uint32_t palette_resize(const uint32_t size)
{
	return (palette_size = MIN(palette_max_size, size));
}

const uint32_t palette_get_size()
{
	return palette_size;
}

const uint32_t palette_get_max_size()
{
	return palette_max_size;
}

/* looking for closest color using Euclidean distance */
const uint32_t palette_find_nearest_color(const uint32_t value, const int skipAlpha)
{
	COLOR32 color, tmp, result;
	uint32_t *begin, *end;
	uint32_t min, distance;
	int16_t dr, dg, db, da;

	if (skipAlpha)
	{
		tmp.CH_R = 0xFF;
		tmp.CH_G = 0xFF;
		tmp.CH_B = 0xFF;
		tmp.CH_A = 0x00;
		min = tmp.raw_data;
	}
	else
	{
		min = 0xFFFFFFFF; /* init by maximum */
	}

	color.raw_data = value;
	result.raw_data = value;

	begin = palette;
	end = palette + palette_size;

	while (begin < end)
	{
		tmp.raw_data = *begin++;
		dr = color.CH_R - tmp.CH_R;
		dg = color.CH_G - tmp.CH_G;
		db = color.CH_B - tmp.CH_B;
		if (skipAlpha)
		{
			distance = (uint32_t)(dr * dr + dg * dg + db * db);
		}
		else
		{
			da = color.CH_A - tmp.CH_A;
			distance = (uint32_t)(dr * dr + dg * dg + db * db + da * da);
		}
		if (distance < min)
		{
			min = distance;
			result.raw_data = tmp.raw_data;
		}
	}

	return result.raw_data;
}

const int palette_apply(uint32_t *layer, const int skipAlpha)
{
	if (palette_size < 1) return 0; /* No reason to continue */

	FOR_EACH_LINE_USE_INDEXES({
		palette_apply_t(layer + begin, layer + end, skipAlpha);
	});

	return 1;
}
