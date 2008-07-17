#ifndef _SECTOR_H
#define _SECTOR_H

#include "libs.h"
#include <string>
#include <vector>
#include "StarSystem.h"

class Sector {
public:
	// lightyears
	enum { SIZE=8 };
	Sector(int x, int y);
	static float DistanceBetween(const Sector *a, int sysIdxA, const Sector *b, int sysIdxB);
	
	int m_numSystems;
	struct System {
		std::string name;
		vector3f p;
		StarSystem::BodyType primaryStarClass;
	};
	std::vector<System> m_systems;
private:
	std::string GenName(MTRand &rand);
	int sx, sy;
};

#endif /* _SECTOR_H */
