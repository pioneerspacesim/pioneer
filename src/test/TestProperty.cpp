// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Color.h"
#include "Quaternion.h"
#include "core/Log.h"
#include "core/Property.h"

#include <algorithm>
#include <string>
#include <iostream>
#include "doctest.h"

static constexpr uint32_t ITERATIONS = 10000;
static constexpr uint32_t LOOKUP_ITERATIONS = 20000;

static void property_map_stress_test(PropertyMap *map, uint32_t test_num)
{
	std::string_view strings[16] = {
		"alpha", "bravo", "charlie", "delta", "echo", "foxtrot", "golf", "hotel",
		"india", "juliet", "kilo", "lima", "mike", "november", "oscar", "papa"
	};

	Profiler::Clock clock{};
	clock.Start();

	for (uint32_t idx = 0; idx < ITERATIONS; idx++) {
		map->Set(strings[idx & 15], idx);
	}

	clock.Stop();
	Log::Info("PropertyMap[run {}]: Insertion/Update of {} elements using StringView keys took {}ms\n", test_num, ITERATIONS, clock.milliseconds());

	clock.Reset();
	clock.Start();
	bool _t = true;

	for (uint32_t idx = 0; idx < LOOKUP_ITERATIONS; idx++) {
		const Property &prop = map->Get(strings[idx & 15]);
		_t &= prop.is_integer();
	}

	clock.Stop();
	Log::Info("PropertyMap[run {}]: Lookup of {} elements using StringView keys took {}ms\n", test_num, LOOKUP_ITERATIONS, clock.milliseconds());

	map->Clear();
	clock.Reset();
	clock.Start();

	StringName string_names[16] = {};
	for (uint32_t idx = 0; idx < 16; idx++)
		string_names[idx] = StringName(strings[idx]);

	for (uint32_t idx = 0; idx < ITERATIONS; idx++) {
		map->Set(string_names[idx & 15], idx);
	}

	clock.Stop();
	Log::Info("PropertyMap[run {}]: Insertion/Update of {} elements using StringName keys took {}ms\n", test_num, ITERATIONS, clock.milliseconds());

	clock.Reset();
	clock.Start();

	for (uint32_t idx = 0; idx < LOOKUP_ITERATIONS; idx++) {
		const Property &prop = map->Get(string_names[idx & 15]);
		_t &= prop.is_integer();
	}

	clock.Stop();
	Log::Info("PropertyMap[run {}]: Lookup of {} elements using StringName keys took {}ms\n", test_num, LOOKUP_ITERATIONS, clock.milliseconds());
}

