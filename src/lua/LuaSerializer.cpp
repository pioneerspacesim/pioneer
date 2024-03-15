// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaSerializer.h"
#include "GameSaveError.h"
#include "JsonUtils.h"
#include "LuaObject.h"

// Needed for LuaComponent serialization
#include "Body.h"
#include "Space.h"

#include "core/Log.h"
#include "profiler/Profiler.h"

// Well-known names of various serialization-related caches stored in the
// Lua Registry
static const char *NS_REFTABLE = "PiSerializerTableRefs";
static const char *NS_CLASSES = "PiSerializerClasses";
static const char *NS_CALLBACKS = "PiSerializerCallbacks";
static const char *NS_PERSISTENT = "PiSerializerPersistent";

// every module can save one object. that will usually be a table.  we call
// each serializer in turn and capture its return value we build a table like
// so:
// {
//   'Assassination' = { ... },
//   'DeliverPackage' = { ... },
//   ...
// }
// entire table then gets pickled and handed to the writer
//
// on load, we unpickle the table then call the registered unserialize
// function for each module with its table
//
// we keep a copy of this table around. next time we save we overwrite the
// each individual module's data. that way if a player loads a game with data
// for a module that is not currently loaded, we don't lose its data in the
// next save

// pickler can handle simple types (boolean, number, string) and will drill
// down into tables. it can do userdata assuming the appropriate Lua wrapper
// class has registered a serializer and deserializer
//
// Data is picked into JSON format, with a few specifics about file structure.
// Tables are pickled into a JSON Array with key,value pairs occupying
// consecutive entries in the array. Thus, one key-value pair takes 2 array
// slots.
// A top-level JSON object can represent several different types of lua objects
// using a scheme as follows:
//   "lua_table": {"table": []} - defines a new lua table with values pickled
//                                in the 'table' array
//   "ref": 10491324            - references a previously-defined lua table
//   "lua_class": "ClassName"   - indicates this Lua table is an object of a
//                                specific class object registered by Lua
//   "cpp_class": "ClassName"   - this object is a C++ userdata of a specific
//                                class registered by C++ of the same name

// on serialize, if an item has a metatable with a "class" attribute, the
// "Serialize" function under that namespace will be called with the type. the
// data returned will then be serialized as an "object" above.
//
// on deserialize, the data after an "object" item will be passed to the
// "Deserialize" function under that namespace. that data returned will be
// given back to the module

