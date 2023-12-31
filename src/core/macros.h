// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include <cstddef>

#ifndef __GNUC__
#define __attribute(x)
#endif /* __GNUC__ */

// GCC warns when a function marked __attribute((noreturn)) actually returns a value
// but other compilers which don't see the noreturn attribute of course require that
// a function with a non-void return type should return something.
#ifndef __GNUC__
#define RETURN_ZERO_NONGNU_ONLY return 0;
#else
#define RETURN_ZERO_NONGNU_ONLY
#endif

// align x to a. taken from the Linux kernel
#define ALIGN(x, a) __ALIGN_MASK(x, (a - 1))
#define __ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))

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

// Helper to implement stack-based variable-length arrays in a crossplatform way
// Avoids a heap allocation for std::vector in "hot" code

#ifdef _MSC_VER
#include <malloc.h>
#define stackalloc(T, n) reinterpret_cast<T *>(_alloca(sizeof(T) * n))
#else
#include <alloca.h>
#define stackalloc(T, n) reinterpret_cast<T *>(alloca(sizeof(T) * n))
#endif
