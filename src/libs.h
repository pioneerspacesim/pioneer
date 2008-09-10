#ifndef _LIBS_H
#define _LIBS_H

#include <assert.h>
#include <stdio.h>
#include <sigc++/sigc++.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <ode/ode.h>
#include <float.h>
#include <limits>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#define snprintf _snprintf
#define alloca _alloca
#endif

#include "fixed.h"
#include "vector3.h"
#include "Aabb.h"
#include "matrix4x4.h"
#include "mtrand.h"

#include "date.h"

#ifndef dDOUBLE
# error LibODE is not compiled with double-precision floating point. Please get/compile libode with double-precision floating point.
#endif

#define DEBUG

#define UNIVERSE_SEED	0xabcd1234

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
