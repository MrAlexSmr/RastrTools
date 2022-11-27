/**
 * Author: Oleksandr Smorchkov
 *
 * Common header.
 * Any public unit should include this file.
 */

#ifndef API_H
#define API_H

#include "config.h"
#include "stdlib.h"
#include "common.h"
#include "utils.h"

/* ------------------------------------------------------------------------- */

/* Move respective channel by given offset */
PUBLIC(void)						channel_move(
										uint32_t *layer,
										const uint8_t ch, const int16_t offset);

/* Set respective channel to the value */
PUBLIC(void)						channel_set(
										uint32_t *layer,
										const uint8_t ch, const uint8_t value);

/* Clamp up respective channel by the value */
PUBLIC(void)						channel_clamp_up(
										uint32_t *layer,
										const uint8_t ch, const uint8_t value);

/* Clamp down respective channel by the value */
PUBLIC(void)						channel_clamp_down(
										uint32_t *layer,
										const uint8_t ch, const uint8_t value);

/* ------------------------------------------------------------------------- */

/* FIlls/clears the destination layer by the source color */
PUBLIC(void)						clear(
										uint32_t *layer,
										const uint32_t color);

/* Low-level method. Sets the value into the given range */
PUBLIC(void)						clear_in_range(
										uint32_t *begin, const uint32_t *end,
										const uint32_t value);

/* ------------------------------------------------------------------------- */

/* Allocate memory for clip_indexes */
PROTECTED(void)						clip_indexes_alloc(const uint32_t bytes);

/* Reset clip indexes to default (empties clip indxes) */
PROTECTED(void)						clip_indexes_reset();

/*
 * Add new [begin;end) indexes pair;
 * Indexes should always increase over pushing,
 * And comes from top to bottom and from left to right,
 * Example: [0,4, 8,12, 13,17...]
 */
PROTECTED(const int)				clip_indexes_push_pair(
										const uint32_t begin, const uint32_t end);

/* Provides a pointer to the memory of the clip indexes data */
PROTECTED(const uint32_t*)			clip_indexes_get();

/* Current count of clip indexes active */
PROTECTED(const uint32_t)			clip_indexes_get_count();

/* Returns nearest index of the pair relative to "index" param */
PROTECTED(const int32_t)			clip_indexes_get_nearest_pair_index(
										const uint32_t index);

/* ------------------------------------------------------------------------- */

/* Count of colors of destination layer.
	Will utilize extra memory! */
PUBLIC(const uint32_t)				colors_get_count(
										uint32_t *layer);

/*
 * Should return a color from a given position;
 * or bad_color value in case of any error
 */
PUBLIC(const uint32_t)				color_get_at(
										const uint32_t *layer,
										const uint32_t x, const uint32_t y,
										const uint32_t bad_color);

/* ------------------------------------------------------------------------- */

/* Compose colors of source and destination layers */
PROTECTED(void)						compose(
										uint32_t *begin, const uint32_t *end,
										const uint32_t *src);

/* Returns currently active compose operation */
PUBLIC(const COMPOSITE_OPERATION)	compose_get_operation();

/* Set active compose operation */
PUBLIC(void)						compose_set_operation(
										const uint32_t op);

/* Apply compose operation only for only abstract color values */
PUBLIC(const uint32_t)				compose_colors(
										const uint32_t dest, const uint32_t src);

/* ------------------------------------------------------------------------- */

/*
 * Decompresses compressed data,
 * return whether there was an error during decompression,
 * reason may be: corrupt headers or data itself
 */
PUBLIC(const ERROR)					compression_decompress(
										const uint32_t *src, uint32_t *layer);

/*
 * Compresses given layer into the dest _begin _end range,
 * returns occupied size or 0 in case of error,
 * reason may be: out of memory or range issues
 */
PUBLIC(const uint32_t)				compression_compress(
										uint32_t *dest_begin, const uint32_t *dest_end,
										const uint32_t *layer,
										const int compressWholeLayer);

/* ------------------------------------------------------------------------- */

/*
 * Unsafe. Copy pixels from source to destination layer
 * Do not use this as "move pixels" on same layer!
 * Depends on compose operation!
 */
PUBLIC(const int)					copy(
										uint32_t *dest, const uint32_t *src,
										const int ignore_compose_op);

/* Unsafe. Low-level method. Copies data of one stream to another */
PUBLIC(void)						copy_in_range(
										uint32_t *begin, const uint32_t *end,
										const uint32_t *src);

/* ------------------------------------------------------------------------- */

