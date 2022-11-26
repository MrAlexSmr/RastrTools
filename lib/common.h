/**
 * Author: Oleksandr Smorchkov
 */

#ifndef COMMON_H
#define COMMON_H

#include "stdlib.h"


typedef union HISTORY_PAGE_DATA
{
	struct
	{
		uint16_t page;
		uint16_t count;
	};

	uint32_t raw_data;
} HISTORY_PAGE_DATA;

/* WASM is little-endian */
typedef union COLOR32
{
	struct
	{
		uint8_t CH_R;
		uint8_t CH_G;
		uint8_t CH_B;
		uint8_t CH_A;
	};
	uint32_t raw_data;
} COLOR32;

/* It is better to consider this as position or a size at a same time */
typedef union VECTOR
{
	struct
	{
		int16_t x;
		int16_t y;
	};

	struct
	{
		uint16_t width;
		uint16_t height;
	};

	uint32_t raw_data;
} VECTOR;

typedef enum ERROR
{
	NO_ERROR = 0,
	CORRUPTED_EDGES = 1,
	VERTICES_BUILD_NOT_PREPARED = 2,
	VERTICES_WRONG_COUNT = 3,
	CLIP_INDEXES_OVERFLOW = 4,
	LZW_DECODE_OUT_OF_SCOPE = 5,
	DECOMPRESS_WRONG_HEADER = 6,
	DECOMPRESS_WRONG_LEVEL = 7,
	DECOMPRESS_OUT_OF_SCOPE = 8,
	DECOMPRESS_CORRUPTED_DATA = 9
} ERROR;

typedef enum COMPOSITE_OPERATION
{
	COPY = 0,
	SOURCE_OVER = 1,
	SOURCE_IN = 2,
	SOURCE_OUT = 3,
	SOURCE_ATOP = 4,
	DESTINATION_OVER = 5,
	DESTINATION_IN = 6,
	DESTINATION_OUT = 7,
	DESTINATION_ATOP = 8,
	LIGHTER = 9,
	XOR = 10,
	MULTIPLY = 11,
	SCREEN = 12,
	OVERLAY = 13,
	DARKEN = 14,
	LIGHTEN = 15,
	COLOR_DODGE = 16,
	COLOR_BURN = 17,
	HARD_LIGHT = 18,
	SOFT_LIGHT = 19,
	DIFFERENCE = 20,
	EXCLUSION = 21,
	HUE = 22,
	SATURATION = 23,
	COLOR = 24,
	LUMINOSITY = 25
} COMPOSITE_OPERATION;

typedef enum FILTER_OPERATION
{
	FILTER_BRIGHTNESS = 1,
	FILTER_CONTRAST = 2,
	FILTER_INVERT = 3,
	FILTER_SEPIA = 4,
	FILTER_SATURATE = 5,
	FILTER_GRAYSCALE = 6,
	FILTER_HUE_ROTATE = 7,
	FILTER_GAMMA = 8
} FILTER_OPERATION;

#endif /*COMMON_H*/
