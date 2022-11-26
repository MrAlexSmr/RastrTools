/**
 * Author: Oleksandr Smorchkov
 *
 * Pixel Art compression_compression utils.
 * Based on LZW encoding.
 */

#include "api.h"


/* performs no encoding on the layer (NOT IMPLEMENTED) */
#define LVL_0 0

/* will compression_compress the layer using rle algo (NOT IMPLEMENTED)
	and there is a doubt whether it is needed, since as result the algo may occupy more memory
*/
#define LVL_1 1

/* will compression_compress the layer using lzw algo */
#define LVL_2 2


PRIVATE(INLINE const uint8_t)	get_level(const uint32_t head)
{
	const uint32_t index_level = (head & 0xFF000000) >> 24;
	return (index_level & 0x0F);
}

PRIVATE(INLINE const uint32_t)	get_bytes_count(const uint32_t head)
{
	return (head & 0x00FFFFFF);
}

/* layer_index is an index of layer to  [0..16),
	size is compression_compressed bytes count, max 0x00FFFFFF,
	level compression_compression level [0..16) */
PRIVATE(INLINE const uint32_t)	make_head(const uint32_t bytes_count, const uint8_t level)
{
	return (uint32_t)(((((uint32_t)level) & 0x000000FF) << 24) | (bytes_count & 0x00FFFFFF));
}

PRIVATE(INLINE const ERROR)		decompress_lvl2(
	uint32_t *layer, const uint32_t *src, const uint32_t bytes_count)
{
	ERROR err = NO_ERROR;
	VECTOR xy;
	VECTOR size_wh;
	xy.raw_data = *src++;
	size_wh.raw_data = *src++;
	uint8_t *src_bytes = (uint8_t*)src;
	if (0 == xy.x && 0 == xy.y &&
		layers_get_width() == size_wh.width && layers_get_height() == size_wh.height)
	{
		(void)lzw_decode_bytes(
			(uint8_t*)layer, (const uint8_t*)(layer + (size_wh.width * size_wh.height)),
			(const uint8_t*)src_bytes, (const uint8_t*)(src_bytes + bytes_count),
			&err);

		/* Optimisation, in case size and offset are match the layer,
			decompress right into the dest layer.
			But, in case of error, it is bad situation,
			since some part of data has been written into the real dest layer :( */
		return err;
	}

	/* Else, decompress into tmp buffer (which may represent an image),
		and then put this tmp image into the dest layer */
	uint32_t *extra_begin = (uint32_t*)layers_get_extra();
	const uint32_t *extra_end = (uint32_t*)layers_get(0); /* layer base should beggins where extra ends */

	(void)lzw_decode_bytes(
		(uint8_t*)extra_begin, (const uint8_t*)extra_end,
		(const uint8_t*)src_bytes, (const uint8_t*)(src_bytes + bytes_count),
		&err);

	if (NO_ERROR != err) return err;

	if (0 == put_image(layer, extra_begin, xy.x, xy.y, size_wh.width, size_wh.height))
	{
		return DECOMPRESS_CORRUPTED_DATA;
	}
	return NO_ERROR;
}


/*const uint32_t compression_get_index(const uint32_t value)
{
	const uint32_t index_level = (value & 0xFF000000) >> 24;
	return ((index_level >> 4) & 0x0F);
}*/

/* compression format:
	HEAD+POSITION+SIZE+RAWDATA
	4bytes + 4bytes + 4bytes + data*/
const uint32_t compression_compress(
	uint32_t *dest_begin, const uint32_t *dest_end,
	const uint32_t *layer,
	const int compressWholeLayer)
{
	if (NULL == dest_begin || (dest_begin + 3 > dest_end))
	{
		return 0;
	}

	uint32_t compressed_size;
	uint32_t *head = dest_begin++;
	uint32_t *xy = dest_begin++;
	uint32_t *wh = dest_begin++;
	uint8_t *compres_begin = (uint8_t*)dest_begin;
	uint8_t *cursor = compres_begin;
	const uint8_t *dest_end8 = (const uint8_t*)dest_end;
	lzw_encode_bytes_prepare();
	if (1 == compressWholeLayer)
	{
		cursor = lzw_encode_bytes(
			cursor, dest_end8,
			(const uint8_t*)layer, (const uint8_t*)(layer + layers_get_size()));
		if (NULL == cursor) return 0;
	}
	else
	{
		/* Save inner rect of currently active shape */
		FOR_EACH_LINE_USE_CLIP_RECT({
			cursor = lzw_encode_bytes(
				cursor, dest_end8,
				(const uint8_t*)(layer + begin), (const uint8_t*)(layer + end));
			if (NULL == cursor) return 0;
		});
	}
	cursor = lzw_encode_bytes_finish(cursor, dest_end8);
	if (NULL == cursor) return 0;

	/* WARN: if cursor appears to be NULL for max possible layer,
		may only mean that the source layer is not pixel art, but a total mess of unrepeatable chars
		I hope this is highly impossible, but keep in mind that everything is possible */

	compressed_size = cursor - compres_begin;
	/* if (compressed_size >= (layers_get_size()<<2)) { THIS IS BAAAD } */
	*head = make_head(compressed_size, LVL_2);
	if (1 == compressWholeLayer)
	{
		*xy = (VECTOR){ .x=0, .y=0 }.raw_data;
		*wh = (VECTOR){
			.width=(uint16_t)layers_get_width(),
			.height=(uint16_t)layers_get_height()
		}.raw_data;
	}
	else
	{
		*xy = (VECTOR){ .x=vertices_get_x(), .y=vertices_get_y() }.raw_data;
		*wh = (VECTOR){
			.width=(uint16_t)vertices_get_width(),
			.height=(uint16_t)vertices_get_height()
		}.raw_data;
	}
	/* divide by 4 to convert compression_compressed 8bit data size to 32bit data size,
		1 to align in cases when int division is not even (not a big deal to waste 1 byte),
		3 is to count head, index and wh */
	return (compressed_size / 4) + 1 + 3;
}

const ERROR compression_decompress(const uint32_t *src, uint32_t *layer)
{
	uint32_t bytes_count, head;

	if (NULL == src)
	{
		return DECOMPRESS_CORRUPTED_DATA;
	}

	if (NULL == layer)
	{
		return DECOMPRESS_WRONG_HEADER;
	}

	head = *src++;
	bytes_count = get_bytes_count(head);
	if (0 == bytes_count/* || LIMIT < bytes_count*/)
	{
		return DECOMPRESS_OUT_OF_SCOPE;
	}

	switch (get_level(head))
	{
	case LVL_2: return decompress_lvl2(layer, src, bytes_count);
	default: return DECOMPRESS_WRONG_LEVEL;
	}
}
