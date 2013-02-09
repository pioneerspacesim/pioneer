// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SPACESTATIONTYPE_H
#define _SPACESTATIONTYPE_H

#include "libs.h"

//Space station definition, loaded from data/stations

class Ship;
namespace SceneGraph { class Model; }

struct SpaceStationType {
	struct SBayGroup {
		SBayGroup() : minShipSize(-1), maxShipSize(-1), inUse(false) {}
		int minShipSize, maxShipSize;
		bool inUse;
		std::vector<int> bayIDs;
	};
	typedef std::vector<SBayGroup> TBayGroups;

	std::string id;
	SceneGraph::Model *model;
	std::string modelName;
	float angVel;
	enum DOCKMETHOD { SURFACE, ORBITAL } dockMethod;
	int numDockingPorts;
	int numDockingStages;
	int numUndockStages;
	int shipLaunchStage;
	double *dockAnimStageDuration;
	double *undockAnimStageDuration;
	float parkingDistance;
	float parkingGapSize;
	std::string dockAnimFunction;
	std::string approachWaypointsFunction;
	bool bHasDockAnimFunction;
	bool bHasApproachWaypointsFunction;
	TBayGroups bayGroups;

	struct positionOrient_t {
		vector3d pos;
		vector3d xaxis;
		vector3d yaxis;
		vector3d zaxis;
	};

	SpaceStationType();
	// Call functions in the station .lua
	bool GetShipApproachWaypoints(int port, int stage, positionOrient_t &outPosOrient) const;
	/** when ship is on rails it returns true and fills outPosOrient.
	 * when ship has been released (or docked) it returns false.
	 * Note station animations may continue for any number of stages after
	 * ship has been released and is under player control again */
	bool GetDockAnimPositionOrient(int port, int stage, double t, const vector3d &from, positionOrient_t &outPosOrient, const Ship *ship) const;

	static void Init();
	static void Uninit();
	static std::vector<SpaceStationType> surfaceStationTypes;
	static std::vector<SpaceStationType> orbitalStationTypes;
};

#endif
