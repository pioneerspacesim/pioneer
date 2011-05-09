#include "LuaShip.h"
#include "LuaSpaceStation.h"
#include "LuaSBodyPath.h"
#include "LuaShipType.h"
#include "LuaBody.h"
#include "LuaUtils.h"
#include "LuaConstants.h"
#include "Ship.h"
#include "SpaceStation.h"
#include "ShipType.h"
#include "Space.h"
#include "Pi.h"
#include "Player.h"
#include "HyperspaceCloud.h"
#include "LmrModel.h"

static int l_ship_is_player(lua_State *l)
{
    lua_pushboolean(l, false);
    return 1;
}

static int l_ship_get_stats(lua_State *l)
{
	LUA_DEBUG_START(l);

	Ship *s = LuaShip::GetFromLua(1);
	const shipstats_t *stats = s->CalcStats();
	
	lua_newtable(l);
    pi_lua_table_ro(l);
	pi_lua_settable(l, "maxCapacity",        stats->max_capacity);
	pi_lua_settable(l, "usedCapacity",       stats->used_capacity);
	pi_lua_settable(l, "usedCargo",          stats->used_cargo);
	pi_lua_settable(l, "freeCapacity",       stats->free_capacity);
	pi_lua_settable(l, "totalMass",          stats->total_mass);
	pi_lua_settable(l, "hullMassLeft",       stats->hull_mass_left);
	pi_lua_settable(l, "hyperspaceRange",    stats->hyperspace_range);
	pi_lua_settable(l, "maxHyperspaceRange", stats->hyperspace_range_max);
	pi_lua_settable(l, "shieldMass",         stats->shield_mass);
	pi_lua_settable(l, "shieldMassLeft",     stats->shield_mass_left);

	LUA_DEBUG_END(l, 1);

	return 1;
}

static int l_ship_set_label(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	const char *label = luaL_checkstring(l, 2);

	ShipFlavour f = *(s->GetFlavour());
	strncpy(f.regid, label, 16);
	s->UpdateFlavour(&f);

	s->SetLabel(label);
	return 0;
}

static void _prepare_colour(lua_State *l, LmrMaterial &m)
{
	float r, g, b;

	if (lua_type(l, 2) == LUA_TSTRING) {
		const char *hexstr = lua_tostring(l, 2);
		if (strlen(hexstr) != 7 || *hexstr != '#')
			luaL_error(l, "Colour string should be a hex triple (#rrggbb)");
		int n = strtol(&hexstr[1], NULL, 16);
		r = (float)((n & 0xff0000) >> 16);
		g = (float)((n & 0xff00) >> 8);
		b = (float)(n & 0xff);
	}

	else {
		r = luaL_checknumber(l, 2);
		g = luaL_checknumber(l, 3);
		b = luaL_checknumber(l, 4);
	}

	r /= 256;
	g /= 256;
	b /= 256;

	float invmax = 1.0f / std::max(r, std::max(g, b));

	r *= invmax;
	g *= invmax;
	b *= invmax;

	m.diffuse[0] = 0.5f * r;
	m.diffuse[1] = 0.5f * g;
	m.diffuse[2] = 0.5f * b;
	m.diffuse[3] = 1.0f;
	m.specular[0] = r;
	m.specular[1] = g;
	m.specular[2] = b;
	m.specular[3] = 0.0f;
	m.emissive[0] = m.emissive[1] = m.emissive[2] = m.emissive[3] = 0.0f;
	m.shininess = 50.0f + (float)Pi::rng.Double()*50.0f;
}

static int l_ship_set_primary_colour(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);

	ShipFlavour f = *(s->GetFlavour());
	_prepare_colour(l, f.primaryColor);
	s->UpdateFlavour(&f);

	return 0;
}

static int l_ship_set_secondary_colour(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);

	ShipFlavour f = *(s->GetFlavour());
	_prepare_colour(l, f.secondaryColor);
	s->UpdateFlavour(&f);

	return 0;
}

static int l_ship_get_equip_slot_size(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	Equip::Slot slot = static_cast<Equip::Slot>(LuaConstants::GetConstant(l, "EquipSlot", luaL_checkstring(l, 2)));
	lua_pushinteger(l, s->m_equipment.GetSlotSize(slot));
	return 1;
}

