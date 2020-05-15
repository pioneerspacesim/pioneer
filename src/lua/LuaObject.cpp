// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaObject.h"
#include "Json.h"
#include "LuaUtils.h"
#include "PropertiedObject.h"
#include "PropertyMap.h"
#include "libs.h"

#include <map>
#include <utility>

/*
 * Namespace: Object
 *
 * Provides core methods to all engine objects.
 *
 * <Object> is not a true class but does provide a few methods that are
 * applied to every Pioneer engine object that gets exposed to the Lua
 * environment.
 *
 *
 * Method: exists
 *
 * Determines if the engine object underpinning the Lua object still exists in
 * the engine.
 *
 * > exists = object:exists()
 *
 * It is possible for a Pioneer engine object to be deleted while a Lua script
 * is holding a reference to it. In this case, the Lua object is simply a
 * shell, with no internals, and any attempt to use it will result in a Lua
 * error. Calling <exists> allows a script to determine if the object is still
 * valid before it uses it.
 *
 * Modules that carry their own state or do things out of the normal event
 * flow need to consider this. The documentation for <Timer> contains a more
 * concrete example of this.
 *
 * Returns:
 *
 *   exists - true if the object is valid, false otherwise
 *
 * Example:
 *
 * > if not ship:exists() then return
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 *
 *
 * Method: isa
 *
 * Determines if a object is of or inherits from a given class.
 *
 * > isa = object:isa(classname)
 *
 * The object model used in Pioneer's Lua environment is a classical
 * single-inheritance system. <isa> operates much like similarly-named methods
 * in other languages; it tells you if the object has the named class
 * somewhere in its inheritence hierarchy.
 *
 * Parameters:
 *
 *   classname - the name of the class to check against the object
 *
 * Returns:
 *
 *   isa - true if the object inherits from the named class, false otherwise
 *
 * Example:
 *
 * > if body:isa("Ship") then
 * >     body:AIKill(Game.player)
 * > end
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */

// since LuaManager is a singleton, these must be heap-allocated. If they're
// stack-allocated, they will be torn down at program shutdown before the
// singleton is. This will cause LuaObject to crash during garbage collection
// FIXME(sturnclaw): LuaManager isn't a singleton any more (since when?) so this is unnecessary.
static bool instantiated = false;

static std::map<std::string, std::map<std::string, PromotionTest>> *promotions;
static std::map<std::string, SerializerPair> *serializers;

static void _teardown()
{
	delete promotions;
	delete serializers;
}

static inline void _instantiate()
{
	if (!instantiated) {
		promotions = new std::map<std::string, std::map<std::string, PromotionTest>>;
		serializers = new std::map<std::string, SerializerPair>;

		// XXX atexit is not a very nice way to deal with this in C++
		atexit(_teardown);

		instantiated = true;
	}
}

class LuaObjectHelpers {
public:
	// lua method to determine if the underlying object is still present in
	// the registry (ie still exists)
	static int l_exists(lua_State *l);

	// lua method to determine if the object inherits from a type. wrapper
	// around ::Isa()
	static int l_isa(lua_State *l);

	// lua method to set a property on a propertied object
	static int l_setprop(lua_State *l);

	// lua method to unset a property on a propertied object
	static int l_unsetprop(lua_State *l);

	// lua method to check the existence of a specific property on an object
	static int l_hasprop(lua_State *l);

	// the lua object "destructor" that gets called by the garbage collector.
	static int l_gc(lua_State *l);

	// default tostring. shows a little more info about the object, like its
	// type
	static int l_tostring(lua_State *l);
};

int LuaObjectHelpers::l_exists(lua_State *l)
{
	luaL_checktype(l, 1, LUA_TUSERDATA);
	LuaObjectBase *lo = static_cast<LuaObjectBase *>(lua_touserdata(l, 1));
	lua_pushboolean(l, lo->GetObject() != 0);
	return 1;
}

int LuaObjectHelpers::l_hasprop(lua_State *l)
{
	luaL_checktype(l, 1, LUA_TUSERDATA);
	luaL_checktype(l, 2, LUA_TSTRING);
	lua_getuservalue(l, 1);

	if (lua_isnil(l, -1)) { // Doesn't have properties
		lua_pushboolean(l, false);
	} else {
		lua_pushvalue(l, 2);
		lua_gettable(l, -2);
		// We consider that a value isn't set if it is nil
		lua_pushboolean(l, !lua_isnil(l, -1));
	}
	return 1;
}

