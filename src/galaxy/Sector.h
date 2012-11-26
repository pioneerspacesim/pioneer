// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SECTOR_H
#define _SECTOR_H

#include "libs.h"
#include "galaxy/SystemPath.h"
#include "galaxy/StarSystem.h"
#include "galaxy/CustomSystem.h"
#include <string>
#include <vector>

class Faction;

class Sector {
public:
	// lightyears
	static const float SIZE;
	Sector(int x, int y, int z);
	static float DistanceBetween(const Sector *a, int sysIdxA, const Sector *b, int sysIdxB);
	static void Init();

	// Sector is within a bounding rectangle - used for SectorView m_sectorCache pruning.
	bool WithinBox(const int Xmin, const int Xmax, const int Ymin, const int Ymax, const int Zmin, const int Zmax) const;
	bool Contains(const SystemPath sysPath) const;

	// sets appropriate factions for all systems in the sector
	void AssignFactions();

	class System {
	public:
		System(int x, int y, int z): customSys(0), population(-1), sx(x), sy(y), sz(z) {};
		~System() {};

		// Check that we've had our habitation status set

		// public members
		std::string name;
		vector3f p;
		int numStars;
		SystemBody::BodyType starType[4];
		Uint32 seed;
		const CustomSystem *customSys;
		Faction *faction;
		fixed population;

		vector3f FullPosition() { return Sector::SIZE*vector3f(float(sx), float(sy), float(sz)) + p; };

		int sx, sy, sz;
	};
	std::vector<System> m_systems;

private:
	int sx, sy, sz;
	void GetCustomSystems();
	std::string GenName(System &sys, int si, MTRand &rand);
};

#endif /* _SECTOR_H */
