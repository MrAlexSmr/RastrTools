/**
 * Author: Oleksandr Smorchkov
 *
 * Holds clip indexes.
 */

#include "api.h"


/* pairs of [begin,end) indexes. */
PRIVATE(uint32_t*)	clip_indexes;
PRIVATE(uint32_t)	clip_indexes_count;
PRIVATE(uint32_t)	clip_indexes_max_count;


void clip_indexes_alloc(const uint32_t bytes)
{
	clip_indexes = (uint32_t*)malloc(bytes);
	clip_indexes_max_count = bytes / sizeof(uint32_t);
	clip_indexes_reset();
}

const uint32_t *clip_indexes_get()
{
	return clip_indexes;
}

const uint32_t clip_indexes_get_count()
{
	return clip_indexes_count;
}

void clip_indexes_reset()
{
	clip_indexes_count = 0;
}

const int clip_indexes_push_pair(const uint32_t begin, const uint32_t end)
{
	if (clip_indexes_count < clip_indexes_max_count)
	{
		clip_indexes[clip_indexes_count++] = begin;
		clip_indexes[clip_indexes_count++] = end;
		return 1;
	}
	return 0;
}

/*
 * Return found begin index of [begin;end) pair,
 * which corresponds to "index" argument,
 * index >= begin || index < end
 * [ 2,6,  8,10,  14,20 ] .. looking for 15 .. the result is 4(index of 14)
 */
const int32_t clip_indexes_get_nearest_pair_index(const uint32_t index)
{
	uint32_t mid, pair_ind, l, r, begin, end;
	if (clip_indexes_count > 0)
	{
		l = 0;
		r = clip_indexes_get_count() >> 1;
		while (l <= r)
		{
			mid = l + ((r - l) >> 1);

			pair_ind = mid + mid;
			if (pair_ind >= clip_indexes_count) break;

			begin = clip_indexes[pair_ind];
			end = clip_indexes[pair_ind + 1];
			if (index >= begin && index < end)
			{
				return pair_ind;
			}

			if (begin < index)
			{
				l = mid + 1;
			}
			else
			{
				r = mid - 1;
			}
		}
	}
	return -1;
}