int LuaObjectHelpers::l_unsetprop(lua_State *l)
{
	luaL_checktype(l, 1, LUA_TUSERDATA);
	const std::string key(luaL_checkstring(l, 2));

	// quick check to make sure this object actually has properties
	// before we go diving through the stack etc
	lua_getuservalue(l, 1);
	if (lua_isnil(l, -1))
		return luaL_error(l, "Object has no property map");

	LuaObjectBase *lo = static_cast<LuaObjectBase *>(lua_touserdata(l, 1));
	LuaWrappable *o = lo->GetObject();
	if (!o)
		return luaL_error(l, "Object is no longer valid");

	PropertiedObject *po = dynamic_cast<PropertiedObject *>(o);
	assert(po);

	po->Properties().PushLuaTable();
	lua_pushvalue(l, 2);
	lua_pushnil(l);
	lua_settable(l, -3);

	return 0;
}

int LuaObjectHelpers::l_setprop(lua_State *l)
{
	luaL_checktype(l, 1, LUA_TUSERDATA);
	const std::string key(luaL_checkstring(l, 2));

	int isnum;
	double vn = lua_tonumberx(l, 3, &isnum);
	std::string vs;
	if (!isnum)
		vs = luaL_checkstring(l, 3);

	// quick check to make sure this object actually has properties
	// before we go diving through the stack etc
	lua_getuservalue(l, 1);
	if (lua_isnil(l, -1))
		return luaL_error(l, "Object has no property map");

	LuaObjectBase *lo = static_cast<LuaObjectBase *>(lua_touserdata(l, 1));
	LuaWrappable *o = lo->GetObject();
	if (!o)
		return luaL_error(l, "Object is no longer valid");

	PropertiedObject *po = dynamic_cast<PropertiedObject *>(o);
	assert(po);

	if (isnum)
		po->Properties().Set(key, vn);
	else
		po->Properties().Set(key, vs);

	return 0;
}

int LuaObjectHelpers::l_isa(lua_State *l)
{
	luaL_checktype(l, 1, LUA_TUSERDATA);
	LuaObjectBase *lo = static_cast<LuaObjectBase *>(lua_touserdata(l, 1));
	if (!lo->GetObject())
		return luaL_error(l, "Object is no longer valid");

	lua_pushboolean(l, lo->Isa(luaL_checkstring(l, 2)));
	return 1;
}

int LuaObjectHelpers::l_gc(lua_State *l)
{
	luaL_checktype(l, 1, LUA_TUSERDATA);
	LuaObjectBase *lo = static_cast<LuaObjectBase *>(lua_touserdata(l, 1));

	LuaObjectBase::Deregister(lo);

	lo->~LuaObjectBase();

	return 0;
}

int LuaObjectHelpers::l_tostring(lua_State *l)
{
	luaL_checktype(l, 1, LUA_TUSERDATA);
	lua_getmetatable(l, 1);
	lua_pushstring(l, "type");
	lua_rawget(l, -2);
	lua_pushfstring(l, "userdata [%s]: %p", lua_tostring(l, -1), lua_topointer(l, 1));
	return 1;
}

PropertyMap *LuaObjectBase::GetPropertiesFromObject(lua_State *l, int object)
{
	if (!lua_isuserdata(l, object))
		return nullptr;

	LuaObjectBase *lo = static_cast<LuaObjectBase *>(lua_touserdata(l, object));
	LuaWrappable *o = lo->GetObject();
	if (!o)
		return nullptr;

	PropertiedObject *po = dynamic_cast<PropertiedObject *>(o);
	if (!po)
		return nullptr;

	return &po->Properties();
}

static void register_functions(lua_State *l, const luaL_Reg *methods, bool protect)
{
	for (const luaL_Reg *m = methods; m->name; m++) {
		lua_pushstring(l, m->name);
		lua_pushcfunction(l, m->func);
		if (protect)
			lua_pushcclosure(l, secure_trampoline, 1);
		lua_rawset(l, -3);
	}
}

void LuaObjectBase::CreateObject(LuaMetaTypeBase *metaType)
{
	assert(metaType);
	assert(metaType->GetMetatable().IsValid());

	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	// create "object"
	lua_newtable(l);

	// apply the metatable
	metaType->GetMetatable().PushCopyToStack();
	lua_setmetatable(l, -2);

	// leave the finished object on the stack

	LUA_DEBUG_END(l, 1);
}

