/**
 * Author: Oleksandr Smorchkov
 *
 * Scan-line algo source: http://www.sunshine2k.de/coding/java/Polygon/Filling/FillPolygon.htm
 */

#include "api.h"


#define PICK_MOST_Y(func, dest) do {				\
		uint32_t i = 1;								\
		dest = vertices[0].y;						\
		while (i < vertices_count) {				\
			const int16_t tmp = vertices[i++].y;	\
			dest = func(tmp, dest);					\
		}											\
	} while(0)

typedef struct EDGE
{
	float m;
	float x;
	VECTOR a;
	VECTOR b;
} EDGE;

PRIVATE(VECTOR*) vertices;
PRIVATE(EDGE*) edges;
PRIVATE(int16_t*) x_list;

PRIVATE(uint32_t) edges_count;
PRIVATE(uint32_t) vertices_count;
PRIVATE(uint32_t) vertices_max_count;

PRIVATE(uint32_t) rect_x;
PRIVATE(uint32_t) rect_y;
PRIVATE(uint32_t) rect_w;
PRIVATE(uint32_t) rect_h;

PRIVATE(const float)			get_slope_invert(VECTOR a, VECTOR b)
{
	return (1.0f / (I_2_F(a.y - b.y) / I_2_F(a.x - b.x)));
}

PRIVATE(INLINE void)			calculate_rect(VECTOR vert)
{
	const uint32_t x = (uint32_t)MAX(vert.x, 0);
	const uint32_t y = (uint32_t)MAX(vert.y, 0);
	rect_x = MIN(x, rect_x);
	rect_y = MIN(y, rect_y);
	rect_w = MAX(x, rect_w);
	rect_h = MAX(y, rect_h);
}

PRIVATE(INLINE void)			fix_rect()
{
	rect_w = (rect_w - rect_x) + 1;
	rect_h = (rect_h - rect_y) + 1;
	rect_w = MIN(rect_w, layers_get_width());
	rect_h = MIN(rect_h, layers_get_height());
}

PRIVATE(INLINE void)			reset_rect()
{
	/* calculate_rect will fix those: */
	rect_w = rect_h = 0;
	rect_x = rect_y = 0xFFFFFFFF;
}

PRIVATE(INLINE const uint32_t)	build_x_list(
	int16_t *x_list, const uint32_t scanline, const uint32_t scanline_end)
{
#define PUSH_X(x) x_list[x_list_count++] = (int16_t)(x)

	EDGE *edge;
	uint32_t ay, by, i = 0, x_list_count = 0;
	while (i < edges_count)
	{
		edge = edges + i++;
		ay = edge->a.y;
		by = edge->b.y;
		/* here the scanline intersects the smaller vertex */
		if (scanline == ay)
		{
			if (scanline == by)
			{
				/* the current edge is horizontal, so we add both vertices */
				PUSH_X(edge->x = I_2_F(edge->b.x));
			}
			else
			{
				/* the current edge is horizontal, so we add both vertices */
				PUSH_X(edge->x = I_2_F(edge->a.x));
			}
		}

		if (scanline > ay && scanline < by)
		{
			edge->x += edge->m;
			PUSH_X(edge->x);
		}
		else if (scanline == (scanline_end-1) && scanline == by)
		{
			/* strange thing, algo often skips the last vertex */
			edge->x += edge->m;
			PUSH_X(edge->x);
		}
	}
	return x_list_count;

#undef PUSH_X
}

PRIVATE(INLINE const ERROR)		push_indexes(
	int16_t *x_list, const uint32_t x_list_count, const uint32_t scanline)
{
	uint32_t i, begin, end, index, rw;

	i = 0;
	rw = rect_x + rect_w;

	while (i < x_list_count)
	{
		begin = (uint32_t)x_list[i++];
		end = (uint32_t)(x_list[i++] + 1);
		if (begin >= rw) break;
		if (end > rect_x)
		{
			if (begin < rect_x)
			{
				begin = rect_x;
			}
			if (end > rw)
			{
				end = rw;
			}
			if (begin < end)
			{
				index = INDEX_AT(begin, scanline, STRIDE);
				if (0 == clip_indexes_push_pair(index, index + (end - begin)))
				{
					return CLIP_INDEXES_OVERFLOW;
				}
			}
		}
	}
	return NO_ERROR;
}

