#ifndef _CITYONPLANET_H
#define _CITYONPLANET_H

#include "libs.h"
#include "mtrand.h"

class Planet;
class SpaceStation;
class Frame;

class CityOnPlanet {
public:
	CityOnPlanet(const Planet *planet, const SpaceStation *station, Uint32 seed);
	void Render(const SpaceStation *station, const Frame *camFrame);
private:
	void PutCityBit(MTRand &rand, const matrix4x4d &rot, vector3d p1, vector3d p2, vector3d p3, vector3d p4);

	struct BuildingDef {
		int modelnum;
		int rotation; // 0-3
		vector3d pos;
	};

	const Planet *m_planet;
	std::vector<BuildingDef> m_buildings;
};

#endif /* _CITYONPLANET_H */
