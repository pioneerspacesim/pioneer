// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "core/Property.h"

#include "LuaColor.h"
#include "LuaMetaType.h"
#include "LuaObject.h"
#include "LuaVector.h"
#include "LuaVector2.h"

void pi_lua_generic_pull(lua_State *l, int idx, Property &out)
{
	out = nullptr; // default case if no branches match
	int type = lua_type(l, idx);
	if (type == LUA_TBOOLEAN)
		out = lua_toboolean(l, idx);
	if (type == LUA_TNUMBER)
		out = lua_tonumber(l, idx);
	if (type == LUA_TSTRING)
		out = LuaPull<std::string_view>(l, idx);
	if (type == LUA_TUSERDATA) {
		const vector2d *vec2 = LuaVector2::GetFromLua(l, idx);
		if (vec2)
			out = *vec2;

		const vector3d *vec3 = LuaVector::GetFromLua(l, idx);
		if (vec3)
			out = *vec3;

		const Color4ub *color = LuaColor::GetFromLua(l, idx);
		if (color)
			out = *color;

		// quaternions not currently exposed
	}
	if (type == LUA_TTABLE) {
		PropertyMap *map = new PropertyMap();
		lua_pushnil(l);
		while (lua_next(l, idx)) {
			if (lua_type(l, -2) != LUA_TSTRING) {
				lua_pop(l, 1);
				continue; // XXX should we log a warning here?
			}

			Property prop = LuaPull<Property>(l, -1);
			std::string_view key = LuaPull<std::string_view>(l, -2);

			map->Set(key, std::move(prop));
			lua_pop(l, 1);
		}
	}
}

void pi_lua_generic_push(lua_State *l, const Property &property)
{
	if (property.is_null())
		return lua_pushnil(l);
	if (property.is_bool())
		return lua_pushboolean(l, property.get_bool());
	if (property.is_number())
		return lua_pushnumber(l, property.get_number());
	if (property.is_vector2())
		return LuaPush(l, property.get_vector2());
	if (property.is_vector3())
		return LuaPush(l, property.get_vector3());
	if (property.is_color())
		return LuaPush(l, property.get_color());
	if (property.is_quat())
		return lua_pushnil(l); // for now, quaternions are not exposed to lua
	if (property.is_string())
		return LuaPush(l, property.get_string().sv());
	if (property.is_map())
		return LuaPush(l, property.get_map());
}

static int l_iter_next(lua_State *l)
{
	auto &iter = *static_cast<PropertyMap::iterator *>(lua_touserdata(l, 1));
	if (iter) {
		LuaPush(l, iter->first.sv());
		LuaPush(l, iter->second);
		++iter;
		return 2;
	}

	return 0;
}

template <>
const char *LuaObject<PropertyMap>::s_type = "PropertyMap";

template <>
void LuaObject<PropertyMap>::RegisterClass()
{
	LuaMetaType<PropertyMap> metaType(s_type);
	metaType.CreateMetaType(Lua::manager->GetLuaState());

	metaType.StartRecording();
	metaType.AddMeta("__len", &PropertyMap::Size);
	metaType.AddMeta("__index", [](lua_State *l, PropertyMap *m) -> int {
		auto key = LuaPull<std::string_view>(l, 2);
		LuaPush(l, m->Get(key));
		return 1;
	});
	metaType.AddMeta("__newindex", [](lua_State *l, PropertyMap *m) -> int {
		auto key = LuaPull<std::string_view>(l, 2);
		auto prop = LuaPull<Property>(l, 3);
		m->Set(key, prop);
		return 0;
	});
	metaType.AddMeta("__pairs", [](lua_State *l, PropertyMap *m) -> int {
		lua_pushcfunction(l, l_iter_next);
		LuaObjectBase::AllocateNew<PropertyMap::iterator>(m->begin());

		return 2;
	});
	metaType.StopRecording();
}
