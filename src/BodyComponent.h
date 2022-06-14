// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "JsonFwd.h"
#include "core/TypeId.h"

#include <cassert>
#include <cstddef>
#include <map>
#include <vector>

class Body;
class Space;
class BodyComponent {};

/*
	BodyComponentDB provides a simple interface to support dynamic composition
	of game objects.

	It is intended for use as a transitional interface to facilitate the
	migration of Pioneer's inheritance-hierarchy based object model to an ECS-
	style system, and thus is not and never will be a permanent solution.

	Please direct all concerns about speed and efficiency to /dev/null
	Improvements to compile times are welcomed wherever possible.
*/
class BodyComponentDB {
public:
	// Polymorphic interface to support generic serialization operations
	// This functionality is separated to facilitate components that do not wish
	// to be serialized.
	struct SerializerBase {
		SerializerBase(std::string name) :
			typeName(name) {}
		std::string typeName;
		virtual void toJson(const Body *body, Json &obj, Space *space) = 0;
		virtual BodyComponent *fromJson(Body *body, const Json &obj, Space *space) = 0;
	};

	// Polymorphic interface to support generic deletion operations
	struct PoolBase {
		PoolBase(size_t index, size_t type) :
			componentIndex(index),
			componentType(type) {}
		size_t componentIndex = 0;
		size_t componentType = 0;
		SerializerBase *serializer = nullptr;

		virtual void deleteComponent(Body *body) = 0;
	};

	template <typename T>
	struct Serializer;

	// Type-specific component pool; uses std::map as a backing store.
	// This is not meant to be particularly performant, merely to transition API usage.
	template <typename T>
	struct Pool final : public PoolBase {
		using PoolBase::PoolBase;

		virtual void deleteComponent(Body *body) override { m_components.erase(body); }
		// Create a new component, or return the existing one.
		T *newComponent(const Body *body) { return &m_components[body]; }
		// Assert that a component exists for this body and return it
		T *get(const Body *body) { return &m_components.at(body); }

	private:
		template <typename U>
		friend struct BodyComponentDB::Serializer;
		std::map<const Body *, T> m_components;
	};

	// Type-specific serialization implementation. Delegates to the type's
	// internal serialization methods and provides the needed glue code.
	template <typename T>
	struct Serializer final : public SerializerBase {
		Serializer(std::string name, Pool<T> *pool) :
			SerializerBase(name),
			pool(pool)
		{}
		Pool<T> *pool;

		virtual void toJson(const Body *body, Json &obj, Space *space) override
		{
			pool->get(body)->SaveToJson(obj, space);
		}

		virtual BodyComponent *fromJson(Body *body, const Json &obj, Space *space) override
		{
			pool->deleteComponent(body);
			auto *component = pool->newComponent(body);
			component->LoadFromJson(obj, space);
			return component;
		}
	};

	// Returns (and creates) a type-specific pool.
	template <typename T>
	static Pool<T> *GetComponentType()
	{
		auto iter = m_componentPools.find(TypeId<T>::Get());
		if (iter == m_componentPools.end()) {
			iter = m_componentPools.emplace(TypeId<T>::Get(), new Pool<T>(m_componentIdx++, TypeId<T>::Get())).first;
			m_componentTypes.push_back(TypeId<T>::Get());
		}

		return static_cast<Pool<T> *>(iter->second.get());
	}

	// Returns (if present) the polymorphic interface to component associated with the given index
	// This differs from the type-ID and is volatile between program restarts
	static PoolBase *GetComponentType(size_t componentIndex) { return m_componentPools.at(m_componentTypes[componentIndex]).get(); }

	// Register a serializer for the given type.
	template <typename T>
	static bool RegisterSerializer(std::string typeName)
	{
		assert(!m_componentSerializers.count(typeName));
		SerializerBase *serial = new Serializer<T>(typeName, GetComponentType<T>());
		m_componentSerializers.emplace(typeName, serial);
		GetComponentType<T>()->serializer = serial;
		return true;
	}

	// Returns a pointer to the registered Serializer instance for a type identified by the given name, or nullptr.
	// To retrieve the serializer instance for a given type index, use GetComponentType(idx)->serializer
	// or GetComponentType<T>()->serializer.
	static SerializerBase *GetSerializer(const std::string &typeName)
	{
		auto iter = m_componentSerializers.find(typeName);
		if (iter != m_componentSerializers.end())
			return iter->second.get();

		return nullptr;
	}

private:
	static std::map<size_t, std::unique_ptr<PoolBase>> m_componentPools;
	static std::map<std::string, std::unique_ptr<SerializerBase>> m_componentSerializers;
	static std::vector<size_t> m_componentTypes;
	static size_t m_componentIdx;
};
