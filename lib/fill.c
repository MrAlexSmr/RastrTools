/**
 * Author: Oleksandr Smorchkov
 */

#include "api.h"


PRIVATE(const uint32_t)		process_near_line_clip_indexes(
	const uint32_t *layer,
	uint32_t *stack, uint32_t stack_size,
	uint32_t begin, const uint32_t end, const uint32_t value)
{
	int32_t token_index, locked;
	uint32_t right_border;
	const uint32_t *clip_indexes = clip_indexes_get();
	locked = 0;
	right_border = 0;
	for (; begin < end; ++begin)
	{
		if (value == layer[begin])
		{
			if (0 == locked)
			{
				token_index = clip_indexes_get_nearest_pair_index(begin);
				if (-1 != token_index)
				{
					locked = 1;
					/* no need to check out of bouns, since extra_memory is same as max possible layer: */
					stack[stack_size++] = begin;
					right_border = clip_indexes[token_index + 1];
				}
			}
			else if (begin == right_border)
			{
				/* reset lock in case 'begin' iterator reached the current border */
				locked = 0;
			}
		}
		else
		{
			locked = 0;
		}
	}
	return stack_size;
}

/*PRIVATE(const uint32_t)		process_near_line_frame(
	const uint32_t *layer,
	uint32_t *stack, uint32_t stack_size,
	uint32_t begin, const uint32_t end, const uint32_t value)
{
	int locked = 0;
	for (; begin < end; ++begin)
	{
		if (value == layer[begin])
		{
			if (0 == locked)
			{
				locked = 1;
				stack[stack_size++] = begin;
			}
		}
		else
		{
			locked = 0;
		}
	}
	return stack_size;
}*/

PRIVATE(INLINE void)		flood_fill_clip_indexes(
	uint32_t *layer, uint32_t *stack, const uint32_t old_value, const uint32_t new_value)
{
	int32_t cy, i, cursor, begin, end, token_index, current_left_border;
	uint32_t stack_size = 1;

	const uint32_t x = vertices_get_x();
	const uint32_t y = vertices_get_y();
	const uint32_t rx = x + vertices_get_width();
	const uint32_t stride = STRIDE;
	const uint32_t ty = y + vertices_get_height();
	const uint32_t *clip_indexes = clip_indexes_get();

	do {
		cursor = token_index = stack[--stack_size];

		i = clip_indexes_get_nearest_pair_index(token_index);
		if (i == -1)
		{
			continue;
		}

		cy = GET_Y(token_index, STRIDE);
		begin = INDEX_AT(x, cy, STRIDE);
		end = INDEX_AT(rx, cy, STRIDE);

		begin = MAX(begin, clip_indexes[i]);
		end = MIN(end, clip_indexes[i + 1]);

		while (cursor >= begin && old_value == layer[cursor]) --cursor;

		current_left_border = cursor + 1;
		cursor = token_index;

		while (++cursor < end && old_value == layer[cursor]);

		i = current_left_border;
		while (i < cursor) layer[i++] = new_value;

		if (cy > y)
		{
			stack_size = process_near_line_clip_indexes(
				layer, stack, stack_size, current_left_border - stride, cursor - stride, old_value);
		}

		if (cy <= ty)
		{
			stack_size = process_near_line_clip_indexes(
				layer, stack, stack_size, current_left_border + stride, cursor + stride, old_value);
		}
	} while (stack_size > 0);
}

/*PRIVATE(INLINE void)		flood_fill_rect(
	uint32_t *layer,
	uint32_t *stack,
	const uint32_t old_value, const uint32_t new_value,
	const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height)
{
	int32_t i, cy, cursor, begin, end, current_left_border;
	uint32_t stack_size = 1;

	const uint32_t rx = x + width;
	const uint32_t ty = y + height;
	const uint32_t stride = STRIDE;

	do {
		cursor = i = stack[--stack_size];

		cy = GET_Y(i, STRIDE);
		begin = INDEX_AT(x, cy, STRIDE);
		end = INDEX_AT(rx, cy, STRIDE);

		while (cursor >= begin && old_value == layer[cursor]) --cursor;

		current_left_border = cursor + 1;
		cursor = i;

		while (++cursor < end && old_value == layer[cursor]);

		i = current_left_border;
		while (i < cursor) layer[i++] = new_value;

		if (cy > y)
		{
			stack_size = process_near_line_frame(
				layer, stack, stack_size, current_left_border - stride, cursor - stride, old_value);
		}

		if (cy <= ty)
		{
			stack_size = process_near_line_frame(
				layer, stack, stack_size, current_left_border + stride, cursor + stride, old_value);
		}
	} while (stack_size > 0);
}*/


const int fill(
	uint32_t *layer,
	const uint32_t x, const uint32_t y,
	const uint32_t color)
{
#define SET_CHECK_COLORS \
	if (color == (old_color = layer[index])) return 0

	uint32_t *stack, old_color;
	const uint32_t index = INDEX_AT(x, y, STRIDE);

	stack = (uint32_t*)layers_get_extra();
	stack[0] = index;

	/*if (WITHOUT_CLIP_INDEXES)
	{
		if (POINT_IN_RECT(x, y, 0, 0, layers_get_width(), layers_get_height()))
		{
			SET_CHECK_COLORS;
			flood_fill_rect(
				layer,
				stack,
				old_color, color,
				0, 0, layers_get_width(), layers_get_height());
			return 1;
		}
		return 0;
	}*/

	if (clip_indexes_get_nearest_pair_index(index) == -1)
	{
		return 0; // Stop.
	}

	SET_CHECK_COLORS;

	flood_fill_clip_indexes(layer, stack, old_color, color);
	return 1;

#undef SET_CHECK_COLORS
}
