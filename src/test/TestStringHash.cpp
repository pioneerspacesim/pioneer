// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "core/StringHash.h"
#include "doctest.h"

#include <cstring>
#include <string>

static size_t hash_sv(std::string_view s) { return hash_64_fnv1a(s.data(), s.size()); }
static size_t hash_str(std::string s) { return hash_64_fnv1a(s.data(), s.size()); }
static size_t hash_cstr(const char *s) { return hash_64_fnv1a(s, strlen(s)); }

#define TEST_STR "this is a test"
#define TEST_STR_HASH "this is a test"_hash

TEST_CASE("Hashed Strings")
{
	size_t baseline_hash = TEST_STR_HASH;

	CHECK(hash_sv(TEST_STR) == baseline_hash);
	CHECK(hash_str(TEST_STR) == baseline_hash);
	CHECK(hash_cstr(TEST_STR) == baseline_hash);
}
