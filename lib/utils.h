/**
 * Author: Oleksandr Smorchkov
 *
 * Set of inline simple utility methods and base data structures.
 */

#ifndef UTILS_H
#define UTILS_H

#include "config.h"
#include "stdlib.h"


#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

#define CLAMP(n,mi,ma) ((n) > (ma) ? (ma) : ((n) < (mi) ? (mi) : (n)))

/* Returns x-value from an index (relative to layer's stride) */
#define GET_X(i, stride) ((i) % (stride))

/* Returns y-value from an index (relative to layer's stride) */
#define GET_Y(i, stride) ((i) / (stride))

#define INDEX_AT(x, y, stride) ((x) + (y) * (stride))

/* Cast int to float */
#define I_2_F(n) ((float)(n))

/* Cast float to int8 */
#define F_2_I8(f) ((uint8_t)(f))

#define POINT_IN_RECT(x, y, fx, fy, fw, fh) \
	((x) >= (fx) && (x) < ((fx) + (fw)) && (y) >= (fy) && (y) < ((fy) + (fh)))

#define STRIDE layers_get_width()

/*
 * @param {any} for_line - just anything: code, method, e.t.c.
 */
#define FOR_EACH_LINE_USE_INDEXES(for_line) do {						\
		const uint32_t *_inds_ = clip_indexes_get();					\
		const uint32_t *_indxs_end_ = _inds_ + clip_indexes_get_count();\
		while (_inds_ < _indxs_end_) {									\
			const uint32_t begin = *(_inds_++);							\
			const uint32_t end = *(_inds_++);							\
			for_line;													\
		}																\
	} while(0)

/*
 * @param {any} for_line - just anything: code, method, e.t.c.
 */
#define FOR_EACH_LINE_USE_CLIP_RECT(for_line) do {				\
		uint32_t _y_ = vertices_get_y();						\
		const uint32_t _x_ = vertices_get_x();					\
		const uint32_t _w_ = vertices_get_width();				\
		const uint32_t _yh_ = _y_ + vertices_get_height();		\
		while (_y_ < _yh_) {									\
			const uint32_t begin = INDEX_AT(_x_, _y_++, STRIDE);\
			const uint32_t end = begin + _w_;					\
			for_line;											\
		}														\
	} while(0)

#define GO_TO_NEXT_CLIP_LINE continue;

#endif /* UTILS_H */
