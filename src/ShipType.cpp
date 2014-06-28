// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ShipType.h"
#include "LuaVector.h"
#include "LuaUtils.h"
#include "LuaTable.h"
#include "LuaConstants.h"
#include "FileSystem.h"
#include "utils.h"
#include "Lang.h"
#include <algorithm>

const char *ShipType::gunmountNames[GUNMOUNT_MAX] = {
	Lang::FRONT, Lang::REAR };

std::map<ShipType::Id, ShipType> ShipType::types;

std::vector<ShipType::Id> ShipType::player_ships;
std::vector<ShipType::Id> ShipType::static_ships;
std::vector<ShipType::Id> ShipType::missile_ships;

std::vector<ShipType::Id> ShipType::playable_atmospheric_ships;

std::string ShipType::POLICE				= "kanara";
std::string ShipType::MISSILE_GUIDED		= "missile_guided";
std::string ShipType::MISSILE_NAVAL			= "missile_naval";
std::string ShipType::MISSILE_SMART			= "missile_smart";
std::string ShipType::MISSILE_UNGUIDED		= "missile_unguided";

float ShipType::GetFuelUseRate() const
{
	const float denominator = fuelTankMass * effectiveExhaustVelocity * 10;
	return denominator > 0 ? -linThrust[THRUSTER_FORWARD]/denominator : 1e9;
}

// returns velocity of engine exhausts in m/s
static double GetEffectiveExhaustVelocity(double fuelTankMass, double thrusterFuelUse, double forwardThrust) {
	double denominator = fuelTankMass * thrusterFuelUse * 10;
	return fabs(denominator > 0 ? forwardThrust/denominator : 1e9);
}

static bool ShipIsUnbuyable(const ShipType::Id &id)
{
	const ShipType &t = ShipType::types[id];
	return (t.baseprice == 0);
}

static std::string s_currentShipFile;

