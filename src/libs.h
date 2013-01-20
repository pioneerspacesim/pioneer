// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LIBS_H
#define _LIBS_H

#include "buildopts.h"

#include <cassert>
#include <cstdio>
#include <sigc++/sigc++.h>
#include <SDL.h>
#include <GL/glew.h>
#include <SDL_image.h>
#include <cfloat>
#include <limits>
#include <ctime>
#include <cstdarg>
#include <cstdlib>
#include <cerrno>
#include <string>
#include <vector>
#include <map>

/* on unix this would probably become $PREFIX/pioneer */
#ifndef PIONEER_DATA_DIR
#define PIONEER_DATA_DIR "data"
#endif /* PIONEER_DATA_DIR */

#ifdef _WIN32
#	include <malloc.h>

#	ifdef _MSC_VER
#		pragma warning(disable : 4244) // "conversion from x to x: possible loss of data"
#		pragma warning(disable : 4800) // int-to-bool "performance warning"
#		pragma warning(disable : 4355) // 'this' used in base member initializer list
#		pragma warning(disable : 4351) // new behavior [after vs2003!]: elements of array 'array' will be default initialized
#	endif

#	ifndef __MINGW32__
#
#		define alloca _alloca
#		define strncasecmp _strnicmp
#		define strcasecmp _stricmp
#		define snprintf _snprintf

#		ifndef isfinite
inline int isfinite(double x) { return _finite(x); }
#		endif
#	endif /* __MINGW32__ */
#endif

#include "fixed.h"
#include "vector2.h"
#include "vector3.h"
#include "Aabb.h"
#include "matrix3x3.h"
#include "matrix4x4.h"
#include "Color.h"
#include "mtrand.h"

#include "FloatComparison.h"
#include "SmartPtr.h"
#include "RefCounted.h"

#ifdef NDEBUG
#define	PiVerify(x) ((void)(x))
#else
#define PiVerify(x) assert(x)
#endif

#define UNIVERSE_SEED	0xabcd1234

#define EARTH_RADIUS	6378135.0
#define EARTH_MASS	5.9742e24
#define JUPITER_MASS	(317.8*EARTH_MASS)
// brown dwarfs above 13 jupiter masses fuse deuterium
#define MIN_BROWN_DWARF	(13.0*JUPITER_MASS)
#define SOL_RADIUS	6.955e8
#define SOL_MASS	1.98892e30
#define AU		149598000000.0
#define G		6.67428e-11
#define GAS_CONSTANT_R 8.3144621
#define EARTH_ATMOSPHERE_SURFACE_DENSITY 1.225

#define HUD_ALPHA 0.34f

template<class T> inline const T& Clamp(const T& x, const T& min, const T& max) { return x > max ? max : (x < min ? min : x); }

#define DEG_2_RAD	0.0174532925
inline double DEG2RAD(double x) { return x*(M_PI/180.); }
inline float  DEG2RAD(float  x) { return x*(float(M_PI)/180.f); }

// from StackOverflow: http://stackoverflow.com/a/1500517/52251
// Q: "Compile time sizeof_array without using a macro"
template <typename T, size_t N>
char ( &COUNTOF_Helper( T (&array)[N] ))[N];
#define COUNTOF( array ) (sizeof( COUNTOF_Helper( array ) ))

#endif /* _LIBS_H */
