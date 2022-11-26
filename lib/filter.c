/**
 * Author: Oleksandr Smorchkov
 */

#include "api.h"


PRIVATE(INLINE void)	filter_apply_increase(
	COLOR32 *begin, const COLOR32 *end, const float value)
{
#define PROCESS_CH(CH) CH = F_2_I8(I_2_F(CH) * value)

	while (begin < end)
	{
		PROCESS_CH(begin->CH_R);
		PROCESS_CH(begin->CH_G);
		PROCESS_CH(begin->CH_B);
		++begin;
	}

#undef PROCESS_CH
}

PRIVATE(INLINE void)	filter_apply_increase_from_base(
	COLOR32 *begin, const COLOR32 *end, const float base, const float value)
{
#define PROCESS_CH(CH)							\
	tmp = F_2_I8(base + (I_2_F(CH) * value));	\
	CH = MIN(0xFF, tmp);

	uint8_t tmp;
	while (begin < end)
	{
		PROCESS_CH(begin->CH_R);
		PROCESS_CH(begin->CH_G);
		PROCESS_CH(begin->CH_B);
		++begin;
	}

#undef PROCESS_CH
}

PRIVATE(INLINE void)	filter_apply_matrix(
	COLOR32 *begin, const COLOR32 *end,
	const float v0, const float v1, const float v2,
	const float v5, const float v6, const float v7,
	const float v10, const float v11, const float v12)
{
#define PROCESS_CH(CH,val1,val2,val3)					\
	tmp = F_2_I8((val1 * r) + (val2 * g) + (val3 * b)); \
	CH = MIN(tmp, 0xFF);

	float r, g, b;
	uint8_t tmp;
	while (begin < end)
	{
		r = begin->CH_R;
		g = begin->CH_G;
		b = begin->CH_B;
		PROCESS_CH(begin->CH_R, v0, v1, v2);
		PROCESS_CH(begin->CH_G, v5, v6, v7);
		PROCESS_CH(begin->CH_B, v10, v11, v12);
		++begin;
	}

#undef PROCESS_CH
}

PRIVATE(INLINE void)	filter_apply_saturation(
	uint32_t *dest, const float value)
{
	const float v0 = 0.2126f + 0.7874f * value;
	const float v1 = 0.7152f - 0.7152f * value;
	const float v2 = 0.0722f - 0.0722f * value;
	const float v5 = 0.2126f - 0.2126f * value;
	const float v6 = 0.7152f + 0.2848f * value;
	const float v12= 0.0722f + 0.9278f * value;

	filter_custom_matrix(
		dest,
		v0, v1, v2,
		v5, v6, v2,
		v5, v1, v12);
}

PRIVATE(INLINE void)	filter_apply_gamma(
	COLOR32 *begin, const COLOR32 *end, const float value)
{
#define PROCESS_CH(CH) CH = F_2_I8(powf(I_2_F(CH) / 255.0f, value) * 255.0f)

	while (begin < end)
	{
		PROCESS_CH(begin->CH_R);
		PROCESS_CH(begin->CH_G);
		PROCESS_CH(begin->CH_B);
		++begin;
	}

#undef PROCESS_CH
}

/*
	https://www.w3.org/TR/filter-effects/#funcdef-brightness
	https://www.w3.org/TR/SVG/filters.html#feFuncRElement - @see linear
*/
PRIVATE(void)			filter_brightness(
	uint32_t *dest, const float amount)
{
	const float value = CLAMP(amount, 0.0f, 1.0f);
	FOR_EACH_LINE_USE_INDEXES({
		filter_apply_increase((COLOR32*)(dest + begin), (COLOR32*)(dest + end), value);
	});
}

/*
	https://www.w3.org/TR/filter-effects/#funcdef-contrast
	https://www.w3.org/TR/SVG/filters.html#feFuncRElement - @see linear
*/
PRIVATE(void)			filter_contrast(
	uint32_t *dest, const float amount)
{
	const float slope = CLAMP(amount, 0.0f, 1.0f);
	const float intercept = (-(0.5f * slope) + 0.5f) * 255.0f;
	FOR_EACH_LINE_USE_INDEXES({
		filter_apply_increase_from_base((COLOR32*)(dest + begin), (COLOR32*)(dest + end), intercept, slope);
	});
}

/*
	https://www.w3.org/TR/filter-effects/#funcdef-invert
	https://www.w3.org/TR/filter-effects/#invertEquivalent
	https://www.w3.org/TR/SVG/filters.html#feFuncRElement - @see table
 vk === value
 vk+1 === 1 - value
 n === 2 (according to spec there are two tableValues --> vk and vk+1)
 k === C or k === (C + 1)
 vk + (C - k/n)*n * (vk+1 - vk)
 Which may be (somehow) translated to:
 255 * vk + Channel * (vk+1 - vk)
*/
PRIVATE(void)			filter_invert(
	uint32_t *dest, const float amount)
{
	const float vk = CLAMP(amount, 0.0f, 1.0f);
	const float val = 255.0f * vk;
	const float diff = (1.0f - vk) - vk;
	FOR_EACH_LINE_USE_INDEXES({
		filter_apply_increase_from_base((COLOR32*)(dest + begin), (COLOR32*)(dest + end), val, diff);
	});
}

