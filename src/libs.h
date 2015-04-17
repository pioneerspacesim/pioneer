// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LIBS_H
#define _LIBS_H

#include "buildopts.h"

#include <cassert>
#include <cstdio>
#include <sigc++/sigc++.h>
#include <SDL.h>
#include <SDL_image.h>
#include <cfloat>
#include <limits>
#include <ctime>
#include <cstdarg>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <string>
#include <deque>
#include <vector>
#include <map>
#include <algorithm>
#include <memory>

// The glLoadGen header was generated using the following command line:
// lua LoadGen.lua -style=pointer_c -spec=gl -version=3.3 -profile=compatibility core_3_x -stdext=gl_ubiquitous.txt -stdext=gl_core_post_3_3.txt -ext ARB_seamless_cube_map ARB_seamless_cubemap_per_texture ARB_draw_instanced ARB_uniform_buffer_object ARB_instanced_arrays
#include "graphics/opengl/gl_core_3_x.h"

#ifdef _WIN32
#	include <malloc.h>

#	ifdef _MSC_VER
#		pragma warning(disable : 4244) // "conversion from x to x: possible loss of data"
#		pragma warning(disable : 4800) // int-to-bool "performance warning"
#		pragma warning(disable : 4355) // 'this' used in base member initializer list
#		pragma warning(disable : 4351) // new behavior [after vs2003!]: elements of array 'array' will be default initialized
#	endif

#	ifndef __MINGW32__
#		define alloca _alloca
#		define strncasecmp _strnicmp
#		define strcasecmp _stricmp
#		define snprintf _snprintf
#	endif
#endif

#ifdef _WIN32 // MSVC doesn't support the %z specifier, but has its own %I specifier
#	ifndef __MINGW32__
#	define SIZET_FMT "%Iu"
#	else
#	define SIZET_FMT "%zu"
#	endif
#else
#define SIZET_FMT "%zu"
#endif

#include "fixed.h"
#include "vector2.h"
#include "vector3.h"
#include "Aabb.h"
#include "matrix3x3.h"
#include "matrix4x4.h"
#include "Color.h"
#include "Random.h"

#include "FloatComparison.h"
#include "SmartPtr.h"
#include "RefCounted.h"

#include "profiler/Profiler.h"

#ifdef NDEBUG
#define	PiVerify(x) ((void)(x))
#else
#define PiVerify(x) assert(x)
#endif

template<class T> inline const T& Clamp(const T& x, const T& min, const T& max) { return x > max ? max : (x < min ? min : x); }

inline double DEG2RAD(double x) { return x*(M_PI/180.); }
inline float  DEG2RAD(float  x) { return x*(float(M_PI)/180.f); }
inline double RAD2DEG(double x) { return x*(180./M_PI); }
inline float  RAD2DEG(float  x) { return x*(180.f/float(M_PI)); }

// from StackOverflow: http://stackoverflow.com/a/1500517/52251
// Q: "Compile time sizeof_array without using a macro"
template <typename T, size_t N>
char ( &COUNTOF_Helper( T (&array)[N] ))[N];
#define COUNTOF( array ) (sizeof( COUNTOF_Helper( array ) ))

#ifdef _WIN32
// Why oh why!
// MSVC thinks near and far are actually supposed to be used with pointers
// but because they are no longer used, it consequently causes an error
#undef near
#define near ex_near
#undef far
#define far ex_far
#undef FAR
#define FAR
#undef NEAR
#define NEAR
#endif // _WIN32

#endif /* _LIBS_H */
