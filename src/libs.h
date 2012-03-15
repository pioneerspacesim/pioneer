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

/* on unix this would probably become $PREFIX/pioneer */
#ifndef PIONEER_DATA_DIR
#define PIONEER_DATA_DIR "data"
#endif /* PIONEER_DATA_DIR */

#ifdef _WIN32
#	include <malloc.h>

#	ifdef _MSC_VER
#		pragma warning(disable : 4244) // "conversion from x to x: possible loss of data"
#		pragma warning(disable : 4800) // int-to-bool "performance warning"
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

#define HUD_ALPHA 0.34f

template<class T> inline const T& Clamp(const T& x, const T& min, const T& max) { return x > max ? max : (x < min ? min : x); }

#define DEG_2_RAD	0.0174532925
#define DEG2RAD(x) ((x)*M_PI/180.0)

#endif /* _LIBS_H */