void LuaObjectBase::CreateObject(const luaL_Reg *methods, const luaL_Reg *attrs, const luaL_Reg *meta, bool protect)
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	// create a throwaway, non-inheriting metatype
	LuaMetaTypeBase metaType("");
	metaType.CreateMetaType(l);

	// add methods
	if (methods) {
		lua_getfield(l, -1, "methods");
		register_functions(l, methods, protect);
		lua_pop(l, 1);
	}

	// add attributes
	if (attrs) {
		lua_getfield(l, -1, "attrs");
		register_functions(l, attrs, protect);
		lua_pop(l, 1);
	}

	// add metafunctions (if applicable)
	if (meta) {
		register_functions(l, meta, protect);
	}

	lua_pop(l, 1);

	// leave the finished object on the stack
	CreateObject(&metaType);

	LUA_DEBUG_END(l, 1);
}

static void initialize_object_registry(lua_State *l)
{
	// create the object registry if it doesn't already exist. this is the
	// best place we have to do this since classes will always be registered
	// before any objects actually turn up
	lua_getfield(l, LUA_REGISTRYINDEX, "LuaObjectRegistry");
	if (lua_isnil(l, -1)) {
		// create the LuaObjectRegistry table
		lua_newtable(l);

		// configure the registry to use weak values
		lua_newtable(l);
		lua_pushstring(l, "__mode");
		lua_pushstring(l, "v");
		lua_rawset(l, -3);
		lua_setmetatable(l, -2);

		lua_setfield(l, LUA_REGISTRYINDEX, "LuaObjectRegistry");
	}
	lua_pop(l, 1);
}

void LuaObjectBase::CreateClass(const char *type, const char *parent, const luaL_Reg *methods, const luaL_Reg *attrs, const luaL_Reg *meta)
{
	assert(type);

	lua_State *l = Lua::manager->GetLuaState();

	_instantiate();

	LUA_DEBUG_START(l);

	initialize_object_registry(l);

	// create and register a new metatype with the type's name.
	// this will warn in the log if there's already a metatype with this name for ease of debugging
	LuaMetaTypeBase metaType(type);
	if (parent)
		metaType.SetParent(parent);
	metaType.CreateMetaType(l);

	// add attributes
	if (attrs) {
		lua_getfield(l, -1, "attrs");
		luaL_setfuncs(l, attrs, 0);
		lua_pop(l, 1);
	}

	// attach methods to the methods table
	lua_getfield(l, -1, "methods");
	if (methods) luaL_setfuncs(l, methods, 0);
	lua_pop(l, 1); // pop the methods table

	if (meta) luaL_setfuncs(l, meta, 0);

	lua_pop(l, 1); // pop the metatable

	// delegate the rest of the setup to CreateClass.
	CreateClass(&metaType);

	LUA_DEBUG_END(l, 0);
}

void LuaObjectBase::CreateClass(LuaMetaTypeBase *metaType)
{
	assert(metaType);
	assert(metaType->GetMetatable().IsValid());

	lua_State *l = Lua::manager->GetLuaState();
	assert(l == metaType->GetMetatable().GetLua());

	_instantiate();

	LUA_DEBUG_START(l);

	initialize_object_registry(l);

	// drill down to the proper "global" table to add the method table to
	pi_lua_split_table_path(l, metaType->GetTypeName());

	// Get the method table from the metatype
	metaType->GetMetatable().PushCopyToStack();
	lua_getfield(l, -1, "methods");
	lua_remove(l, -2); // "global" table, type name, methods table

	// add the exists method
	lua_pushcfunction(l, LuaObjectHelpers::l_exists);
	lua_setfield(l, -2, "exists");

	// add the isa method
	lua_pushcfunction(l, LuaObjectHelpers::l_isa);
	lua_setfield(l, -2, "isa");

	// add the setprop, unsetprop and hasprop methods
	lua_pushcfunction(l, LuaObjectHelpers::l_setprop);
	lua_setfield(l, -2, "setprop");

	lua_pushcfunction(l, LuaObjectHelpers::l_unsetprop);
	lua_setfield(l, -2, "unsetprop");

	lua_pushcfunction(l, LuaObjectHelpers::l_hasprop);
	lua_setfield(l, -2, "hasprop");

	// publish the method table
	lua_rawset(l, -3);

	// remove the "global" table
	lua_pop(l, 1);

	// Get the metatype's table (again)
	metaType->GetMetatable().PushCopyToStack();

	// default tostring method.
	// if the metatype already has a tostring metamethod, don't override it
	lua_getfield(l, -1, "__tostring");
	if (lua_isnil(l, -1)) {
		lua_pushcfunction(l, LuaObjectHelpers::l_tostring);
		lua_setfield(l, -3, "__tostring");
	}
	lua_pop(l, 1);

	// the __index function and other metamethods are already set up by the metatype

	// add a generic garbage collector
	lua_pushcfunction(l, LuaObjectHelpers::l_gc);
	lua_setfield(l, -2, "__gc");

	// pop the metatable, leaving the stack clean
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}

