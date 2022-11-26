/* Basic LZW Data Compression program published in DDJ October 1989 issue.
 * Original Author: Mark R. Nelson
 * http://collaboration.cmc.ec.gc.ca/science/rpn/biblio/ddj/Website/articles/DDJ/1989/8910/8910b/8910b.htm
 *
 * Updated by: Shawn M. Regan, January 1990
 *             http://mirror.bagelwood.com/textfiles/computers/regan.lst
 *             Changed:
 *             - Added method to clear table when compression_compression ratio degrades
 *             - Added self adjusting code size capability (up to 14 bits)
 *             Updated functions are marked with "MODIFIED". main() has been updated also
 *
 * Updated by: Daniel Marschall, 11 February 2018
 *             https://misc.daniel-marschall.de/code/c/lzw.c
 *             Changed:
 *             - Added includes stdint.h, stdlib. and string.h
 *             - Reordered functions and added/changed return types
 *               (Code is now C99+ compatible)
 *             - Changed "unsigned long" to "uint32_t"
 *               (otherwise the code won't work for 64 bit machines)
 *             - Made the static input/output buffer fields global, and added reset-functions
 *               reset_input_buffer() and reset_output_buffer().
 *               For some reason, the output buffers did not get correctly flushed for the next compression_compression
 *             - bytes_in, bytes_out and checkpoint now get automatically resetted in each compression_compress() run
 *             - Formatted the code (usage of tabulators)
 *
 * Compile with -ml (large model) for LZW_MAX_BITS == 14 only
 *
 * Updated by: Oleksandr Smorchkov, 29 February 2020
 *             Adapted for custom needs.
 *              - Removed main and io routines.
 *              - Added strict types size.
 *              - Added ability to "pause" and "resume" compression routines.
 */
#include "api.h"


#define INIT_BITS 9
#define HASHING_SHIFT (LZW_MAX_BITS - 8)

#if LZW_MAX_BITS == 14
	/* Set the table size. Must be a prime */
	/* number somewhat larger than 2^LZW_MAX_BITS. */
	#define TABLE_SIZE 18041
#elif LZW_MAX_BITS == 13
	#define TABLE_SIZE 9029
#else
	#define TABLE_SIZE 5021
#endif /* LZW_MAX_BITS */

/* Code to flush the string table */
#define CLEAR_TABLE 256

/* To mark EOF Condition, instead of MAX_VALUE */
#define TERMINATOR 257

/* First available code for code_value table */
#define FIRST_CODE 258

/* Check comp ratio every CHECK_TIME chars input */
#define CHECK_TIME 100

#define DECODE_STACK_SIZE 4000

/* max_value formula macro */
#define MAXVAL(n) ((1 << (n)) -1)

#define OUTPUT_CODE(code) \
	if ((output = output_code(output, output_end, (code))) == NULL) return NULL;


/* This is the code value array */
PRIVATE(int16_t*) code_value_base;
PRIVATE(int16_t*) code_value;

/* This array holds the prefix codes */
PRIVATE(uint16_t*) prefix_code_base;
PRIVATE(uint16_t*) prefix_code;

/* This array holds the appended chars */
PRIVATE(uint8_t*) append_character_base;
PRIVATE(uint8_t*) append_character;

/* This array holds the decoded string */
PRIVATE(uint8_t*) decode_stack_base;
PRIVATE(uint8_t*) decode_stack;

PRIVATE(uint32_t) output_bit_buffer;

/* For compression_compression ratio monitoring */
PRIVATE(uint32_t) checkpoint;

PRIVATE(uint32_t) input_bit_buffer;

/* Used to monitor compression_compression ratio */
PRIVATE(uint32_t) bytes_in;
PRIVATE(uint32_t) bytes_out;

/* Whether there is a first range to encode */
PRIVATE(int) is_first_encoded_range;

/* Starting with 9 bit codes */
PRIVATE(int16_t) num_bits;

/* old MAX_CODE */
PRIVATE(int16_t) max_code;

/* MODIFIED Output a variable length code. */
PRIVATE(int16_t) output_bit_count;

PRIVATE(uint16_t) string_code, next_code, ind;
PRIVATE(int16_t) ratio_old;

PRIVATE(int16_t) input_bit_count;


/* volatile is to prevent it to optimize to memset (which is absent in wasm) */
PRIVATE(void)					reset_code_value()
{
	volatile int16_t *ptr = code_value; /* volatile to prevent to be optimized by memset */
	const int16_t *end = code_value + TABLE_SIZE;
	while (ptr < end) *ptr++ = -1;
}

/* UNCHANGED from original
 * This is the hashing routine.
 */
