// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ShipType.h"
#include "LmrModel.h"
#include "LuaVector.h"
#include "LuaUtils.h"
#include "LuaConstants.h"
#include "FileSystem.h"
#include "utils.h"
#include "Lang.h"

const char *ShipType::gunmountNames[GUNMOUNT_MAX] = {
	Lang::FRONT, Lang::REAR };

std::map<ShipType::Id, ShipType> ShipType::types;

std::vector<ShipType::Id> ShipType::player_ships;
std::vector<ShipType::Id> ShipType::static_ships;
std::vector<ShipType::Id> ShipType::missile_ships;

std::vector<ShipType::Id> ShipType::playable_atmospheric_ships;

std::string ShipType::LADYBIRD				= "ladybird_starfighter";
std::string ShipType::SIRIUS_INTERDICTOR	= "sirius_interdictor";
std::string ShipType::EAGLE_LRF				= "eagle_lrf";
std::string ShipType::EAGLE_MK3				= "eagle_mk3";
std::string ShipType::MISSILE_GUIDED		= "missile_guided";
std::string ShipType::MISSILE_NAVAL			= "missile_naval";
std::string ShipType::MISSILE_SMART			= "missile_smart";
std::string ShipType::MISSILE_UNGUIDED		= "missile_unguided";

static void _get_string_attrib(lua_State *L, const char *key, std::string &output,
		const char *default_output)
{
	LUA_DEBUG_START(L);
	lua_pushstring(L, key);
	lua_gettable(L, -2);
	if (lua_isnil(L, -1)) {
		output = default_output;
	} else {
		output = lua_tostring(L,-1);
	}
	lua_pop(L, 1);
	LUA_DEBUG_END(L, 0);
}

static void _get_float_attrib(lua_State *L, const char *key, float &output,
		const float default_output)
{
	LUA_DEBUG_START(L);
	lua_pushstring(L, key);
	lua_gettable(L, -2);
	if (lua_isnil(L, -1)) {
		output = default_output;
	} else {
		output = lua_tonumber(L,-1);
	}
	lua_pop(L, 1);
	LUA_DEBUG_END(L, 0);
}

static void _get_int_attrib(lua_State *L, const char *key, int &output,
		const int default_output)
{
	LUA_DEBUG_START(L);
	lua_pushstring(L, key);
	lua_gettable(L, -2);
	if (lua_isnil(L, -1)) {
		output = default_output;
	} else {
		output = lua_tointeger(L,-1);
	}
	lua_pop(L, 1);
	LUA_DEBUG_END(L, 0);
}

static void _get_vec_attrib(lua_State *L, const char *key, vector3d &output,
	const vector3d default_output)
{
	LUA_DEBUG_START(L);
	lua_pushstring(L, key);
	lua_gettable(L, -2);
	if (lua_isnil(L, -1)) {
		output = default_output;
	} else {
		output = *LuaVector::CheckFromLua(L, -1);
	}
	lua_pop(L, 1);
	LUA_DEBUG_END(L, 0);
}