static int l_ship_get_equip(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	Equip::Slot slot = static_cast<Equip::Slot>(LuaConstants::GetConstant(l, "EquipSlot", luaL_checkstring(l, 2)));

	int size = s->m_equipment.GetSlotSize(slot);
	if (size == 0)
		return 0;
	
	if (lua_isnumber(l, 3)) {
		int idx = lua_tonumber(l, 3);
		lua_pushinteger(l, s->m_equipment.Get(slot, idx));
		return 1;
	}

	for (int idx = 0; idx < size; idx++)
		lua_pushinteger(l, s->m_equipment.Get(slot, idx));
	return size;
}

static int l_ship_set_equip(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	Equip::Slot slot = static_cast<Equip::Slot>(LuaConstants::GetConstant(l, "EquipSlot", luaL_checkstring(l, 2)));
	int idx = luaL_checkinteger(l, 3);
	Equip::Type e = static_cast<Equip::Type>(LuaConstants::GetConstant(l, "EquipType", luaL_checkstring(l, 4)));

	s->m_equipment.Set(slot, idx, e);
	s->UpdateMass();
	return 0;
}

static int l_ship_add_equip(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	Equip::Type e = static_cast<Equip::Type>(LuaConstants::GetConstant(l, "EquipType", luaL_checkstring(l, 2)));

	int num = 1;
	if (lua_isnumber(l, 3))
		num = lua_tointeger(l, 3);

	const shipstats_t *stats = s->CalcStats();
	if (stats->free_capacity < EquipType::types[e].mass) {
		lua_pushboolean(l, false);
		return 1;
	}
	
	lua_pushboolean(l, s->m_equipment.Add(e, num));
	s->UpdateMass();
	return 1;
}

static int l_ship_remove_equip(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	Equip::Type e = static_cast<Equip::Type>(LuaConstants::GetConstant(l, "EquipType", luaL_checkstring(l, 2)));

	int num = 1;
	if (lua_isnumber(l, 3))
		num = lua_tointeger(l, 3);

	lua_pushinteger(l, s->m_equipment.Remove(e, num));
	s->UpdateMass();
	return 1;
}

static int l_ship_get_equip_count(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	Equip::Slot slot = static_cast<Equip::Slot>(LuaConstants::GetConstant(l, "EquipSlot", luaL_checkstring(l, 2)));
	Equip::Type e = static_cast<Equip::Type>(LuaConstants::GetConstant(l, "EquipType", luaL_checkstring(l, 3)));
	lua_pushinteger(l, s->m_equipment.Count(slot, e));
	return 1;
}

static int l_ship_get_equip_free(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	Equip::Slot slot = static_cast<Equip::Slot>(LuaConstants::GetConstant(l, "EquipSlot", luaL_checkstring(l, 2)));

	lua_pushinteger(l, s->m_equipment.FreeSpace(slot));
	return 1;
}

static int l_ship_jettison(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	Equip::Type e = static_cast<Equip::Type>(LuaConstants::GetConstant(l, "EquipType", luaL_checkstring(l, 2)));

	lua_pushboolean(l, s->Jettison(e));
	return 1;
}

static int l_ship_fire_missile_at(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	Equip::Type e = static_cast<Equip::Type>(LuaConstants::GetConstant(l, "EquipType", luaL_checkstring(l, 2)));
	Ship *target = LuaShip::GetFromLua(3);

	if (e < Equip::MISSILE_UNGUIDED || e > Equip::MISSILE_NAVAL)
		luaL_error(l, "Equipment type '%s' is not a valid missile type", lua_tostring(l, 2));
	
	int max_missiles = s->m_equipment.GetSlotSize(Equip::SLOT_MISSILE);
	int idx;
	for (idx = 0; idx < max_missiles; idx++)
		if (s->m_equipment.Get(Equip::SLOT_MISSILE, idx) == e)
			break;
	
	if (idx == max_missiles) {
		lua_pushboolean(l, false);
		return 1;
	}

	lua_pushboolean(l, s->FireMissile(idx, target));
	return 1;
}

static int l_ship_get_docked_with(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	SpaceStation *station = s->GetDockedWith();
	if (!station) return 0;
	LuaSpaceStation::PushToLua(station);
	return 1;
}

static int l_ship_undock(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	if (!s->GetDockedWith())
		luaL_error(l, "Can't undock if not already docked");
	bool undocking = s->Undock();
	lua_pushboolean(l, undocking);
	return 1;
}

static int l_ship_ai_kill(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	Ship *target = LuaShip::GetFromLua(2);
	s->AIKill(target);
	return 0;
}

