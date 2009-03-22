#ifndef _LIBS_H
#define _LIBS_H

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

#ifdef _WIN32
#include <malloc.h>
#define alloca _alloca
#else
#include <alloca.h>
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#define snprintf _snprintf
#endif

#include "fixed.h"
#include "vector3.h"
#include "Aabb.h"
#include "matrix4x4.h"
#include "mtrand.h"

#include "utils.h"

#define DEBUG

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

#define WORLDVIEW_ZNEAR	10.0
#define WORLDVIEW_ZFAR	10000.0

/* 
 * Normal use:
 * foreach (container, iter) { do_something (*iter); }
 *
 * When removing items:
 * foreach (container, iter) {
 * 	if (*iter == some_value) {
 * 		iter = container.erase (iter);      // assign not necessary for maps
 * 		--iter;
 * 		continue;
 * 	}
 * }
 */
#define foreach(_collection,_iterator) for (__typeof__ (_collection.end()) _iterator = (_collection).begin (); _iterator != (_collection).end(); ++(_iterator))

#define MIN(x,y)        ((x)<(y)?(x):(y))
#define MAX(x,y)        ((x)>(y)?(x):(y))
#define CLAMP(a, min, max)      (((a) > (max)) ? (max) : (((a) < (min)) ? (min) : (a)))
#define DEG_2_RAD	0.0174532925
#define DEG2RAD(x) ((x)*M_PI/180.0)

#endif /* _LIBS_H */
