// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "SpaceStationType.h"
#include "FileSystem.h"
#include "Lua.h"
#include "LuaVector.h"
#include "LuaVector.h"
#include "Pi.h"
#include "Ship.h"
#include "StringF.h"
#include "scenegraph/Model.h"

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
, parkingDistance(0)
, parkingGapSize(0)
, dockAnimFunction("")
, approachWaypointsFunction("")
, bHasDockAnimFunction(false)
, bHasApproachWaypointsFunction(false)
{}

#pragma optimize( "", off )
void SpaceStationType::OnSetupComplete()
{
	SceneGraph::Model::TVecMT approach_mts;
	SceneGraph::Model::TVecMT docking_mts;
	SceneGraph::Model::TVecMT leaving_mts;
	model->FindTagsByStartOfName("approach_", approach_mts);
	model->FindTagsByStartOfName("docking_", docking_mts);
	model->FindTagsByStartOfName("leaving_", leaving_mts);

	{
		SceneGraph::Model::TVecMT::const_iterator apprIter = approach_mts.begin();
		for (; apprIter!=approach_mts.end() ; ++apprIter)
		{
			int bay, stage;
			PiVerify(2 == sscanf((*apprIter)->GetName().c_str(), "approach_stage%d_bay%d", &stage, &bay));
			PiVerify(bay>0 && stage>0);
			SBayGroup* pGroup = GetGroupByBay(bay-1);
			assert(pGroup);
			pGroup->m_approach[stage] = (*apprIter)->GetTransform();
		}

		SceneGraph::Model::TVecMT::const_iterator dockIter = docking_mts.begin();
		for (; dockIter!=docking_mts.end() ; ++dockIter)
		{
			int bay, stage;
			PiVerify(2 == sscanf((*dockIter)->GetName().c_str(), "docking_stage%d_bay%d", &stage, &bay));
			PiVerify(bay>0 && stage>0);
			m_ports[bay].m_docking[stage+1] = (*dockIter)->GetTransform();
		}
		
		SceneGraph::Model::TVecMT::const_iterator leaveIter = leaving_mts.begin();
		for (; leaveIter!=leaving_mts.end() ; ++leaveIter)
		{
			int bay, stage;
			PiVerify(2 == sscanf((*leaveIter)->GetName().c_str(), "leaving_stage%d_bay%d", &stage, &bay));
			PiVerify(bay>0 && stage>0);
			m_ports[bay].m_leaving[stage] = (*leaveIter)->GetTransform();
		}
	}
}

const SpaceStationType::SBayGroup* SpaceStationType::FindGroupByBay(const int zeroBaseBayID) const
{
	TBayGroups::const_iterator bayIter = bayGroups.begin();
	for ( ; bayIter!=bayGroups.end() ; ++bayIter ) {
		std::vector<int>::const_iterator idIter = (*bayIter).bayIDs.begin();
		for ( ; idIter!=(*bayIter).bayIDs.end() ; ++idIter ) {
			if ((*idIter)==zeroBaseBayID) {
				return &(*bayIter);
			}
		}
	}
	// is it safer to return that the bay is locked?
	return NULL;
}

SpaceStationType::SBayGroup* SpaceStationType::GetGroupByBay(const int zeroBaseBayID)
{
	TBayGroups::iterator bayIter = bayGroups.begin();
	for ( ; bayIter!=bayGroups.end() ; ++bayIter ) {
		std::vector<int>::iterator idIter = (*bayIter).bayIDs.begin();
		for ( ; idIter!=(*bayIter).bayIDs.end() ; ++idIter ) {
			if ((*idIter)==zeroBaseBayID) {
				return &(*bayIter);
			}
		}
	}
	// is it safer to return that the bay is locked?
	return NULL;
}

#pragma optimize( "", off )
bool SpaceStationType::GetShipApproachWaypoints(const int port, const int stage, positionOrient_t &outPosOrient) const
{
	bool gotOrient = false;

	if (!bHasApproachWaypointsFunction)
	{
		const SBayGroup* pGroup = FindGroupByBay(port);
		if (pGroup && stage>0) {
			const bool bHasStageData = (pGroup->m_approach.find( stage ) != pGroup->m_approach.end());
			if (bHasStageData) {
				const matrix4x4f &mt = pGroup->m_approach.at(stage);
				outPosOrient.pos	= vector3d(mt.GetTranslate());
				outPosOrient.xaxis	= vector3d(mt.GetOrient().VectorX());
				outPosOrient.yaxis	= vector3d(mt.GetOrient().VectorY());
				outPosOrient.zaxis	= vector3d(mt.GetOrient().VectorZ());
				outPosOrient.xaxis	= outPosOrient.xaxis.Normalized();
				outPosOrient.yaxis	= outPosOrient.yaxis.Normalized();
				outPosOrient.zaxis	= outPosOrient.zaxis.Normalized();
				gotOrient = true;
			}
		}
		return gotOrient;
	}

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

		// calculate te zaxis from the x^y
		outPosOrient.zaxis = outPosOrient.xaxis.Cross(outPosOrient.yaxis).Normalized();
		lua_pop(L, 1);
	} else {
		gotOrient = false;
	}
	lua_pop(L, 2);

	LUA_DEBUG_END(L, 0);

	return gotOrient;
}

//for station waypoint interpolation
vector3d vlerp(const double t, const vector3d& v1, const vector3d& v2)
{
	return t*v2 + (1.0-t)*v1;
}