static int l_ship_ai_fly_to(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	Body *target = LuaBody::GetFromLua(2);
	s->AIFlyTo(target);
	return 0;
}

static int l_ship_ai_dock_with(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	SpaceStation *target = LuaSpaceStation::GetFromLua(2);
	s->AIDock(target);
	return 0;
}

static int l_ship_ai_enter_low_orbit(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	Body *target = LuaBody::GetFromLua(2);
	s->AIOrbit(target, 1.1);
	return 0;
}

static int l_ship_ai_enter_medium_orbit(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	Body *target = LuaBody::GetFromLua(2);
	s->AIOrbit(target, 2.0);
	return 0;
}

static int l_ship_ai_enter_high_orbit(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	Body *target = LuaBody::GetFromLua(2);
	s->AIOrbit(target, 5.0);
	return 0;
}

static int l_ship_cancel_ai(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	s->AIClearInstructions();
	return 0;
}

static int l_ship_can_hyperspace_to(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	SBodyPath *dest = LuaSBodyPath::GetFromLua(2);

	int fuel;
	double duration;
	Ship::HyperjumpStatus status;

	if (s->CanHyperspaceTo(dest, fuel, duration, &status)) {
		lua_pushinteger(l, status);
		lua_pushinteger(l, fuel);
		lua_pushnumber(l, duration);
		return 3;
	}

	lua_pushstring(l, LuaConstants::GetConstantString(l, "ShipJumpStatus", status));
	return 1;
}

static int l_ship_hyperspace_to(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	SBodyPath *dest = LuaSBodyPath::GetFromLua(2);

	int fuel;
	double duration;
	Ship::HyperjumpStatus status;

	if (!s->CanHyperspaceTo(dest, fuel, duration, &status))
	{
		lua_pushstring(l, LuaConstants::GetConstantString(l, "ShipJumpStatus", Ship::HYPERJUMP_OK));
		return 1;
	}

	Space::StartHyperspaceTo(s, dest);

	lua_pushstring(l, LuaConstants::GetConstantString(l, "ShipJumpStatus", Ship::HYPERJUMP_OK));
	return 1;
}

static int l_ship_attr_alert_status(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	lua_pushstring(l, LuaConstants::GetConstantString(l, "ShipAlertStatus", s->GetAlertState()));
	return 1;
}

static bool promotion_test(DeleteEmitter *o)
{
	return dynamic_cast<Ship*>(o);
}

template <> const char *LuaObject<Ship>::s_type = "Ship";

template <> void LuaObject<Ship>::RegisterClass()
{
	static const char *l_parent = "Body";

	static const luaL_reg l_methods[] = {
		{ "IsPlayer", l_ship_is_player },

		{ "GetStats", l_ship_get_stats },

		{ "SetLabel",           l_ship_set_label            },
		{ "SetPrimaryColour",   l_ship_set_primary_colour   },
		{ "SetSecondaryColour", l_ship_set_secondary_colour },

		{ "GetEquipSlotSize", l_ship_get_equip_slot_size },
		{ "GetEquip",         l_ship_get_equip           },
		{ "SetEquip",         l_ship_set_equip           },
		{ "AddEquip",         l_ship_add_equip           },
		{ "RemoveEquip",      l_ship_remove_equip        },
		{ "GetEquipCount",    l_ship_get_equip_count     },
		{ "GetEquipFree",     l_ship_get_equip_free      },

		{ "Jettison", l_ship_jettison },

		{ "FireMissileAt", l_ship_fire_missile_at },

		{ "GetDockedWith", l_ship_get_docked_with },
		{ "Undock",        l_ship_undock          },

		{ "AIKill",             l_ship_ai_kill               },
		{ "AIFlyTo",            l_ship_ai_fly_to             },
		{ "AIDockWith",         l_ship_ai_dock_with          },
		{ "AIEnterLowOrbit",    l_ship_ai_enter_low_orbit    },
		{ "AIEnterMediumOrbit", l_ship_ai_enter_medium_orbit },
		{ "AIEnterHighOrbit",   l_ship_ai_enter_high_orbit   },
		{ "CancelAI",           l_ship_cancel_ai             },

		{ "CanHyperspaceTo", l_ship_can_hyperspace_to },
		{ "HyperspaceTo",    l_ship_hyperspace_to     },

		{ 0, 0 }
	};

	static const luaL_reg l_attrs[] = {
		{ "alertStatus", l_ship_attr_alert_status },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, l_attrs, NULL);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, promotion_test);
}