PRIVATE(INLINE const uint16_t)	find_match(
	int16_t hash_prefix, uint16_t hash_character)
{
	int32_t index, offset;
	index = (hash_character << HASHING_SHIFT ) ^ hash_prefix;
	offset = (index == 0) ? 1 : TABLE_SIZE - index;
	while (1)
	{
		if (code_value[index] == -1)
		{
			return index;
		}
		if (prefix_code[index] == hash_prefix && append_character[index] == hash_character)
		{
			return index;
		}
		index -= offset;
		if (index < 0)
		{
			index += TABLE_SIZE;
		}
	}
}

PRIVATE(uint8_t*)				output_code(
	uint8_t *begin, const uint8_t *end, uint16_t code)
{
	output_bit_buffer |= (uint32_t)(code << (32 - num_bits - output_bit_count));
	output_bit_count += num_bits;
	while (output_bit_count >= 8)
	{
		if (begin >= end)
		{
			return NULL;
		}
		*begin++ = (output_bit_buffer >> 24);
		output_bit_buffer <<= 8;
		output_bit_count -= 8;
		/* ADDED for compression_compression monitoring */
		bytes_out++;
	}
	return begin;
}

PRIVATE(INLINE const uint8_t*)	input_code(
	uint16_t *result, const uint8_t *begin, const uint8_t *end)
{
	while (begin < end && input_bit_count <= 24)
	{
		input_bit_buffer |= (*begin++) << (24 - input_bit_count);
		input_bit_count += 8;
	}
	*result = input_bit_buffer >> (32 - num_bits);
	input_bit_buffer <<= num_bits;
	input_bit_count -= num_bits;
	return begin;
}

/* UNCHANGED from original
 * Decode a string from the string table, storing it in a buffer.
 * The buffer can then be output in reverse order by the expansion
 * program.
 */
PRIVATE(uint8_t*)				decode_string(
	uint8_t *buffer, uint16_t code)
{
	uint32_t i = 0;
	while (code > 255)
	{
		*buffer++ = append_character[code];
		code = prefix_code[code];
		if (i++ >= DECODE_STACK_SIZE)
		{
			return NULL;
		}
	}
	*buffer = code;
	return buffer;
}


void lzw_encode_bytes_prepare()
{
	is_first_encoded_range = 1;

	ratio_old = 100;

	append_character = append_character_base;
	prefix_code = prefix_code_base;
	code_value = code_value_base;

	bytes_in = 0;
	bytes_out = 0;
	output_bit_count = 0;
	output_bit_buffer = 0;
	checkpoint = CHECK_TIME;
	next_code = FIRST_CODE;

	num_bits = INIT_BITS;
	max_code = MAXVAL(num_bits);

	/* Initialize the string table first */
	reset_code_value();
}

/* MODIFIED This is the new compression_compression routine. The first two 9-bit codes
 * have been reserved for communication between the compression_compressor and expander.
 */
uint8_t* lzw_encode_bytes(uint8_t *output, const uint8_t *output_end, const uint8_t *begin, const uint8_t *end)
{
	uint16_t ch;
	int16_t ratio_new;

	if (1 == is_first_encoded_range)
	{
		/* Get the first code */
		string_code = (uint16_t)*begin++;
		is_first_encoded_range = 0;
	}

	/* This is the main compression_compression loop. Notice when the table is full we try
	 * to increment the code size. Only when num_bits == LZW_MAX_BITS and the code
	 * value table is full do we start to monitor the compression_compression ratio.
	 */
	while (begin < end)
	{
		ch = (uint16_t)*begin++;
		++bytes_in;
		ind = find_match(string_code, ch);
		if (code_value[ind] == -1)
		{
			if (next_code <= max_code)
			{
				code_value[ind] = next_code++;
				prefix_code[ind] = string_code;
				append_character[ind] = ch;
			}
			OUTPUT_CODE(string_code); /* Send out current code */
			string_code = ch;
			if (next_code > max_code) /* Is table Full? */
			{
				if (num_bits < LZW_MAX_BITS) /* Any more bits? */
				{
					/* Increment code size then */
					max_code = MAXVAL(++num_bits);
				}
				else if (bytes_in > checkpoint) /* At checkpoint? */
				{
					if (num_bits == LZW_MAX_BITS)
					{
						/* New compression_compression ratio */
						ratio_new = bytes_out * 100 / bytes_in;
						if (ratio_new > ratio_old) /* Has ratio degraded? */
						{
							OUTPUT_CODE(CLEAR_TABLE); /* YES,flush string table */
							num_bits = INIT_BITS;
							next_code = FIRST_CODE; /* Reset to FIRST_CODE */
							max_code = MAXVAL(num_bits); /* Re-Initialize this stuff */
							bytes_in = bytes_out = 0;
							ratio_old = 100; /* Reset compression_compression ratio */
							reset_code_value();
						}
						else
						{
							/* NO, then save new compression_compression ratio */
							ratio_old = ratio_new;
						}
					}
					/* Set new checkpoint */
					checkpoint = bytes_in + CHECK_TIME;
				}
			}
		}
		else
		{
			string_code = code_value[ind];
		}
	}
	return output;
}