bool LuaObjectBase::PushRegistered(LuaWrappable *o)
{
	assert(instantiated);

	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	if (!o) {
		lua_pushnil(l);
		return true;
	}

	lua_getfield(l, LUA_REGISTRYINDEX, "LuaObjectRegistry");
	assert(lua_istable(l, -1));

	lua_pushlightuserdata(l, o);
	lua_gettable(l, -2);

	if (lua_isuserdata(l, -1)) {
		lua_insert(l, -2);
		lua_pop(l, 1);

		LUA_DEBUG_END(l, 1);

		return true;
	}
	assert(lua_isnil(l, -1));

	lua_pop(l, 2);

	LUA_DEBUG_END(l, 0);

	return false;
}

void LuaObjectBase::Register(LuaObjectBase *lo)
{
	assert(instantiated);
	assert(lo->GetObject());

	bool have_promotions = true;
	bool tried_promote = false;

	while (have_promotions && !tried_promote) {
		std::map<std::string, std::map<std::string, PromotionTest>>::const_iterator base_iter = promotions->find(lo->m_type);
		if (base_iter != promotions->end()) {
			tried_promote = true;

			for (
				std::map<std::string, PromotionTest>::const_iterator target_iter = (*base_iter).second.begin();
				target_iter != (*base_iter).second.end();
				++target_iter) {
				if ((*target_iter).second(lo->GetObject())) {
					lo->m_type = (*target_iter).first.c_str();
					tried_promote = false;
				}
			}

			assert(lo->Isa((*base_iter).first.c_str()));
		} else
			have_promotions = false;
	}

	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l); // lo userdata

	lua_getfield(l, LUA_REGISTRYINDEX, "LuaObjectRegistry"); // lo userdata, registry table
	assert(lua_istable(l, -1));

	lua_pushlightuserdata(l, lo->GetObject()); // lo userdata, registry table, o lightuserdata
	lua_pushvalue(l, -3);					   // lo userdata, registry table, o lightuserdata, lo userdata
	lua_settable(l, -3);					   // lo userdata, registry table

	lua_pop(l, 1); // lo userdata

	LuaMetaTypeBase::GetMetatableFromName(l, lo->m_type); // lo userdata, lo metatable
	lua_pushvalue(l, -1);
	lua_setmetatable(l, -3); // setup the metatable early to make constructor searching work

	lua_getfield(l, -1, "__index");
	lua_pushvalue(l, -3); // Copy the object to begin the search.
	lua_pushstring(l, "Constructor");
	pi_lua_protected_call(l, 2, 1); // call the index lookup method

	// try an alternate constructor method __init()
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);

		lua_getfield(l, -1, "__index");
		lua_pushvalue(l, -3); // Copy the object to begin the search.
		lua_pushstring(l, "__init");
		pi_lua_protected_call(l, 2, 1); // call the index lookup method
	}

	// replace the metatable with the (maybe) constructor
	lua_replace(l, -2); // lo userdata, cons

	// attach properties table if available
	PropertiedObject *po = dynamic_cast<PropertiedObject *>(lo->GetObject());
	if (po) {
		po->Properties().PushLuaTable();
		lua_setuservalue(l, -3);
	}

	// Finally, call the initializer once the object is fully set up
	if (lua_isfunction(l, -1)) {
		lua_pushvalue(l, -2); // lo userdata, cons, lo userdata
		lua_call(l, 1, 0);	  // lo userdata
	} else {
		lua_pop(l, 1); // Pop the nil constructor
	}

	LUA_DEBUG_END(l, 0);
}

void LuaObjectBase::Deregister(LuaObjectBase *lo)
{
	LuaWrappable *o = lo->GetObject();

	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	lua_getfield(l, LUA_REGISTRYINDEX, "LuaObjectRegistry");
	assert(lua_istable(l, -1));

	lua_pushlightuserdata(l, o);
	lua_pushnil(l);
	lua_rawset(l, -3);

	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);

	return;
}