void LuaSerializer::pickle_json(lua_State *l, int to_serialize, Json &out, const std::string &key)
{
	PROFILE_SCOPED()
	LUA_DEBUG_START(l);

	// tables are pickled recursively, so we can run out of Lua stack space if we're not careful
	// start by ensuring we have enough (this grows the stack if necessary)
	// (20 is somewhat arbitrary)
	if (!lua_checkstack(l, 20))
		luaL_error(l, "The Lua stack couldn't be extended (out of memory?)");

	to_serialize = lua_absindex(l, to_serialize);
	int idx = to_serialize;

	if (lua_getmetatable(l, idx)) {
		lua_getfield(l, -1, "class");
		if (lua_isnil(l, -1))
			lua_pop(l, 2);

		else {
			const char *cl = lua_tostring(l, -1);

			out["lua_class"] = cl;

			lua_getfield(l, LUA_REGISTRYINDEX, NS_CLASSES);

			lua_getfield(l, -1, cl);
			if (lua_isnil(l, -1))
				luaL_error(l, "Class '%s' not registered for serialization\n", cl);

			lua_getfield(l, -1, "Serialize");
			if (lua_isnil(l, -1))
				luaL_error(l, "No Serialize method found for class '%s'\n", cl);

			lua_pushvalue(l, idx);
			pi_lua_protected_call(l, 1, 1);

			idx = lua_gettop(l);

			if (lua_isnil(l, idx)) {
				lua_pop(l, 5);
				LUA_DEBUG_END(l, 0);
				return;
			}
		}
	}

	switch (lua_type(l, idx)) {
	case LUA_TNIL:
		out = Json();
		break;

	case LUA_TBOOLEAN: {
		out = Json(static_cast<bool>(lua_toboolean(l, idx)));
		break;
	}

	case LUA_TSTRING: {
		lua_pushvalue(l, idx);
		size_t len;
		const char *str = lua_tolstring(l, -1, &len);
		out = Json(std::string(str, str + len));
		lua_pop(l, 1);
		break;
	}

	case LUA_TNUMBER: {
		out = lua_tonumber(l, idx);
		break;
	}

	case LUA_TTABLE: {
		lua_Integer ptr = lua_Integer(lua_topointer(l, to_serialize));
		lua_pushinteger(l, ptr); // ptr

		lua_getfield(l, LUA_REGISTRYINDEX, NS_REFTABLE); // ptr reftable
		lua_pushvalue(l, -2);							 // ptr reftable ptr
		lua_rawget(l, -2);								 // ptr reftable ???

		out["ref"] = ptr;

		if (!lua_isnil(l, -1)) {
			lua_pop(l, 3); // [empty]
		} else {
			lua_pushvalue(l, -3);			// ptr reftable nil ptr
			lua_pushvalue(l, to_serialize); // ptr reftable nil ptr table
			lua_rawset(l, -4);				// ptr reftable nil
			lua_pop(l, 3);					// [empty]

			Json inner = Json::array();

			lua_pushvalue(l, idx);
			lua_pushnil(l);
			while (lua_next(l, -2)) {
				lua_pushvalue(l, -2);
				const char *k = lua_tostring(l, -1);
				std::string new_key = key + "." + (k ? std::string(k) : "<" + std::string(lua_typename(l, lua_type(l, -1))) + ">");
				lua_pop(l, 1);

				// Copy the values to pickle, as they might be mutated by the pickling process.
				Json out_k, out_v;

				pickle_json(l, -2, out_k, new_key);
				pickle_json(l, -1, out_v, new_key);

				inner.push_back(out_k);
				inner.push_back(out_v);

				lua_pop(l, 1);
			}
			lua_pop(l, 1);

			out["table"] = Json(inner);
		}

		break;
	}

	case LUA_TUSERDATA: {
		lua_pushvalue(l, to_serialize);
		Json obj = Json::object();
		if (LuaObjectBase::SerializeToJson(l, obj))
			out = obj;
		else
			Log::Error("Lua serializer '{}' tried to serialize an invalid object\n"
					   "The save file may be invalid.\n",
				key.c_str());

		lua_pop(l, 1);
		break;
	}

	default:
		Log::Error("Lua serializer '{}' tried to serialize {} value", key.c_str(), lua_typename(l, lua_type(l, idx)));
		break;
	}

	if (idx != lua_absindex(l, to_serialize)) // It means we called a transformation function on the data, so we clean it up.
		lua_pop(l, 5);

	LUA_DEBUG_END(l, 0);
}

