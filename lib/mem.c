/**
 * Author: Oleksandr Smorchkov
 */

#include "api.h"

/* Delay this call after all other allocations,
	since this memory may be not aligned and it is impossible to deref it in JS world,
	also, it should not be possible to defer this memory from somewhere else */
void mem_internal_init(const int use_single_layer)
{
	uint32_t max_side_pixels;

#if TOTAL_BYTES == 67108864
	max_side_pixels = use_single_layer ? 2484 : 1024;
#elif TOTAL_BYTES == 33554432
	max_side_pixels = use_single_layer ? 1562 : 644;
#endif /* TOTAL_BYTES */

	/* +2048 bytes of stack */

	/* Necessary steps
		prepare memory by allocating 4 bytes for NULL ptr */
	free(NULL);
	(void)malloc(sizeof(uint32_t));

	/* Modules that require static memory should come in first place: */
	palette_alloc();
	lzw_alloc();
	history_mem_alloc();
	vertices_alloc();

	/* For now, 11532766 bytes are occupied */


	/* Modules, which depends on layers max size should come after: */
	layers_alloc((uint16_t)max_side_pixels, use_single_layer);
	clip_indexes_alloc(max_side_pixels * max_side_pixels);
}
