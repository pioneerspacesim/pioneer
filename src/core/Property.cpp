// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Property.h"

#include "Json.h"
#include "JsonUtils.h"

PropertyMapWrapper::PropertyMapWrapper(PropertyMap *m) :
	m_map(m)
{
	if (m_map)
		m_map->IncRefCount();
}

PropertyMapWrapper::~PropertyMapWrapper()
{
	if (m_map)
		m_map->DecRefCount();
}

PropertyMapWrapper &PropertyMapWrapper::operator=(const PropertyMapWrapper &rhs)
{
	if (m_map)
		m_map->DecRefCount();
	if (rhs.m_map)
		rhs.m_map->IncRefCount();

	m_map = rhs.m_map;
	return *this;
}

// =============================================================================

void from_json(const Json &obj, Property &n)
{
	n = nullptr;
	if (!obj.is_array() || obj.size() != 2 || !obj[0].is_string())
		return;

	const std::string &typeKey = obj[0].get_ref<const std::string &>();
	switch (hash_64_fnv1a(typeKey.data(), typeKey.size())) {
	case ("bool"_hash): {
		n = obj[1].get<bool>();
	} break;
	case ("num"_hash): {
		n = obj[1].get<double>();
	} break;
	case ("int"_hash): {
		n = obj[1].get<int64_t>();
	} break;
	case ("vec2"_hash): {
		n = obj[1].get<vector2d>();
	} break;
	case ("vec3"_hash): {
		n = obj[1].get<vector3d>();
	} break;
	case ("color"_hash): {
		n = obj[1].get<Color>();
	} break;
	case ("quat"_hash): {
		n = obj[1].get<Quaternionf>();
	} break;
	case ("str"_hash): {
		n = obj[1].get<std::string>();
	} break;
	case ("map"_hash): {
		n = new PropertyMap();
		n.get_map()->LoadFromJson(obj[1]);
	}
	default:
		break;
	}
}

void to_json(Json &obj, const Property &n)
{
	switch (n.index()) {
	case 1:
		obj = Json::array({ "bool", n.get_bool() });
		break;
	case 2:
		obj = Json::array({ "num", n.get_number() });
		break;
	case 3:
		obj = Json::array({ "int", n.get_integer() });
		break;
	case 4:
		obj = Json::array({ "vec2", n.get_vector2() });
		break;
	case 5:
		obj = Json::array({ "vec3", n.get_vector3() });
		break;
	case 6:
		obj = Json::array({ "color", n.get_color() });
		break;
	case 7:
		obj = Json::array({ "quat", n.get_quat() });
		break;
	case 8:
		obj = Json::array({ "str", n.get_string().sv() });
		break;
	case 9:
		obj = Json::array({ "map", {} });
		n.get_map()->SaveToJson(obj[1]);
		break;
	default:
		obj = {};
	}
}

// =============================================================================

PropertyMap::iterator PropertyMap::iterator::operator++()
{
	size_t sz = map->m_keys.size();
	if (map && sz > idx)
		while (++idx < sz && !map->m_keys[idx]) {
		}
	return *this;
}

PropertyMap::iterator PropertyMap::iterator::operator++(int)
{
	iterator res = *this;
	++(*this);
	return res;
}

// =============================================================================

PropertyMap::PropertyMap() :
	m_keys(),
	m_values(),
	m_entries(0)
{}

PropertyMap::PropertyMap(uint32_t size) :
	m_keys(size),
	m_values(size),
	m_entries(0)
{}

PropertyMap::~PropertyMap()
{
	Clear();
}

// =============================================================================

void PropertyMap::LoadFromJson(const Json &obj)
{
	Clear();
	if (!obj.is_object())
		return;

	for (auto &pair : obj.items()) {
		Set(pair.key(), pair.value().get<Property>());
	}
}

void PropertyMap::SaveToJson(Json &obj)
{
	obj = Json::object();

	for (auto &pair : *this) {
		obj[pair.first.c_str()] = pair.second;
	}
}

static PropertyMap::value_type null_map_value{};

// Implementation adapted from:
// https://preshing.com/20130605/the-worlds-simplest-lock-free-hash-table/
// PropertyMap requires that the underlying vector size be a power of two for
// best performance.

PropertyMap::reference PropertyMap::GetRef(uint32_t hash) const
{
	if (Empty()) return null_map_value;

	for (uint32_t idx = hash;; idx++) {
		idx &= m_keys.size() - 1;

		uint32_t probed_key = m_keys[idx];
		if (probed_key == hash || probed_key == 0)
			return m_values[idx];
	}
}

void PropertyMap::SetRef(uint32_t hash, value_type &&value)
{
	if (m_entries >= uint32_t(m_keys.size() * 0.8))
		Grow();

	for (uint32_t idx = hash;; idx++) {
		idx &= m_keys.size() - 1;

		uint32_t probed_key = m_keys[idx];
		if (probed_key == hash || probed_key == 0) {
			if (probed_key == 0)
				m_entries++;

			m_keys[idx] = hash;
			m_values[idx] = std::move(value);
			return;
		}
	}
}

void PropertyMap::Clear()
{
	for (uint32_t idx = 0; idx < m_keys.size(); idx++) {
		uint32_t probed_key = m_keys[idx];
		if (probed_key)
			m_values[idx] = {};
	}

	if (m_keys.size())
		std::fill(m_keys.begin(), m_keys.end(), 0);
	m_entries = 0;
}

void PropertyMap::Grow()
{
	// we need a "fresh" map to rehash all keys into without making temporaries
	size_t new_size = m_keys.size() ? m_keys.size() * 2 : 32;
	PropertyMap newMap(new_size);

	for (uint32_t idx = 0; idx < m_keys.size(); idx++) {
		uint32_t hash = m_keys[idx];
		if (hash) {
			m_keys[idx] = 0;

			// inline the important bits of Set for slightly faster execution
			for (uint32_t jdx = hash;; jdx++) {
				jdx &= new_size - 1;

				uint32_t probed_key = newMap.m_keys[jdx];
				if (probed_key == 0) {

					newMap.m_keys[jdx] = hash;
					newMap.m_values[jdx] = std::move(m_values[idx]);
					break;
				}
			}
		}
	}

	// move map storage
	std::swap(m_keys, newMap.m_keys);
	std::swap(m_values, newMap.m_values);
}
