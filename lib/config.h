/**
 * Author: Oleksandr Smorchkov
 */

#ifndef CONFIG_H
#define CONFIG_H


#ifdef __EMSCRIPTEN__
	/* EMCC tools on top of clang */

	/* 
		WARN: there was problems with newest versions of EMCC
		(exported memory has been not shared between js and wasm)
		So the last version wich works fine for our needs was sdk-1.38.45-64bit
	*/

	#include <emscripten.h>
	#define ATTR EMSCRIPTEN_KEEPALIVE

	/* EMCC exports all the extern functions */
	#define EXPORT extern
	#define EXTERN extern
#else
	/* Raw clang */

	#define ATTR

	#define EXPORT __attribute__ ((visibility("default")))
	#define EXTERN extern
#endif /* EMSCRIPTEN */

#define INLINE inline

#define PUBLIC(rettype) EXPORT rettype ATTR

#define PROTECTED(rettype) EXTERN rettype ATTR

#define PRIVATE(rettype) static rettype


#endif /* CONFIG_H */
