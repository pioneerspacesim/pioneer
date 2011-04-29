#include "LuaShip.h"
#include "LuaSpaceStation.h"
#include "LuaSBodyPath.h"
#include "LuaShipType.h"
#include "LuaBody.h"
#include "LuaUtils.h"
#include "Ship.h"
#include "SpaceStation.h"
#include "ShipType.h"
#include "Space.h"
#include "Pi.h"
#include "Player.h"
#include "HyperspaceCloud.h"
#include "LmrModel.h"

/*
 * Class: LuaShip
 *
 * Lua class that represents a ship. Inherits from <LuaBody>.
 */

/*
 * Function: Ship.IsPlayer
 *
 * Determines if the ship is the player ship
 * 
 * > isplayer = ship:IsPlayer()
 *
 * Returns:
 *
 *   isplayer - true if the ship is the player, false otherwise
 *
 * Example:
 *
 * > if Game.player:IsPlayer() then
 * >     print("this is the player")
 * > end
 */
static int l_ship_is_player(lua_State *l)
{
    lua_pushboolean(l, false);
    return 1;
}

/*
 * Function: Ship.GetStats
 *
 * Returns statistics for the ship
 *
 * > stats - ship:GetStats()
 *
 * Returns:
 *
 *   stats - a table with the following fields
 *
 *     max_capacity - maximum space for cargo and equipment (t)
 *     used_capacity - amount of space used (t)
 *     used_cargo - amount of cargo space used (t)
 *     free_capacity - total space remaining (t)
 *     total_mass - total mass of the ship (cargo, equipment & hull) (t)
 *     hull_mass_left - remaining hull mass. when this reaches 0, the ship is destroyed (t)
 *     shield_mass - total mass equivalent of all shields (t)
 *     shield_mass_left - remaining shield mass. when this reaches 0, the shields are depleted and the hull is exposed (t)
 *     hyperspace_range - distance of furthest possible jump based on current contents (ly)
 *     hyperspace_range_max - distance furthest possible jump under ideal conditions (ly)
 *
 * Example:
 *
 * > local stats = ship:GetStats()
 * > if stats.shield_mass == stats.shield_mass_left then
 * >     print("shields at full strength")
 * > end
 */
static int l_ship_get_stats(lua_State *l)
{
	LUA_DEBUG_START(l);

	Ship *s = LuaShip::GetFromLua(1);
	const shipstats_t *stats = s->CalcStats();
	
	lua_newtable(l);
	pi_lua_settable(l, "max_capacity",         stats->max_capacity);
	pi_lua_settable(l, "used_capacity",        stats->used_capacity);
	pi_lua_settable(l, "used_cargo",           stats->used_cargo);
	pi_lua_settable(l, "free_capacity",        stats->free_capacity);
	pi_lua_settable(l, "total_mass",           stats->total_mass);
	pi_lua_settable(l, "hull_mass_left",       stats->hull_mass_left);
	pi_lua_settable(l, "hyperspace_range",     stats->hyperspace_range);
	pi_lua_settable(l, "hyperspace_range_max", stats->hyperspace_range_max);
	pi_lua_settable(l, "shield_mass",          stats->shield_mass);
	pi_lua_settable(l, "shield_mass_left",     stats->shield_mass_left);

	LUA_DEBUG_END(l, 1);

	return 1;
}

/*
 * Function: Ship.SetLabel
 *
 * Changes the ship's label text.
 *
 * > ship:SetLabel(newlabel)
 *
 * Parameters:
 *
 *   newlabel - the new label. Only the first 16 characters will be available
 *              to the model. The full label will be shown in the HUD.
 * 
 * Example:
 *
 * > ship:SetLabel("AB-1234")
 */
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

/*
 * Function: Ship.SetPrimaryColour
 *
 * Change the ship model's primary colour
 *
 * > ship:SetPrimaryColour(hex)
 * > ship:SetPrimaryColour(red, green, blue)
 *
 * Parameters:
 *
 *   hex - a hex RGB triplet describing the colour, eg. "#99cc99"
 *
 *   red - a real number describing the red component of the colour. 0.0 is no
 *         red component, 1.0 is full red
 *
 *   green - a real number describing the green component of the colour. 0.0
 *           is no green component, 1.0 is full green
 *
 *   blue - a real number describing the blue component of the colour. 0.0 is
 *          no blue component, 1.0 is full blue
 * 
 * Example:
 *
 * > ship:SetPrimaryColour("#002366")       -- royal blue
 * > ship:SetPrimaryColour(1.0, 0.27, 0.0)  -- orange red
 *
 * See also:
 *
 *   <Ship.SetSecondaryColour>
 */
