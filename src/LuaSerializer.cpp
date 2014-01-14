// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaSerializer.h"
#include "LuaObject.h"
#include "galaxy/StarSystem.h"
#include "Body.h"
#include "Ship.h"
#include "SpaceStation.h"
#include "Planet.h"
#include "Star.h"
#include "Player.h"
#include "Pi.h"
#include "Game.h"

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
// each individual module's data. that way if a player loads a game with dat
// for a module that is not currently loaded, we don't lose its data in the
// next save


// pickler can handle simple types (boolean, number, string) and will drill
// down into tables. it can do userdata for a specific set of types - Body and
// its kids and SystemPath. anything else will cause a lua error
//
// pickle format is newline-seperated. each line begins with a type value,
// followed by data for that type as follows
//   fNNN.nnn - number (float)
//   bN       - boolean. N is 0 or 1 for true/false
//   sNNN     - string. number is length, followed by newline, then string of bytes
//   t        - table. followed by a float (fNNN.nnn) uniquely identifying the
//            - table, then more pickled stuff (ie recursive)
//   n        - end of table
//   r        - reference to previously-seen table. followed by the table id
//   uXXXX    - userdata. XXXX is type, followed by newline, followed by data
//     Body       - data is a single stringified number for Serializer::LookupBody
//     SystemPath - data is four stringified numbers, newline separated
//   oXXXX    - object. XXX is type, followed by newline, followed by one
//              pickled item (typically t[able])


// on serialize, if an item has a metatable with a "class" attribute, the
// "Serialize" function under that namespace will be called with the type. the
// data returned will then be serialized as an "object" above.
//
// on deserialize, the data after an "object" item will be passed to the
// "Deserialize" function under that namespace. that data returned will be
// given back to the module

