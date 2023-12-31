// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "core/FNV1a.h"
#include "core/Log.h"
#include "core/StringName.h"
#include "profiler/Profiler.h"

#include <iostream>
#include "doctest.h"

static constexpr uint32_t ITERATIONS = 10000;
static constexpr uint32_t LOOKUP_ITERATIONS = 20000;

void insertion_stress_test(StringTable *st, uint32_t test_num)
{
	Profiler::Clock clock{};
	clock.Start();

	for (uint32_t idx = 0; idx < ITERATIONS; idx++) {
		uint32_t hash = hash_32_fnv1a(reinterpret_cast<char *>(&idx), sizeof(uint32_t));
		st->FindOrCreate(hash);
	}

	clock.Stop();
	Log::Info("StringTable[run {}]: Insertion of {} elements took {}ms\n", test_num, ITERATIONS, clock.milliseconds());

	clock.Reset();
	clock.Start();

	for (uint32_t idx = 0; idx < LOOKUP_ITERATIONS; idx++) {
		uint32_t hash = hash_32_fnv1a(reinterpret_cast<char *>(&idx), sizeof(uint32_t));
		st->Find(hash);
	}

	clock.Stop();
	Log::Info("StringTable[run {}]: Lookup of {} elements took {}ms\n", test_num, LOOKUP_ITERATIONS, clock.milliseconds());
}

TEST_CASE("String Table")
{
	StringTable *st = new StringTable(1 << 10);
	uint32_t test_hash = "test12345"_hash32;

	REQUIRE(st != nullptr);

	SUBCASE("Creation")
	{
		StringTable::Data *entry = st->Find(test_hash);

		CHECK(entry == nullptr);

		entry = st->Create(test_hash);
		CHECK(entry != nullptr);
		CHECK(*entry == nullptr);

		StringTable::Data *entry2 = st->Find(test_hash);
		CHECK(entry == entry2);
	}

	SUBCASE("Deletion")
	{
		StringTable::Data *entry = st->Create(test_hash);
		StringTable::Data *entry2 = st->Create(test_hash + 1);

		CHECK(entry != nullptr);
		CHECK(entry2 != nullptr);

		st->Erase(test_hash);

		CHECK(st->Find(test_hash) == nullptr);
		CHECK(st->Find(test_hash + 1) == entry2);

		st->Erase(test_hash + 1);

		CHECK(st->Find(test_hash + 1) == nullptr);
	}

	SUBCASE("Contention")
	{
		// note: more than 256 items sharing the same slot in the hash table
		// will cause erasure to fail (due to storing item offsets in uint8)
		// This is *extremely* unlikely to happen outside of synthetic test cases
		uint32_t mask = uint32_t(st->Capacity() - 1);
		std::vector<uint32_t> hashes(255, test_hash & mask);

		for (uint32_t idx = 0; idx < uint32_t(hashes.size()); idx++) {
			hashes[idx] |= idx * st->Capacity();
			CHECK(st->Create(hashes[idx]) != nullptr);
		}

		CHECK(st->Size() == 255);

		for (uint32_t idx = 0; idx < uint32_t(hashes.size()); idx++) {
			CHECK(st->Find(hashes[idx]) != nullptr);
		}

		for (uint32_t idx = 0; idx < uint32_t(hashes.size()); idx++) {
			INFO(hashes[idx]);
			CHECK(st->Find(hashes[idx]) != nullptr);
			if (!(idx % 8))
				st->Erase(hashes[idx]);
		}
	}

	SUBCASE("Performance")
	{
		for (uint32_t idx = 0; idx < 3; idx++) {
			StringTable local_st(1 << 14);
			insertion_stress_test(&local_st, idx);
		}
	}

	delete st;
}

TEST_CASE("StringName")
{

	SUBCASE("Creation")
	{
		CHECK(StringTable::Get()->Size() == 0);

		auto name = StringName("testing one two three");
		auto name2 = StringName(std::string_view("testing one two three"));
		CHECK(StringTable::Get()->Size() == 1);

		CHECK(name.size() == 21);
		CHECK(name.hash() != 0);
		CHECK(name.sv() == "testing one two three");

		CHECK(name == name2);
		CHECK(name.hash() == name2.hash());

		name = {};
		StringTable::Get()->Reclaim(true);
		CHECK(StringTable::Get()->Size() == 1);

		name2 = {};
		StringTable::Get()->Reclaim(true);
		CHECK(StringTable::Get()->Size() == 0);
	}

	SUBCASE("Copy Construction")
	{
		std::vector<StringName> test_names;

		test_names.emplace_back("this is a test");
		CHECK(test_names[0].hash() == "this is a test"_hash32);
		test_names.emplace_back("this is a test");
		CHECK(test_names[0].hash() == "this is a test"_hash32);

		StringName a("this is a test");
		CHECK(a.hash() == "this is a test"_hash32);
		StringName b = a;
		CHECK(a.hash() == "this is a test"_hash32);
		CHECK(b.hash() == "this is a test"_hash32);
	}

	SUBCASE("Occupancy")
	{
		static constexpr uint32_t PERSISTENT_SIZE = 256;
		StringTable::Get()->Reclaim(true);
		CHECK(StringTable::Get()->Size() == 0);

		std::vector<StringName> persistent_names;
		for (uint32_t idx = 0; idx < PERSISTENT_SIZE; idx++) {
			persistent_names.emplace_back(fmt::format("this is some test {}", idx));
		}

		CHECK(StringTable::Get()->Size() == PERSISTENT_SIZE);

		std::vector<StringName> temporary_names;
		for (uint32_t idx = 0; idx < 1024; idx++) {
			temporary_names.emplace_back("this is some test 1");
		}

		CHECK(StringTable::Get()->Size() == PERSISTENT_SIZE);

		temporary_names.clear();
		CHECK(StringTable::Get()->Size() == PERSISTENT_SIZE);

		persistent_names.clear();
		CHECK(StringTable::Get()->Size() == PERSISTENT_SIZE);

		StringTable::Get()->Reclaim(true);
		CHECK(StringTable::Get()->Size() == 0);
	}
}