static int l_ship_set_primary_colour(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);

	ShipFlavour f = *(s->GetFlavour());
	_prepare_colour(l, f.primaryColor);
	s->UpdateFlavour(&f);

	return 0;
}

/*
 * Function: Ship.SetSecondaryColour
 *
 * Change the ship model's secondary colour
 *
 * > ship:SetSecondaryColour(hex)
 * > ship:SetSecondaryColour(red, green, blue)
 *
 * Parameters:
 *
 *   hex - a hex RGB triplet describing the colour, eg. "#99cc99"
 *
 *   red - a real number describing the red component of the colour. 0.0 is no
 *         red component, 1.0 is full red
 *
 *   green - a real number describing the green component of the colour. 0.0
 *           is no green component, 1.0 is full green
 *
 *   blue - a real number describing the blue component of the colour. 0.0 is
 *          no blue component, 1.0 is full blue
 * 
 * Example:
 *
 * > ship:SetSecondaryColour("#002366")       -- royal blue
 * > ship:SetSecondaryColour(1.0, 0.27, 0.0)  -- orange red
 *
 * See also:
 *
 *   <Ship.SetPrimaryColour>
 */
static int l_ship_set_secondary_colour(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);

	ShipFlavour f = *(s->GetFlavour());
	_prepare_colour(l, f.secondaryColor);
	s->UpdateFlavour(&f);

	return 0;
}

/*
 * Function: Ship.GetEquipSlotSize
 */
static int l_ship_get_equip_slot_size(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	Equip::Slot slot = static_cast<Equip::Slot>(luaL_checkinteger(l, 2));
	if (slot < 0 || slot >= Equip::SLOT_MAX)
		luaL_error(l, "Invalid equipment slot '%d'", slot);
	lua_pushinteger(l, s->m_equipment.GetSlotSize(slot));
	return 1;
}

/*
 * Function: Ship.GetEquip
 */
