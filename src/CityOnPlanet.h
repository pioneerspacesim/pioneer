#ifndef _CITYONPLANET_H
#define _CITYONPLANET_H

#include "libs.h"
#include "mtrand.h"
#include "Object.h"
#include "LmrModel.h"

class Planet;
class SpaceStation;
class Frame;
class Geom;
class Camera;
namespace Graphics { class Renderer; }

#define CITY_ON_PLANET_RADIUS 5000.0

class CityOnPlanet: public Object {
public:
	OBJDEF(CityOnPlanet, Object, CITYONPLANET);
	CityOnPlanet(Planet *planet, SpaceStation *station, Uint32 seed);
	virtual ~CityOnPlanet();
	void Render(Graphics::Renderer *r, const Camera *camera, const SpaceStation *station, const vector3d &viewCoords, const matrix4x4d &viewTransform, double illumination, double minIllumination);
	inline Planet *GetPlanet() const { return m_planet; }

	static void Init();
	static void Uninit();
private:
	void PutCityBit(MTRand &rand, const matrix4x4d &rot, vector3d p1, vector3d p2, vector3d p3, vector3d p4);
	void AddStaticGeomsToCollisionSpace();
	void RemoveStaticGeomsFromCollisionSpace();

	struct BuildingDef {
		LmrModel *model;
		float clipRadius;
		int rotation; // 0-3
		vector3d pos;
		Geom *geom;
		// may not be at lower detail level
		bool isEnabled;
	};

	Planet *m_planet;
	Frame *m_frame;
	std::vector<BuildingDef> m_buildings;
	int m_detailLevel;
	// position of city center
	vector3d m_position;
};

#endif /* _CITYONPLANET_H */
