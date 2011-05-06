#include "LuaSpaceStation.h"
#include "LuaUtils.h"
#include "SpaceStation.h"
#include "LuaChatForm.h"
#include "LuaConstants.h"
#include "Pi.h"

static std::map<SpaceStation*,sigc::connection> _station_delete_conns;

static void _delete_station_ads(SpaceStation *s)
{
	lua_State *l = Pi::luaManager.GetLuaState();

	LUA_DEBUG_START(l);

	lua_getfield(l, LUA_REGISTRYINDEX, "PiAdverts");
	assert(lua_istable(l, -1));

	const std::list<const BBAdvert*> bbadverts = s->GetBBAdverts();
	for (std::list<const BBAdvert*>::const_iterator i = bbadverts.begin(); i != bbadverts.end(); i++) {
		lua_pushinteger(l, (*i)->ref);
		lua_gettable(l, -2);
        if (lua_isnil(l, -1)) {
            lua_pop(l, 1);
            continue;
        }
		assert(lua_istable(l, -1));

		lua_getfield(l, -1, "onDelete");
		if (lua_isnil(l, -1))
			lua_pop(l, 1);
		else {
			lua_pushinteger(l, (*i)->ref);
			lua_call(l, 1, 0);
		}

		lua_pop(l, 1);
	}

	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);

	_station_delete_conns.erase(s);
}

static void _register_for_station_delete(SpaceStation *s)
{
	if (_station_delete_conns.find(s) != _station_delete_conns.end()) return;
	_station_delete_conns.insert( std::make_pair(s, s->onBulletinBoardDeleted.connect(sigc::bind(sigc::ptr_fun(&_delete_station_ads), s))) );
}

static BBAdvertChatForm *_create_chat_form(SpaceStation *station, const BBAdvert *ad)
{
	return new LuaChatForm(station, ad);
}

static int l_spacestation_add_advert(lua_State *l)
{
	LUA_DEBUG_START(l);

	SpaceStation *s = LuaSpaceStation::GetFromLua(1);
	std::string description = luaL_checkstring(l, 2);

	if (!lua_isfunction(l, 3))
		luaL_typerror(l, 3, lua_typename(l, LUA_TFUNCTION));

	int ref = s->AddBBAdvert(description, _create_chat_form);

	lua_getfield(l, LUA_REGISTRYINDEX, "PiAdverts");
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		lua_newtable(l);
		lua_pushvalue(l, -1);
		lua_setfield(l, LUA_REGISTRYINDEX, "PiAdverts");
	}

	lua_pushinteger(l, ref);

	lua_newtable(l);

	lua_pushstring(l, "onChat");
	lua_pushvalue(l, 3);
	lua_settable(l, -3);

	if (!lua_isnil(l, 4) && !lua_isfunction(l, 4))
		luaL_typerror(l, 4, lua_typename(l, LUA_TFUNCTION));
	else {
		lua_pushstring(l, "onDelete");
		lua_pushvalue(l, 4);
		lua_settable(l, -3);
	}

	lua_settable(l, -3);
	lua_pop(l, 1);

	LUA_DEBUG_END(l,0);

	_register_for_station_delete(s);

	lua_pushinteger(l, ref);
	return 1;
} 

static int l_spacestation_remove_advert(lua_State *l)
{
	LUA_DEBUG_START(l);

	SpaceStation *s = LuaSpaceStation::GetFromLua(1);
	int ref = luaL_checkinteger(l, 2);

	if (!s->RemoveBBAdvert(ref))
		return 0;

	lua_getfield(l, LUA_REGISTRYINDEX, "PiAdverts");
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		return 0;
	}
	
	lua_pushinteger(l, ref);
	lua_gettable(l, -2);
	if (lua_isnil(l, -1)) {
		lua_pop(l, 2);
		return 0;
	}

	lua_getfield(l, -1, "onDelete");
	if (lua_isnil(l, -1))
		lua_pop(l, 1);
	else {
		lua_pushinteger(l, ref);
		lua_call(l, 1, 0);
	}

	lua_pop(l, 1);

	lua_pushinteger(l, ref);
	lua_pushnil(l);
	lua_settable(l, -3);

	lua_pop(l, 1);

	LUA_DEBUG_END(l,0);

	return 0;
} 

static int l_spacestation_get_equipment_price(lua_State *l)
{
	SpaceStation *s = LuaSpaceStation::GetFromLua(1);
	Equip::Type e = static_cast<Equip::Type>(LuaConstants::GetConstant(l, "EquipType", luaL_checkstring(l, 2)));
	Sint64 cost = s->GetPrice(e);
	lua_pushnumber(l, cost * 0.01);
	return 1;
}

static bool promotion_test(DeleteEmitter *o)
{
	return dynamic_cast<SpaceStation*>(o);
}

template <> const char *LuaObject<SpaceStation>::s_type = "SpaceStation";

template <> void LuaObject<SpaceStation>::RegisterClass()
{
	const char *l_parent = "Body";

	static const luaL_reg l_methods[] = {
		{ "AddAdvert",    l_spacestation_add_advert    },
		{ "RemoveAdvert", l_spacestation_remove_advert },

		{ "GetEquipmentPrice", l_spacestation_get_equipment_price },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, NULL, NULL);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, promotion_test);
}
