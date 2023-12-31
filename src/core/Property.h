// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "Color.h"
#include "JsonFwd.h"
#include "Quaternion.h"
#include "RefCounted.h"
#include "core/StringName.h"
#include "vector2.h"
#include "vector3.h"

#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

class PropertyMap;
class PropertyArray;

// little indirection class to allow nesting refcounted property maps in a property
// without direct circular compilation dependency
struct PropertyMapWrapper {
public:
	PropertyMapWrapper() :
		m_map(nullptr) {}
	PropertyMapWrapper(PropertyMap *);
	~PropertyMapWrapper();

	PropertyMapWrapper(const PropertyMapWrapper &rhs) :
		PropertyMapWrapper(rhs.m_map) {}
	PropertyMapWrapper &operator=(const PropertyMapWrapper &rhs);

	// implicit conversion
	operator PropertyMap *() const { return m_map; }

	// pointer mimic operators
	PropertyMap *operator*() const { return m_map; }
	PropertyMap *operator->() const { return m_map; }

	PropertyMap *get() const { return m_map; }

	auto begin();
	auto end();

private:
	mutable PropertyMap *m_map;
};

using PropertyBase = std::variant<
	std::nullptr_t,
	bool,
	double,
	int64_t,
	vector2d,
	vector3d,
	Color,
	Quaternionf,
	StringName,
	PropertyMapWrapper>;

/*
 * A property is a simple tagged union type, storing the most common datatypes
 * interchanged between C++ <-> Lua. It's meant to be small (32 bytes),
 * efficient, and flexible.
 *
 * It purposefully does not store matrix types, arbitrary pointers, classes,
 * or LuaObjects, as those should be handled by more domain-specific structures
 * and interfaces.
 */
class Property : public PropertyBase {
public:
	using PropertyBase::PropertyBase;

	// Overloads for integral and floating-point type construction for GCC 9/MSVC
	template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
	Property(const T &arg) : PropertyBase(int64_t(arg)) {}
	template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
	Property(const T &arg) : PropertyBase(double(arg)) {}

	// Promoting overload for PropertyMap*, because GCC 9 converts it to bool instead...
	Property(PropertyMap *map) : PropertyBase(PropertyMapWrapper(map)) {};

	// construction from string literal
	template <size_t N>
	Property(const char (&s)[N]) :
		Property(StringName(std::string_view(s, N - 1), hash_32_fnv1a(s, N - 1))) {}
	// have to have all of these string constructors to get around the one-implicit-conversion rule
	Property(const char *s) :
		Property(std::string_view(s)) {}
	Property(std::string_view s) :
		Property(StringName(s)) {}
	Property(const std::string &s) :
		Property(StringName(std::string_view(s))) {}

	bool is_null() const { return index() == 0 || index() == std::variant_npos; }
	bool is_bool() const { return index() == 1; }
	bool is_number() const { return index() == 2 || index() == 3; }
	bool is_integer() const { return index() == 2 || index() == 3; }
	bool is_vector2() const { return index() == 4; }
	bool is_vector3() const { return index() == 5; }
	bool is_color() const { return index() == 6; }
	bool is_quat() const { return index() == 7; }
	bool is_string() const { return index() == 8; }
	bool is_map() const { return index() == 9; }

	bool get_bool(bool def = false) const { return _get(std::move(def)); }
	double get_number(double def = 0.0) const
	{
		if (index() == 3)
			return double(_get(int64_t(def)));
		else
			return _get(std::move(def));
	}
	int64_t get_integer(int64_t def = 0) const
	{
		if (index() == 2)
			return int64_t(_get(double(def)));
		else
			return _get(std::move(def));
	}
	vector2d get_vector2(vector2d def = {}) const { return _get(std::move(def)); }
	vector3d get_vector3(vector3d def = {}) const { return _get(std::move(def)); }
	Color get_color(Color def = {}) const { return _get(std::move(def)); }
	Quaternionf get_quat(Quaternionf def = {}) const { return _get(std::move(def)); }
	StringName get_string(std::string_view def = {}) const { return _get<std::string_view, StringName>(std::move(def)); }
	PropertyMap *get_map(PropertyMap *def = {}) const { return _get<PropertyMapWrapper>(std::move(def)); }