/* Apply global filtering operation */
PUBLIC(void)						filter(
										uint32_t *layer,
										const uint32_t op, const float value);

/* Apply custom filtering operation */
PUBLIC(void)						filter_custom_matrix(
										uint32_t *layer,
										const float v0, const float v1, const float v2,
										const float v5, const float v6, const float v7,
										const float v10, const float v11, const float v12);

/* ------------------------------------------------------------------------- */

/*
 * Flood-fill the layer by source color starting from given "index" param;
 * WARN: Will utilize extra memory!
 */
PUBLIC(const int)					fill(
										uint32_t *layer,
										const uint32_t x, const uint32_t y,
										const uint32_t color);

/* ------------------------------------------------------------------------- */

/* Similar to mirror function, but mirros pixels of the layer vertically */
PUBLIC(void)						flip(
										uint32_t *layer);

/* ------------------------------------------------------------------------- */

PROTECTED(void)						history_mem_alloc();

/* View on history pages data */
PUBLIC(uint32_t*)					get_history_data_at(const uint32_t page_data);

/* Mark the given page as free or occupied */
PUBLIC(void)						history_mark_pages(
										const uint32_t page_data, const int flag);

/* Return free bytes left */
PUBLIC(const uint32_t)				get_free_bytes();

/*
 * Update page's index/position;
 * Part of defragmentation "api", should be used by external controller logic
 */
PUBLIC(const uint32_t)				history_update_page_index(
										const uint32_t page_data, const uint16_t page);

/*
 * Provides page index from raw page data;
 * Part of defragmentation "api", should be user by external controller logic
 */
PUBLIC(const uint32_t)				get_history_page_index(const uint32_t page_data);

/*
 * Provides the size of occupied memory of the page from raw page data;
 * Part of defragmentation "api", should be user by external controller logic
 */
PUBLIC(const uint32_t)				get_history_page_size(const uint32_t page_data);

/*
 * Will try to occupy bytes_count amount of memory,
 * and will save bytes data;
 * zero may be returned in case of error
 */
PUBLIC(const uint32_t)				history_save(
										const uint32_t *bytes, const uint32_t bytes_count);

/* ------------------------------------------------------------------------- */

/* Allocates memory for all layers */
PROTECTED(void)						layers_alloc(
										const uint16_t max_side,
										const int use_single_layer);

/*
 * Extra layer for extra needs (like custom call-stack implementation, layer swap, e.t.c.)
 * Do not rely on this as a communication between units or smth like this.
 * Data of this layer is "true" within a "microtask",
 * so consider that at the next call this data may and will be corrupted!
 *
 * TIP: To get the begin of extra call layers_get_extra(),
 * and to get the end - call layers_get(),
 * this means, that the address of first layer is the end of the extra
 */
PUBLIC(uint32_t*)					layers_get_extra();

/* Address of layer base */
PUBLIC(uint32_t*)					layers_get();

/* Get maximum possible size of layer's side */
PUBLIC(const uint32_t)				layers_get_max_side_size();

/* Get maximum possible layers count */
PUBLIC(const uint32_t)				layers_get_max_layers_count();

/* Get current layer width */
PUBLIC(const uint32_t)				layers_get_width();

/* Get current layer height */
PUBLIC(const uint32_t)				layers_get_height();

/* Get current layer pixels count */
PUBLIC(const uint32_t)				layers_get_size();

/* Update layer size */
PUBLIC(void)						layers_resize(
										const uint32_t width, const uint32_t height);

/* ------------------------------------------------------------------------- */

/* Lempel–Ziv–Welch compression algo */

PROTECTED(void)						lzw_alloc();

PROTECTED(void)						lzw_encode_bytes_prepare();

PROTECTED(uint8_t*)					lzw_encode_bytes(
										uint8_t *output, const uint8_t *output_end,
										const uint8_t *begin, const uint8_t *end);

PROTECTED(uint8_t*)					lzw_encode_bytes_finish(
										uint8_t *output, const uint8_t *output_end);

PROTECTED(uint8_t*)					lzw_decode_bytes(
										uint8_t *output, const uint8_t *output_end,
										const uint8_t *begin, const uint8_t *end,
										ERROR *error);

/* ------------------------------------------------------------------------- */

/* Should be called first when using this module. Allocates required memory. */
PUBLIC(void)						mem_internal_init(const int use_single_layer);

/* ------------------------------------------------------------------------- */

/* Mirrors the pixels of given layer */
PUBLIC(void)						mirror(
										uint32_t *layer);

