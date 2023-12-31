// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "core/StringName.h"

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>

StringName::StringName(std::string_view s, uint32_t hash) :
	StringName()
{
	if (s.empty() || s.size() >= UINT32_MAX)
		return;

	m_hash = hash ? hash : hash_32_fnv1a(s.data(), s.size());
	m_size = uint32_t(s.size());
	if (m_size <= MAX_SSO_SIZE) {
		std::memcpy(m_str.data, s.data(), m_size);
		m_str.data[m_size] = '\0';
	} else {
		m_str.ptr = make_data(s.data(), m_size, m_hash);
	}
}

StringName &StringName::operator=(const StringName &rhs)
{
	if (m_size > MAX_SSO_SIZE)
		m_str.ptr->unref();

	m_size = rhs.m_size;
	m_hash = rhs.m_hash;
	m_str = rhs.m_str;
	if (m_size > MAX_SSO_SIZE)
		m_str.ptr->ref();

	return *this;
}

StringName &StringName::operator=(StringName &&rhs)
{
	if (this == &rhs)
		return *this;

	if (m_size > MAX_SSO_SIZE)
		m_str.ptr->unref();

	std::memcpy(reinterpret_cast<char *>(this), reinterpret_cast<char *>(&rhs), sizeof(StringName));
	std::memset(reinterpret_cast<char *>(&rhs), '\0', sizeof(StringName));

	return *this;
}

StringName::StringData *StringName::make_data(const char *c, uint32_t s, uint32_t h)
{
	auto &entry = StringTable::Get()->FindOrCreate(h);
	if (!entry) {
		entry = new (std::malloc(sizeof(StringData) + s + 1)) StringData();
		std::memcpy(entry->get(), c, s);
		entry->get()[s] = '\0';
	}

	entry->ref();
	return entry;
}

// =============================================================================

StringTable::Data *StringTable::Find(uint32_t key)
{
	if (!key)
		return nullptr;

	uint8_t slot_dist = 0;
	for (uint32_t idx = key;; idx++, slot_dist++) {
		idx &= keys.size() - 1;

		uint32_t probed_key = keys[idx];
		if (probed_key == key)
			return &values[idx];

		if (!probed_key || dist[idx] < slot_dist)
			return nullptr;
	}
}

StringTable::Data *StringTable::Create(uint32_t key)
{
	if (entries > uint32_t(keys.size() * 0.9))
		Grow();

	uint8_t slot_dist = 0;
	uint32_t ret = 0;
	for (uint32_t idx = key;; idx++, slot_dist++) {
		idx &= keys.size() - 1;

		uint32_t probed_key = keys[idx];
		if (probed_key == key)
			return &values[idx];

		if (!probed_key || dist[idx] < slot_dist) {
			ret = idx;
			break;
		}
	}

	StringTable::Data cached_value = {};
	for (uint32_t idx = ret;; idx++, slot_dist++) {
		idx &= keys.size() - 1;

		uint32_t probed_key = keys[idx];
		if (!probed_key || dist[idx] < slot_dist) {
			std::swap(keys[idx], key);
			std::swap(dist[idx], slot_dist);
			std::swap(values[idx], cached_value);
			if (!probed_key)
				break;
		}
	}

	entries++;
	return &values[ret];
}

void StringTable::Erase(uint32_t key)
{
	uint8_t slot_dist = 0;
	for (uint32_t idx = key;; idx++, slot_dist++) {
		idx &= keys.size() - 1;

		uint32_t probed_key = keys[idx];
		if (probed_key == key) {
			entries--;

			// backshift all following entries
			for (uint32_t jdx = idx + 1;; idx++, jdx++) {
				idx &= keys.size() - 1;
				jdx &= keys.size() - 1;

				// stopcode, no need to continue swapping
				if (!keys[jdx] || !dist[jdx]) {
					keys[idx] = 0;
					dist[idx] = 0;
					values[idx] = nullptr;
					return;
				} else {
					keys[idx] = keys[jdx];
					dist[idx] = dist[jdx] - 1;
					values[idx] = values[jdx];
				}
			}
		}

		if (!probed_key || dist[idx] < slot_dist)
			return;
	}
}

void StringTable::Reclaim(bool force)
{
	m_reclaimClock.SoftStop();
	if (m_reclaimClock.seconds() < 15.0 && !force)
		return;

	m_reclaimClock.SoftReset();
	for (uint32_t idx = 0; idx < keys.size(); idx++) {
		uint32_t probed_key = keys[idx];

		// the refcount can be decremented to zero from another thread,
		// and it can be incremented on another thread as long as it is >0
		// but only this thread can increment it from zero
		if (probed_key && !values[idx]->get_ref()) {
			Erase(probed_key);
		}
	}
}

void StringTable::Grow()
{
	size_t new_size = keys.size() * 2;
	dist = std::vector<uint8_t>(new_size);

	std::vector<uint32_t> old_keys(new_size);
	std::swap(keys, old_keys);

	std::vector<Data> old_values(new_size);
	std::swap(values, old_values);

	for (uint32_t idx = 0; idx < old_keys.size(); idx++) {
		uint32_t hash = old_keys[idx];

		if (hash)
			*Create(hash) = old_values[idx];
	}
}

// Lookup is extremely cheap when the string table has low to medium occupancy,
// and resize/rehash is extremely expensive. We trade some static memory
// allocated once for each thread to make reallocations very unlikely.

// 16k slots == 208kb per thread
thread_local StringTable tl_stringTable = StringTable(1 << 14);
StringTable *StringTable::Get() { return &tl_stringTable; }
