/**
 * Author: Oleksandr Smorchkov
 */

#include "api.h"


void move(
	uint32_t *layer,
	const int32_t x, const int32_t y,
	const uint32_t frame_x, const uint32_t frame_y, const uint32_t frame_w, const uint32_t frame_h,
	const int should_clear, const uint32_t clear_color)
{
	uint32_t *data, begin_;
	int32_t begin_offset, end_offset, b_x, e_x, y_;

	const int32_t frame_w_ = (int32_t)CLAMP(frame_w, 1, layers_get_width());
	const int32_t frame_h_ = (int32_t)CLAMP(frame_h, 1, layers_get_height());
	const int32_t frame_x_ = (int32_t)CLAMP(frame_x, 0, frame_w_);
	const int32_t frame_y_ = (int32_t)CLAMP(frame_y, 0, frame_h_);
	const int32_t frame_rx = frame_x_ + frame_w_;

	const int32_t clip_x = vertices_get_x();
	const int32_t clip_y = vertices_get_y();
	const int32_t clip_w = vertices_get_width();
	const int32_t clip_h = vertices_get_height();

	const int intersects = (
		x <= (clip_x + clip_w) && clip_x <= (x + clip_w) &&
		y <= (clip_y + clip_h) && clip_y <= (y + clip_h));

	if (0 == x && 0 == y)
	{
		return;
	}

	if (1 == intersects)
	{
		data = (uint32_t*)layers_get_extra();
		copy(data, layer, 1);
		if (1 == should_clear) clear(layer, clear_color);
	}
	else
	{
		data = layer;
	}

	FOR_EACH_LINE_USE_INDEXES({
		begin_ = begin;
		b_x = GET_X(begin_, layers_get_width()) + x;
		if (b_x > frame_rx)
		{
			GO_TO_NEXT_CLIP_LINE
		}

		y_ = GET_Y(begin_, layers_get_width()) + y;
		if (y_ < frame_y_ || y_ >= (frame_y_ + frame_h_))
		{
			GO_TO_NEXT_CLIP_LINE
		}

		begin_offset = INDEX_AT(b_x, y_, STRIDE);
		if (b_x < frame_x_)
		{
			begin_ += frame_x_ - b_x;
			begin_offset += frame_x_ - b_x;
		}

		e_x = GET_X(end, layers_get_width());
		if (0 == e_x)
		{
			/* Tricky case when "end" is the last unreachable index of the row
				(which is the first index of next row)
				this is the ase when, for example 5%5==0 */
			e_x = GET_X(end - 1, layers_get_width()) + 1;
		}
		e_x += x;
		if (b_x >= e_x)
		{
			GO_TO_NEXT_CLIP_LINE
		}

		end_offset = INDEX_AT(e_x, y_, STRIDE);
		if (e_x > frame_rx)
		{
			end_offset -= e_x - frame_rx;
		}

		copy_in_range(layer + begin_offset, layer + end_offset, data + begin_);
	});

	if (1 == should_clear && 0 == intersects)
	{
		clear(layer, clear_color);
	}
}