void LuaSerializer::unpickle_json(lua_State *l, const Json &value)
{
	PROFILE_SCOPED()
	LUA_DEBUG_START(l);

	// tables are also unpickled recursively, so we can run out of Lua stack space if we're not careful
	// start by ensuring we have enough (this grows the stack if necessary)
	// (20 is somewhat arbitrary)
	if (!lua_checkstack(l, 20))
		luaL_error(l, "The Lua stack couldn't be extended (not enough memory?)");

	switch (value.type()) {
	case Json::value_t::null:
		lua_pushnil(l);
		break;
	case Json::value_t::number_integer:
	case Json::value_t::number_unsigned:
		lua_pushinteger(l, value);
		LUA_DEBUG_CHECK(l, 1);
		break;
	case Json::value_t::number_float:
		lua_pushnumber(l, value);
		LUA_DEBUG_CHECK(l, 1);
		break;
	case Json::value_t::string:
		// FIXME: Should do something to make sure we can unpickle strings that include null bytes.
		// However I'm not sure that the JSON library actually supports strings containing nulls which would make it moot.
		lua_pushstring(l, value.get<std::string>().c_str());
		LUA_DEBUG_CHECK(l, 1);
		break;
	case Json::value_t::boolean:
		lua_pushboolean(l, value);
		LUA_DEBUG_CHECK(l, 1);
		break;
	case Json::value_t::array:
		// Pickle doesn't emit array type values except as part of another structure.
		throw SavedGameCorruptException();
		break;
	case Json::value_t::object:
		if (value.count("userdata")) {
			if (!LuaObjectBase::DeserializeFromJson(l, value["userdata"])) {
				throw SavedGameCorruptException();
			}
			LUA_DEBUG_CHECK(l, 1);
		} else if (value.count("cpp_class")) {
			if (!LuaObjectBase::DeserializeFromJson(l, value))
				throw SavedGameCorruptException();
			LUA_DEBUG_CHECK(l, 1);
		} else {
			// Object, table, or table-reference.
			if (value["ref"].is_null()) {
				throw SavedGameCorruptException();
			}

			lua_Integer ptr = value["ref"];

			if (value.count("table")) {
				lua_newtable(l);

				lua_getfield(l, LUA_REGISTRYINDEX, NS_REFTABLE); // [t] [refs]
				lua_pushinteger(l, ptr);						 // [t] [refs] [key]
				lua_pushvalue(l, -3);							 // [t] [refs] [key] [t]
				lua_rawset(l, -3);								 // [t] [refs]
				lua_pop(l, 1);									 // [t]

				const Json &inner = value["table"];
				if (inner.size() % 2 != 0) {
					throw SavedGameCorruptException();
				}
				for (size_t i = 0; i < inner.size(); i += 2) {
					unpickle_json(l, inner[i + 0]);
					unpickle_json(l, inner[i + 1]);
					lua_rawset(l, -3);
				}

				LUA_DEBUG_CHECK(l, 1);
			} else {
				// Reference to a previously-pickled table.
				lua_getfield(l, LUA_REGISTRYINDEX, NS_REFTABLE); // [refs]
				lua_pushinteger(l, ptr);						 // [refs] [key]
				lua_rawget(l, -2);								 // [refs] [out]

				if (lua_isnil(l, -1))
					throw SavedGameCorruptException();

				lua_remove(l, -2); // [out]

				LUA_DEBUG_CHECK(l, 1);
			}

			if (value.count("lua_class")) {
				const char *cl = value["lua_class"].get_ref<const std::string &>().c_str();
				// If this was a full definition (not just a reference) then run the class's unserialiser function.
				if (value.count("table")) {
					lua_getfield(l, LUA_REGISTRYINDEX, NS_CLASSES);
					lua_pushstring(l, cl);
					lua_gettable(l, -2);
					lua_remove(l, -2);

					if (lua_isnil(l, -1)) {
						lua_pop(l, 1);
					} else {
						lua_getfield(l, -1, "Unserialize"); // [t] [klass] [klass.Unserialize]
						if (lua_isnil(l, -1)) {
							luaL_error(l, "No Unserialize method found for class '%s'\n", cl);
						} else {
							lua_insert(l, -3); // [klass.Unserialize] [t] [klass]
							lua_pop(l, 1);	   // [klass.Unserialize] [t]

							pi_lua_protected_call(l, 1, 1); // [t]
							if (lua_isnil(l, -1)) {
								luaL_error(l, "The Unserialize method for class '%s' didn't return a value\n", cl);
							}

							// Update the TableRefs cache with the new value
							// NOTE: recursive references to the original table will not be affected,
							// only references in tables deserialized later.
							lua_getfield(l, LUA_REGISTRYINDEX, NS_REFTABLE); // [t] [refs]
							lua_pushinteger(l, ptr);						 // [t] [refs] [key]
							lua_pushvalue(l, -3);							 // [t] [refs] [key] [t]
							lua_rawset(l, -3);								 // [t] [refs]
							lua_pop(l, 1);									 // [t]
						}
					}
				}
				LUA_DEBUG_CHECK(l, 1);
			}
		}
		break;

	default:
		throw SavedGameCorruptException();
	}

	LUA_DEBUG_END(l, 1);
}

void LuaSerializer::InitTableRefs()
{
	lua_State *l = Lua::manager->GetLuaState();

	lua_pushlightuserdata(l, this);
	lua_setfield(l, LUA_REGISTRYINDEX, "PiSerializer");

	lua_newtable(l);
	lua_setfield(l, LUA_REGISTRYINDEX, NS_REFTABLE);

	// NOTE: this is depended on by LuaRef.cpp
	lua_newtable(l);
	lua_setfield(l, LUA_REGISTRYINDEX, "PiLuaRefLoadTable");
}

