// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _CITYONPLANET_H
#define _CITYONPLANET_H

#include "CollMesh.h"
#include "FrameId.h"
#include "Random.h"

#include <set>

class Geom;
class Planet;
class SpaceStation;
class Frame;
class SystemPath;

namespace Graphics {
	class Renderer;
	class Frustum;
} // namespace Graphics
namespace SceneGraph {
	class Model;
	class Animation;
} // namespace SceneGraph

class CityOnPlanet {
public:
	CityOnPlanet() = delete;
	CityOnPlanet(Planet *planet, SpaceStation *station, const Uint32 seed);
	virtual ~CityOnPlanet();
	void Render(Graphics::Renderer *r, const Graphics::Frustum &camera, const SpaceStation *station, const vector3d &viewCoords, const matrix4x4d &viewTransform);
	inline Planet *GetPlanet() const { return m_planet; }

	static void Init();
	static void Uninit();
	static void SetCityModelPatterns(const SystemPath &path);

	static const int citygridmidpoint = 100;
	static const int citygridlimit = citygridmidpoint * 2;

private:
	void AddStaticGeomsToCollisionSpace();
	void RemoveStaticGeomsFromCollisionSpace();
	bool isCityCellsOccupied(uint32_t x, uint32_t z, uint32_t layout);
	void setCityCellsOccupied(uint32_t x, uint32_t z, uint32_t layout);

	struct BuildingDef {
		Uint32 instIndex;
		float clipRadius;
		int rotation; // 0-3
		vector3d pos;
		Geom *geom;
	};

	Planet *m_planet;
	FrameId m_frame;
	std::vector<BuildingDef> m_buildings;
	std::vector<BuildingDef> m_enabledBuildings;
	std::vector<Uint32> m_buildingCounts;
	uint8_t citybits[citygridlimit][citygridlimit >> 3];
	int m_detailLevel;
	vector3d m_realCentre;
	float m_clipRadius;

	struct citybuilding_t {
		// primitive data first
		uint32_t index;
		uint32_t layout; // for multi-cell buildings
		int32_t x_offset;
		int32_t z_offset;
		float airless_rarity; // attribute for adjusting building rarity - airless worlds
		float atmo_rarity;		// attribute for adjusting building rarity - atmospheric worlds
		bool storage;
		bool industry;
		bool monument;
		bool habitat;
		const char *modelname;
		SceneGraph::Model *resolvedModel;
		SceneGraph::Animation *idleanimation;
		RefCountedPtr<CollMesh> collMesh;
	};

	struct citybuildinglist_t {
		const char *modelTagName;
		unsigned int numBuildings;
		citybuilding_t *buildings;
	};

	static citybuildinglist_t s_buildingList;

	static void EnumerateNewBuildings(std::set<std::string> &filenames);
	static void LookupBuildingListModels(citybuildinglist_t *list);
};

#endif /* _CITYONPLANET_H */