/*
	https://www.w3.org/TR/filter-effects/#sepiaEquivalent
	https://drafts.fxtf.org/filter-effects/#sepiaEquivalent
*/
PRIVATE(void)			filter_sepia(
	uint32_t *dest, const float amount)
{
	const float value = 1.0f - CLAMP(amount, 0.0f, 1.0f);
	const float v0 = 0.393f + 0.607f * value;
	const float v1 = 0.769f - 0.769f * value;
	const float v2 = 0.189f - 0.189f * value;
	const float v5 = 0.349f - 0.349f * value;
	const float v6 = 0.686f + 0.314f * value;
	const float v7 = 0.168f - 0.168f * value;
	const float v10 = 0.272f - 0.272f * value;
	const float v11 = 0.534f - 0.534f * value;
	const float v12 = 0.131f + 0.869f * value;

	filter_custom_matrix(
		dest,
		v0, v1, v2,
		v5, v6, v7,
		v10, v11, v12);
}

/*
	https://www.w3.org/TR/SVG/filters.html#feColorMatrixElement
*/
PRIVATE(void)			filter_saturate(
	uint32_t *dest, const float amount)
{
	filter_apply_saturation(dest, CLAMP(amount, 0.0f, 1.0f));
}

/*
	https://www.w3.org/TR/filter-effects/#grayscaleEquivalent
	https://drafts.fxtf.org/filter-effects/#grayscaleEquivalent
	Basically, it is a grayscale, the only difference is the "value"
*/
PRIVATE(void)			filter_grayscale(
	uint32_t *dest, const float amount)
{
	filter_apply_saturation(dest, 1.0f - CLAMP(amount, 0.0f, 1.0f));
}

/*
	https://www.w3.org/TR/SVG11/filters.html#feColorMatrixElement
*/
PRIVATE(void)			filter_hue_rotate(
	uint32_t *dest, const float angle)
{
#define ROT(v0,v1,v2) (v0 + (cos_a * v1) + (sin_a * v2))

	const float a = angle * 0.01745329252f;
	const float cos_a = cosf(a);
	const float sin_a = sinf(a);
	const float v0 = ROT(0.213f, 0.787f, -0.213f);
	const float v1 = ROT(0.715f, -0.715f, -0.715f);
	const float v2 = ROT(0.072f, -0.072f, 0.928f);
	const float v5 = ROT(0.213f, -0.213f, 0.143f);
	const float v6 = ROT(0.715f, 0.285f, 0.140f);
	const float v7 = ROT(0.072f, -0.072f, -0.283f);
	const float v10= ROT(0.213f, -0.213f, -0.787f);
	const float v11= ROT(0.715f, -0.715f, 0.715f);
	const float v12= ROT(0.072f, 0.928f, 0.072f);

	filter_custom_matrix(
		dest,
		v0, v1, v2,
		v5, v6, v7,
		v10, v11, v12);

#undef ROT
}

PRIVATE(void)			filter_gamma(
	uint32_t *dest, const float value)
{
	FOR_EACH_LINE_USE_INDEXES({
		filter_apply_gamma((COLOR32*)(dest + begin), (COLOR32*)(dest + end), value);
	});
}


void filter(uint32_t *layer, const uint32_t op, const float value)
{
	switch ((FILTER_OPERATION)op)
	{
	case FILTER_BRIGHTNESS:	filter_brightness(layer, value);	break;
	case FILTER_CONTRAST:	filter_contrast(layer, value);		break;
	case FILTER_INVERT:		filter_invert(layer, value);		break;
	case FILTER_SEPIA:		filter_sepia(layer, value);			break;
	case FILTER_SATURATE:	filter_saturate(layer, value);		break;
	case FILTER_GRAYSCALE:	filter_grayscale(layer, value);		break;
	case FILTER_HUE_ROTATE:	filter_hue_rotate(layer, value);	break;
	case FILTER_GAMMA:		filter_gamma(layer, value);			break;
	default: break;
	}
}

void filter_custom_matrix(
	uint32_t *layer,
	const float v0, const float v1, const float v2,
	const float v5, const float v6, const float v7,
	const float v10, const float v11, const float v12)
{
	FOR_EACH_LINE_USE_INDEXES({
		filter_apply_matrix((COLOR32*)(layer + begin), (COLOR32*)(layer + end),
			v0, v1, v2,
			v5, v6, v7,
			v10, v11, v12);
	});
}