LuaWrappable *LuaObjectBase::CheckFromLua(int index, const char *type)
{
	assert(instantiated);

	lua_State *l = Lua::manager->GetLuaState();

	luaL_checktype(l, index, LUA_TUSERDATA);
	LuaObjectBase *lo = static_cast<LuaObjectBase *>(lua_touserdata(l, index));

	LuaWrappable *o = lo->GetObject();
	if (!o) {
		luaL_error(l, "Object is no longer valid");
		return 0;
	}

	if (!lo->Isa(type))
		luaL_error(l, "Object on stack has type %s which can not be used as type %s\n", lo->m_type, type);

	// found it
	return o;
}

LuaWrappable *LuaObjectBase::GetFromLua(int index, const char *type)
{
	assert(instantiated);

	lua_State *l = Lua::manager->GetLuaState();

	if (lua_type(l, index) != LUA_TUSERDATA)
		return 0;

	LuaObjectBase *lo = static_cast<LuaObjectBase *>(lua_touserdata(l, index));

	LuaWrappable *o = lo->GetObject();
	if (!o)
		return 0;

	if (!lo->Isa(type))
		return 0;

	// found it
	return o;
}

bool LuaObjectBase::Isa(const char *base) const
{
	// fast path
	if (strcmp(m_type, base) == 0)
		return true;

	assert(instantiated);

	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	std::string current = m_type;
	while (current.compare(base) != 0) {
		if (!LuaMetaTypeBase::GetMetatableFromName(l, current.c_str())) {
			LUA_DEBUG_END(l, 0);
			return false;
		}

		lua_getfield(l, -1, "parent");
		// if it doesn't have a parent then we can go no further
		if (lua_isnil(l, -1)) {
			lua_pop(l, 2);
			LUA_DEBUG_END(l, 0);
			return false;
		}

		current = lua_tostring(l, -1);
		lua_pop(l, 2);
	}

	// otherwise, we can assume we've broken out of the loop because
	// we found a match.
	LUA_DEBUG_END(l, 0);

	return true;
}

void LuaObjectBase::RegisterPromotion(const char *base_type, const char *target_type, PromotionTest test_fn)
{
	(*promotions)[base_type][target_type] = test_fn;
}

void LuaObjectBase::RegisterSerializer(const char *type, SerializerPair pair)
{
	(*serializers)[type] = pair;
}

std::string LuaObjectBase::Serialize()
{
	static char buf[256];

	lua_State *l = Lua::manager->GetLuaState();

	auto i = serializers->find(m_type);
	if (i == serializers->end()) {
		luaL_error(l, "No registered serializer for type %s\n", m_type);
		abort();
	}

	snprintf(buf, sizeof(buf), "%s\n", m_type);

	return std::string(buf) + (*i).second.serialize(GetObject());
}

bool LuaObjectBase::Deserialize(const char *stream, const char **next)
{
	static char buf[256];

	const char *end = strchr(stream, '\n');
	int len = end - stream;
	end++; // skip newline

	snprintf(buf, sizeof(buf), "%.*s", len, stream);

	lua_State *l = Lua::manager->GetLuaState();

	auto i = serializers->find(buf);
	if (i == serializers->end()) {
		luaL_error(l, "No registered deserializer for type %s\n", buf);
		abort();
	}

	return (*i).second.deserialize(end, next);
}

void LuaObjectBase::ToJson(Json &out)
{
	lua_State *l = Lua::manager->GetLuaState();

	const auto it = serializers->find(m_type);
	if (it == serializers->end()) {
		luaL_error(l, "No registered serializer for type %s\n", m_type);
		abort();
	}

	out["cpp_class"] = Json(m_type);
	Json inner = Json::object();
	it->second.to_json(inner, GetObject());
	out["inner"] = inner;
}

bool LuaObjectBase::FromJson(const Json &obj)
{
	if (obj["cpp_class"].is_null() || obj["inner"].is_null()) return false;
	const std::string type = obj["cpp_class"];
	auto it = serializers->find(type);
	if (it == serializers->end()) {
		lua_State *l = Lua::manager->GetLuaState();
		luaL_error(l, "No registered deserializer for type %s\n", type.c_str());
		abort();
	}

	return it->second.from_json(obj["inner"]);
}

void *LuaObjectBase::Allocate(size_t n)
{
	lua_State *l = Lua::manager->GetLuaState();
	return lua_newuserdata(l, n);
}
