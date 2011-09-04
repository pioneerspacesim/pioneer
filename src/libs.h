#ifndef _LIBS_H
#define _LIBS_H

#include "buildopts.h"

#include <assert.h>
#include <stdio.h>
#include <sigc++/sigc++.h>
#include <SDL.h>
#include <GL/glew.h>
#include <SDL_image.h>
#include <float.h>
#include <limits>
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>


/* on unix this would probably become $PREFIX/pioneer */
#ifndef PIONEER_DATA_DIR
#define PIONEER_DATA_DIR "data"
#endif /* PIONEER_DATA_DIR */

#ifdef _WIN32
#include <malloc.h>
#ifndef __MINGW32__
#define alloca _alloca
#define strncasecmp _strnicmp
#define strcasecmp _stricmp

#ifndef isfinite
inline int isfinite(double x) { return _finite(x); }
#endif
#endif /* __MINGW32__ */
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef __MINGW32__
#define NOMINMAX
#endif
#include <windows.h>
#define snprintf _snprintf
#endif

#ifdef __MINGW32__
#undef WINVER
#define WINVER 0x0500
#include <w32api.h>
#define _WIN32_IE IE5
#endif

#ifdef _WIN32

#ifdef __MINGW32__
#include <dirent.h>
#include <sys/stat.h>
#include <stdexcept>
#define WINSHLWAPI
#else /* !__MINGW32__ */
#include "win32-dirent.h"
#endif

#include <shlobj.h>
#include <shlwapi.h>

#else /* !_WIN32 */
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "fixed.h"
#include "vector3.h"
#include "Aabb.h"
#include "matrix4x4.h"
#include "Color.h"
#include "mtrand.h"

#include "utils.h"
#include "FloatComparison.h"

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
