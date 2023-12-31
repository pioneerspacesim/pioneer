// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "FNV1a.h"

inline constexpr size_t operator""_hash(const char *str, size_t len)
{
	return hash_64_fnv1a(str, len);
}

inline constexpr uint32_t operator""_hash32(const char *str, size_t len)
{
	return hash_32_fnv1a(str, len);
}
