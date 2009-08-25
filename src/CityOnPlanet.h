#ifndef _CITYONPLANET_H
#define _CITYONPLANET_H

#include "libs.h"
#include "mtrand.h"
#include "Object.h"

class Planet;
class SpaceStation;
class Frame;
class Geom;

class CityOnPlanet: public Object {
public:
	OBJDEF(CityOnPlanet, Object, CITYONPLANET);
	CityOnPlanet(const Planet *planet, const SpaceStation *station, Uint32 seed);
	virtual ~CityOnPlanet();
	void Render(const SpaceStation *station, const Frame *camFrame);
private:
	void PutCityBit(MTRand &rand, const matrix4x4d &rot, vector3d p1, vector3d p2, vector3d p3, vector3d p4);

	struct BuildingDef {
		int modelnum;
		int rotation; // 0-3
		vector3d pos;
		Geom *geom;
	};

	const Planet *m_planet;
	Frame *m_frame;
	std::vector<BuildingDef> m_buildings;
};

#endif /* _CITYONPLANET_H */