void LuaSerializer::UninitTableRefs()
{
	lua_State *l = Lua::manager->GetLuaState();

	lua_pushnil(l);
	lua_setfield(l, LUA_REGISTRYINDEX, "PiSerializer");

	lua_pushnil(l);
	lua_setfield(l, LUA_REGISTRYINDEX, NS_REFTABLE);

	lua_pushnil(l);
	lua_setfield(l, LUA_REGISTRYINDEX, "PiLuaRefLoadTable");
}

void LuaSerializer::SavePersistent(Json &json)
{
	lua_State *l = Lua::manager->GetLuaState();
	LUA_DEBUG_START(l);

	// NOTE: this must be an array for consistent deserialization order
	Json persist = Json::array();

	luaL_getsubtable(l, LUA_REGISTRYINDEX, NS_PERSISTENT);
	lua_pushnil(l);

	while (lua_next(l, -2)) {
		Json entry = Json::object();

		// persistent, id, value
		std::string id = LuaPull<std::string>(l, -2);

		// Serialize all registered persistent objects as a "backup" version
		// This will populate the table identity cache to avoid persistent
		// objects being serialized twice.
		entry["persistId"] = id;
		pickle_json(l, -1, entry, id);

		persist.push_back(std::move(entry));

		lua_pop(l, 1);
	}

	lua_pop(l, 1);

	json["lua_persistent_json"] = std::move(persist);

	LUA_DEBUG_END(l, 0);
}

void LuaSerializer::LoadPersistent(const Json &json)
{
	lua_State *l = Lua::manager->GetLuaState();
	LUA_DEBUG_START(l);

	// Old savefile with no persistent table.
	if (!json.count("lua_persistent_json"))
		return;

	const Json &persist = json["lua_persistent_json"];

	luaL_getsubtable(l, LUA_REGISTRYINDEX, NS_REFTABLE);
	int idx_reftable = lua_gettop(l);

	luaL_getsubtable(l, LUA_REGISTRYINDEX, NS_PERSISTENT);
	int idx_persist = lua_gettop(l);

	for (const auto &item : persist) {
		std::string id = item["persistId"].get<std::string>();

		lua_pushinteger(l, item["ref"].get<lua_Integer>());
		lua_getfield(l, idx_persist, id.c_str()); // reftable, persist, ptr, pvalue

		// Serialization order is first-in first-out - because persistent objects
		// are serialized before all other objects, their serialized representation
		// may contain the definition of "common subtables" referred to by other
		// objects not serialized in the persistent table.
		// Thus, (as a rule) we must always unserialize every object that was
		// initially serialized, in serialization order, to populate the reference
		// table and avoid any potential dangling references to serialized tables.
		unpickle_json(l, item);

		// No valid persistent value (mismatch in serialization!)
		if (lua_isnil(l, -2)) {
			Log::Warning("Restoring missing persistent object '{}' from savefile", id);
			lua_replace(l, -2); // reftable, persistent, ptr, svalue

			// Write this back to the persistent table so it will be included
			// if the file is re-saved.
			// TODO: because the Lua state is shared across save/load cycles,
			// this value will "leak" into newly-started games. This is not
			// something that can be feasibly addressed except by creating new
			// lua_States on starting a new game.
			lua_pushvalue(l, -1); // reftable, persistent, ptr, svalue, svalue
			lua_setfield(l, idx_persist, id.c_str());
		} else {
			lua_pop(l, 1); // reftable, persistent, ptr, pvalue
		}

		// All references to the prior saved value are replaced with the persistent object
		lua_settable(l, idx_reftable);
	}

	lua_pop(l, 2);

	LUA_DEBUG_END(l, 0);
}