void LuaSerializer::pickle(lua_State *l, int idx, std::string &out, const char *key = 0)
{
	static char buf[256];

	LUA_DEBUG_START(l);

	idx = lua_absindex(l, idx);

	if (lua_getmetatable(l, idx)) {
		lua_getfield(l, -1, "class");
		if (lua_isnil(l, -1))
			lua_pop(l, 2);

		else {
			const char *cl = lua_tostring(l, -1);
			snprintf(buf, sizeof(buf), "o%s\n", cl);

			lua_getfield(l, LUA_REGISTRYINDEX, "PiSerializerClasses");

			lua_getfield(l, -1, cl);
			if (lua_isnil(l, -1))
				luaL_error(l, "No Serialize method found for class '%s'\n", cl);

			lua_getfield(l, -1, "Serialize");
			if (lua_isnil(l, -1))
				luaL_error(l, "No Serialize method found for class '%s'\n", cl);

			lua_pushvalue(l, idx);
			pi_lua_protected_call(l, 1, 1);

			lua_remove(l, idx);
			lua_insert(l, idx);

			lua_pop(l, 4);

			if (lua_isnil(l, idx)) {
				LUA_DEBUG_END(l, 0);
				return;
			}

			out += buf;
		}
	}

	switch (lua_type(l, idx)) {
		case LUA_TNIL:
			break;

		case LUA_TNUMBER: {
			snprintf(buf, sizeof(buf), "f%f\n", lua_tonumber(l, idx));
			out += buf;
			break;
		}

		case LUA_TBOOLEAN: {
			snprintf(buf, sizeof(buf), "b%d", lua_toboolean(l, idx) ? 1 : 0);
			out += buf;
			break;
		}

		case LUA_TSTRING: {
			lua_pushvalue(l, idx);
			size_t len;
			const char *str = lua_tolstring(l, -1, &len);
			snprintf(buf, sizeof(buf), "s" SIZET_FMT "\n", len);
			out += buf;
			out.append(str, len);
			lua_pop(l, 1);
			break;
		}

		case LUA_TTABLE: {
			lua_pushinteger(l, lua_Integer(lua_topointer(l, idx)));         // ptr

			lua_getfield(l, LUA_REGISTRYINDEX, "PiSerializerTableRefs");    // ptr reftable
			lua_pushvalue(l, -2);                                           // ptr reftable ptr
			lua_rawget(l, -2);                                              // ptr reftable ???

			if (!lua_isnil(l, -1)) {
				out += "r";
				pickle(l, -3, out, key);
				lua_pop(l, 3);                                              // [empty]
			}

			else {
				out += "t";

				lua_pushvalue(l, -3);                                       // ptr reftable nil ptr
				lua_pushvalue(l, idx);                                      // ptr reftable nil ptr table
				lua_rawset(l, -4);                                          // ptr reftable nil
				pickle(l, -3, out, key);
				lua_pop(l, 3);                                              // [empty]

				lua_pushvalue(l, idx);
				lua_pushnil(l);
				while (lua_next(l, -2)) {
					if (key) {
						pickle(l, -2, out, key);
						pickle(l, -1, out, key);
					}
					else {
						lua_pushvalue(l, -2);
						const char *k = lua_tostring(l, -1);
						pickle(l, -3, out, k);
						pickle(l, -2, out, k);
						lua_pop(l, 1);
					}
					lua_pop(l, 1);
				}
				lua_pop(l, 1);
				out += "n";
			}

			break;
		}

		case LUA_TUSERDATA: {
			out += "u";

			LuaObjectBase *lo = static_cast<LuaObjectBase*>(lua_touserdata(l, idx));
			void *o = lo->GetObject();
			if (!o)
				Error("Lua serializer '%s' tried to serialize an invalid '%s' object", key, lo->GetType());

			// XXX object wrappers should really have Serialize/Unserialize
			// methods to deal with this
			if (lo->Isa("SystemPath")) {
				SystemPath *sbp = static_cast<SystemPath*>(o);
				snprintf(buf, sizeof(buf), "SystemPath\n%d\n%d\n%d\n%u\n%u\n",
					sbp->sectorX, sbp->sectorY, sbp->sectorZ, sbp->systemIndex, sbp->bodyIndex);
				out += buf;
				break;
			}

			if (lo->Isa("Body")) {
				Body *b = static_cast<Body*>(o);
				snprintf(buf, sizeof(buf), "Body\n%u\n", Pi::game->GetSpace()->GetIndexForBody(b));
				out += buf;
				break;
			}

			if (lo->Isa("SceneGraph.ModelSkin")) {
				SceneGraph::ModelSkin *skin = static_cast<SceneGraph::ModelSkin*>(o);
				Serializer::Writer wr;
				skin->Save(wr);
				const std::string &ser = wr.GetData();
				snprintf(buf, sizeof(buf), "ModelSkin\n%lu\n", ser.size());
				out += buf;
				out += ser;
				break;
			}

			Error("Lua serializer '%s' tried to serialize unsupported '%s' userdata value", key, lo->GetType());
			break;
		}

		default:
			Error("Lua serializer '%s' tried to serialize %s value", key, lua_typename(l, lua_type(l, idx)));
			break;
	}

	LUA_DEBUG_END(l, 0);
}

