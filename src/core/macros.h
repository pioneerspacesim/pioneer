// Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include <cstddef>

// This file stores most utility macros used across the whole codebase that
// do not depend on large header includes.

#ifdef NDEBUG
#define PiVerify(x) ((void)(x))
#else
#include <cassert>
#define PiVerify(x) assert(x)
#endif

// from StackOverflow: http://stackoverflow.com/a/1500517/52251
// Q: "Compile time sizeof_array without using a macro"
template <typename T, size_t N>
char (&COUNTOF_Helper(T (&array)[N]))[N];
#define COUNTOF(array) (sizeof(COUNTOF_Helper(array)))
