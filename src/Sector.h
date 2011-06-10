#ifndef _SECTOR_H
#define _SECTOR_H

#include "libs.h"
#include <string>
#include <vector>
#include "StarSystem.h"
#include "CustomSystem.h"

class Sector {
public:
	// lightyears
	static const float SIZE;
	Sector(int x, int y);
	static float DistanceBetween(const Sector *a, int sysIdxA, const Sector *b, int sysIdxB);
	static void Init();
	
	class System {
	public:
		System() : customSys(0), pStarSystem(0) {};
		~System() {
			if( NULL!=pStarSystem ) {
				pStarSystem->Release();
			}
		}
	
		std::string name;
		vector3f p;
		int numStars;
		SBody::BodyType starType[4];
		Uint32 seed;
		const CustomSystem *customSys;
		StarSystem* pStarSystem;
	};
	std::vector<System> m_systems;
private:
	void GetCustomSystems();
	std::string GenName(System &sys, MTRand &rand);
	int sx, sy;
};

#endif /* _SECTOR_H */
