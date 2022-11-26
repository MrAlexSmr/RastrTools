/**
 * Author: Oleksandr Smorchkov
 *
 * Manages history pages
 */

#include "api.h"

#define HISTORY_PAGES_COUNT 2048


PRIVATE(uint32_t*) data_base;
PRIVATE(uint8_t*) pages;

PRIVATE(uint32_t) page_size;
PRIVATE(uint32_t) free_bytes_count;


/* First-fit algo */
PRIVATE(const HISTORY_PAGE_DATA) history_alloc_pages(const uint32_t bytes_to_occupy)
{
	uint32_t pages_count = 0;
	for (uint32_t i = 0; i < HISTORY_PAGES_COUNT; ++i)
	{
		if (0x00 == pages[i])
		{
			if (((++pages_count) * page_size) >= bytes_to_occupy)
			{
				return (HISTORY_PAGE_DATA){ .page=(i - pages_count + 1), .count=pages_count };
			}
		}
		else
		{
			pages_count = 0;
		}
	}
	return (HISTORY_PAGE_DATA){ .raw_data=0x00000000 };
}

const uint32_t history_save(const uint32_t *bytes, const uint32_t bytes_count)
{
	if (NULL != bytes && bytes_count > 0 && bytes_count < get_free_bytes())
	{
		const HISTORY_PAGE_DATA page_data = history_alloc_pages(bytes_count);
		if (0x00000000 != page_data.raw_data)
		{
			uint32_t *dest = get_history_data_at(page_data.raw_data);
			history_mark_pages(page_data.raw_data, 1);
			copy_in_range(dest, dest + bytes_count, bytes);
			return page_data.raw_data;
		}
	}
	return 0x00000000;
}

void history_mark_pages(const uint32_t page_data, const int flag)
{
	const HISTORY_PAGE_DATA page = (HISTORY_PAGE_DATA){ .raw_data=page_data };
	uint32_t page_begin = page.page;
	const uint32_t page_end = page_begin + page.count;
	const uint32_t acc = (flag == 0 ? page_size : -page_size);
	while (page_begin < page_end)
	{
		pages[page_begin++] = (uint8_t)flag;
		free_bytes_count += acc;
	}
}

uint32_t *get_history_data_at(const uint32_t page_data)
{
	const HISTORY_PAGE_DATA page = (HISTORY_PAGE_DATA){ .raw_data=page_data };
	return (data_base + page.page * page_size);
}

const uint32_t history_update_page_index(const uint32_t page_data, const uint16_t page)
{
	HISTORY_PAGE_DATA tmp = (HISTORY_PAGE_DATA){ .raw_data=page_data };
	tmp.page = page;
	return tmp.raw_data;
}

const uint32_t get_history_page_index(const uint32_t page_data)
{
	return (HISTORY_PAGE_DATA){ .raw_data=page_data }.page;
}

const uint32_t get_history_page_size(const uint32_t page_data)
{
	return (HISTORY_PAGE_DATA){ .raw_data=page_data }.count * page_size;
}

void history_mem_alloc()
{
	const uint32_t bytes = 11433984;
	page_size = (bytes - HISTORY_PAGES_COUNT) / HISTORY_PAGES_COUNT;
	free_bytes_count = HISTORY_PAGES_COUNT * page_size;

	pages = (uint8_t*)malloc(HISTORY_PAGES_COUNT);
	data_base = (uint32_t*)malloc(bytes - HISTORY_PAGES_COUNT);
}

const uint32_t get_free_bytes()
{
	return free_bytes_count;
}