uint8_t* lzw_encode_bytes_finish(uint8_t *output, const uint8_t *output_end)
{
	/* Output the last code */
	OUTPUT_CODE(string_code);
	if (next_code == max_code)
	{
		/* Handles special case for bit */
		/* increment on EOF */
		++num_bits;
	}
	/* Output the end of buffer code */
	OUTPUT_CODE(TERMINATOR);

	/* Flush the output buffer,
		I suppose, three following calls are here to prevent possible misaligments
		when cast back to uint32 from uint8: */
	OUTPUT_CODE(0);
	OUTPUT_CODE(0);
	OUTPUT_CODE(0);

	return output;
}

/* MODIFIED This is the modified expansion routine. It must now check for the
 * CLEAR_TABLE code and know when to increment the code size.
 */
uint8_t *lzw_decode_bytes(uint8_t *output, const uint8_t *output_end, const uint8_t *begin, const uint8_t *end, ERROR *error)
{
#define CHECK_OUT_OF_SCOPE \
	if (output >= output_end) { *error = LZW_DECODE_OUT_OF_SCOPE; return NULL; } 

	uint8_t *string;
	uint16_t next_code, new_code, old_code;
	int16_t ch, counter, clear_flag;

	append_character = append_character_base;
	prefix_code = prefix_code_base;
	decode_stack = decode_stack_base;

	counter = 0;
	clear_flag = 1; /* Need to clear the code value array */
	input_bit_count = 0;
	input_bit_buffer = 0;
	next_code = FIRST_CODE;

	num_bits = INIT_BITS;
	max_code = MAXVAL(num_bits);

	while (1)
	{
		begin = input_code(&new_code, begin, end);
		if (new_code == TERMINATOR)
		{
			break;
		}

		if (clear_flag)
		{
			/* Initialize or Re-Initialize */
			clear_flag = 0;
			old_code = new_code; /* The next three lines have been moved */
			ch = old_code; /* from the original */

			CHECK_OUT_OF_SCOPE

			*output++ = old_code;
			continue;
		}
		if (new_code == CLEAR_TABLE)
		{
			/* Clear string table */
			clear_flag = 1;
			num_bits = INIT_BITS;
			next_code = FIRST_CODE;
			max_code = MAXVAL(num_bits);
			continue;
		}
		if (++counter == 1000)
		{
			/* Pacifier */
			counter = 0;
		}
		if (new_code >= next_code)
		{
			/* Check for string+int8_t+string */
			*decode_stack = ch;
			string = decode_string(decode_stack + 1, old_code);
		}
		else
		{
			string = decode_string(decode_stack, new_code);
		}
		if (NULL == string)
		{
			/* Is this so bad that error should be fired?! */
			*error = LZW_DECODE_OUT_OF_SCOPE;
			return NULL;
		}

		/* Output decoded string in reverse */
		ch = *string;
		while (string >= decode_stack)
		{
			CHECK_OUT_OF_SCOPE

			*output++ = *string--;
		}

		if (next_code <= max_code)
		{
			/* Add to string table if not full */
			prefix_code[next_code] = old_code;
			append_character[next_code++] = ch;
			if (next_code == max_code && num_bits < LZW_MAX_BITS)
			{
				max_code = MAXVAL(++num_bits);
			}
		}
		old_code = new_code;
	}

	return output;

#undef CHECK_OUT_OF_SCOPE
}

void lzw_alloc()
{
	is_first_encoded_range = 0;
	// 18042 + (18041*2) + (18041*2)
	prefix_code_base = (uint16_t*)malloc(TABLE_SIZE * sizeof(uint16_t));
	code_value_base = (int16_t*)malloc(TABLE_SIZE * sizeof(int16_t));
	append_character_base = (uint8_t*)malloc(TABLE_SIZE * sizeof(uint8_t) + 1); // +1 to align to be even
	decode_stack_base = (uint8_t*)code_value_base; /* This array holds the decoded string */

	/* code_value_base and decode_stack_base share same memory,
		since they are not overlap during executing (one is for encode, other for decode) */
}