void LuaSerializer::ToJson(Json &jsonObj)
{
	PROFILE_SCOPED()
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	lua_newtable(l);
	int savetable = lua_gettop(l);

	luaL_getsubtable(l, LUA_REGISTRYINDEX, NS_CALLBACKS);

	lua_pushnil(l);
	while (lua_next(l, -2) != 0) {
		lua_pushinteger(l, 1);			// 1, fntable, key
		lua_gettable(l, -2);			// fn, fntable, key
		pi_lua_protected_call(l, 0, 1); // table, fntable, key
		lua_pushvalue(l, -3);			// key, table, fntable, key
		lua_insert(l, -2);				// table, key, fntable, key
		lua_settable(l, savetable);		// fntable, key
		lua_pop(l, 1);
	}

	lua_pop(l, 1);

	Json pickled;
	pickle_json(l, savetable, pickled);
	jsonObj["lua_modules_json"] = std::move(pickled);

	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}

void LuaSerializer::FromJson(const Json &jsonObj)
{
	PROFILE_SCOPED()
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	if (jsonObj.count("lua_modules_json")) {
		const Json &value = jsonObj["lua_modules_json"];
		if (!value.is_object()) {
			throw SavedGameCorruptException();
		}
		unpickle_json(l, value);
	} else if (jsonObj.count("lua_modules")) {
		// old text-based serialization
		throw SavedGameCorruptException();
	} else {
		throw SavedGameCorruptException();
	}

	if (!lua_istable(l, -1)) throw SavedGameCorruptException();
	int savetable = lua_gettop(l);

	luaL_getsubtable(l, LUA_REGISTRYINDEX, NS_CALLBACKS);

	lua_pushnil(l);
	while (lua_next(l, -2) != 0) {
		lua_pushvalue(l, -2);
		lua_pushinteger(l, 2);
		lua_gettable(l, -3);
		lua_getfield(l, savetable, lua_tostring(l, -2));
		if (lua_isnil(l, -1)) {
			lua_pop(l, 1);
			lua_newtable(l);
		}
		pi_lua_protected_call(l, 1, 0);
		lua_pop(l, 2);
	}

	lua_pop(l, 2);

	LUA_DEBUG_END(l, 0);
}

void LuaSerializer::SaveComponents(Json &jsonObj, Space *space)
{
	Json &bodies = jsonObj["space"]["bodies"];
	if (!bodies.is_array())
		return;

	// Note: this loop relies on the ordering and contents of Space::m_bodies not changing
	// between when bodies were serialized and when this function is called.
	// Space should not do update, because it invalidates m_bodyIndex vector.

	for (size_t idx = 0; idx < bodies.size(); idx++) {
		// First index of m_bodyIndex is reserved to
		// nullptr or bad index
		Body *body = space->GetBodyByIndex(idx + 1);

		// Serialize lua components
		Json luaComponentsObj = Json::object();
		if (!LuaObjectBase::SerializeComponents(body, luaComponentsObj))
			continue;

		if (!luaComponentsObj.empty()) {
			bodies[idx]["lua_components"] = luaComponentsObj;
		}
	}
}

void LuaSerializer::LoadComponents(const Json &jsonObj, Space *space)
{
	const Json &bodies = jsonObj["space"]["bodies"];
	if (!bodies.is_array())
		return;

	// Note: this loop relies on the ordering and contents of Space::m_bodies not changing
	// between when bodies were deserialized and when this function is called.
	// Space should not do update, because it invalidates m_bodyIndex vector.

	for (size_t idx = 0; idx < bodies.size(); idx++) {
		const Json &bodyObj = bodies[idx];

		if (bodyObj.count("lua_components") != 0) {
			const Json &luaComponents = bodyObj["lua_components"];
			if (luaComponents.is_object() && !luaComponents.empty()) {
				// First index of m_bodyIndex is reserved to
				// nullptr or bad index
				Body *body = space->GetBodyByIndex(idx + 1);

				// Ensure we've registered the body object in Lua
				LuaObject<Body>::PushToLua(body);
				lua_pop(Lua::manager->GetLuaState(), 1);

				LuaObjectBase::DeserializeComponents(body, luaComponents);
			}
		}
	}
}

/*
 * Interface: Serializer
 */

/*
 * Function: Register
 *
 * Registers a function pair to serialize per-module data across savegames.
 *
 * The serializer function can only serialize a single table object, but may
 * store any serializable objects inside that table.
 *
 * Example:
 *
 * > Serializer:Register("MyModule", function() return {} end, function(data) ... end)
 *
 * Parameters:
 *
 *   key          - unique string key of the module serializer to register
 *   serializer   - function that should return the object to be serialized
 *   unserializer - function that receives the unserialized object and should
 *                  restore needed module state
 *
 */
