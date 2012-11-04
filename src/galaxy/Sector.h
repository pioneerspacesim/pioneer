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

	// sets appropriate base factionColours for all systems in the sector
	void ColourFactions();

	class System {
	public:
		System() : customSys(0), population(-1) {};
		~System() {};

		// Check that we've had our habitation status set

		// public members
		std::string name;
		vector3f p;
		int numStars;
		SystemBody::BodyType starType[4];
		Uint32 seed;
		const CustomSystem *customSys;
		Color factionColour;
		fixed population;

	private:

	};
	std::vector<System> m_systems;
private:
	void GetCustomSystems();
	std::string GenName(System &sys, MTRand &rand);
	int sx, sy, sz;
};

#endif /* _SECTOR_H */
