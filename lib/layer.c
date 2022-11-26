/**
 * Author: Oleksandr Smorchkov
 *
 * Manages layer memory, size and mode
 */

#include "api.h"


/* Persistent layers */
PRIVATE(uint32_t*) extra;
PRIVATE(uint32_t*) layers_base;

PRIVATE(uint16_t) width;
PRIVATE(uint16_t) height;
PRIVATE(uint16_t) max_side_size;
PRIVATE(uint16_t) layers_count;


void layers_alloc(const uint16_t max_side, const int use_single_layer)
{
	const uint32_t layer_mem_size = max_side * max_side * 4;
	layers_count = use_single_layer ? 1 : 12;

	extra = (uint32_t*)malloc(layer_mem_size);

	layers_base = (uint32_t*)malloc(layer_mem_size * layers_count);
	max_side_size = max_side;
}

uint32_t *layers_get_extra()
{
	return extra;
}

uint32_t *layers_get()
{
	return layers_base;
}

const uint32_t layers_get_max_side_size()
{
	return (uint32_t)max_side_size;
}

const uint32_t layers_get_max_layers_count()
{
	return (uint32_t)layers_count;
}

const uint32_t layers_get_width()
{
	return (uint32_t)width;
}

const uint32_t layers_get_height()
{
	return (uint32_t)height;
}

const uint32_t layers_get_size()
{
	return (uint32_t)(width * height);
}

void layers_resize(const uint32_t w, const uint32_t h)
{
	width = (uint16_t)MAX(w, 1);
	height = (uint16_t)MAX(h, 1);
}
