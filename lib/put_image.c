/**
 * Author: Oleksandr Smorchkov
 *
 * Copies pixels of source image into the layer.
 * Image should be less or equal to layer size.
 */

#include "api.h"


const int put_image(
	uint32_t *dest, const uint32_t *src,
	int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	uint32_t offset_x, offset_y, origin_src_width;
	int32_t rx = x + (int32_t)width;
	int32_t ry = y + (int32_t)height;
	int32_t layer_width = (int32_t)layers_get_width();
	int32_t layer_height = (int32_t)layers_get_height();

	if (0 == x && 0 == y &&
		width == layers_get_width() && height == layers_get_height())
	{
		copy_in_range(dest, dest + (width * height), src);
		return 1;
	}

	if (x >= layer_width || y >= layer_height || rx <= 0 || ry <= 0)
	{
		return 0;
	}

	origin_src_width = width;
	if (x < 0 || y < 0 || layer_width < rx || layer_height < ry)
	{
		offset_x = 0;
		offset_y = 0;
		if (rx > layer_width)
		{
			width = abs(layer_width - x);
		}
		if (ry > layer_height)
		{
			height = abs(layer_height - y);
		}
		if (x < 0)
		{
			offset_x = -x;
			width -= offset_x;
			x = 0;
		}
		if (y < 0)
		{
			offset_y = -y;
			height -= offset_y;
			y = 0;
		}
		src += INDEX_AT(offset_x, offset_y, origin_src_width);
	}
	dest += INDEX_AT(x, y, STRIDE);
	y = 0;
	while (y++ < height)
	{
		copy_in_range(dest, dest + width, src);
		src += origin_src_width;
		dest += STRIDE;
	}
	return 1;
}
