// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "core/FNV1a.h"

#ifdef _MSC_VER
#define FUNC_SOURCE __FUNCSIG__
#else
#define FUNC_SOURCE __PRETTY_FUNCTION__
#endif

/**
 * Simple class to provide compile-type constant type identifiers.
 * These identifiers work across boundaries and remain the same between
 * different runs of the program.
 * They are not portable between compilers and should not be serialized.
 */

template <typename T>
class TypeId {
public:
	inline static constexpr size_t Get()
	{
		return hash_64_fnv1a(FUNC_SOURCE, Len(FUNC_SOURCE));
	}

	inline static constexpr uint32_t Get32()
	{
		return hash_32_fnv1a(FUNC_SOURCE, Len(FUNC_SOURCE));
	}

private:
	template <size_t N>
	inline static constexpr size_t Len(char const (&s)[N]) { return N; }
};

#undef FUNC_SOURCE
