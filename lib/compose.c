/*
 * Author: Oleksandr Smorchkov
 *
 * For all needed information refer to https://www.cairographics.org/operators/
 */

#include "api.h"


#define BLEND_CH(N,op)				\
	dest_ch = I_2_F(dest.CH_##N);	\
	src_ch = I_2_F(src.CH_##N);		\
	dest.CH_##N = F_2_CH((opt2 * src_ch + opt1 * dest_ch + opt3 * blend_##op(dest_ch, src_ch)) / res_a);

#define BUILD_BLEND(op)														\
	PRIVATE(const uint32_t) cmps_##op(COLOR32 dest, const COLOR32 src) {	\
		float dest_ch = I_2_F(dest.CH_A) / 255.0f;							\
		float src_ch = I_2_F(src.CH_A) / 255.0f;							\
		const float opt1 = dest_ch*(1.0f-src_ch);							\
		const float opt2 = src_ch*(1.0f-dest_ch);							\
		const float opt3 = src_ch*dest_ch;									\
		const float res_a = src_ch+opt1;									\
		BLEND_CH(R,op)														\
		BLEND_CH(G,op)														\
		BLEND_CH(B,op)														\
		dest.CH_A = F_2_CH(res_a * 255.0f);									\
		return dest.raw_data;												\
	}

#define BUILD_NON_SEP_BLEND(op)												\
	PRIVATE(const uint32_t) cmps_##op(COLOR32 dest, const COLOR32 src) {	\
		return ((void)blend_##op(0.0f,0.0f), 0x00000000);					\
	}

/*
#define SET_LUM(r,g,b)		\
	D = L - lum(r, g, b);	\
	res_r = r + D;			\
	res_g = g + D;			\
	res_b = b + D;

#define SET_SAT(r,g,b) \

	return [
        "rgb[0]=", r, ";",
        "rgb[1]=", g, ";",
        "rgb[2]=", b, ";",
        "maxI=0;",
        "if(rgb[1]>rgb[maxI])maxI=1;",
        "if(rgb[2]>rgb[maxI])maxI=2;",
        "minI=0;",
        "if(rgb[1]<rgb[minI])minI=1;",
        "if(rgb[2]<rgb[minI])minI=2;",
        "if(maxI===minI){rgb[0]=rgb[1]=rgb[2]=0.0;}",
        "else{",
        "minEl=rgb[minI];",
        "midI=3-(maxI+minI);",
        "rgb[minI]=.0;",
        "rgb[midI]=((rgb[midI]-minEl)*S)/(rgb[maxI]-minEl);",
        "rgb[maxI]=S;",
        "}",
        r, "=rgb[0];",
        g, "=rgb[1];",
        b, "=rgb[2];"
    ].join("");

#define MAX3(a,b,c) MAX(MAX((a), (b)), (c))
#define MIN3(a,b,c) MIN(MIN((a), (b)), (c))

static INLINE float sat(r, g, b)
{
	return MAX3(r, g, b) - MIN3(r, g, b);
}

static INLINE float lum(r, g, b)
{
	return 0.3f * r + 0.59f * g + 0.11f * b;
}
*/


PRIVATE(uint32_t) prev_dest, prev_src, prev_result;

PRIVATE(COMPOSITE_OPERATION) cmps_op_id;

const uint32_t (*cmps_ptr)(COLOR32, const COLOR32);


PRIVATE(INLINE const uint8_t) F_2_CH(const float v)
{
	const uint8_t tmp = F_2_I8(v);
	return MIN(0xFF, tmp);
}

/* ------------------------------------------------------------------------- */
/* Separable blend-modes: */

PRIVATE(INLINE const float)	blend_multiply(float dest_ch, float src_ch)
{
	return (dest_ch * src_ch) / 255.0f;
}

PRIVATE(INLINE const float)	blend_screen(float dest_ch, float src_ch)
{
	return 255.0f - ((255.0f - dest_ch) * (255.0f - src_ch)) / 255.0f;
}

PRIVATE(INLINE const float)	blend_overlay(float dest_ch, float src_ch)
{
	return (dest_ch < 128.0f
		? (2.0f * src_ch * dest_ch / 255.0f)
		: (255.0f - 2.0f * (255.0f - src_ch) * (255.0f - dest_ch) / 255.0f));
}

PRIVATE(INLINE const float)	blend_darken(float dest_ch, float src_ch)
{
	return MIN(dest_ch, src_ch);
}

PRIVATE(INLINE const float)	blend_lighten(float dest_ch, float src_ch)
{
	return MAX(dest_ch, src_ch);
}

PRIVATE(INLINE const float)	blend_color_dodge(float dest_ch, float src_ch)
{
	float tmp = (dest_ch * 255.0f) / ((255.0f - src_ch) + 1.0f);
	return MIN(255.0f, tmp);
}

PRIVATE(INLINE const float)	blend_color_burn(float dest_ch, float src_ch)
{
	float tmp = 255.0f - (((255.0f - dest_ch) * 255.0f) / (src_ch + 1.0f));
	return MAX(0.0f, tmp);
}

PRIVATE(INLINE const float)	blend_hard_light(float dest_ch, float src_ch)
{
	return (src_ch > 128.0f
		? 255.0f - (((255.0f - 2.0f * (src_ch - 128.0f)) * (255.0f - dest_ch)) / 255.0f)
		: (dest_ch * src_ch) / 128.0f);
}

PRIVATE(INLINE const float)	blend_soft_light(float dest_ch, float src_ch)
{
	float opt1 = src_ch * 2.0f;
	float opt2 = 255.0f - opt1;
	return (src_ch < 128.0f
		? (dest_ch * (opt1 + (dest_ch * opt2 / 256.0f))) / 256.0f
		: (dest_ch * (511.0f - opt1) + (sqrtf(dest_ch * 256.0f) * opt2)) / 256.0f);
}

PRIVATE(INLINE const float)	blend_difference(float dest_ch, float src_ch)
{
	return fabs(dest_ch - src_ch);
}

PRIVATE(INLINE const float)	blend_exclusion(float dest_ch, float src_ch)
{
	return dest_ch + src_ch - (2.0f * dest_ch * src_ch) / 255.0f;
}

/* ------------------------------------------------------------------------- */
/* Non-separable blend modes */

PRIVATE(INLINE const float)	blend_hue(float dest_ch, float src_ch)
{
	return 0.0f;
}

PRIVATE(INLINE const float)	blend_saturation(float dest_ch, float src_ch)
{
	return 0.0f;
}

PRIVATE(INLINE const float)	blend_color(float dest_ch, float src_ch)
{
	return 0.0f;
}

PRIVATE(INLINE const float)	blend_luminosity(float dest_ch, float src_ch)
{
	return 0.0f;
}

/* ------------------------------------------------------------------------- */
/* Composite operations */

PRIVATE(const uint32_t)		cmps_copy(COLOR32 dest, const COLOR32 src)
{
	return src.raw_data;
}

PRIVATE(const uint32_t)		cmps_source_over(COLOR32 dest, const COLOR32 src)
{
	float opt, res_a, src_a;
	if (0x00 == dest.CH_A || 0xFF == src.CH_A)
	{
		return src.raw_data;
	}
	src_a = I_2_F(src.CH_A) / 255.0f;
	opt = (I_2_F(dest.CH_A) / 255.0f) * (1.0f - src_a);
	res_a = src_a + opt;
	dest.CH_R = F_2_CH((I_2_F(src.CH_R) * src_a + I_2_F(dest.CH_R) * opt) / res_a);
	dest.CH_G = F_2_CH((I_2_F(src.CH_G) * src_a + I_2_F(dest.CH_G) * opt) / res_a);
	dest.CH_B = F_2_CH((I_2_F(src.CH_B) * src_a + I_2_F(dest.CH_B) * opt) / res_a);
	dest.CH_A = F_2_CH(res_a * 255.0f);
	return dest.raw_data;
}

PRIVATE(const uint32_t)		cmps_source_in(COLOR32 dest, const COLOR32 src)
{
	if (0x00 == dest.CH_A)
	{
		return 0x00000000;
	}
	dest.CH_R = src.CH_R;
	dest.CH_G = src.CH_G;
	dest.CH_B = src.CH_B;
	dest.CH_A = F_2_CH((I_2_F(dest.CH_A) / 255.0f * I_2_F(src.CH_A) / 255.0f) * 255.0f);
	return dest.raw_data;
}

PRIVATE(const uint32_t)		cmps_source_out(COLOR32 dest, const COLOR32 src)
{
	if (0x00 == dest.CH_A)
	{
		return 0x00000000;
	}
	dest.CH_R = src.CH_R;
	dest.CH_G = src.CH_G;
	dest.CH_B = src.CH_B;
	dest.CH_A = F_2_CH((I_2_F(src.CH_A) / 255.0f * (1.0f - I_2_F(dest.CH_A) / 255.0f)) * 255.0f);
	return dest.raw_data;
}

PRIVATE(const uint32_t)		cmps_source_atop(COLOR32 dest, const COLOR32 src)
{
	float opt1, src_a;
	if (0x00 == dest.CH_A)
	{
		return 0x00000000;
	}
	src_a = I_2_F(src.CH_A) / 255.0f;
	opt1 = 1.0f - src_a;
	dest.CH_R = F_2_CH(I_2_F(src.CH_R) * src_a + I_2_F(dest.CH_R) * opt1);
	dest.CH_G = F_2_CH(I_2_F(src.CH_G) * src_a + I_2_F(dest.CH_G) * opt1);
	dest.CH_B = F_2_CH(I_2_F(src.CH_B) * src_a + I_2_F(dest.CH_B) * opt1);
	/*dest.CH_A = dest.CH_A;*/
	return dest.raw_data;
}

PRIVATE(const uint32_t)		cmps_destination_over(COLOR32 dest, const COLOR32 src)
{
	float opt1, res_a, dest_a;
	if (0xFF != dest.CH_A && 0x00 != src.CH_A)
	{
		dest_a = I_2_F(dest.CH_A) / 255.0f;
		opt1 = (1.0f - dest_a) * (I_2_F(src.CH_A) / 255.0f);
		res_a = opt1 + dest_a;
		dest.CH_R = F_2_CH((I_2_F(src.CH_R) * opt1 + I_2_F(dest.CH_R) * dest_a) / res_a);
		dest.CH_G = F_2_CH((I_2_F(src.CH_G) * opt1 + I_2_F(dest.CH_G) * dest_a) / res_a);
		dest.CH_B = F_2_CH((I_2_F(src.CH_B) * opt1 + I_2_F(dest.CH_B) * dest_a) / res_a);
		dest.CH_A = F_2_CH(res_a * 255.0f);
	}
	return dest.raw_data;
}

PRIVATE(const uint32_t)		cmps_destination_in(COLOR32 dest, const COLOR32 src)
{
	if (0x00 == src.CH_A)
	{
		return 0x00000000;
	}
	dest.CH_A = F_2_I8(I_2_F(dest.CH_A * src.CH_A) / 255.0f);
	return dest.raw_data;
}

PRIVATE(const uint32_t)		cmps_destination_out(COLOR32 dest, const COLOR32 src)
{
	if (0xFF == src.CH_A)
	{
		dest.raw_data = 0x00000000;
	}
	dest.CH_A = F_2_I8((I_2_F(dest.CH_A) / 255.0f * (1.0f - I_2_F(src.CH_A) / 255.0f)) * 255.0f);
	return dest.raw_data;
}

PRIVATE(const uint32_t)		cmps_destination_atop(COLOR32 dest, const COLOR32 src)
{
	float opt1, dest_a;
	if (0x00 == src.CH_A)
	{
		return 0x00000000;
	}
	dest_a = I_2_F(dest.CH_A) / 255.0f;
	opt1 = 1.0f - dest_a;
	dest.CH_R = F_2_CH(I_2_F(src.CH_R) * opt1 + I_2_F(dest.CH_R) * dest_a);
	dest.CH_G = F_2_CH(I_2_F(src.CH_G) * opt1 + I_2_F(dest.CH_G) * dest_a);
	dest.CH_B = F_2_CH(I_2_F(src.CH_B) * opt1 + I_2_F(dest.CH_B) * dest_a);
	dest.CH_A = src.CH_A;
	return dest.raw_data;
}

PRIVATE(const uint32_t)		cmps_lighter(COLOR32 dest, const COLOR32 src)
{
	float dest_a, src_a, res_a;
	if (0x00 == dest.CH_A && 0x00 == src.CH_A)
	{
		return 0x00000000;
	}
	dest_a = I_2_F(dest.CH_A) / 255.0f;
	src_a = I_2_F(src.CH_A) / 255.0f;
	res_a = src_a + dest_a;
	res_a = MIN(1.0f, res_a);
	dest.CH_R = F_2_CH((I_2_F(src.CH_R) * src_a + I_2_F(dest.CH_R) * dest_a) / res_a);
	dest.CH_G = F_2_CH((I_2_F(src.CH_G) * src_a + I_2_F(dest.CH_G) * dest_a) / res_a);
	dest.CH_B = F_2_CH((I_2_F(src.CH_B) * src_a + I_2_F(dest.CH_B) * dest_a) / res_a);
	dest.CH_A = F_2_I8(res_a * 255.0f);
	return dest.raw_data;
}

PRIVATE(const uint32_t)		cmps_xor(COLOR32 dest, const COLOR32 src)
{
	float opt1, opt2;
	float dest_a = I_2_F(dest.CH_A) / 255.0f;
	float src_a = I_2_F(src.CH_A) / 255.0f;
	float res_a = src_a + dest_a - 2.0f * src_a * dest_a;
	if (0.0f == res_a)
	{
		return 0x00000000;
	}
	opt1 = src_a * (1.0f - dest_a);
	opt2 = dest_a * (1.0f - src_a);
	dest.CH_R = F_2_CH((I_2_F(src.CH_R) * opt1 + I_2_F(dest.CH_R) * opt2) / res_a);
	dest.CH_G = F_2_CH((I_2_F(src.CH_G) * opt1 + I_2_F(dest.CH_G) * opt2) / res_a);
	dest.CH_B = F_2_CH((I_2_F(src.CH_B) * opt1 + I_2_F(dest.CH_B) * opt2) / res_a);
	dest.CH_A = F_2_I8(res_a * 255.0f);
	return dest.raw_data;
}

BUILD_BLEND(multiply)
BUILD_BLEND(screen)
BUILD_BLEND(overlay)
BUILD_BLEND(darken)
BUILD_BLEND(lighten)
BUILD_BLEND(color_dodge)
BUILD_BLEND(color_burn)
BUILD_BLEND(hard_light)
BUILD_BLEND(soft_light)
BUILD_BLEND(difference)
BUILD_BLEND(exclusion)

BUILD_NON_SEP_BLEND(hue)
BUILD_NON_SEP_BLEND(saturation)
BUILD_NON_SEP_BLEND(color)
BUILD_NON_SEP_BLEND(luminosity)

/* ------------------------------------------------------------------------- */

const COMPOSITE_OPERATION compose_get_operation()
{
	return cmps_op_id;
}

void compose_set_operation(const uint32_t op)
{
	cmps_op_id = (COMPOSITE_OPERATION)op;
	switch (cmps_op_id)
	{
	case COPY:				cmps_ptr = cmps_copy;				break;
	case SOURCE_OVER:		cmps_ptr = cmps_source_over;		break;
	case SOURCE_IN:			cmps_ptr = cmps_source_in;			break;
	case SOURCE_OUT:		cmps_ptr = cmps_source_out;			break;
	case SOURCE_ATOP:		cmps_ptr = cmps_source_atop;		break;
	case DESTINATION_OVER:	cmps_ptr = cmps_destination_over;	break;
	case DESTINATION_IN:	cmps_ptr = cmps_destination_in;		break;
	case DESTINATION_OUT:	cmps_ptr = cmps_destination_out;	break;
	case DESTINATION_ATOP:	cmps_ptr = cmps_destination_atop;	break;
	case LIGHTER:			cmps_ptr = cmps_lighter;			break;
	case XOR:				cmps_ptr = cmps_xor;				break;
	case MULTIPLY:			cmps_ptr = cmps_multiply;			break;
	case SCREEN:			cmps_ptr = cmps_screen;				break;
	case OVERLAY:			cmps_ptr = cmps_overlay;			break;
	case DARKEN:			cmps_ptr = cmps_darken;				break;
	case LIGHTEN:			cmps_ptr = cmps_lighten;			break;
	case COLOR_DODGE:		cmps_ptr = cmps_color_dodge;		break;
	case COLOR_BURN:		cmps_ptr = cmps_color_burn;			break;
	case HARD_LIGHT:		cmps_ptr = cmps_hard_light;			break;
	case SOFT_LIGHT:		cmps_ptr = cmps_soft_light;			break;
	case DIFFERENCE:		cmps_ptr = cmps_difference;			break;
	case EXCLUSION:			cmps_ptr = cmps_exclusion;			break;
	case HUE:				cmps_ptr = cmps_hue;				break;
	case SATURATION:		cmps_ptr = cmps_saturation;			break;
	case COLOR:				cmps_ptr = cmps_color;				break;
	case LUMINOSITY:		cmps_ptr = cmps_luminosity;			break;
	default:				cmps_ptr = cmps_copy;				break;
	}
	prev_dest = 0x00000000;
	prev_src = 0x00000000;
	prev_result = cmps_ptr((COLOR32){ .raw_data=prev_dest }, (COLOR32){ .raw_data=prev_src });
}

const uint32_t compose_colors(const uint32_t dest, const uint32_t src)
{
	if (dest == prev_dest && src == prev_src)
	{
		return prev_result;
	}
	return (prev_result=cmps_ptr(
		(COLOR32){ .raw_data=(prev_dest=dest) },
		(COLOR32){ .raw_data=(prev_src=src) }));
}

void compose(uint32_t *begin, const uint32_t *end, const uint32_t *src)
{
	while (begin < end)
	{
		*begin = compose_colors(*begin, *src++);
		++begin;
	}
}
