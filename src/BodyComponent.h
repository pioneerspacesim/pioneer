// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
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

// Macro to handle registering components at startup
#define REGISTER_COMPONENT_TYPE(type)                                              \
	namespace type##RegisterComponent                                              \
	{                                                                              \
		void Register();                                                           \
		bool type##Registered = BodyComponentDB::AddComponentRegistrar(&Register); \
	}                                                                              \
	void type##RegisterComponent::Register()

/*
	BodyComponentDB provides a simple interface to support dynamic composition
	of game objects. It is intended to be an interim solution to assist in
	transitioning Pioneer's inheritance hierarchy to a simpler composition
	model.
*/
class BodyComponentDB {
public:
	static void Init();
	static void Uninit();

	// Add a registrar to create and register a component pool using the REGISTER_COMPONENT_TYPE macro
	static bool AddComponentRegistrar(void (*registrar)());

	// Polymorphic interface to support generic serialization operations
	// This functionality is separated to facilitate components that do not wish
	// to be serialized.
	struct SerializerBase {
		SerializerBase() {}
		virtual ~SerializerBase() {}

		virtual void toJson(const Body *body, Json &obj, Space *space) = 0;
		virtual void fromJson(Body *body, const Json &obj, Space *space) = 0;
	};

	// Polymorphic interface to support pushing type-erased components to Lua
	// This functionality is separated to facilitate components that do not wish
	// to be accessed from Lua.
	struct LuaInterfaceBase {
		LuaInterfaceBase() {};
		virtual ~LuaInterfaceBase() {}

		virtual void PushToLua(const Body *body) = 0;
		virtual void DeregisterComponent(const Body *body) = 0;
	};

	// Primary polymorphic interface to component storage. PoolBase contains all
	// functionality for operating on specific components in a type-erased manner.
	struct PoolBase {
		PoolBase(size_t index, size_t type) :
			componentIndex(index),
			componentType(type) {}
		virtual ~PoolBase();

		size_t componentIndex = 0;
		size_t componentType = 0;

		// Pointer to an optional type-erased serializer instance.
		// The serializer will persist for the lifetime of the program.
		SerializerBase *serializer = nullptr;

		// Pointer to an optional type-erased lua interface helper.
		// The lua interface will be deleted by the pool's destructor.
		LuaInterfaceBase *luaInterface = nullptr;

		// Primary name of the component type for serialization purposes.
		std::string typeName;

		virtual void deleteComponent(Body *body) = 0;
	};

	// Forward-declared to allow access to specializations of the Pool struct.
	template <typename T>
	struct Serializer;

	// Intentionally left undefined, implemented in lua/LuaBodyComponent.h
	template <typename T>
	struct LuaInterface;

	// Type-specific component pool; uses std::map as a backing store.
	// This is not meant to be particularly performant, merely to transition API usage.
	// Future optimization may include use of a custom sparse-set or hash-table for
	// faster lookup times.
	template <typename T>
	struct Pool final : public PoolBase {
		using PoolBase::PoolBase;

		// Delete the component and delegate removing it from lua if necessary
		virtual void deleteComponent(Body *body) override
		{
			if (luaInterface)
				luaInterface->DeregisterComponent(body);

			m_components.erase(body);
		}

		// Create a new component, or return the existing one.
		T *newComponent(const Body *body) { return &m_components[body]; }
		// Assert that a component exists for this body and return it
		T *get(const Body *body) { return &m_components.at(body); }

	private:
		template <typename U>
		friend struct BodyComponentDB::Serializer;
		template <typename U>
		friend struct BodyComponentDB::LuaInterface;

		std::map<const Body *, T> m_components;
	};

	// Type-specific serialization implementation. Delegates to the type's
	// internal serialization methods and provides the needed glue code.
	//
	// The Component::LoadFromJson method will be called after the component
	// is constructed and added to the body, and may potentially have defaults
	// set by the owning Body before it is deserialized.
	template <typename T>
	struct Serializer final : public SerializerBase {
		Serializer(Pool<T> *pool) :
			pool(pool)
		{}
		Pool<T> *pool;

		virtual void toJson(const Body *body, Json &obj, Space *space) override
		{
			pool->get(body)->SaveToJson(obj, space);
		}

		virtual void fromJson(Body *body, const Json &obj, Space *space) override
		{
			auto *component = pool->newComponent(body);
			component->LoadFromJson(obj, space);
		}
	};

	// Returns (and creates) a type-specific pool.
	template <typename T>
	static Pool<T> *GetComponentType()
	{
		auto iter = m_componentPools.find(TypeId<T>::Get());
		if (iter == m_componentPools.end()) {
			return nullptr;
		}

		return static_cast<Pool<T> *>(iter->second.get());
	}

	// Returns (if present) the polymorphic interface to component associated with the given index
	// This differs from the type-ID and is volatile between program restarts
	static PoolBase *GetComponentType(size_t componentIndex)
	{
		assert(componentIndex < m_componentTypes.size());
		return m_componentTypes[componentIndex];
	}

	// Returns (if present) the polymorphic interface to the component type associated with the given name
	// This name can be used for serialization or to interface with Lua
	static PoolBase *GetComponentType(const std::string &typeName)
	{
		auto iter = m_componentNames.find(typeName);
		if (iter != m_componentNames.end())
			return iter->second;

		return nullptr;
	}

	// Explicitly create and register a component pool. This function should be called during startup
	// for all component types used at runtime.
	template <typename T>
	static void RegisterComponent(std::string typeName)
	{
		// We cannot have more components registered than fit in the Body bitset
		assert(m_componentIdx < 64);
		assert(m_componentPools.find(TypeId<T>::Get()) == m_componentPools.end());

		auto *pool = new Pool<T>(m_componentIdx++, TypeId<T>::Get());
		pool->typeName = typeName;
		m_componentPools.emplace(TypeId<T>::Get(), pool);
		m_componentNames.emplace(typeName, pool);
		m_componentTypes.push_back(pool);
	}

	// Register a component type to be queryable from Lua with body:GetComponent()
	template <typename T>
	static bool RegisterLuaInterface()
	{
		Pool<T> *pool = GetComponentType<T>();
		if (pool->luaInterface)
			return false;

		pool->luaInterface = new LuaInterface<T>(pool);
		return true;
	}

	// Register a serializer for the given type.
	// Multiple serializers can be registered for the given type and used while
	// loading for backwards compatibility, however only the last-registered
	// serializer will be used when serializing to JSON
	template <typename T>
	static bool RegisterSerializer(std::string typeName = {})
	{
		Pool<T> *pool = GetComponentType<T>();
		if (typeName.empty())
			typeName = pool->typeName;

		assert(!m_componentSerializers.count(typeName));

		SerializerBase *serial = new Serializer<T>(pool);
		pool->serializer = serial;

		m_componentSerializers.emplace(typeName, serial);
		m_componentNames.emplace(typeName, pool);
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
	static std::map<std::string, PoolBase *> m_componentNames;
	static std::vector<PoolBase *> m_componentTypes;
	static size_t m_componentIdx;
};
