// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "StringHash.h"
#include "profiler/Profiler.h"

#include <atomic>
#include <string_view>
#include <type_traits>
#include <vector>

/*
 * Lightweight immutable refcounted string class. Internally stores string data
 * in the string object or in a thread-local hashtable for storage efficiency.
 */
class StringName {
	static constexpr uint32_t MAX_SSO_SIZE = 15;

public:
	StringName() :
		m_hash(0),
		m_size(0),
		m_str{} {}

	StringName(std::string_view s, uint32_t hash = 0);
	StringName(const StringName &rhs) :
		StringName() { *this = rhs; }
	StringName &operator=(const StringName &rhs);
	StringName(StringName &&rhs) :
		StringName() { *this = std::move(rhs); };
	StringName &operator=(StringName &&rhs);

	~StringName()
	{
		if (m_size > MAX_SSO_SIZE)
			m_str.ptr->unref();
		m_size = 0;
		m_hash = 0;
	}

	uint32_t hash() const { return m_hash; }
	size_t size() const { return m_size; }
	const char *c_str() const { return m_size > MAX_SSO_SIZE ? m_str.ptr->get() : m_str.data; }
	std::string_view sv() const { return { c_str(), size() }; }

	operator std::string_view() const { return sv(); }
	operator bool() const { return m_size; }

	bool operator==(std::string_view rhs) const { return sv() == rhs; }
	bool operator!=(std::string_view rhs) const { return !(*this == rhs); }
	bool operator<(std::string_view rhs) const { return sv() < rhs; }

	bool operator==(const StringName &rhs) const { return hash() == rhs.hash(); }
	bool operator!=(const StringName &rhs) const { return !(*this == rhs); }
	bool operator<(const StringName &rhs) const { return sv() < rhs.sv(); }

private:
	friend class StringTable;

	struct StringData {
		mutable std::atomic<uint32_t> refcount;
		char *get() { return reinterpret_cast<char *>(&this[1]); }

		uint32_t ref() const { return refcount.fetch_add(1) + 1; }
		uint32_t unref() const { return refcount.fetch_sub(1) - 1; }
		uint32_t get_ref() const { return refcount.load(); }
	};

	// handle interning and de-duplicating the string memory
	static StringData *make_data(const char *s, uint32_t size, uint32_t hash);

	uint32_t m_hash;
	uint32_t m_size;
	union { // small string optimization
		StringData *ptr;
		char data[16];
	} m_str;
};

/*
 * Thread-local hash table for efficient storage of immutable strings.
 * StringTable uses a power-of-two based robin-hood hash table to keep
 * indexing overhead as small as possible.
 */
class StringTable {
public:
	using Data = StringName::StringData *;

	StringTable(uint32_t size) :
		keys(size),
		dist(size),
		values(size),
		entries(0) {}

	static StringTable *Get();

	size_t Size() const { return entries; }
	size_t Capacity() const { return keys.size(); }

	Data *Find(uint32_t key);
	Data *Create(uint32_t key);

	Data &FindOrCreate(uint32_t key)
	{
		if (auto *ptr = Find(key))
			return *ptr;

		return *Create(key);
	}

	void Erase(uint32_t key);

	// Quiescent memory reclamation - call this function every 30s or so
	// from the owning thread
	void Reclaim(bool force = false);

private:
	void Grow();

	std::vector<uint32_t> keys;
	std::vector<uint8_t> dist;
	std::vector<Data> values;
	uint32_t entries;
	Profiler::Clock m_reclaimClock;
};

inline StringName operator""_name(const char *c, size_t l) { return StringName(std::string_view(c, l)); }
