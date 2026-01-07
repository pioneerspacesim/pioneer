// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include <cstdint>
#include "doctest.h"
#include "fixed.h"

TEST_CASE("Fixed")
{
	SUBCASE("operator ==")
	{
		// 42.0 == 42.0
		CHECK(fixed(42, 1) == fixed(42, 1));
		// 123.0 != 321.0
		CHECK(fixed(123, 1) != fixed(321, 1));
		// 7.0 == 7.0
		CHECK(fixed(0x7'00000000) == fixed(7, 1));
	}

	SUBCASE("operator /")
	{
		// 3.0 / 3.0 == 1.0
		CHECK(fixed(3, 1) / fixed(3, 1) == fixed(1, 1));
		// Negative numerators used to cause incorrect results
		// Example: -3.0 / 3.0 should be -1.0
		CHECK(fixed(-0x3'00000000) / fixed(3, 1) == fixed(-0x1'00000000));
		// Denominators greater then 0x40000000.00000000 or less then -0x40000000.00000000 used to cause incorrect results
		// Example: 0x40000000.00000000 / 0x40000000.00000001 should return 0x00000000.ffffffff (because we round down)
		CHECK(fixed(0x40000000'00000000) / fixed(0x40000000'00000001) == fixed(0x00000000'ffffffff));
		// There used to be an edge case that triggered a signed int overflow when negating
		// the final output if the truncation to 64 bits and conversio to Sint64 produced INT64_MIN
		// because -INT64_MIN is undefined behavour
		// Example: 0x00000000.80000000 / -0x00000000.00000001
		CHECK(fixed(0x80000000) / fixed(-1) == fixed(INT64_MIN));
		// For completeness:
		// 3.0 / -3.0 == -1.0
		CHECK(fixed(3, 1) / fixed(-0x3'00000000) == fixed(-0x1'00000000));
		// -3.0 / -3.0 == 1.0
		CHECK(fixed(-0x3'00000000) / fixed(-0x3'00000000) == fixed(1, 1));
	}
}
