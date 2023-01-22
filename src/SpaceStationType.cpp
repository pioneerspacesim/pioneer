// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "SpaceStationType.h"
#include "FileSystem.h"
#include "Json.h"
#include "MathUtil.h"
#include "Pi.h"
#include "Ship.h"
#include "StringF.h"
#include "scenegraph/MatrixTransform.h"
#include "scenegraph/Model.h"

#include <algorithm>

// TODO: Fix the horrible control flow that makes this exception type necessary.
struct StationTypeLoadError {};

std::vector<SpaceStationType> SpaceStationType::surfaceTypes;
std::vector<SpaceStationType> SpaceStationType::orbitalTypes;

SpaceStationType::SpaceStationType(const std::string &id_, const std::string &path_) :
	id(id_),
	model(0),
	modelName(""),
	angVel(0.f),
	dockMethod(SURFACE),
	numDockingPorts(0),
	numDockingStages(0),
	numUndockStages(0),
	shipLaunchStage(3),
	parkingDistance(0),
	parkingGapSize(0)
{
	Json data = JsonUtils::LoadJsonDataFile(path_);
	if (data.is_null()) {
		Output("couldn't read station def '%s'\n", path_.c_str());
		throw StationTypeLoadError();
	}

	modelName = data.value("model", "");

	const std::string type = data.value("type", "");
	if (type == "surface")
		dockMethod = SURFACE;
	else if (type == "orbital")
		dockMethod = ORBITAL;
	else {
		Output("couldn't parse station def '%s': unknown type '%s'\n", path_.c_str(), type.c_str());
		throw StationTypeLoadError();
	}

	angVel = data.value("angular_velocity", 0.0f);

	parkingDistance = data.value("parking_distance", 0.0f);
	parkingGapSize = data.value("parking_gap_size", 0.0f);

	padOffset = data.value("pad_offset", 150.f);

	model = Pi::FindModel(modelName, /* allowPlaceholder = */ false);
	if (!model) {
		Output("couldn't initialize station type '%s' because the corresponding model ('%s') could not be found.\n", path_.c_str(), modelName.c_str());
		throw StationTypeLoadError();
	}
	OnSetupComplete();
}