	// helpers for common conversions
	template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
	operator T() const { return get_integer(); }
	template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
	operator T() const { return get_number(); }

	operator bool() const { return get_bool(); }
	operator double() const { return get_number(); }
	operator int64_t() const { return get_integer(); }
	operator vector2d() const { return get_vector2(); }
	operator vector3d() const { return get_vector3(); }
	operator Color() const { return get_color(); }
	operator Quaternionf() const { return get_quat(); }
	operator StringName() const { return get_string(); }
	operator PropertyMap *() const { return get_map(); }

private:
	template <typename T, typename U = T>
	U _get(T &&def) const
	{
		if (auto *ptr = std::get_if<U>(this))
			return *ptr;
		else
			return std::move(def);
	}
};

// JSON overloads for Property types
void from_json(const Json &obj, Property &n);
void to_json(Json &obj, const Property &n);

/*
 * PropertyMap implements an efficient key-value store of Properties that can
 * be shared between Lua and C++ and persisted across the destruction of a Lua
 * VM.
 *
 * Internally, a power-of-two based Robin-Hood hash map is used to associate
 * StringName keys with Property values with extremely low hashing and lookup
 * overhead.
 */
class PropertyMap : public RefCounted {
public:
	using value_type = std::pair<StringName, Property>;
	using reference = const value_type &;
	using pointer = const value_type *;

	struct iterator {
		iterator(const PropertyMap *m, uint32_t i) :
			map(m),
			idx(i)
		{
			if (map && map->m_keys.size() > idx && !map->m_keys[idx])
				++(*this);
		}

		pointer operator->() { return &(**this); }
		reference operator*() { return map->m_values[idx]; }

		operator bool() { return map && idx < map->Capacity(); }

		bool operator==(const iterator &rhs) { return map == rhs.map && idx == rhs.idx; }
		bool operator!=(const iterator &rhs) { return !(*this == rhs); }
		iterator operator++();
		iterator operator++(int);

	private:
		const PropertyMap *map;
		uint32_t idx;
	};

public:
	PropertyMap();
	~PropertyMap();

	void SaveToJson(Json &obj);
	void LoadFromJson(const Json &obj);

	size_t Size() const { return m_entries; }
	size_t Capacity() const { return m_keys.size(); }
	bool Empty() const { return !m_entries; }

	// Explicit literal string overload to ensure hashing is run at compile time
	template <size_t N>
	const Property &Get(const char str[N]) const
	{
		constexpr uint32_t hash = hash_32_fnv1a(str, N - 1);
		return GetRef(hash).second;
	}

	const Property &Get(const StringName &str) const { return GetRef(str.hash()).second; }
	const Property &Get(std::string_view str) const { return GetRef(hash_32_fnv1a(str.data(), str.size())).second; }

	void Set(const StringName &key, Property &&prop) { SetRef(key.hash(), { key, std::move(prop) }); }
	void Set(std::string_view str, Property &&prop) { Set(StringName(str), std::move(prop)); }

	// Use template-based forwarding for older compilers which cannot convert e.g. int to Property&&
	template<typename T>
	void Set(const StringName &key, const T &prop) { Set(key, Property(prop)); }
	template<typename T>
	void Set(std::string_view str, const T &prop) { Set(StringName(str), Property(prop)); }

	void Clear();

	iterator begin() { return iterator{ this, 0 }; }
	iterator end() { return iterator{ this, uint32_t(m_keys.size()) }; }
	iterator cbegin() const { return iterator{ this, 0 }; }
	iterator cend() const { return iterator{ this, uint32_t(m_keys.size()) }; }

	operator PropertyMapWrapper() { return PropertyMapWrapper(this); }

private:
	reference GetRef(uint32_t hash) const;
	void SetRef(uint32_t hash, value_type &&value);

	PropertyMap(uint32_t size);
	void Grow();

	std::vector<uint32_t> m_keys;
	std::vector<value_type> m_values;
	uint32_t m_entries;
};

inline auto PropertyMapWrapper::begin() { return m_map->begin(); }
inline auto PropertyMapWrapper::end() { return m_map->end(); }
