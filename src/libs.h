// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LIBS_H
#define _LIBS_H

#include <SDL.h>
#include <SDL_image.h>
#include <sigc++/sigc++.h>
#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cfloat>
#include <cinttypes>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <deque>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <vector>

#ifdef _WIN32
#include <malloc.h>

#ifdef _MSC_VER
#pragma warning(disable : 4244) // "conversion from x to x: possible loss of data"
#pragma warning(disable : 4800) // int-to-bool "performance warning"
#pragma warning(disable : 4355) // 'this' used in base member initializer list
#pragma warning(disable : 4351) // new behavior [after vs2003!]: elements of array 'array' will be default initialized
#endif

#ifndef __MINGW32__
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif
#endif

#define SIZET_FMT "%zu"

#include "fixed.h"
#include "matrix3x3.h"
#include "matrix4x4.h"
#include "vector2.h"
#include "vector3.h"

#include "Aabb.h"
#include "Color.h"
#include "Random.h"

#include "FloatComparison.h"
#include "RefCounted.h"
#include "SmartPtr.h"

#include "profiler/Profiler.h"

#ifdef NDEBUG
#define PiVerify(x) ((void)(x))
#else
#define PiVerify(x) assert(x)
#endif

template <class T>
inline const T &Clamp(const T &x, const T &min, const T &max) { return x > max ? max : (x < min ? min : x); }

inline constexpr double DEG2RAD(double x) { return x * (M_PI / 180.); }
inline constexpr float DEG2RAD(float x) { return x * (float(M_PI) / 180.f); }
inline constexpr double RAD2DEG(double x) { return x * (180. / M_PI); }
inline constexpr float RAD2DEG(float x) { return x * (180.f / float(M_PI)); }

// from StackOverflow: http://stackoverflow.com/a/1500517/52251
// Q: "Compile time sizeof_array without using a macro"
template <typename T, size_t N>
char (&COUNTOF_Helper(T (&array)[N]))[N];
#define COUNTOF(array) (sizeof(COUNTOF_Helper(array)))

#endif /* _LIBS_H */