void SpaceStationType::OnSetupComplete()
{
	// Since the model contains (almost) all of the docking information we have to extract that
	// and then generate any additional locators and information the station will need from it.

	// First we gather the MatrixTransforms that contain the location and orientation of the docking
	// locators/waypoints. We store some information within the name of these which needs parsing too.

	// Next we build the additional information required for docking ships with SPACE stations
	// on autopilot - this is the only option for docking with SPACE stations currently.
	// This mostly means offsetting from one locator to create the next in the sequence.

	// ground stations have a "special-fucking-case" 0 stage launch process
	shipLaunchStage = ((SURFACE == dockMethod) ? 0 : 3);

	// gather the tags
	SceneGraph::Model::TVecMT entrance_mts;
	SceneGraph::Model::TVecMT locator_mts;
	SceneGraph::Model::TVecMT exit_mts;
	model->FindTagsByStartOfName("entrance_", entrance_mts);
	model->FindTagsByStartOfName("loc_", locator_mts);
	model->FindTagsByStartOfName("exit_", exit_mts);

	Output("%s has:\n %lu entrances,\n %lu pads,\n %lu exits\n", modelName.c_str(), entrance_mts.size(), locator_mts.size(), exit_mts.size());

	// Add the partially initialised ports
	for (auto apprIter : entrance_mts) {
		int portId;
		PiVerify(1 == sscanf(apprIter->GetName().c_str(), "entrance_port%d", &portId));
		PiVerify(portId > 0);

		SPort new_port;
		new_port.portId = portId;
		new_port.name = apprIter->GetName();
		if (SURFACE == dockMethod) {
			const vector3f offDir = apprIter->GetTransform().Up().Normalized();
			new_port.m_approach[1] = apprIter->GetTransform();
			new_port.m_approach[1].SetTranslate(apprIter->GetTransform().GetTranslate() + (offDir * 500.0f));
		} else {
			const vector3f offDir = -apprIter->GetTransform().Back().Normalized();
			new_port.m_approach[1] = apprIter->GetTransform();
			new_port.m_approach[1].SetTranslate(apprIter->GetTransform().GetTranslate() + (offDir * 1500.0f));
		}
		new_port.m_approach[2] = apprIter->GetTransform();
		m_ports.push_back(new_port);
	}

	for (auto locIter : locator_mts) {
		int bay, portId;
		int minSize, maxSize;
		char padname[8];
		const matrix4x4f &locTransform = locIter->GetTransform();

		// eg:loc_A001_p01_s0_500_b01
		PiVerify(5 == sscanf(locIter->GetName().c_str(), "loc_%4s_p%d_s%d_%d_b%d", &padname[0], &portId, &minSize, &maxSize, &bay));
		PiVerify(bay > 0 && portId > 0);

		// find the port and setup the rest of it's information
#ifndef NDEBUG
		bool bFoundPort = false;
#endif
		matrix4x4f approach1(0.0);
		matrix4x4f approach2(0.0);
		for (auto &rPort : m_ports) {
			if (rPort.portId == portId) {
				rPort.minShipSize = std::min(minSize, rPort.minShipSize);
				rPort.maxShipSize = std::max(maxSize, rPort.maxShipSize);
				rPort.bayIDs.push_back(std::make_pair(bay - 1, padname));
#ifndef NDEBUG
				bFoundPort = true;
#endif
				approach1 = rPort.m_approach[1];
				approach2 = rPort.m_approach[2];
				break;
			}
		}
		assert(bFoundPort);

		// now build the docking/leaving waypoints
		if (SURFACE == dockMethod) {
			// ground stations don't have leaving waypoints.
			m_portPaths[bay].m_docking[2] = locTransform; // final (docked)
			numDockingStages = 2;
			numUndockStages = 1;
		} else {
			struct TPointLine {
				// for reference: http://paulbourke.net/geometry/pointlineplane/
				static bool ClosestPointOnLine(const vector3f &Point, const vector3f &LineStart, const vector3f &LineEnd, vector3f &Intersection)
				{
					const float LineMag = (LineStart - LineEnd).Length();

					const float U = (((Point.x - LineStart.x) * (LineEnd.x - LineStart.x)) +
										((Point.y - LineStart.y) * (LineEnd.y - LineStart.y)) +
										((Point.z - LineStart.z) * (LineEnd.z - LineStart.z))) /
						(LineMag * LineMag);

					if (U < 0.0f || U > 1.0f)
						return false; // closest point does not fall within the line segment

					Intersection.x = LineStart.x + U * (LineEnd.x - LineStart.x);
					Intersection.y = LineStart.y + U * (LineEnd.y - LineStart.y);
					Intersection.z = LineStart.z + U * (LineEnd.z - LineStart.z);

					return true;
				}
			};

			// create the docking locators
			// start
			m_portPaths[bay].m_docking[2] = approach2;
			m_portPaths[bay].m_docking[2].SetRotationOnly(locTransform.GetOrient());
			// above the pad
			vector3f intersectionPos(0.0f);
			const vector3f approach1Pos = approach1.GetTranslate();
			const vector3f approach2Pos = approach2.GetTranslate();
			{
				const vector3f p0 = locTransform.GetTranslate();			   // plane position
				const vector3f l = (approach2Pos - approach1Pos).Normalized(); // ray direction
				const vector3f l0 = approach1Pos + (l * 10000.0f);

				if (!TPointLine::ClosestPointOnLine(p0, approach1Pos, l0, intersectionPos)) {
					Output("No point found on line segment");
				}
			}
			m_portPaths[bay].m_docking[3] = locTransform;
			m_portPaths[bay].m_docking[3].SetTranslate(intersectionPos);
			// final (docked)
			m_portPaths[bay].m_docking[4] = locTransform;
			numDockingStages = 4;

			// leaving locators ...
			matrix4x4f orient = locTransform.GetOrient(), EndOrient;
			if (exit_mts.empty()) {
				// leaving locators need to face in the opposite direction
				const matrix4x4f rot = matrix3x3f::Rotate(DEG2RAD(180.0f), orient.Back());
				orient = orient * rot;
				orient.SetTranslate(locTransform.GetTranslate());
				EndOrient = approach2;
				EndOrient.SetRotationOnly(orient);
			} else {
				// leaving locators, use whatever orientation they have
				orient.SetTranslate(locTransform.GetTranslate());
				int exitport = 0;
				for (auto &exitIt : exit_mts) {
					PiVerify(1 == sscanf(exitIt->GetName().c_str(), "exit_port%d", &exitport));
					if (exitport == portId) {
						EndOrient = exitIt->GetTransform();
						break;
					}
				}
				if (exitport == 0) {
					EndOrient = approach2;
				}
			}

			// create the leaving locators
			m_portPaths[bay].m_leaving[1] = locTransform;				 // start - maintain the same orientation and position as when docked.
			m_portPaths[bay].m_leaving[2] = orient;						 // above the pad - reorient...
			m_portPaths[bay].m_leaving[2].SetTranslate(intersectionPos); //  ...and translate to new position
			m_portPaths[bay].m_leaving[3] = EndOrient;					 // end (on manual after here)
			numUndockStages = 3;
		}
	}

	numDockingPorts = m_portPaths.size();

	// sanity
	assert(!m_portPaths.empty());
	assert(numDockingStages > 0);
	assert(numUndockStages > 0);

	// insanity
	for (PortPathMap::const_iterator pIt = m_portPaths.begin(), pItEnd = m_portPaths.end(); pIt != pItEnd; ++pIt) {
		if (Uint32(numDockingStages - 1) < pIt->second.m_docking.size()) {
			Error(
				"(%s): numDockingStages (%d) vs number of docking stages (" SIZET_FMT ")\n"
				"Must have at least the same number of entries as the number of docking stages "
				"PLUS the docking timeout at the start of the array.",
				modelName.c_str(), (numDockingStages - 1), pIt->second.m_docking.size());

		} else if (Uint32(numDockingStages - 1) != pIt->second.m_docking.size()) {
			Warning(
				"(%s): numDockingStages (%d) vs number of docking stages (" SIZET_FMT ")\n",
				modelName.c_str(), (numDockingStages - 1), pIt->second.m_docking.size());
		}

		if (0 != pIt->second.m_leaving.size() && Uint32(numUndockStages) < pIt->second.m_leaving.size()) {
			Error(
				"(%s): numUndockStages (%d) vs number of leaving stages (" SIZET_FMT ")\n"
				"Must have at least the same number of entries as the number of leaving stages.",
				modelName.c_str(), (numDockingStages - 1), pIt->second.m_docking.size());

		} else if (0 != pIt->second.m_leaving.size() && Uint32(numUndockStages) != pIt->second.m_leaving.size()) {
			Warning(
				"(%s): numUndockStages (%d) vs number of leaving stages (" SIZET_FMT ")\n",
				modelName.c_str(), numUndockStages, pIt->second.m_leaving.size());
		}
	}
}

