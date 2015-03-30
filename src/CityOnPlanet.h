// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _CITYONPLANET_H
#define _CITYONPLANET_H

#include "libs.h"
#include "Random.h"
#include "Object.h"
#include "CollMesh.h"
#include "collider/Geom.h"
#include "galaxy/StarSystem.h"

class Planet;
class SpaceStation;
class Frame;
namespace Graphics { class Renderer; class Frustum; }
namespace SceneGraph { class Model; }

#define CITY_ON_PLANET_RADIUS 5000.0

class CityOnPlanet: public Object {
public:
	OBJDEF(CityOnPlanet, Object, CITYONPLANET);
	CityOnPlanet(Planet *planet, SpaceStation *station, const Uint32 seed);
	virtual ~CityOnPlanet();
	void Render(Graphics::Renderer *r, const Graphics::Frustum &camera, const SpaceStation *station, const vector3d &viewCoords, const matrix4x4d &viewTransform);
	inline Planet *GetPlanet() const { return m_planet; }

	static void Init();
	static void Uninit();
	static void SetCityModelPatterns(const SystemPath &path);
private:
	void PutCityBit(Random &rand, const matrix4x4d &rot, vector3d p1, vector3d p2, vector3d p3, vector3d p4);
	void AddStaticGeomsToCollisionSpace();
	void RemoveStaticGeomsFromCollisionSpace();

	struct BuildingDef {
		Uint32 instIndex;
		float clipRadius;
		int rotation; // 0-3
		vector3d pos;
		Geom *geom;
	};

	Planet *m_planet;
	Frame *m_frame;
	std::vector<BuildingDef> m_buildings;
	std::vector<BuildingDef> m_enabledBuildings;
	std::vector<Uint32> m_buildingCounts;
	int m_detailLevel;
	vector3d m_realCentre;
	float m_clipRadius;

	// --------------------------------------------------------
	// statics
	static const unsigned int CITYFLAVOURS = 5;

	struct citybuilding_t {
		const char *modelname;
		double xzradius;
		SceneGraph::Model *resolvedModel;
		RefCountedPtr<CollMesh> collMesh;
		Uint32 instIndex;
	};

	struct citybuildinglist_t {
		const char *modelTagName;
		double minRadius, maxRadius;
		unsigned int numBuildings;
		citybuilding_t *buildings;
	};

	struct cityflavourdef_t {
		vector3d center;
		double size;
	};

	static bool s_cityBuildingsInitted;

	static citybuildinglist_t s_buildingList;
	static cityflavourdef_t cityflavour[CITYFLAVOURS];

	static void EnumerateNewBuildings(std::set<std::string> &filenames);
	static void LookupBuildingListModels(citybuildinglist_t *list);
};

#endif /* _CITYONPLANET_H */