int _define_ship(lua_State *L, ShipType::Tag tag, std::vector<ShipType::Id> *list)
{
	if (s_currentShipFile.empty())
		return luaL_error(L, "ship file contains multiple ship definitions");

	ShipType s;
	s.tag = tag;
	s.id = s_currentShipFile;

	LUA_DEBUG_START(L);
	LuaTable t(L, -1);

	s.name = t.Get("name", "");
	s.shipClass = t.Get("ship_class", "unknown");
	s.manufacturer = t.Get("manufacturer", "unknown");
	s.modelName = t.Get("model", "");

	s.cockpitName = t.Get("cockpit", "");
	s.linThrust[ShipType::THRUSTER_REVERSE] = t.Get("reverse_thrust", 0.0f);
	s.linThrust[ShipType::THRUSTER_FORWARD] = t.Get("forward_thrust", 0.0f);
	s.linThrust[ShipType::THRUSTER_UP] = t.Get("up_thrust", 0.0f);
	s.linThrust[ShipType::THRUSTER_DOWN] = t.Get("down_thrust", 0.0f);
	s.linThrust[ShipType::THRUSTER_LEFT] = t.Get("left_thrust", 0.0f);
	s.linThrust[ShipType::THRUSTER_RIGHT] = t.Get("right_thrust", 0.0f);
	s.angThrust = t.Get("angular_thrust", 0.0f);
	// invert values where necessary
	s.linThrust[ShipType::THRUSTER_FORWARD] *= -1.f;
	s.linThrust[ShipType::THRUSTER_LEFT] *= -1.f;
	s.linThrust[ShipType::THRUSTER_DOWN] *= -1.f;
	// angthrust fudge (XXX: why?)
	s.angThrust = s.angThrust / 2;

	lua_pushstring(L, "camera_offset");
	lua_gettable(L, -2);
	if (!lua_isnil(L, -1))
		Output("ship definition for '%s' has deprecated 'camera_offset' field\n", s.id.c_str());
	lua_pop(L, 1);
	s.cameraOffset = t.Get("camera_offset", vector3d(0.0));

	for (int i=0; i<Equip::SLOT_MAX; i++) s.equipSlotCapacity[i] = 0;
	s.equipSlotCapacity[Equip::SLOT_CARGO] = t.Get("max_cargo", 0);
	s.equipSlotCapacity[Equip::SLOT_ENGINE] = t.Get("max_engine", 1);
	s.equipSlotCapacity[Equip::SLOT_LASER] = t.Get("max_laser", 1);
	s.equipSlotCapacity[Equip::SLOT_MISSILE] = t.Get("max_missile", 0);
	s.equipSlotCapacity[Equip::SLOT_ECM] = t.Get("max_ecm", 1);
	s.equipSlotCapacity[Equip::SLOT_SCANNER] = t.Get("max_scanner", 1);
	s.equipSlotCapacity[Equip::SLOT_RADARMAPPER] = t.Get("max_radarmapper", 1);
	s.equipSlotCapacity[Equip::SLOT_HYPERCLOUD] = t.Get("max_hypercloud", 1);
	s.equipSlotCapacity[Equip::SLOT_HULLAUTOREPAIR] = t.Get("max_hullautorepair", 1);
	s.equipSlotCapacity[Equip::SLOT_ENERGYBOOSTER] = t.Get("max_energybooster", 1);
	s.equipSlotCapacity[Equip::SLOT_ATMOSHIELD] = t.Get("max_atmoshield", 1);
	s.equipSlotCapacity[Equip::SLOT_CABIN] = t.Get("max_cabin", 50);
	s.equipSlotCapacity[Equip::SLOT_SHIELD] = t.Get("max_shield", 9999);
	s.equipSlotCapacity[Equip::SLOT_FUELSCOOP] = t.Get("max_fuelscoop", 1);
	s.equipSlotCapacity[Equip::SLOT_CARGOSCOOP] = t.Get("max_cargoscoop", 1);
	s.equipSlotCapacity[Equip::SLOT_LASERCOOLER] = t.Get("max_lasercooler", 1);
	s.equipSlotCapacity[Equip::SLOT_CARGOLIFESUPPORT] = t.Get("max_cargolifesupport", 1);
	s.equipSlotCapacity[Equip::SLOT_AUTOPILOT] = t.Get("max_autopilot", 1);

	s.capacity = t.Get("capacity", 0);
	s.hullMass = t.Get("hull_mass", 100);
	s.fuelTankMass = t.Get("fuel_tank_mass", 5);

	LuaTable slot_table = t.Sub("slots");
	if (slot_table.GetLua()) {
		s.slots = slot_table.GetMap<std::string, int>();
	}
	lua_pop(L, 1);

	// fuel_use_rate can be given in two ways
	float thruster_fuel_use = 0;
	s.effectiveExhaustVelocity = t.Get("effective_exhaust_velocity", -1.0f);
	thruster_fuel_use = t.Get("thruster_fuel_use", -1.0f);
	if(s.effectiveExhaustVelocity < 0 && thruster_fuel_use < 0) {
		// default value of v_c is used
		s.effectiveExhaustVelocity = 55000000;
	} else if(s.effectiveExhaustVelocity < 0 && thruster_fuel_use >= 0) {
		// v_c undefined and thruster fuel use defined -- use it!
		s.effectiveExhaustVelocity = GetEffectiveExhaustVelocity(s.fuelTankMass, thruster_fuel_use, s.linThrust[ShipType::THRUSTER_FORWARD]);
	} else {
		if(thruster_fuel_use >= 0)
			Output("Warning: Both thruster_fuel_use and effective_exhaust_velocity defined for %s, using effective_exhaust_velocity.\n", s.modelName.c_str());
	}

	s.baseprice = t.Get("price", 0);
	s.baseprice *= 100; // in hundredths of credits

	s.minCrew = t.Get("min_crew", 1);
	s.maxCrew = t.Get("max_crew", 1);

	s.equipSlotCapacity[Equip::SLOT_ENGINE] = Clamp(s.equipSlotCapacity[Equip::SLOT_ENGINE], 0, 1);

	s.hyperdriveClass = t.Get("hyperdrive_class", 1);

	for (int i = 0; i < ShipType::GUNMOUNT_MAX; i++) {
		s.gunMount[i].pos = vector3f(0,0,0);
		s.gunMount[i].dir = vector3f(0,0,1);
		s.gunMount[i].sep = 5;
		s.gunMount[i].orient = ShipType::DUAL_LASERS_HORIZONTAL;
	}

	lua_pushstring(L, "gun_mounts");
	lua_gettable(L, -2);
	if (lua_istable(L, -1)) {
		Output("ship definition for '%s' has deprecated 'gun_mounts' field\n", s.id.c_str());
		for (unsigned int i=0; i<lua_rawlen(L,-1); i++) {
			lua_pushinteger(L, i+1);
			lua_gettable(L, -2);
			if (lua_istable(L, -1) && lua_rawlen(L,-1) == 4)	{
				lua_pushinteger(L, 1);
				lua_gettable(L, -2);
				s.gunMount[i].pos = LuaVector::CheckFromLuaF(L, -1);
				lua_pop(L, 1);
				lua_pushinteger(L, 2);
				lua_gettable(L, -2);
				s.gunMount[i].dir = LuaVector::CheckFromLuaF(L, -1);
				lua_pop(L, 1);
				lua_pushinteger(L, 3);
				lua_gettable(L, -2);
				s.gunMount[i].sep = lua_tonumber(L,-1);
				lua_pop(L, 1);
				lua_pushinteger(L, 4);
				lua_gettable(L, -2);
				s.gunMount[i].orient = static_cast<ShipType::DualLaserOrientation>(
						LuaConstants::GetConstantFromArg(L, "DualLaserOrientation", -1));
				lua_pop(L, 1);
			}
			lua_pop(L, 1);
		}
	}
	lua_pop(L, 1);
	LUA_DEBUG_END(L, 0);

	//sanity check
	if (s.name.empty())
		return luaL_error(L, "Ship has no name");

	if (s.modelName.empty())
		return luaL_error(L, "Missing model name in ship");

	if (s.minCrew < 1 || s.maxCrew < 1 || s.minCrew > s.maxCrew)
		return luaL_error(L, "Invalid values for min_crew and max_crew");

	const std::string& id = s_currentShipFile;
	typedef std::map<ShipType::Id, ShipType>::iterator iter;
	std::pair<iter, bool> result = ShipType::types.insert(std::make_pair(id, s));
	if (result.second)
		list->push_back(s_currentShipFile);
	else
		return luaL_error(L, "Ship '%s' was already defined by a different file", id.c_str());
	s_currentShipFile.clear();

	return 0;
}