int LuaSerializer::l_register(lua_State *l)
{
	LUA_DEBUG_START(l);

	std::string key = luaL_checkstring(l, 2);

	luaL_checktype(l, 3, LUA_TFUNCTION); // any type of function
	luaL_checktype(l, 4, LUA_TFUNCTION); // any type of function

	luaL_getsubtable(l, LUA_REGISTRYINDEX, NS_CALLBACKS);

	lua_newtable(l);

	lua_pushinteger(l, 1);
	lua_pushvalue(l, 3);
	lua_rawset(l, -3);
	lua_pushinteger(l, 2);
	lua_pushvalue(l, 4);
	lua_rawset(l, -3);

	lua_pushstring(l, key.c_str());
	lua_pushvalue(l, -2);
	lua_rawset(l, -4);

	lua_pop(l, 2);

	LUA_DEBUG_END(l, 0);

	return 0;
}

/*
 * Function: RegisterClass
 *
 * Registers a class object to be serialized across savegames. Objects of the
 * passed class will be serialized via the Serialize / Unserialize class
 * methods if referred to by other serialized data.
 *
 * Example:
 *
 * > Serializer:RegisterClass("MyClass", MyClass)
 *
 * Parameters:
 *
 *   key   - unique string key of the class to register
 *   class - a class object containing a Serialize() and Unserialize() method.
 *           These methods will be called to save / restore state for instances
 *           of this class.
 */
int LuaSerializer::l_register_class(lua_State *l)
{
	LUA_DEBUG_START(l);

	std::string key = luaL_checkstring(l, 2);
	luaL_checktype(l, 3, LUA_TTABLE);

	lua_getfield(l, 3, "Serialize");
	if (lua_isnil(l, -1))
		return luaL_error(l, "Serializer class '%s' has no 'Serialize' method", key.c_str());
	lua_getfield(l, 3, "Unserialize");
	if (lua_isnil(l, -1))
		return luaL_error(l, "Serializer class '%s' has no 'Unserialize' method", key.c_str());
	lua_pop(l, 2);

	luaL_getsubtable(l, LUA_REGISTRYINDEX, NS_CLASSES);

	lua_pushvalue(l, 3);
	lua_setfield(l, -2, key.c_str());

	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);

	return 0;
}

/*
 * Function: RegisterPersistent
 *
 * Register the given table as a "persistent value" which should not
 * be serialized but instead have references to it in a saved game replaced
 * with the value stored under the same ID at load-time.
 *
 * The passed value must be able to be serialized into a saved game, as the
 * value will be loaded from the savefile to maintain backwards compatibility
 * if no persistent object is registered with that ID at load time.
 *
 * Parameters:
 *   key - string, unique ID of the table to serialize
 *   value - table, persistent value to store
 */
int LuaSerializer::l_register_persistent(lua_State *l)
{
	luaL_checktype(l, 2, LUA_TSTRING);
	luaL_checktype(l, 3, LUA_TTABLE);

	lua_getfield(l, LUA_REGISTRYINDEX, NS_PERSISTENT);
	lua_replace(l, 1);
	lua_settable(l, -3);

	return 0;
}

template <>
const char *LuaObject<LuaSerializer>::s_type = "Serializer";

template <>
void LuaObject<LuaSerializer>::RegisterClass()
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_Reg l_methods[] = {
		{ "Register", LuaSerializer::l_register },
		{ "RegisterClass", LuaSerializer::l_register_class },
		{ "RegisterPersistent", LuaSerializer::l_register_persistent },
		{ 0, 0 }
	};

	lua_newtable(l);
	lua_setfield(l, LUA_REGISTRYINDEX, NS_PERSISTENT);

	lua_newtable(l);
	lua_setfield(l, LUA_REGISTRYINDEX, NS_CLASSES);

	lua_getfield(l, LUA_REGISTRYINDEX, "CoreImports");
	LuaObjectBase::CreateObject(l_methods, 0, 0);
	lua_setfield(l, -2, "Serializer");
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}