TEST_CASE("PropertyValidation")
{
	Property prop{};

	SUBCASE("Default Initialization")
	{
		CHECK(prop.is_null() == true);
		CHECK(prop.is_bool() == false);
		CHECK(prop.is_number() == false);
		CHECK(prop.is_integer() == false);
		CHECK(prop.is_vector2() == false);
		CHECK(prop.is_vector3() == false);
		CHECK(prop.is_quat() == false);
		CHECK(prop.is_string() == false);
		CHECK(prop.is_map() == false);
	}

	SUBCASE("Default Getter Values")
	{
		CHECK(prop.get_bool() == false);
		CHECK(prop.get_number() == 0.0);
		CHECK(prop.get_integer() == 0);
		CHECK(prop.get_vector2() == vector2d(0, 0));
		CHECK(prop.get_vector3() == vector3d(0, 0, 0));
		CHECK(prop.get_quat() == Quaternionf(1, 0, 0, 0));
		CHECK(prop.get_color() == Color4ub(0, 0, 0, 255));
		CHECK(prop.get_string() == "");
		CHECK(prop.get_map() == PropertyMapWrapper());
	}

	SUBCASE("Unique Getter Values")
	{
		CHECK(prop.get_bool(true) == true);
		CHECK(prop.get_number(3.0) == 3.0);
		CHECK(prop.get_integer(3) == 3);
		CHECK(prop.get_vector2(vector2d(1, 2)) == vector2d(1, 2));
		CHECK(prop.get_vector3(vector3d(1, 2, 3)) == vector3d(1, 2, 3));
		CHECK(prop.get_quat(Quaternionf(5, 0, 0, 0)) == Quaternionf(5, 0, 0, 0));
		CHECK(prop.get_color(Color4ub::WHITE) == Color4ub::WHITE);
		CHECK(prop.get_string("test12345") == "test12345");
		CHECK(prop.get_map(nullptr) == PropertyMapWrapper(nullptr));
	}

	SUBCASE("Boolean Value")
	{
		prop = true;

		CHECK(prop.is_null() == false);
		CHECK(prop.is_bool() == true);
		CHECK(prop.get_bool() == true);
	}

	SUBCASE("Number Value")
	{
		prop = 1.0;

		CHECK(prop.is_null() == false);
		CHECK(prop.is_number() == true);
		CHECK(prop.is_integer() == true);
		CHECK(prop.get_number() == 1.0);
		CHECK(prop.get_integer() == 1);
	}

	SUBCASE("Integer Value")
	{
		prop = 1;

		CHECK(prop.is_null() == false);
		CHECK(prop.is_number() == true);
		CHECK(prop.is_integer() == true);
		CHECK(prop.get_number() == 1.0);
		CHECK(prop.get_integer() == 1);
	}

	SUBCASE("Vector2 Value")
	{
		prop = vector2d(1, 1);

		CHECK(prop.is_null() == false);
		CHECK(prop.is_vector2() == true);
		CHECK(prop.get_vector2() == vector2d(1, 1));
	}

	SUBCASE("Vector3 Value")
	{
		prop = vector3d(1, 1, 1);

		CHECK(prop.is_null() == false);
		CHECK(prop.is_vector3() == true);
		CHECK(prop.get_vector3() == vector3d(1, 1, 1));
	}

	SUBCASE("Color Value")
	{
		prop = Color4ub(255, 215, 205, 185);

		CHECK(prop.is_null() == false);
		CHECK(prop.is_color() == true);
		CHECK(prop.get_color() == Color4ub(255, 215, 205, 185));
	}

	SUBCASE("Quat Value")
	{
		auto val = Quaternionf(M_PI_2, vector3f(1, 1, 0.5).Normalized());
		prop = val;

		CHECK(prop.is_null() == false);
		CHECK(prop.is_quat() == true);
		CHECK(prop.get_quat() == val);
	}

	SUBCASE("String Value")
	{
		prop = "test";

		CHECK(prop.is_null() == false);
		CHECK(prop.is_string() == true);
		CHECK(prop.get_string() == "test");
	}

	SUBCASE("Property Map")
	{
		PropertyMapWrapper map = PropertyMapWrapper(new PropertyMap());

		prop = map;
		CHECK(map->GetRefCount() == 2);
		CHECK(map.get() == prop.get_map());

		CHECK(prop.is_null() == false);
		CHECK(prop.is_map() == true);
		CHECK(prop.get_map() != nullptr);
		CHECK(prop.get_map()->GetRefCount() == 2);

		prop = new PropertyMap();
		CHECK(prop.is_map() == true);
		CHECK(map->GetRefCount() == 1);
		CHECK(prop.get_map()->GetRefCount() == 1);

		auto prop2 = Property(prop.get_map());
		CHECK(prop2.is_map() == true);
		CHECK(prop.get_map()->GetRefCount() == 2);
	}
}

TEST_CASE("PropertyMap")
{
	PropertyMapWrapper map = new PropertyMap();

	REQUIRE(map->Size() == 0);
	REQUIRE(map->GetRefCount() == 1);

	SUBCASE("Empty Access")
	{
		const Property &prop = map->Get("test");

		CHECK(prop.is_null() == true);
		CHECK(map->Empty() == true);
		CHECK(map->Size() == 0);
	}

	SUBCASE("Strict Insertion")
	{
		Property prop = 1001;

		map->Set("test", prop);
		CHECK(map->Size() == 1);
		CHECK(map->Empty() == false);

		const Property &prop2 = map->Get("test");
		CHECK(prop2.is_null() == false);
		CHECK(prop2.get_integer() == 1001);
		CHECK(prop == prop2);
	}

	SUBCASE("Optimized Insertion")
	{
		map->Set("test", 1002);
		CHECK(map->Get("test").get_integer() == 1002);
	}

	SUBCASE("Clearing")
	{
		map->Set("test1", 1);
		map->Set("test2", 2);
		CHECK(map->Size() == 2);

		map->Clear();
		CHECK(map->Size() == 0);
	}

	SUBCASE("String Hashing Equality")
	{
		Property prop = 1003;

		map->Set("test", prop);

		CHECK(map->Get("test").is_integer());
		CHECK(map->Get("test").get_integer() == 1003);

		map->Set("test2", prop);
		CHECK(map->Get("test2").is_integer());
		CHECK(map->Get("test2").get_integer() == 1003);
	}

	SUBCASE("Iteration")
	{
		std::vector<uint32_t> values;
		CHECK(map->Size() == 0);

		for (uint32_t idx = 0; idx < 24; idx++) {
			values.push_back(idx);
			map->Set(std::to_string(idx), idx);
		}

		CHECK(map->Size() == values.size());

		uint32_t counter = 0;
		for (auto &value : map) {
			(void)value;
			counter++;
		}

		CHECK(counter == map->Size());

		for (auto &value : map) {
			auto it = std::find(values.begin(), values.end(), value.second.get_integer());
			CHECK(it != values.end());
			values.erase(it);
		}

		CHECK(values.size() == 0);
	}

	SUBCASE("Stress Test")
	{
		for (uint32_t idx = 0; idx < 3; idx++) {
			property_map_stress_test(map, idx);
			map->Clear();
		}
	}
}