#pragma optimize( "", off )
static bool GetPosOrient(const SpaceStationType::TMapBayIDMat &bayMap, const int stage, const double t, const vector3d &from, 
				  SpaceStationType::positionOrient_t &outPosOrient, const Ship *ship)
{
	bool gotOrient = false;

	vector3d toPos;

	const bool bHasStageData = (bayMap.find( stage ) != bayMap.end());
	assert(bHasStageData);
	if (bHasStageData) {
		const matrix4x4f &mt = bayMap.at(stage);
		outPosOrient.xaxis	= vector3d(mt.GetOrient().VectorX()).Normalized();
		outPosOrient.yaxis	= vector3d(mt.GetOrient().VectorY()).Normalized();
		outPosOrient.zaxis	= vector3d(mt.GetOrient().VectorZ()).Normalized();
		toPos				= vector3d(mt.GetTranslate());
		gotOrient = true;
	}

	if (gotOrient)
	{
		vector3d pos		= vlerp(t, from, toPos);
		outPosOrient.pos	= pos;
	}

	return gotOrient;
}

/* when ship is on rails it returns true and fills outPosOrient.
 * when ship has been released (or docked) it returns false.
 * Note station animations may continue for any number of stages after
 * ship has been released and is under player control again */
#pragma optimize( "", off )
bool SpaceStationType::GetDockAnimPositionOrient(const int port, int stage, double t, const vector3d &from, positionOrient_t &outPosOrient, const Ship *ship) const
{
	if (stage < -shipLaunchStage) { stage = -shipLaunchStage; t = 1.0; }
	if (stage > numDockingStages || !stage) { stage = numDockingStages; t = 1.0; }
	// note case for stageless launch (shipLaunchStage==0)

	bool gotOrient = false;

	if (!bHasDockAnimFunction)
	{
		assert(port<=m_ports.size());
		const Port &rPort = m_ports.at(port+1);
		const Aabb &aabb = ship->GetAabb();
		if (stage<0) {
			const int leavingStage = (-1*stage);
			gotOrient = GetPosOrient(rPort.m_leaving, leavingStage, t, from, outPosOrient, ship);
			const vector3d up = outPosOrient.yaxis.Normalized() * aabb.min.y;
			outPosOrient.pos = outPosOrient.pos - up;
		} else if (stage>0) {
			gotOrient = GetPosOrient(rPort.m_docking, stage, t, from, outPosOrient, ship);
			const vector3d up = outPosOrient.yaxis.Normalized() * aabb.min.y;
			outPosOrient.pos = outPosOrient.pos - up;
		}

		return gotOrient;
	}

	lua_State *L = s_lua;

	LUA_DEBUG_START(L);

	lua_pushcfunction(L, pi_lua_panic);
	lua_getglobal(L, this->dockAnimFunction.c_str());
	// It's a function of form function(stage, t, from)
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

		outPosOrient.zaxis = outPosOrient.xaxis.Cross(outPosOrient.yaxis);
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

#pragma optimize( "", off )
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

// Data format example:
//	bay_groups = {
//		{0, 500, {1}},
//	},
#pragma optimize( "", off )
static int _get_bay_ids(lua_State *L, const char *key, SpaceStationType::TBayGroups &outBayGroups)
{
	LUA_DEBUG_START(L);
	lua_pushstring(L, key);
	lua_gettable(L, -2);
	if (lua_istable(L, -1)) {
		const int numBayGroups = lua_rawlen(L, -1);
		if (numBayGroups < 1) {
			return luaL_error(L, "Station must have at least 1 group of bays in %s", key);
		}

		outBayGroups.reserve(numBayGroups);

		for (int iGroup=1; iGroup <= numBayGroups; iGroup++) {
			// get the number of items meaning minSize, maxSize and the array of day ids
			lua_pushinteger(L, iGroup);
			lua_gettable(L, -2);
			if (lua_istable(L, -1)) {
				const int numItems = lua_rawlen(L, -1);
				if (numItems != 3) {
					return luaL_error(L, "??? wtf %s", key);
				}

				SpaceStationType::SBayGroup newBay;
				for (int iItem=1; iItem <= numItems; iItem++) {
					lua_pushinteger(L, iItem);
					lua_gettable(L, -2);
					switch(iItem) {
					case 1: 
						newBay.minShipSize = lua_tointeger(L, -1);
						break;
					case 2: 
						newBay.maxShipSize = lua_tointeger(L, -1);
						break;
					case 3: 
						if (lua_istable(L, -1)) {
							const int numBays = lua_rawlen(L, -1);
							if (numBays < 1) {
								return luaL_error(L, "Group must have at least 1 bay %s", key);
							}
							newBay.bayIDs.reserve(numBays);
							for (int i=1; i <= numBays; i++) {
								lua_pushinteger(L, i);
								lua_gettable(L, -2);
								const int bayID = lua_tointeger(L, -1);
								if (bayID < 1) {
									return luaL_error(L, "Valid bay ID ranges start from 1 %s", key);
								}
								newBay.bayIDs.push_back(bayID-1);
								lua_pop(L, 1);
							}
						} 
						break;
					}
					lua_pop(L, 1);
				}
				outBayGroups.push_back(newBay);

			}
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
	_get_bay_ids(L, "bay_groups", station.bayGroups);
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
	//assert(!station.dockAnimFunction.empty());
	//assert(!station.approachWaypointsFunction.empty());
	station.bHasDockAnimFunction = (!station.dockAnimFunction.empty());
	station.bHasApproachWaypointsFunction = (!station.approachWaypointsFunction.empty());

	station.model = Pi::FindModel(station.modelName);
	station.OnSetupComplete();
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
