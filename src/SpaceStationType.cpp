// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "SpaceStationType.h"
#include "FileSystem.h"
#include "ModelBase.h"
#include "Lua.h"
#include "LuaVector.h"
#include "LuaVector.h"
#include "Pi.h"
#include "Ship.h"
#include "StringF.h"

static lua_State *s_lua;
static std::string s_currentStationFile = "";
std::vector<SpaceStationType> SpaceStationType::surfaceStationTypes;
std::vector<SpaceStationType> SpaceStationType::orbitalStationTypes;

SpaceStationType::SpaceStationType()
: id("")
, model(0)
, modelName("")
, angVel(0.f)
, dockMethod(SURFACE)
, numDockingPorts(0)
, numDockingStages(0)
, numUndockStages(0)
, shipLaunchStage(0)
, dockAnimStageDuration(0)
, undockAnimStageDuration(0)
, dockOneAtATimePlease(false)
, parkingDistance(0)
, parkingGapSize(0)
, dockAnimFunction("")
, approachWaypointsFunction("")
{}

bool SpaceStationType::GetShipApproachWaypoints(int port, int stage, positionOrient_t &outPosOrient) const
{
	lua_State *L = s_lua;

	LUA_DEBUG_START(L);

	lua_pushcfunction(L, pi_lua_panic);
	lua_getglobal(L, this->approachWaypointsFunction.c_str());

	if (!lua_isfunction(L, -1)) {
		printf("no function\n");
		lua_pop(L, 2);
		LUA_DEBUG_END(L, 0);
		return false;
	}

	lua_pushinteger(L, port+1);
	lua_pushinteger(L, stage);
	lua_pcall(L, 2, 1, -4);
	bool gotOrient;
	if (lua_istable(L, -1)) {
		gotOrient = true;
		lua_pushinteger(L, 1);
		lua_gettable(L, -2);
		outPosOrient.pos = *LuaVector::CheckFromLua(L, -1);
		lua_pop(L, 1);

		lua_pushinteger(L, 2);
		lua_gettable(L, -2);
		outPosOrient.xaxis = *LuaVector::CheckFromLua(L, -1);
		lua_pop(L, 1);

		lua_pushinteger(L, 3);
		lua_gettable(L, -2);
		outPosOrient.yaxis = *LuaVector::CheckFromLua(L, -1);
		lua_pop(L, 1);
	} else {
		gotOrient = false;
	}
	lua_pop(L, 2);

	LUA_DEBUG_END(L, 0);

	return gotOrient;
}

/* when ship is on rails it returns true and fills outPosOrient.
 * when ship has been released (or docked) it returns false.
 * Note station animations may continue for any number of stages after
 * ship has been released and is under player control again */
bool SpaceStationType::GetDockAnimPositionOrient(int port, int stage, double t, const vector3d &from, positionOrient_t &outPosOrient, const Ship *ship) const
{
	if (stage < -shipLaunchStage) { stage = -shipLaunchStage; t = 1.0; }
	if (stage > numDockingStages || !stage) { stage = numDockingStages; t = 1.0; }
	// note case for stageless launch (shipLaunchStage==0)

	lua_State *L = s_lua;

	LUA_DEBUG_START(L);

	lua_pushcfunction(L, pi_lua_panic);
	lua_getglobal(L, this->dockAnimFunction.c_str());
	// It's a function of form function(stage, t, from)
	//model->PushAttributeToLuaStack("ship_dock_anim");
	if (!lua_isfunction(L, -1)) {
		Error("Spacestation %s needs ship_dock_anim method", id.c_str());
	}
	lua_pushinteger(L, port+1);
	lua_pushinteger(L, stage);
	lua_pushnumber(L, double(t));
	LuaVector::PushToLua(L, from);
	// push model aabb as lua table: { min: vec3, max: vec3 }
	{
		Aabb aabb = ship->GetAabb();
		lua_createtable (L, 0, 2);
		LuaVector::PushToLua(L, aabb.max);
		lua_setfield(L, -2, "max");
		LuaVector::PushToLua(L, aabb.min);
		lua_setfield(L, -2, "min");
	}

	lua_pcall(L, 5, 1, -7);
	bool gotOrient;
	if (lua_istable(L, -1)) {
		gotOrient = true;
		lua_pushinteger(L, 1);
		lua_gettable(L, -2);
		outPosOrient.pos = *LuaVector::CheckFromLua(L, -1);
		lua_pop(L, 1);

		lua_pushinteger(L, 2);
		lua_gettable(L, -2);
		outPosOrient.xaxis = *LuaVector::CheckFromLua(L, -1);
		lua_pop(L, 1);

		lua_pushinteger(L, 3);
		lua_gettable(L, -2);
		outPosOrient.yaxis = *LuaVector::CheckFromLua(L, -1);
		lua_pop(L, 1);
	} else {
		gotOrient = false;
	}
	lua_pop(L, 2);

	LUA_DEBUG_END(L, 0);

	return gotOrient;
}

static inline void _get_string(lua_State *l, const char *key, std::string &output)
{
	lua_pushstring(l, key);
	lua_gettable(l, -2);
	output = lua_tostring(l, -1);
	lua_pop(l, 1);
}

static inline void _get_int(lua_State *l, const char *key, int &output)
{
	lua_pushstring(l, key);
	lua_gettable(l, -2);
	output = lua_tointeger(l, -1);
	lua_pop(l, 1);
}

static inline void _get_bool(lua_State *l, const char *key, bool &output)
{
	lua_pushstring(l, key);
	lua_gettable(l, -2);
	output = lua_toboolean(l, -1);
	lua_pop(l, 1);
}

