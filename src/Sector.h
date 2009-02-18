#ifndef _SECTOR_H
#define _SECTOR_H

#include "libs.h"
#include <string>
#include <vector>
#include "StarSystem.h"

class CustomSBody;

class Sector {
public:
	// lightyears
	enum { SIZE=8 };
	Sector(int x, int y);
	static float DistanceBetween(const Sector *a, int sysIdxA, const Sector *b, int sysIdxB);
	
	struct System {
		std::string name;
		vector3f p;
		int numStars;
		SBody::BodyType starType[4];
		const CustomSBody *customDef;
	};
	std::vector<System> m_systems;
private:
	void GetCustomSystems();
	std::string GenName(System &sys, MTRand &rand);
	int sx, sy;
};

#endif /* _SECTOR_H */
