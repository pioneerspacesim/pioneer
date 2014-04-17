// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "SpaceStationType.h"
#include "FileSystem.h"
#include "Lua.h"
#include "LuaVector.h"
#include "LuaVector.h"
#include "LuaTable.h"
#include "Pi.h"
#include "MathUtil.h"
#include "Ship.h"
#include "StringF.h"
#include "scenegraph/Model.h"
#include "OS.h"

#include <algorithm>

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
{}

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

		assert(!m_ports.empty());
		assert(numDockingStages > 0);
		assert(numUndockStages > 0);

		for (PortMap::const_iterator pIt = m_ports.begin(), pItEnd = m_ports.end(); pIt!=pItEnd; ++pIt)
		{
			if (Uint32(numDockingStages-1) < pIt->second.m_docking.size()) {
				Error(
					"(%s): numDockingStages (%d) vs number of docking stages (" SIZET_FMT ")\n"
					"Must have at least the same number of entries as the number of docking stages "
					"PLUS the docking timeout at the start of the array.",
					modelName.c_str(), (numDockingStages-1), pIt->second.m_docking.size());

			} else if (Uint32(numDockingStages-1) != pIt->second.m_docking.size()) {
				Warning(
					"(%s): numDockingStages (%d) vs number of docking stages (" SIZET_FMT ")\n",
					modelName.c_str(), (numDockingStages-1), pIt->second.m_docking.size());
			}

			if (0!=pIt->second.m_leaving.size() && Uint32(numUndockStages) < pIt->second.m_leaving.size()) {
				Error(
					"(%s): numUndockStages (%d) vs number of leaving stages (" SIZET_FMT ")\n"
					"Must have at least the same number of entries as the number of leaving stages.",
					modelName.c_str(), (numDockingStages-1), pIt->second.m_docking.size());

			} else if(0!=pIt->second.m_leaving.size() && Uint32(numUndockStages) != pIt->second.m_leaving.size()) {
				Warning(
					"(%s): numUndockStages (%d) vs number of leaving stages (" SIZET_FMT ")\n",
					modelName.c_str(), numUndockStages, pIt->second.m_leaving.size());
			}

		}
	}
}

const SpaceStationType::SBayGroup* SpaceStationType::FindGroupByBay(const int zeroBaseBayID) const
{
	for (TBayGroups::const_iterator bayIter = bayGroups.begin(), grpEnd=bayGroups.end(); bayIter!=grpEnd ; ++bayIter ) {
		for (std::vector<int>::const_iterator idIter = (*bayIter).bayIDs.begin(), idIEnd = (*bayIter).bayIDs.end(); idIter!=idIEnd ; ++idIter ) {
			if ((*idIter)==zeroBaseBayID) {
				return &(*bayIter);
			}
		}
	}
	// is it safer to return that the bay is locked?
	return 0;
}

SpaceStationType::SBayGroup* SpaceStationType::GetGroupByBay(const int zeroBaseBayID)
{
	for (TBayGroups::iterator bayIter = bayGroups.begin(), grpEnd=bayGroups.end(); bayIter!=grpEnd ; ++bayIter ) {
		for (std::vector<int>::const_iterator idIter = (*bayIter).bayIDs.begin(), idIEnd = (*bayIter).bayIDs.end(); idIter!=idIEnd ; ++idIter ) {
			if ((*idIter)==zeroBaseBayID) {
				return &(*bayIter);
			}
		}
	}
	// is it safer to return that the bay is locked?
	return 0;
}

