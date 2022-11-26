/**
 * Author: Oleksandr Smorchkov
 *
 * Channel routines.
 * Provides all required methods to modify channel's value.
 */

#include "api.h"


PRIVATE(void)	move_t(
	uint8_t *begin, const uint8_t *end, const uint8_t ch, const int16_t offset)
{
	int16_t tmp = 0;
	begin += ch;
	while (begin < end)
	{
		tmp = ((int16_t)*begin) + offset;
		*begin = (uint8_t)(CLAMP(tmp, 0, 255));
		begin += 4;
	}
}

PRIVATE(void)	set_t(
	uint8_t *begin, const uint8_t *end, const uint8_t ch, const uint8_t value)
{
	const uint8_t n = MIN(value, 255);
	begin += ch;
	while (begin < end)
	{
		*begin = n;
		begin += 4;
	}
}

PRIVATE(void)	clamp_up_t(
	uint8_t *begin, const uint8_t *end, const uint8_t ch, const uint8_t value)
{
	uint8_t tmp = 0;
	const uint8_t n = value;
	begin += ch;
	while (begin < end)
	{
		tmp = *begin;
		*begin = MIN(tmp, n);
		begin += 4;
	}
}

PRIVATE(void)	clamp_down_t(
	uint8_t *begin, const uint8_t *end, const uint8_t ch, const uint8_t value)
{
	uint8_t tmp = 0;
	const uint8_t n = value;
	begin += ch;
	while (begin < end)
	{
		tmp = *begin;
		*begin = MAX(tmp, n);
		begin += 4;
	}
}


void channel_move(uint32_t *layer, const uint8_t ch, const int16_t offset)
{
	const uint8_t ch_ = MIN(ch, 3);
	
	FOR_EACH_LINE_USE_INDEXES({
		move_t((uint8_t*)(layer + begin), (uint8_t*)(layer + end), ch_, offset);
	});
}

void channel_set(uint32_t *layer, const uint8_t ch, const uint8_t value)
{
	const uint8_t ch_ = MIN(ch, 3);

	FOR_EACH_LINE_USE_INDEXES({
		set_t((uint8_t*)(layer + begin), (uint8_t*)(layer + end), ch_, value);
	});
}

void channel_clamp_up(uint32_t *layer, const uint8_t ch, const uint8_t value)
{
	const uint8_t ch_ = MIN(ch, 3);
	
	FOR_EACH_LINE_USE_INDEXES({
		clamp_up_t((uint8_t*)(layer + begin), (uint8_t*)(layer + end), ch_, value);
	});
}

void channel_clamp_down(uint32_t *layer, const uint8_t ch, const uint8_t value)
{
	const uint8_t ch_ = MIN(ch, 3);
	
	FOR_EACH_LINE_USE_INDEXES({
		clamp_down_t((uint8_t*)(layer + begin), (uint8_t*)(layer + end), ch_, value);
	});
}