static inline void _get_bool(lua_State *l, const char *key, bool &output, const bool default_output)
{
	lua_pushstring(l, key);
	lua_gettable(l, -2);
	if (lua_isnil(l, -1)) {
		output = default_output;
	} else {
		output = lua_toboolean(l, -1);
	}	
	lua_pop(l, 1);
}

static inline void _get_float(lua_State *l, const char *key, float &output, const float default_output)
{
	lua_pushstring(l, key);
	lua_gettable(l, -2);
	if (lua_isnil(l, -1)) {
		output = default_output;
	} else {
		output = lua_tonumber(l, -1);
	}	
	lua_pop(l, 1);
}

static int _get_stage_durations(lua_State *L, const char *key, int &outNumStages, double **outDurationArray)
{
	LUA_DEBUG_START(L);
	lua_pushstring(L, key);
	lua_gettable(L, -2);
	if (lua_istable(L, -1)) {
		const int numStages = lua_rawlen(L, -1);
		if (numStages < 1)
			return luaL_error(L, "Station must have at least 1 stage in %s", key);
		outNumStages = numStages;
		*outDurationArray = new double[numStages];
		for (int i=1; i <= numStages; i++) {
			lua_pushinteger(L, i);
			lua_gettable(L, -2);
			(*outDurationArray)[i-1] = lua_tonumber(L, -1);
			lua_pop(L, 1);
		}
	}
	lua_pop(L, 1);
	LUA_DEBUG_END(L, 0);
	return 0;
}

static std::string _set_global_function(lua_State *L, const char *function_name, const char *global_prefix)
{
	lua_pushstring(L, function_name);
	lua_gettable(L, -2);
	if (lua_isfunction(L, -1)) {
		const std::string fullName = stringf("%0_%1", global_prefix, function_name);
		lua_setglobal(L, fullName.c_str());
		return fullName;
	} else {
		lua_pop(L, 1);
		return "";
	}
}

static int _define_station(lua_State *L, SpaceStationType &station)
{
	station.id = s_currentStationFile;

	LUA_DEBUG_START(L);
	_get_string(L, "model", station.modelName);
	_get_int(L, "num_docking_ports", station.numDockingPorts);
	_get_bool(L, "dock_one_at_a_time", station.dockOneAtATimePlease, false);
	_get_float(L, "angular_velocity", station.angVel, 0.f);
	_get_float(L, "parking_distance", station.parkingDistance, 5000.f);
	_get_float(L, "parking_gap_size", station.parkingGapSize, 2000.f);
	_get_stage_durations(L, "dock_anim_stage_duration", station.numDockingStages, &station.dockAnimStageDuration);
	_get_stage_durations(L, "undock_anim_stage_duration", station.numUndockStages, &station.undockAnimStageDuration);
	_get_int(L, "ship_launch_stage", station.shipLaunchStage);
	station.dockAnimFunction = _set_global_function(L, "ship_dock_anim", station.id.c_str());
	station.approachWaypointsFunction = _set_global_function(L, "ship_approach_waypoints", station.id.c_str());
	LUA_DEBUG_END(L, 0);

	assert(!station.modelName.empty());
	assert(!station.dockAnimFunction.empty());
	assert(!station.approachWaypointsFunction.empty());
	station.model = Pi::FindModel(station.modelName);
	return 0;
}

static int define_orbital_station(lua_State *L)
{
	SpaceStationType station;
	station.dockMethod = SpaceStationType::ORBITAL;
	_define_station(L, station);
	SpaceStationType::orbitalStationTypes.push_back(station);
	return 0;
}

static int define_surface_station(lua_State *L)
{
	SpaceStationType station;
	station.dockMethod = SpaceStationType::SURFACE;
	_define_station(L, station);
	SpaceStationType::surfaceStationTypes.push_back(station);
	return 0;
}

void SpaceStationType::Init()
{
	assert(s_lua == 0);
	if (s_lua != 0) return;

	s_lua = luaL_newstate();
	lua_State *L = s_lua;

	LUA_DEBUG_START(L);
	pi_lua_open_standard_base(L);

	LuaVector::Register(L);

	LUA_DEBUG_CHECK(L, 0);

	lua_register(L, "define_orbital_station", define_orbital_station);
	lua_register(L, "define_surface_station", define_surface_station);

	namespace fs = FileSystem;
	for (fs::FileEnumerator files(fs::gameDataFiles, "stations", fs::FileEnumerator::Recurse);
			!files.Finished(); files.Next()) {
		const fs::FileInfo &info = files.Current();
		if (ends_with(info.GetPath(), ".lua")) {
			const std::string name = info.GetName();
			s_currentStationFile = name.substr(0, name.size()-4);
			pi_lua_dofile(L, info.GetPath());
			s_currentStationFile.clear();
		}
	}
	LUA_DEBUG_END(L, 0);
}

void SpaceStationType::Uninit()
{
	std::vector<SpaceStationType>::iterator i;
	for (i=surfaceStationTypes.begin(); i!=surfaceStationTypes.end(); ++i) {
		delete[] (*i).dockAnimStageDuration;
		delete[] (*i).undockAnimStageDuration;
	}
	for (i=orbitalStationTypes.begin(); i!=orbitalStationTypes.end(); ++i) {
		delete[] (*i).dockAnimStageDuration;
		delete[] (*i).undockAnimStageDuration;
	}

	lua_close(s_lua); s_lua = 0;
}
