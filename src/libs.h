// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LIBS_H
#define _LIBS_H

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

#include "core/StringUtils.h"
#include "core/macros.h"

#include "MathUtil.h"

#endif /* _LIBS_H */