const char *LuaSerializer::unpickle(lua_State *l, const char *pos)
{
	LUA_DEBUG_START(l);

	char type = *pos++;

	switch (type) {

		case 'f': {
			char *end;
			double f = strtod(pos, &end);
			if (pos == end) throw SavedGameCorruptException();
			lua_pushnumber(l, f);
			pos = end+1; // skip newline
			break;
		}

		case 'b': {
			if (*pos != '0' && *pos != '1') throw SavedGameCorruptException();
			bool b = (*pos == '0') ? false : true;
			lua_pushboolean(l, b);
			pos++;
			break;
		}

		case 's': {
			char *end;
			int len = strtol(pos, const_cast<char**>(&end), 0);
			if (pos == end) throw SavedGameCorruptException();
			end++; // skip newline
			lua_pushlstring(l, end, len);
			pos = end + len;
			break;
		}

		case 't': {
			lua_newtable(l);

			lua_getfield(l, LUA_REGISTRYINDEX, "PiSerializerTableRefs");
			pos = unpickle(l, pos);
			lua_pushvalue(l, -3);
			lua_rawset(l, -3);
			lua_pop(l, 1);

			while (*pos != 'n') {
				pos = unpickle(l, pos);
				pos = unpickle(l, pos);
				lua_rawset(l, -3);
			}
			pos++;

			break;
		}

		case 'r': {
			pos = unpickle(l, pos);

			lua_getfield(l, LUA_REGISTRYINDEX, "PiSerializerTableRefs");
			lua_pushvalue(l, -2);
			lua_rawget(l, -2);

			if (lua_isnil(l, -1))
				throw SavedGameCorruptException();

			lua_insert(l, -3);
			lua_pop(l, 2);

			break;
		}

		case 'u': {
			const char *end = strchr(pos, '\n');
			if (!end) throw SavedGameCorruptException();
			int len = end - pos;
			end++; // skip newline

			if (len == 10 && strncmp(pos, "SystemPath", 10) == 0) {
				pos = end;

				Sint32 sectorX = strtol(pos, const_cast<char**>(&end), 0);
				if (pos == end) throw SavedGameCorruptException();
				pos = end+1; // skip newline

				Sint32 sectorY = strtol(pos, const_cast<char**>(&end), 0);
				if (pos == end) throw SavedGameCorruptException();
				pos = end+1; // skip newline

				Sint32 sectorZ = strtol(pos, const_cast<char**>(&end), 0);
				if (pos == end) throw SavedGameCorruptException();
				pos = end+1; // skip newline

				Uint32 systemNum = strtoul(pos, const_cast<char**>(&end), 0);
				if (pos == end) throw SavedGameCorruptException();
				pos = end+1; // skip newline

				Uint32 sbodyId = strtoul(pos, const_cast<char**>(&end), 0);
				if (pos == end) throw SavedGameCorruptException();
				pos = end+1; // skip newline

				const SystemPath sbp(sectorX, sectorY, sectorZ, systemNum, sbodyId);
				LuaObject<SystemPath>::PushToLua(sbp);

				break;
			}

			if (len == 4 && strncmp(pos, "Body", 4) == 0) {
				pos = end;

				Uint32 n = strtoul(pos, const_cast<char**>(&end), 0);
				if (pos == end) throw SavedGameCorruptException();
				pos = end+1; // skip newline

				Body *body = Pi::game->GetSpace()->GetBodyByIndex(n);
				if (pos == end) throw SavedGameCorruptException();

				switch (body->GetType()) {
					case Object::BODY:
						LuaObject<Body>::PushToLua(body);
						break;
					case Object::SHIP:
						LuaObject<Ship>::PushToLua(dynamic_cast<Ship*>(body));
						break;
					case Object::SPACESTATION:
						LuaObject<SpaceStation>::PushToLua(dynamic_cast<SpaceStation*>(body));
						break;
					case Object::PLANET:
						LuaObject<Planet>::PushToLua(dynamic_cast<Planet*>(body));
						break;
					case Object::STAR:
						LuaObject<Star>::PushToLua(dynamic_cast<Star*>(body));
						break;
					case Object::PLAYER:
						LuaObject<Player>::PushToLua(dynamic_cast<Player*>(body));
						break;
					default:
						throw SavedGameCorruptException();
				}

				break;
			}

			if (len == 9 && strncmp(pos, "ModelSkin", 9) == 0) {
				pos = end;

				Uint32 serlen = strtoul(pos, const_cast<char**>(&end), 0);
				if (pos == end) throw SavedGameCorruptException();
				pos = end+1; // skip newline

				std::string buf(pos, serlen);
				const char *bufp = buf.c_str();
				Serializer::Reader rd(ByteRange(bufp, bufp + buf.size()));
				SceneGraph::ModelSkin skin;
				skin.Load(rd);

				LuaObject<SceneGraph::ModelSkin>::PushToLua(skin);

				pos += serlen;

				break;
			}

			throw SavedGameCorruptException();
		}

		case 'o': {
			const char *end = strchr(pos, '\n');
			if (!end) throw SavedGameCorruptException();
			int len = end - pos;
			end++; // skip newline

			const char *cl = pos;

			// unpickle the object, and insert it beneath the method table value
			pos = unpickle(l, end);

			// get PiSerializerClasses[typename]
			lua_getfield(l, LUA_REGISTRYINDEX, "PiSerializerClasses");
			lua_pushlstring(l, cl, len);
			lua_gettable(l, -2);
			lua_remove(l, -2);

			if (lua_isnil(l, -1)) {
				lua_pop(l, 2);
				break;
			}

			lua_getfield(l, -1, "Unserialize");
			if (lua_isnil(l, -1)) {
				lua_pushlstring(l, cl, len);
				luaL_error(l, "No Unserialize method found for class '%s'\n", lua_tostring(l, -1));
			}

			lua_insert(l, -3);
			lua_pop(l, 1);

			pi_lua_protected_call(l, 1, 1);

			break;
		}

		default:
			throw SavedGameCorruptException();
	}

	LUA_DEBUG_END(l, 1);

	return pos;
}

