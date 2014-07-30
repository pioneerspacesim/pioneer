// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SPACESTATIONTYPE_H
#define _SPACESTATIONTYPE_H

#include "libs.h"

//Space station definition, loaded from data/stations

class Ship;
namespace SceneGraph { class Model; }

struct SpaceStationType {
	typedef std::map<Uint32, matrix4x4f> TMapBayIDMat;
	struct Port
	{
		TMapBayIDMat m_docking;
		TMapBayIDMat m_leaving;
	};
	typedef std::map<Uint32, Port> PortMap;
	PortMap m_ports;

	struct SBayGroup {
		SBayGroup() : minShipSize(-1), maxShipSize(-1), inUse(false) {}
		int minShipSize, maxShipSize;
		bool inUse;
		std::vector<int> bayIDs;
		TMapBayIDMat m_approach;
	};
	typedef std::vector<SBayGroup> TBayGroups;

	std::string id;
	SceneGraph::Model *model;
	std::string modelName;
	float angVel;
	enum DOCKMETHOD { SURFACE, ORBITAL } dockMethod;
	unsigned int numDockingPorts;
	int numDockingStages;
	int numUndockStages;
	int shipLaunchStage;
	double *dockAnimStageDuration;
	double *undockAnimStageDuration;
	float parkingDistance;
	float parkingGapSize;
	TBayGroups bayGroups;

	struct positionOrient_t {
		vector3d pos;
		vector3d xaxis;
		vector3d yaxis;
		vector3d zaxis;
	};

	SpaceStationType();

	void OnSetupComplete();
	const SBayGroup* FindGroupByBay(const int zeroBaseBayID) const;
	SBayGroup* GetGroupByBay(const int zeroBaseBayID);

	double GetDockAnimStageDuration(const int stage) const;
	double GetUndockAnimStageDuration(const int stage) const;

	// Call functions in the station .lua
	bool GetShipApproachWaypoints(const unsigned int port, const int stage, positionOrient_t &outPosOrient) const;
	/** when ship is on rails it returns true and fills outPosOrient.
	 * when ship has been released (or docked) it returns false.
	 * Note station animations may continue for any number of stages after
	 * ship has been released and is under player control again */
	bool GetDockAnimPositionOrient(const unsigned int port, int stage, double t, const vector3d &from, positionOrient_t &outPosOrient, const Ship *ship) const;

	static void Init();
	static void Uninit();
	static std::vector<SpaceStationType> surfaceStationTypes;
	static std::vector<SpaceStationType> orbitalStationTypes;
};

#endif