/* ------------------------------------------------------------------------- */

/*
 * Moves pixels inside of the built vertices shape to the x/y position.
 * WARN: May utilize extra memory!
 */
PUBLIC(void)						move(
										uint32_t *layer,
										const int32_t x, const int32_t y,
										const uint32_t frame_x, const uint32_t frame_y,
										const uint32_t frame_w, const uint32_t frame_h,
										const int should_clear, const uint32_t clear_color);

/* ------------------------------------------------------------------------- */

/* Copies pixels from src into the dest layer with given offset */
PUBLIC(const int)					put_image(
										uint32_t *dest, const uint32_t *src,
										int32_t x, int32_t y, uint32_t width, uint32_t height);

/* ------------------------------------------------------------------------- */

/* Global palette. But there is no restriction to use any color value outside this */
PROTECTED(void)						palette_alloc();

/* Provides an address of palette data */
PUBLIC(uint32_t*)					palette_get();

/* Resets the palette to default (makes it empty) */
PUBLIC(const uint32_t)				palette_resize(const uint32_t size);

/* Return current palette size */
PUBLIC(const uint32_t)				palette_get_size();

/* Return max possible palette size */
PUBLIC(const uint32_t)				palette_get_max_size();

/* Gets nearest color to the given value from all available colors in palette */
PUBLIC(const uint32_t)				palette_find_nearest_color(
										const uint32_t value, const int skip_alpha);

/* Reduces colors of the destination layer */
PUBLIC(const int)					palette_apply(
										uint32_t *layer, const int skip_alpha);

/* ------------------------------------------------------------------------- */

/* Replace color by another on the whole layer */
PUBLIC(const int)					replace(
										uint32_t *layer,
										const uint32_t dest, const uint32_t src);

/* ------------------------------------------------------------------------- */

/*  */
PUBLIC(void)						stroke_prepare();

/*  */
PUBLIC(void)						stroke_finish();

/* Draws non-antialiased ellipse from x0:y0 to x1:y1 */
PUBLIC(void)						stroke_ellipse(
										uint32_t *layer,
										const int32_t x0, const int32_t y0,
										const int32_t x1, const int32_t y1,
										const uint32_t color);

/* Draws non-antialiased rectangle from x0:y0 to x1:y1 */
PUBLIC(void)						stroke_rectangle(
										uint32_t *layer,
										const int32_t x0, const int32_t y0,
										const int32_t x1, const int32_t y1,
										const uint32_t color);

/* Draws non-antialiased line from x0:y0 to x1:y1 */
PUBLIC(void)						stroke_line(
										uint32_t *layer,
										const int32_t x0, const int32_t y0,
										const int32_t x1, const int32_t y1,
										const uint32_t width,
										const uint32_t color);

/* ------------------------------------------------------------------------- */

/*  */
PROTECTED(void)						sort_int16_t(
										int16_t *dest, const uint32_t length);

/*  */
PROTECTED(void)						sort_uint32_t(
										uint32_t *dest, const uint32_t length);

/* return first index of found key in sorted sequence */
PROTECTED(const uint32_t)			lower_bound(
										const uint32_t *src, const uint32_t length,
										const uint32_t key);

/* ------------------------------------------------------------------------- */

/* Open an "add transaction" */
PUBLIC(const int)					vertices_prepare_build();

/*
 * Adds new vertex into the list.
 * May be called varios times only after vertices_prepare_build
 */
PUBLIC(const int)					vertices_add(
										const int32_t x, const int32_t y);

/* Move all active vertices by offset */
PUBLIC(const ERROR)					vertices_move(
										const int32_t x, const int32_t y);

/* Adds 4 default vertices which covers whole layer */
PUBLIC(const ERROR)					vertices_build_default();

/*
 * Finishes build vertices;
 * Should be called when all required vertices are added.
 * This will also build up clip indexes
 */
PUBLIC(const ERROR)					vertices_finish_build();

PUBLIC(const uint32_t)				vertices_get_count();

PUBLIC(const uint32_t)				vertices_get_max_count();

/* Provides the outer rectangle of currently built shape: */
PUBLIC(const uint32_t)				vertices_get_x();
PUBLIC(const uint32_t)				vertices_get_y();
PUBLIC(const uint32_t)				vertices_get_width();
PUBLIC(const uint32_t)				vertices_get_height();

PUBLIC(VECTOR*)						vertices_get();

PROTECTED(void)						vertices_alloc();

/* ------------------------------------------------------------------------- */

#endif /* API_H */