void LuaSerializer::Serialize(Serializer::Writer &wr)
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	lua_newtable(l);
	int savetable = lua_gettop(l);

	lua_getfield(l, LUA_REGISTRYINDEX, "PiSerializerCallbacks");
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		lua_newtable(l);
		lua_pushvalue(l, -1);
		lua_setfield(l, LUA_REGISTRYINDEX, "PiSerializerCallbacks");
	}

	lua_pushnil(l);
	while (lua_next(l, -2) != 0) {
		lua_pushinteger(l, 1);
		lua_gettable(l, -2);
		pi_lua_protected_call(l, 0, 1);
		lua_pushvalue(l, -3);
		lua_insert(l, -2);
		lua_settable(l, savetable);
		lua_pop(l, 1);
	}

	lua_pop(l, 1);

	lua_newtable(l);
	lua_setfield(l, LUA_REGISTRYINDEX, "PiSerializerTableRefs");

	std::string pickled;
	pickle(l, savetable, pickled);

	wr.String(pickled);

	lua_pop(l, 1);

	lua_pushnil(l);
	lua_setfield(l, LUA_REGISTRYINDEX, "PiSerializerTableRefs");

	LUA_DEBUG_END(l, 0);
}

void LuaSerializer::Unserialize(Serializer::Reader &rd)
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	lua_newtable(l);
	lua_setfield(l, LUA_REGISTRYINDEX, "PiSerializerTableRefs");

	std::string pickled = rd.String();
	const char *start = pickled.c_str();
	const char *end = unpickle(l, start);
	if (size_t(end - start) != pickled.length()) throw SavedGameCorruptException();
	if (!lua_istable(l, -1)) throw SavedGameCorruptException();
	int savetable = lua_gettop(l);

	lua_pushnil(l);
	lua_setfield(l, LUA_REGISTRYINDEX, "PiSerializerTableRefs");

	lua_getfield(l, LUA_REGISTRYINDEX, "PiSerializerCallbacks");
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		lua_newtable(l);
		lua_pushvalue(l, -1);
		lua_setfield(l, LUA_REGISTRYINDEX, "PiSerializerCallbacks");
	}

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

int LuaSerializer::l_register(lua_State *l)
{
	LUA_DEBUG_START(l);

	std::string key = luaL_checkstring(l, 2);

	luaL_checktype(l, 3, LUA_TFUNCTION); // any type of function
	luaL_checktype(l, 4, LUA_TFUNCTION); // any type of function

	lua_getfield(l, LUA_REGISTRYINDEX, "PiSerializerCallbacks");
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		lua_newtable(l);
		lua_pushvalue(l, -1);
		lua_setfield(l, LUA_REGISTRYINDEX, "PiSerializerCallbacks");
	}

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

	lua_getfield(l, LUA_REGISTRYINDEX, "PiSerializerClasses");
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		lua_newtable(l);
		lua_pushvalue(l, -1);
		lua_setfield(l, LUA_REGISTRYINDEX, "PiSerializerClasses");
	}

	lua_pushvalue(l, 3);
	lua_setfield(l, -2, key.c_str());

	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);

	return 0;
}

template <> const char *LuaObject<LuaSerializer>::s_type = "Serializer";

template <> void LuaObject<LuaSerializer>::RegisterClass()
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_Reg l_methods[] = {
		{ "Register",      LuaSerializer::l_register },
		{ "RegisterClass", LuaSerializer::l_register_class },
		{ 0, 0 }
	};

	lua_getfield(l, LUA_REGISTRYINDEX, "CoreImports");
	LuaObjectBase::CreateObject(l_methods, 0, 0);
	lua_setfield(l, -2, "Serializer");
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}