int define_ship(lua_State *L)
{
	return _define_ship(L, ShipType::TAG_SHIP, &ShipType::player_ships);
}

int define_static_ship(lua_State *L)
{
	return _define_ship(L, ShipType::TAG_STATIC_SHIP, &ShipType::static_ships);
}

int define_missile(lua_State *L)
{
	return _define_ship(L, ShipType::TAG_MISSILE, &ShipType::missile_ships);
}

void ShipType::Init()
{
	static bool isInitted = false;
	if (isInitted) return;
	isInitted = true;

	lua_State *l = luaL_newstate();

	LUA_DEBUG_START(l);

	luaL_requiref(l, "_G", &luaopen_base, 1);
	luaL_requiref(l, LUA_DBLIBNAME, &luaopen_debug, 1);
	luaL_requiref(l, LUA_MATHLIBNAME, &luaopen_math, 1);
	lua_pop(l, 3);

	LuaConstants::Register(l);
	LuaVector::Register(l);
	LUA_DEBUG_CHECK(l, 0);

	// provide shortcut vector constructor: v = vector.new
	lua_getglobal(l, LuaVector::LibName);
	lua_getfield(l, -1, "new");
	assert(lua_iscfunction(l, -1));
	lua_setglobal(l, "v");
	lua_pop(l, 1); // pop the vector library table

	LUA_DEBUG_CHECK(l, 0);

	// register ship definition functions
	lua_register(l, "define_ship", define_ship);
	lua_register(l, "define_static_ship", define_static_ship);
	lua_register(l, "define_missile", define_missile);

	LUA_DEBUG_CHECK(l, 0);

	// load all ship definitions
	namespace fs = FileSystem;
	for (fs::FileEnumerator files(fs::gameDataFiles, "ships", fs::FileEnumerator::Recurse);
			!files.Finished(); files.Next()) {
		const fs::FileInfo &info = files.Current();
		if (ends_with_ci(info.GetPath(), ".lua")) {
			const std::string name = info.GetName();
			s_currentShipFile = name.substr(0, name.size()-4);
			pi_lua_dofile(l, info.GetPath());
			s_currentShipFile.clear();
		}
	}

	LUA_DEBUG_END(l, 0);

	lua_close(l);

	//remove unbuyable ships from player ship list
	ShipType::player_ships.erase(
		std::remove_if(ShipType::player_ships.begin(), ShipType::player_ships.end(), ShipIsUnbuyable),
		ShipType::player_ships.end());

	if (ShipType::player_ships.empty())
		Error("No playable ships have been defined! The game cannot run.");

	//collect ships that can fit atmospheric shields
	for (std::vector<ShipType::Id>::const_iterator it = ShipType::player_ships.begin();
		it != ShipType::player_ships.end(); ++it) {
		const ShipType &ship = ShipType::types[*it];
		if (ship.equipSlotCapacity[Equip::SLOT_ATMOSHIELD] != 0)
			ShipType::playable_atmospheric_ships.push_back(*it);
	}

	if (ShipType::playable_atmospheric_ships.empty())
		Error("No ships can fit atmospheric shields! The game cannot run.");
}