static int l_ship_get_equip(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	Equip::Slot slot = static_cast<Equip::Slot>(luaL_checkinteger(l, 2));
	if (slot < 0 || slot >= Equip::SLOT_MAX)
		luaL_error(l, "Invalid equipment slot '%d'", slot);

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

/*
 * Function: Ship.SetEquip
 */
static int l_ship_set_equip(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	Equip::Slot slot = static_cast<Equip::Slot>(luaL_checkinteger(l, 2));
	int idx = luaL_checkinteger(l, 3);
	Equip::Type e = static_cast<Equip::Type>(luaL_checkinteger(l, 4));

	if (slot < 0 || slot >= Equip::SLOT_MAX)
		luaL_error(l, "Invalid equipment slot '%d'", slot);
	if (e <= Equip::NONE || e >= Equip::TYPE_MAX)
		luaL_error(l, "Invalid equipment type '%d'", e);
	
	s->m_equipment.Set(slot, idx, e);
	s->UpdateMass();
	return 0;
}

/*
 * Function: Ship.AddEquip
 */
static int l_ship_add_equip(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	Equip::Type e = static_cast<Equip::Type>(luaL_checkinteger(l, 2));
	if (e <= Equip::NONE || e >= Equip::TYPE_MAX)
		luaL_error(l, "Invalid equipment type '%d'", e);

	int num = 1;
	if (lua_isnumber(l, 3))
		num = lua_tointeger(l, 3);
	
	lua_pushboolean(l, s->m_equipment.Add(e, num));
	s->UpdateMass();
	return 1;
}

/*
 * Function: Ship.RemoveEquip
 */
static int l_ship_remove_equip(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	Equip::Type e = static_cast<Equip::Type>(luaL_checkinteger(l, 2));
	if (e <= Equip::NONE || e >= Equip::TYPE_MAX)
		luaL_error(l, "Invalid equipment type '%d'", e);

	int num = 1;
	if (lua_isnumber(l, 3))
		num = lua_tointeger(l, 3);

	lua_pushinteger(l, s->m_equipment.Remove(e, num));
	s->UpdateMass();
	return 1;
}

/*
 * Function: Ship.GetEquipCount
 */
static int l_ship_get_equip_count(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	Equip::Slot slot = static_cast<Equip::Slot>(luaL_checkinteger(l, 2));
	Equip::Type e = static_cast<Equip::Type>(luaL_checkinteger(l, 3));

	if (slot < 0 || slot >= Equip::SLOT_MAX)
		luaL_error(l, "Invalid equipment slot '%d'", slot);
	if (e <= Equip::NONE || e >= Equip::TYPE_MAX)
		luaL_error(l, "Invalid equipment type '%d'", e);
	
	lua_pushinteger(l, s->m_equipment.Count(slot, e));
	return 1;
}

/*
 * Function: Ship.GetEquipFree
 */
static int l_ship_get_equip_free(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	Equip::Slot slot = static_cast<Equip::Slot>(luaL_checkinteger(l, 2));
	if (slot < 0 || slot >= Equip::SLOT_MAX)
		luaL_error(l, "Invalid equipment slot '%d'", slot);

	lua_pushinteger(l, s->m_equipment.FreeSpace(slot));
	return 1;
}

/*
 * Function: Ship.Jettison
 */
static int l_ship_jettison(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	Equip::Type e = static_cast<Equip::Type>(luaL_checkinteger(l, 2));
	if (e <= Equip::NONE || e >= Equip::TYPE_MAX)
		luaL_error(l, "Invalid equipment type '%d'", e);

	lua_pushboolean(l, s->Jettison(e));
	return 1;
}

/*
 * Function: Ship.GetDockedWith
 */
static int l_ship_get_docked_with(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	SpaceStation *station = s->GetDockedWith();
	if (!station) return 0;
	LuaSpaceStation::PushToLua(station);
	return 1;
}

/*
 * Function: Ship.Undock
 */
static int l_ship_undock(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	if (!s->GetDockedWith())
		luaL_error(l, "Can't undock if not already docked");
	bool undocking = s->Undock();
	lua_pushboolean(l, undocking);
	return 1;
}

/*
 * Function: Ship.Kill
 */
static int l_ship_kill(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	Ship *target = LuaShip::GetFromLua(2);
	s->AIKill(target);
	return 0;
}

/*
 * Function: Ship.FlyTo
 */
static int l_ship_fly_to(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	Body *target = LuaBody::GetFromLua(2);
	s->AIFlyTo(target);
	return 0;
}

/*
 * Function: Ship.DockWith
 */
static int l_ship_dock_with(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	SpaceStation *target = LuaSpaceStation::GetFromLua(2);
	s->AIDock(target);
	return 0;
}

/*
 * Function: Ship.EnterLowOrbit
 */
static int l_ship_enter_low_orbit(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	Body *target = LuaBody::GetFromLua(2);
	s->AIOrbit(target, 1.1);
	return 0;
}

/*
 * Function: Ship.EnterMediumOrbit
 */
static int l_ship_enter_medium_orbit(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	Body *target = LuaBody::GetFromLua(2);
	s->AIOrbit(target, 2.0);
	return 0;
}

/*
 * Function: Ship.EnterHighOrbit
 */
static int l_ship_enter_high_orbit(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	Body *target = LuaBody::GetFromLua(2);
	s->AIOrbit(target, 5.0);
	return 0;
}

/*
 * Function: Ship.CanHyperspaceTo
 */
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

	lua_pushinteger(l, status);
	return 1;
}

/*
 * Function: Ship.HyperspaceTo
 */
static int l_ship_hyperspace_to(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	SBodyPath *dest = LuaSBodyPath::GetFromLua(2);

	int fuel;
	double duration;
	Ship::HyperjumpStatus status;

	if (!s->CanHyperspaceTo(dest, fuel, duration, &status))
	{
		lua_pushinteger(l, status);
		return 1;
	}

	Space::StartHyperspaceTo(s, dest);

	lua_pushinteger(l, Ship::HYPERJUMP_OK);
	return 1;
}

static bool promotion_test(DeleteEmitter *o)
{
	return dynamic_cast<Ship*>(o);
}

template <> const char *LuaObject<Ship>::s_type = "Ship";

template <> void LuaObject<Ship>::RegisterClass()
{
	static const char *l_inherit = "Body";

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

		{ "GetDockedWith", l_ship_get_docked_with },
		{ "Undock",        l_ship_undock          },

		{ "Kill",             l_ship_kill               },
		{ "FlyTo",            l_ship_fly_to             },
		{ "DockWith",         l_ship_dock_with          },
		{ "EnterLowOrbit",    l_ship_enter_low_orbit    },
		{ "EnterMediumOrbit", l_ship_enter_medium_orbit },
		{ "EnterHighOrbit",   l_ship_enter_high_orbit   },

		{ "CanHyperspaceTo", l_ship_can_hyperspace_to },
		{ "HyperspaceTo",    l_ship_hyperspace_to     },

		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_inherit, l_methods, NULL);
	LuaObjectBase::RegisterPromotion(l_inherit, s_type, promotion_test);
}
