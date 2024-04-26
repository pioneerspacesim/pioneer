// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include <limits>
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
		// -3.0 / 3.0 == -1.0
		// There used to be a bug causing this to produce an incorrect result
		CHECK(fixed(-3'00000000) / fixed(3, 1) == fixed(-1'00000000));
		// 0x40000000.00000000 / 0x40000000.00000001 == 0x00000000.ffffffff (because we round down)
		// There used to be a bug causing this to produce an incorrect result
		CHECK(fixed(0x40000000'00000000) / fixed(0x40000000'00000001) == fixed(0x00000000'ffffffff));
		// fixed(INT_MIN) / fixed(INT_MIN) used to trigger a signed int overflow
		Sint64 min = std::numeric_limits<Sint64>::min();
		CHECK(fixed(min) / fixed(min) == fixed(1, 1));
		// fixed(INT_MIN) / -1.0 used to trigger a signed int overflow
		CHECK(fixed(min) / fixed(-1'00000000) == fixed(min) / fixed(-1'00000000));
	}
}