PRIVATE(INLINE const ERROR)		vertices_apply()
{
	uint32_t x_list_count, th, scanline, scanline_end;
	ERROR error;

	PICK_MOST_Y(MIN, scanline);
	scanline = MAX(scanline, rect_y);

	PICK_MOST_Y(MAX, scanline_end);
	scanline_end += 1;
	th = rect_y + rect_h;
	scanline_end = MIN(scanline_end, th);

	clip_indexes_reset();
	while (scanline < scanline_end)
	{
		x_list_count = build_x_list(x_list, scanline, scanline_end);

		sort_int16_t(x_list, x_list_count);

		error = push_indexes(x_list, x_list_count, scanline);
		if (NO_ERROR != error)
		{
			return error;
		}

		++scanline;
	}

	return NO_ERROR;
}


const uint32_t vertices_get_x()
{
	return rect_x;
}

const uint32_t vertices_get_y()
{
	return rect_y;
}

const uint32_t vertices_get_width()
{
	return rect_w;
}

const uint32_t vertices_get_height()
{
	return rect_h;
}

const int vertices_prepare_build()
{
	edges_count = vertices_count = 0;
	return 1;
}

VECTOR *vertices_get()
{
	return vertices;
}

const uint32_t vertices_get_count()
{
	return vertices_count;
}

const uint32_t vertices_get_max_count()
{
	return vertices_max_count;
}

const int vertices_add(const int32_t x, const int32_t y)
{
	if (vertices_count < vertices_max_count)
	{
		const int32_t rx = layers_get_width() - 1;
		const int32_t ry = layers_get_height() - 1;
		const VECTOR vert = (VECTOR){
			.x=(int16_t)CLAMP(x, 0, rx),
			.y=(int16_t)CLAMP(y, 0, ry)
		};
		if (0 == vertices_count || vert.raw_data != vertices[vertices_count-1].raw_data)
		{
			/* in case new vert is the first one
				or the prev is not the same as new --> add new one */
			vertices[vertices_count++] = vert;
		}
		return 1;
	}
	return 0;
}

const ERROR vertices_move(const int32_t x, const int32_t y)
{
	VECTOR vert;
	uint32_t *old_verts = (uint32_t*)edges; /* temporary utilize this mem to store old vertices */
	const uint32_t *old_verts_end = old_verts + vertices_count;

	copy_in_range(old_verts, old_verts_end, (uint32_t*)vertices);

	(void)vertices_prepare_build();

	while (old_verts < old_verts_end)
	{
		vert.raw_data = *old_verts++;
		vertices_add(x + vert.x, y + vert.y);
	}

	return vertices_finish_build();
}

const ERROR vertices_build_default()
{
	/* Max possible x/y values: */
	const int32_t rx = layers_get_width()-1;
	const int32_t ry = layers_get_height()-1;
	(void)vertices_prepare_build();

	vertices_add(0, 0);
	vertices_add(0, ry);
	vertices_add(rx, ry);
	vertices_add(rx, 0);

	return vertices_finish_build();
}

const ERROR vertices_finish_build()
{
	VECTOR vi, vj;

	if (edges_count > 0) return VERTICES_BUILD_NOT_PREPARED;
	if (vertices_count < 3) return VERTICES_WRONG_COUNT;

	reset_rect();
	for (uint32_t i = 0, j = vertices_count - 1; i < vertices_count; j = i++)
	{
		vi = vertices[i];
		vj = vertices[j];
		calculate_rect(vi);
		if (vi.y != vj.y)
		{
			if (vi.y < vj.y)
			{
				edges[edges_count++] = (EDGE){
					.m=get_slope_invert(vi, vj),
					.a=vi,
					.b=vj,
					.x=vi.x
				};
			}
			else
			{
				edges[edges_count++] = (EDGE){
					.m=get_slope_invert(vj, vi),
					.a=vj,
					.b=vi,
					.x=vj.x
				};
			}
		}
	}

	if (edges_count < 2) return CORRUPTED_EDGES;

	fix_rect();

	return vertices_apply();
}

void vertices_alloc()
{
	const uint32_t bytes = 5500;
	(void)vertices_prepare_build();
	uint8_t *data_base = (uint8_t*)malloc(bytes);

	/* The formula is a linear equasion with one X:
		sizeof(uint16_t)*X + sizeof(VECTOR)*X + sizeof(EDGE)*X = bytes
		22*X = bytes
		X = bytes/22 */
	vertices_max_count = bytes / (sizeof(int16_t) + sizeof(VECTOR) + sizeof(EDGE));

	/* occupies vertices_max_count*sizeof(int16_t) */
	x_list = (int16_t*)(data_base);

	/* occupies vertices_max_count*sizeof(VECTOR) */
	vertices = (VECTOR*)(data_base + vertices_max_count * sizeof(int16_t));

	/* occupies vertices_max_count*sizeof(EDGE) */
	edges = (EDGE*)(data_base + vertices_max_count * sizeof(int16_t) + vertices_max_count * sizeof(VECTOR));
}