bool SpaceStationType::GetShipApproachWaypoints(const unsigned int port, const int stage, positionOrient_t &outPosOrient) const
{
	bool gotOrient = false;

	const SBayGroup* pGroup = FindGroupByBay(port);
	if (pGroup && stage>0) {
		TMapBayIDMat::const_iterator stageDataIt = pGroup->m_approach.find(stage);
		if (stageDataIt != pGroup->m_approach.end()) {
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

double SpaceStationType::GetDockAnimStageDuration(const int stage) const
{
	assert(stage>=0 && stage<numDockingStages);
	return dockAnimStageDuration[stage];
}

double SpaceStationType::GetUndockAnimStageDuration(const int stage) const
{
	assert(stage>=0 && stage<numUndockStages);
	return undockAnimStageDuration[stage];
}

static bool GetPosOrient(const SpaceStationType::TMapBayIDMat &bayMap, const int stage, const double t, const vector3d &from,
				  SpaceStationType::positionOrient_t &outPosOrient)
{
	bool gotOrient = false;

	vector3d toPos;

	const SpaceStationType::TMapBayIDMat::const_iterator stageDataIt = bayMap.find( stage );
	const bool bHasStageData = (stageDataIt != bayMap.end());
	assert(bHasStageData);
	if (bHasStageData) {
		const matrix4x4f &mt = stageDataIt->second;
		outPosOrient.xaxis	= vector3d(mt.GetOrient().VectorX()).Normalized();
		outPosOrient.yaxis	= vector3d(mt.GetOrient().VectorY()).Normalized();
		outPosOrient.zaxis	= vector3d(mt.GetOrient().VectorZ()).Normalized();
		toPos				= vector3d(mt.GetTranslate());
		gotOrient = true;
	}

	if (gotOrient)
	{
		vector3d pos		= MathUtil::mix<vector3d, double>(from, toPos, t);
		outPosOrient.pos	= pos;
	}

	return gotOrient;
}

/* when ship is on rails it returns true and fills outPosOrient.
 * when ship has been released (or docked) it returns false.
 * Note station animations may continue for any number of stages after
 * ship has been released and is under player control again */
bool SpaceStationType::GetDockAnimPositionOrient(const unsigned int port, int stage, double t, const vector3d &from, positionOrient_t &outPosOrient, const Ship *ship) const
{
	assert(ship);
	if (stage < -shipLaunchStage) { stage = -shipLaunchStage; t = 1.0; }
	if (stage > numDockingStages || !stage) { stage = numDockingStages; t = 1.0; }
	// note case for stageless launch (shipLaunchStage==0)

	bool gotOrient = false;

	assert(port<=m_ports.size());
	const Port &rPort = m_ports.at(port+1);
	if (stage<0) {
		const int leavingStage = (-1*stage);
		gotOrient = GetPosOrient(rPort.m_leaving, leavingStage, t, from, outPosOrient);
		const vector3d up = outPosOrient.yaxis.Normalized() * ship->GetLandingPosOffset();
		outPosOrient.pos = outPosOrient.pos - up;
	} else if (stage>0) {
		gotOrient = GetPosOrient(rPort.m_docking, stage, t, from, outPosOrient);
		const vector3d up = outPosOrient.yaxis.Normalized() * ship->GetLandingPosOffset();
		outPosOrient.pos = outPosOrient.pos - up;
	}

	return gotOrient;
}

static int _get_stage_durations(lua_State *L, const char *key, int &outNumStages, double **outDurationArray)
{
	LUA_DEBUG_START(L);
	LuaTable stages = LuaTable(L, -1).Sub(key);
	if (stages.GetLua() == 0) {
		luaL_error(L, "Not a proper table (%s)", key);
	}
	if (stages.Size() < 1)
		return luaL_error(L, "Station must have at least 1 stage in %s", key);
	outNumStages = stages.Size();
	*outDurationArray = new double[stages.Size()];
	std::copy(stages.Begin<double>(), stages.End<double>(), *outDurationArray);
	lua_pop(L, 1); // Popping t
	LUA_DEBUG_END(L, 0);
	return 0;
}

// Data format example:
//	bay_groups = {
//		{0, 500, {1}},
//	},
static int _get_bay_ids(lua_State *L, const char *key, SpaceStationType::TBayGroups &outBayGroups, unsigned int &outNumDockingPorts)
{
	LUA_DEBUG_START(L);
	LuaTable t = LuaTable(L, -1).Sub(key);
	if (t.GetLua() == 0) {
		luaL_error(L, "The bay group isn't a proper table (%s)", key);
	}
	if (t.Size() < 1) {
		return luaL_error(L, "Station must have at least 1 group of bays in %s", key);
	}

	LuaTable::VecIter<LuaTable> it_end = t.End<LuaTable>();
	for (LuaTable::VecIter<LuaTable> it = t.Begin<LuaTable>(); it != it_end; ++it) {
		SpaceStationType::SBayGroup newBay;
		newBay.minShipSize = it->Get<int>(1);
		newBay.maxShipSize = it->Get<int>(2);
		LuaTable group = it->Sub(3);

		if (group.GetLua() == 0) {
			luaL_error(L, "A group is of the form {int, int, table} (%s)", key);
		}

		if (group.Size() == 0) {
			return luaL_error(L, "Group must have at least 1 bay %s", key);
		}

		newBay.bayIDs.reserve(group.Size());
		LuaTable::VecIter<int> jt_end = group.End<int>();
		for (LuaTable::VecIter<int> jt = group.Begin<int>(); jt != jt_end; ++jt) {
			if ((*jt) < 1) {
				return luaL_error(L, "Valid bay ID ranges start from 1 %s", key);
			}
			newBay.bayIDs.push_back((*jt)-1);
			++outNumDockingPorts;
		}
		lua_pop(L, 1); // Popping group
		outBayGroups.push_back(newBay);
	}
	lua_pop(L, 1); // Popping t
	LUA_DEBUG_END(L, 0);
	return 0;
}

static int _define_station(lua_State *L, SpaceStationType &station)
{
	station.id = s_currentStationFile;

	LUA_DEBUG_START(L);
	LuaTable t(L, -1);
	station.modelName = t.Get<std::string>("model");
	station.angVel = t.Get("angular_velocity", 0.f);
	station.parkingDistance = t.Get("parking_distance", 5000.f);
	station.parkingGapSize = t.Get("parking_gap_size", 2000.f);
	station.shipLaunchStage = t.Get<int>("ship_launch_stage");
	_get_bay_ids(L, "bay_groups", station.bayGroups, station.numDockingPorts);
	_get_stage_durations(L, "dock_anim_stage_duration", station.numDockingStages, &station.dockAnimStageDuration);
	_get_stage_durations(L, "undock_anim_stage_duration", station.numUndockStages, &station.undockAnimStageDuration);
	LUA_DEBUG_END(L, 0);

	assert(!station.modelName.empty());

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
		if (ends_with_ci(info.GetPath(), ".lua")) {
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
