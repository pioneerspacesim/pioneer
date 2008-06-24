#ifndef _SECTOR_H
#define _SECTOR_H

#include "libs.h"
#include <string>
#include <vector>
#include "StarSystem.h"

class Sector {
public:
	Sector(int x, int y);
	
	int m_numSystems;
	struct System {
		std::string name;
		vector3f p;
		StarSystem::SBody::SubType primaryStarClass;
	};
	std::vector<System> m_systems;
private:
	std::string GenName(MTRand &rand);
};

#endif /* _SECTOR_H */