const SpaceStationType::SPort *SpaceStationType::FindPortByBay(const int zeroBaseBayID) const
{
	for (TPorts::const_iterator bayIter = m_ports.begin(), grpEnd = m_ports.end(); bayIter != grpEnd; ++bayIter) {
		for (auto idIter : (*bayIter).bayIDs) {
			if (idIter.first == zeroBaseBayID) {
				return &(*bayIter);
			}
		}
	}
	// is it safer to return that the bay is locked?
	return 0;
}

SpaceStationType::SPort *SpaceStationType::GetPortByBay(const int zeroBaseBayID)
{
	for (TPorts::iterator bayIter = m_ports.begin(), grpEnd = m_ports.end(); bayIter != grpEnd; ++bayIter) {
		for (auto idIter : (*bayIter).bayIDs) {
			if (idIter.first == zeroBaseBayID) {
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

	const SPort *pPort = FindPortByBay(port);
	if (pPort && stage > 0) {
		TMapBayIDMat::const_iterator stageDataIt = pPort->m_approach.find(stage);
		if (stageDataIt != pPort->m_approach.end()) {
			const matrix4x4f &mt = pPort->m_approach.at(stage);
			outPosOrient.pos = vector3d(mt.GetTranslate());
			outPosOrient.xaxis = vector3d(mt.GetOrient().VectorX());
			outPosOrient.yaxis = vector3d(mt.GetOrient().VectorY());
			outPosOrient.zaxis = vector3d(mt.GetOrient().VectorZ());
			outPosOrient.xaxis = outPosOrient.xaxis.Normalized();
			outPosOrient.yaxis = outPosOrient.yaxis.Normalized();
			outPosOrient.zaxis = outPosOrient.zaxis.Normalized();
			gotOrient = true;
		}
	}
	return gotOrient;
}

double SpaceStationType::GetDockAnimStageDuration(const int stage) const
{
	return (stage == 0) ? 300.0 : ((SURFACE == dockMethod) ? 0.0 : 3.0);
}

double SpaceStationType::GetUndockAnimStageDuration(const int stage) const
{
	return ((SURFACE == dockMethod) ? 0.0 : 5.0);
}

static bool GetPosOrient(const SpaceStationType::TMapBayIDMat &bayMap, const int stage, const double t, const vector3d &from,
	SpaceStationType::positionOrient_t &outPosOrient)
{
	bool gotOrient = false;

	vector3d toPos;

	const SpaceStationType::TMapBayIDMat::const_iterator stageDataIt = bayMap.find(stage);
	const bool bHasStageData = (stageDataIt != bayMap.end());
	assert(bHasStageData);
	if (bHasStageData) {
		const matrix4x4f &mt = stageDataIt->second;
		outPosOrient.xaxis = vector3d(mt.GetOrient().VectorX()).Normalized();
		outPosOrient.yaxis = vector3d(mt.GetOrient().VectorY()).Normalized();
		outPosOrient.zaxis = vector3d(mt.GetOrient().VectorZ()).Normalized();
		toPos = vector3d(mt.GetTranslate());
		gotOrient = true;
	}

	if (gotOrient) {
		vector3d pos = MathUtil::mix<vector3d, double>(from, toPos, t);
		outPosOrient.pos = pos;
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
	if (stage < -shipLaunchStage) {
		stage = -shipLaunchStage;
		t = 1.0;
	}
	if (stage > numDockingStages || !stage) {
		stage = numDockingStages;
		t = 1.0;
	}
	// note case for stageless launch (shipLaunchStage==0)

	bool gotOrient = false;

	assert(port <= m_portPaths.size());
	const PortPath &rPortPath = m_portPaths.at(port + 1);
	if (stage < 0) {
		const int leavingStage = (-1 * stage);
		gotOrient = GetPosOrient(rPortPath.m_leaving, leavingStage, t, from, outPosOrient);
		const vector3d up = outPosOrient.yaxis.Normalized() * ship->GetLandingPosOffset();
		outPosOrient.pos = outPosOrient.pos - up;
	} else if (stage > 0) {
		gotOrient = GetPosOrient(rPortPath.m_docking, stage, t, from, outPosOrient);
		const vector3d up = outPosOrient.yaxis.Normalized() * ship->GetLandingPosOffset();
		outPosOrient.pos = outPosOrient.pos - up;
	}

	return gotOrient;
}

/*static*/
void SpaceStationType::Init()
{
	PROFILE_SCOPED()
	static bool isInitted = false;
	if (isInitted)
		return;
	isInitted = true;

	// load all station definitions
	namespace fs = FileSystem;
	for (fs::FileEnumerator files(fs::gameDataFiles, "stations", 0); !files.Finished(); files.Next()) {
		const fs::FileInfo &info = files.Current();
		if (ends_with_ci(info.GetPath(), ".json")) {
			const std::string id(info.GetName().substr(0, info.GetName().size() - 5));
			try {
				SpaceStationType st = SpaceStationType(id, info.GetPath());
				switch (st.dockMethod) {
				case SURFACE: surfaceTypes.push_back(st); break;
				case ORBITAL: orbitalTypes.push_back(st); break;
				}
			} catch (StationTypeLoadError) {
				// TODO: Actual error handling would be nice.
				Error("Error while loading Space Station data (check stdout/output.txt).\n");
			}
		}
	}
}

/*static*/
const SpaceStationType *SpaceStationType::RandomStationType(Random &random, const bool bIsGround)
{
	if (bIsGround) {
		return &surfaceTypes[random.Int32(SpaceStationType::surfaceTypes.size())];
	}

	return &orbitalTypes[random.Int32(SpaceStationType::orbitalTypes.size())];
}

/*static*/
const SpaceStationType *SpaceStationType::FindByName(const std::string &name)
{
	for (auto &sst : surfaceTypes)
		if (sst.id == name)
			return &sst;
	for (auto &sst : orbitalTypes)
		if (sst.id == name)
			return &sst;
	return nullptr;
}
