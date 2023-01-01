// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SPACESTATIONTYPE_H
#define _SPACESTATIONTYPE_H

#include "libs.h"

//Space station definition, loaded from data/stations

class Ship;
namespace SceneGraph {
	class Model;
}

class SpaceStationType {
public:
	typedef std::map<Uint32, matrix4x4f> TMapBayIDMat;
	struct PortPath {
		TMapBayIDMat m_docking;
		TMapBayIDMat m_leaving;
	};
	typedef std::map<Uint32, PortPath> PortPathMap;

	struct SPort {
		static const int BAD_PORT_ID = -1;
		SPort() :
			portId(BAD_PORT_ID),
			minShipSize(5000),
			maxShipSize(-1),
			inUse(false) {}
		int portId;
		int minShipSize, maxShipSize;
		bool inUse;
		std::vector<std::pair<int, std::string>> bayIDs;
		std::string name;
		TMapBayIDMat m_approach;
	};
	typedef std::vector<SPort> TPorts;

	struct positionOrient_t {
		vector3d pos;
		vector3d xaxis;
		vector3d yaxis;
		vector3d zaxis;
	};

private:
	std::string id;
	SceneGraph::Model *model;
	std::string modelName;
	float angVel;
	enum DOCKMETHOD { SURFACE,
		ORBITAL } dockMethod;
	unsigned int numDockingPorts;
	int numDockingStages;
	int numUndockStages;
	int shipLaunchStage;
	float parkingDistance;
	float parkingGapSize;
	PortPathMap m_portPaths;
	TPorts m_ports;
	float padOffset;

	static std::vector<SpaceStationType> surfaceTypes;
	static std::vector<SpaceStationType> orbitalTypes;

public:
	SpaceStationType(const std::string &id, const std::string &path);

	void OnSetupComplete();
	const SPort *FindPortByBay(const int zeroBaseBayID) const;
	SPort *GetPortByBay(const int zeroBaseBayID);

	double GetDockAnimStageDuration(const int stage) const;
	double GetUndockAnimStageDuration(const int stage) const;

	// Call functions in the station .lua
	bool GetShipApproachWaypoints(const unsigned int port, const int stage, positionOrient_t &outPosOrient) const;
	/** when ship is on rails it returns true and fills outPosOrient.
	 * when ship has been released (or docked) it returns false.
	 * Note station animations may continue for any number of stages after
	 * ship has been released and is under player control again */
	bool GetDockAnimPositionOrient(const unsigned int port, int stage, double t, const vector3d &from, positionOrient_t &outPosOrient, const Ship *ship) const;

	const std::string &ModelName() const { return modelName; }
	float AngVel() const { return angVel; }
	bool IsSurfaceStation() const { return (SURFACE == dockMethod); }
	bool IsOrbitalStation() const { return (ORBITAL == dockMethod); }
	unsigned int NumDockingPorts() const { return numDockingPorts; }
	int NumDockingStages() const { return numDockingStages; }
	int NumUndockStages() const { return numUndockStages; }
	int ShipLaunchStage() const { return shipLaunchStage; }
	float ParkingDistance() const { return parkingDistance; }
	float ParkingGapSize() const { return parkingGapSize; }
	const TPorts &Ports() const { return m_ports; }

	static void Init();

	static const SpaceStationType *RandomStationType(Random &random, const bool bIsGround);
	static const SpaceStationType *FindByName(const std::string &name);
};

#endif
