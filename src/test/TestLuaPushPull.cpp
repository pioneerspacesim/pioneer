// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "lua/Lua.h"
#include "lua/LuaPushPull.h"

#include <fmt/core.h>

#include "doctest.h"

#include <string>
#include <tuple>

std::string str(int i)
{
	return fmt::format("{}", i);
}

template <typename T>
using tuple_t = std::tuple<T, T, T>;

TEST_CASE("Sequence Points")
{
	SUBCASE("As-Constructor Test")
	{
		int i = 0;

		// Simple test canary to ensure initializer clauses in braced-init-lists
		// are sequenced-before following clauses in accordance with the C++
		// specification
		tuple_t<int> test_tuple = tuple_t<int>{ i++, i++, i++ };

		CHECK(std::get<0>(test_tuple) == 0);
		CHECK(std::get<1>(test_tuple) == 1);
		CHECK(std::get<2>(test_tuple) == 2);
		CHECK(i == 3);
	}

	SUBCASE("Inside Arguments Test")
	{
		int i = 0;

		// Simple test canary to ensure initializer clauses in braced-init-lists
		// are sequenced-before following clauses in accordance with the C++
		// specification
		tuple_t<std::string> test_tuple = tuple_t<std::string>{ str(i++), str(i++), str(i++) };

		CHECK(std::get<0>(test_tuple) == "0");
		CHECK(std::get<1>(test_tuple) == "1");
		CHECK(std::get<2>(test_tuple) == "2");
		CHECK(i == 3);
	}
}

TEST_CASE("Lua Push / Pull Basic Values")
{
	lua_State *l = luaL_newstate();

	SUBCASE("Sanity Check")
	{
		LuaPush<bool>(l, true);
		LuaPush<int>(l, 1);
		LuaPush<std::string>(l, "test");

		CHECK(LuaPull<bool>(l, 1) == true);
		CHECK(LuaPull<int>(l, 2) == 1);
		CHECK(LuaPull<std::string>(l, 3) == "test");
	}

	SUBCASE("Multiple-Push")
	{
		pi_lua_multiple_push<bool, int, std::string>(l, true, 1, "test");

		CHECK(LuaPull<bool>(l, 1) == true);
		CHECK(LuaPull<int>(l, 2) == 1);
		CHECK(LuaPull<std::string>(l, 3) == "test");
	}

	SUBCASE("Multiple-Pull")
	{
		pi_lua_multiple_push<bool, int, std::string>(l, true, 1, "test");

		auto tuple = pi_lua_multiple_pull<bool, int, std::string>(l, 1);

		CHECK(std::get<0>(tuple) == true);
		CHECK(std::get<1>(tuple) == 1);
		CHECK(std::get<2>(tuple) == "test");
	}

	lua_close(l);
}