// returns velocity of engine exhausts in m/s
static double GetEffectiveExhaustVelocity(double fuelTankMass, double thrusterFuelUse, double forwardThrust) {
	double denominator = fuelTankMass * thrusterFuelUse * 10;
	return fabs(denominator > 0 ? forwardThrust/denominator : 1e9);
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
	_get_string_attrib(L, "name", s.name, "");
	_get_string_attrib(L, "model", s.lmrModelName, "");
	_get_float_attrib(L, "reverse_thrust", s.linThrust[ShipType::THRUSTER_REVERSE], 0.0f);
	_get_float_attrib(L, "forward_thrust", s.linThrust[ShipType::THRUSTER_FORWARD], 0.0f);
	_get_float_attrib(L, "up_thrust", s.linThrust[ShipType::THRUSTER_UP], 0.0f);
	_get_float_attrib(L, "down_thrust", s.linThrust[ShipType::THRUSTER_DOWN], 0.0f);
	_get_float_attrib(L, "left_thrust", s.linThrust[ShipType::THRUSTER_LEFT], 0.0f);
	_get_float_attrib(L, "right_thrust", s.linThrust[ShipType::THRUSTER_RIGHT], 0.0f);
	_get_float_attrib(L, "angular_thrust", s.angThrust, 0.0f);
	// invert values where necessary
	s.linThrust[ShipType::THRUSTER_FORWARD] *= -1.f;
	s.linThrust[ShipType::THRUSTER_LEFT] *= -1.f;
	s.linThrust[ShipType::THRUSTER_DOWN] *= -1.f;
	// angthrust fudge (XXX: why?)
	s.angThrust = s.angThrust / 2;

	_get_vec_attrib(L, "camera_offset", s.cameraOffset, vector3d(0.0));

	for (int i=0; i<Equip::SLOT_MAX; i++) s.equipSlotCapacity[i] = 0;
	_get_int_attrib(L, "max_cargo", s.equipSlotCapacity[Equip::SLOT_CARGO], 0);
	_get_int_attrib(L, "max_engine", s.equipSlotCapacity[Equip::SLOT_ENGINE], 1);
	_get_int_attrib(L, "max_laser", s.equipSlotCapacity[Equip::SLOT_LASER], 1);
	_get_int_attrib(L, "max_missile", s.equipSlotCapacity[Equip::SLOT_MISSILE], 0);
	_get_int_attrib(L, "max_ecm", s.equipSlotCapacity[Equip::SLOT_ECM], 1);
	_get_int_attrib(L, "max_scanner", s.equipSlotCapacity[Equip::SLOT_SCANNER], 1);
	_get_int_attrib(L, "max_radarmapper", s.equipSlotCapacity[Equip::SLOT_RADARMAPPER], 1);
	_get_int_attrib(L, "max_hypercloud", s.equipSlotCapacity[Equip::SLOT_HYPERCLOUD], 1);
	_get_int_attrib(L, "max_hullautorepair", s.equipSlotCapacity[Equip::SLOT_HULLAUTOREPAIR], 1);
	_get_int_attrib(L, "max_energybooster", s.equipSlotCapacity[Equip::SLOT_ENERGYBOOSTER], 1);
	_get_int_attrib(L, "max_atmoshield", s.equipSlotCapacity[Equip::SLOT_ATMOSHIELD], 1);
	_get_int_attrib(L, "max_cabin", s.equipSlotCapacity[Equip::SLOT_CABIN], 50);
	_get_int_attrib(L, "max_shield", s.equipSlotCapacity[Equip::SLOT_SHIELD], 9999);
	_get_int_attrib(L, "max_fuelscoop", s.equipSlotCapacity[Equip::SLOT_FUELSCOOP], 1);
	_get_int_attrib(L, "max_cargoscoop", s.equipSlotCapacity[Equip::SLOT_CARGOSCOOP], 1);
	_get_int_attrib(L, "max_lasercooler", s.equipSlotCapacity[Equip::SLOT_LASERCOOLER], 1);
	_get_int_attrib(L, "max_cargolifesupport", s.equipSlotCapacity[Equip::SLOT_CARGOLIFESUPPORT], 1);
	_get_int_attrib(L, "max_autopilot", s.equipSlotCapacity[Equip::SLOT_AUTOPILOT], 1);

	_get_int_attrib(L, "capacity", s.capacity, 0);
	_get_int_attrib(L, "hull_mass", s.hullMass, 100);
	_get_int_attrib(L, "fuel_tank_mass", s.fuelTankMass, 5);

	// fuel_use_rate can be given in two ways
	float thruster_fuel_use = 0;
	_get_float_attrib(L, "effective_exhaust_velocity", s.effectiveExhaustVelocity, -1.0f);
	_get_float_attrib(L, "thruster_fuel_use", thruster_fuel_use, -1.0f);
	if(s.effectiveExhaustVelocity < 0 && thruster_fuel_use < 0) {
		// default value of v_c is used
		s.effectiveExhaustVelocity = 55000000;
	} else if(s.effectiveExhaustVelocity < 0 && thruster_fuel_use >= 0) {
		// v_c undefined and thruster fuel use defined -- use it!
		s.effectiveExhaustVelocity = GetEffectiveExhaustVelocity(s.fuelTankMass, thruster_fuel_use, s.linThrust[ShipType::THRUSTER_FORWARD]);
	} else {
		if(thruster_fuel_use >= 0)
			printf("Warning: Both thruster_fuel_use and effective_exhaust_velocity defined for %s, using effective_exhaust_velocity.\n", s.lmrModelName.c_str());
	}

	_get_int_attrib(L, "price", s.baseprice, 0);
	s.baseprice *= 100; // in hundredths of credits

	s.equipSlotCapacity[Equip::SLOT_ENGINE] = Clamp(s.equipSlotCapacity[Equip::SLOT_ENGINE], 0, 1);

	{
		int hyperclass;
		_get_int_attrib(L, "hyperdrive_class", hyperclass, 1);
		if (!hyperclass) {
			s.hyperdrive = Equip::NONE;
		} else {
			s.hyperdrive = Equip::Type(Equip::DRIVE_CLASS1+hyperclass-1);
		}
	}

	lua_pushstring(L, "gun_mounts");
	lua_gettable(L, -2);
	if (lua_istable(L, -1)) {
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

	if (s.lmrModelName.empty())
		return luaL_error(L, "Missing model name in ship");

	//this shouldn't necessarily be a fatal problem, could just warn+mark ship unusable
	//or replace with proxy geometry
#if 0
	try {
		LmrLookupModelByName(s.lmrModelName.c_str());
	} catch (LmrModelNotFoundException &) {
		return luaL_error(L, "Model %s is not defined", s.lmrModelName.c_str());
	}
#endif

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
		if (ends_with(info.GetPath(), ".lua")) {
			const std::string name = info.GetName();
			s_currentShipFile = name.substr(0, name.size()-4);
			pi_lua_dofile(l, info.GetPath());
			s_currentShipFile.clear();
		}
	}

	LUA_DEBUG_END(l, 0);

	lua_close(l);

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

