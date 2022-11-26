/**
 * @author Zingl Alois
 * @date 22.08.2016
 * @version 1.2
 */
/* The code is taken from http://members.chello.at/easyfilter/bresenham.c */
/**
 * http://members.chello.at/%7Eeasyfilter/Bresenham.pdf
 * The programs have no copyright and could be used and modified by anyone as wanted.
 * The source code was carefully tested but are given without warranty of any kind. Use it at your own risk.
 */
/* Adapted for custom needs by Oleksandr Smorchkov */

#include "api.h"


PRIVATE(int32_t) prev_x;
PRIVATE(int32_t) prev_y;

PRIVATE(const int)		apply_color(
	uint32_t *dest, int32_t x, int32_t y, const uint32_t color)
{
	uint32_t index, dest_color;
	if (prev_x == x && prev_y == y)
	{
		return 0;
	}
	prev_x = x;
	prev_y = y;
	if (0 == POINT_IN_RECT(
		x, y,
		vertices_get_x(), vertices_get_y(),
		vertices_get_width(), vertices_get_height()))
	{
		return 0;
	}
	index = INDEX_AT(x, y, STRIDE);
	if (clip_indexes_get_nearest_pair_index(index) == -1)
	{
		return 0;
	}
	dest += index;
	dest_color = *dest;
	*dest = compose_colors(dest_color, color);
	return 1;
}

PRIVATE(INLINE void)	bresenham_ellipse(
	uint32_t *dest,
	int32_t x0, int32_t y0,
	int32_t x1, int32_t y1,
	const uint32_t color)
{
	int32_t a = abs(x1 - x0);
	int32_t b = abs(y1 - y0);
	int32_t b1 = b & 1;
	int32_t aa = a * a;
	int32_t bb = b * b;
	float dx = (float)(4 * (1 - a) * bb);
	float dy = (float)(4 * (b1 + 1) * aa);
	float err = dx + dy + (float)(b1 * aa);
	float e2;

	if (x0 > x1)
	{
		x0 = x1;
		x1 += a;
	}
	if (y0 > y1)
	{
		y0 = y1;
	}
	y0 += (b + 1) / 2;
	y1 = y0 - b1;
	a = 8 * aa;
	b1 = 8 * bb;

	do {
		(void)apply_color(dest, x1, y0, color);
		(void)apply_color(dest, x0, y0, color);
		(void)apply_color(dest, x0, y1, color);
		(void)apply_color(dest, x1, y1, color);
		e2 = 2.0f * err;
		if (e2 <= dy)
		{
			y0++;
			y1--;
			err += dy += (float)a;
		}
		if (e2 >= dx || (2.0f * err) > dy)
		{
			x0++;
			x1--;
			err += dx += (float)b1;
		}
	} while (x0 <= x1);

	while ((y0 - y1) <= b)
	{
		(void)apply_color(dest, x0 - 1, y0, color);
		(void)apply_color(dest, x1 + 1, y0++, color);
		(void)apply_color(dest, x0 - 1, y1, color);
		(void)apply_color(dest, x1 + 1, y1--, color);
	}
}

PRIVATE(INLINE void)	rectangle(
	uint32_t *dest,
	int32_t x0, int32_t y0,
	int32_t x1, int32_t y1,
	const uint32_t color)
{
	uint32_t tmp;
	if (x0 > x1)
	{
		tmp = x1;
		x1 = x0;
		x0 = tmp;
	}
	if (y0 > y1)
	{
		tmp = y1;
		y1 = y0;
		y0 = tmp;
	}

	tmp = x0;
	while (tmp <= x1)
	{
		(void)apply_color(dest, tmp, y0, color);
		(void)apply_color(dest, tmp, y1, color);
		++tmp;
	}

	tmp = y0;
	--y1;
	while (tmp++ < y1)
	{
		(void)apply_color(dest, x0, tmp, color);
		(void)apply_color(dest, x1, tmp, color);
	}
}

PRIVATE(INLINE void)	bresenham_line(
	uint32_t *dest,
	int32_t x0, int32_t y0,
	int32_t x1, int32_t y1,
	const uint32_t color)
{
	int32_t e2, dx, dy, sx, sy, err;
	dx = x1 - x0;
	dy = y1 - y0;
	dx = abs(dx);
	dy = -abs(dy);
	sx = x0 < x1 ? 1 : -1;
	sy = y0 < y1 ? 1 : -1;
	err = dx + dy;
	for (;;)
	{
		(void)apply_color(dest, x0, y0, color);
		if (x0 == x1 && y0 == y1) break;
		e2 = err + err;
		if (e2 >= dy)
		{
			err += dy;
			x0 += sx;
		}
		if (e2 <= dx)
		{
			err += dx;
			y0 += sy;
		}
	}
}

PRIVATE(INLINE void)	bresenham_line_width(
	uint32_t *dest,
	int32_t x0, int32_t y0,
	int32_t x1, int32_t y1,
	const uint32_t width,
	const uint32_t color)
{
	int32_t e2, dx, dy, sx, sy, x2, y2, err, opt1;
	float ed, wd;
	dx = abs(x1 - x0);
	dy = abs(y1 - y0);
	sx = x0 < x1 ? 1 : -1;
	sy = y0 < y1 ? 1 : -1;
	err = dx - dy;
	wd = ((float)width + 1.0f) / 2.0f;
	ed = (dx + dy == 0 ? 1.0f : sqrtf(I_2_F(dx * dx) + I_2_F(dy * dy))) * wd;
	for (;;)
	{
		(void)apply_color(dest, x0, y0, color);
		e2 = err;
		x2 = x0;
		if ((e2 << 1) >= -dx)
		{
			for (e2 += dy, y2 = y0, opt1 = dx > dy;
				e2 < ed && (y1 != y2 || opt1);
				e2 += dx)
			{
				(void)apply_color(dest, x0, y2 += sy, color);
				e2 += dx;
			}
			if (x0 == x1) break;
			e2 = err;
			err -= dy;
			x0 += sx;
		}
		if ((e2 << 1) <= dy)
		{
			for (e2 = dx - e2, opt1 = dx < dy;
				e2 < ed && (x1 != x2 || opt1);
				e2 += dy)
			{
				(void)apply_color(dest, x2 += sx, y0, color);
				e2 += dy;
			}
			if (y0 == y1) break;
			err += dx;
			y0 += sy;
		}
	}
}

void stroke_prepare()
{
	/* reset cached coords */
	prev_x = prev_y = 2147483647;
}

void stroke_finish()
{
	/* do nothing for this time */
}

void stroke_ellipse(
	uint32_t *dest,
	const int32_t x0, const int32_t y0,
	const int32_t x1, const int32_t y1,
	const uint32_t color)
{
	bresenham_ellipse(dest, x0, y0, x1, y1, color);
}

void stroke_rectangle(
	uint32_t *dest,
	const int32_t x0, const int32_t y0,
	const int32_t x1, const int32_t y1,
	const uint32_t color)
{
	rectangle(dest, x0, y0, x1, y1, color);
}

void stroke_line(
	uint32_t *dest,
	const int32_t x0, const int32_t y0,
	const int32_t x1, const int32_t y1,
	const uint32_t width,
	const uint32_t color)
{
	if (width > 1)
	{
		bresenham_line_width(dest, x0, y0, x1, y1, width, color);
	}
	else if (width == 1)
	{
		bresenham_line(dest, x0, y0, x1, y1, color);
	}
}
