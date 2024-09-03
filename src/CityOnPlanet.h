// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _CITYONPLANET_H
#define _CITYONPLANET_H

#include "CollMesh.h"
#include "FrameId.h"
#include "Random.h"
#include "JsonFwd.h"
#include "matrix4x4.h"

#include <set>

class Geom;
class Planet;
class SpaceStation;
class Frame;
class SystemPath;
class SystemBody;

namespace Graphics {
	class Renderer;
	class Frustum;
	class Material;
} // namespace Graphics

namespace FileSystem {
	class FileInfo;
} // namespace FileSystem

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
	float GetClipRadius() const { return m_clipRadius; }

	static void Init();
	static void Uninit();

	static void SetCityModelPatterns(const SystemPath &path);

	static constexpr double RADIUS = 5000.0;
	// size of a city grid cell
	static constexpr uint32_t CELLSIZE = 50;
	// maximum number of cells a single building may take up
	static constexpr uint32_t CELLMAX = 32;
	static constexpr uint32_t CELLMASK = CELLMAX - 1;

private:

	// Defines which "city sector" a building should optimally be placed in
	enum class SectorKind : uint32_t {
		None,		// no specific sector for a building
		Storage,	// warehouses and personal storage buildings
		Industry,	// industrial buildings with high pollution or power draw
		Monument,	// civic / recreational buildings
		Habitat,	// living spaces and office buildings
		Frontier,	// general buildings scattered at the edge of the city
	};

	// Building type definition
	struct BuildingType {
		uint8_t cellSize[2] = {1, 1};

		float rarityAirless = 0;
		float rarityAtmo = 0;

		SectorKind buildingKind = SectorKind::None;

		SceneGraph::Model *model;
		SceneGraph::Animation *idleAnimation;
	};

	struct CityRadiusDef {
		double population = 0.0;	// billions of people
		float baseSize = 0.0;		// base size of the city (meters)
		float atmoSize = 0.0;		// additional size from atmosphere (meters)
		float randomSize = 0.0;		// random additional size (meters)
		float density = 0.0;        // density of buildings within the city
	};

	struct CityFlavourType {
		std::string flavourName;
		std::vector<CityRadiusDef> sizeDefs;
		std::vector<BuildingType> buildingTypes;
	};

private:

	void Generate(SpaceStation *station);
	void CalcCityRadius(const SystemBody *body);

	void SetGridOccupancy(uint32_t x, uint32_t y, const uint8_t size[2]);
	bool TestGridOccupancy(uint32_t x, uint32_t y, const uint8_t size[2]);

	// Quickly check if the given single grid cell is set.
	// It is expected as a precondition that the position is valid and within
	// the extents of the grid.
	inline bool TestGridQuick(uint32_t x, uint32_t y) const
	{
		// bitset is stored in lsb order with 8 cells per byte
		return m_gridBitset[y * m_gridPitch + x / 8] & (1 << (x & 7));
	}

	void AddStaticGeomsToCollisionSpace();
	void RemoveStaticGeomsFromCollisionSpace();

	struct BuildingInstance {
		Uint32 instIndex;
		float clipRadius;
		int rotation; // 0-3
		vector3d pos;
		Geom *geom;
	};

	const SystemBody *m_body;
	Planet *m_planet;

	double m_cityRadius;
	double m_cityDensity;
	uint32_t m_citySize;

	FrameId m_frame;
	Random m_rand;

	std::vector<BuildingInstance> m_buildings;
	std::vector<BuildingInstance> m_enabledBuildings;
	std::vector<Uint32> m_buildingCounts;

	// bitmask occupancy grid for quick population of the city
	std::unique_ptr<uint8_t[]> m_gridBitset;
	// width of a single grid row in bytes
	uint32_t m_gridPitch;
	uint32_t m_gridLen;

	int m_detailLevel;
	float m_clipRadius;
	vector3d m_realCentre;
	vector3d m_gridOrigin;

	CityFlavourType *m_cityType;

	// --------------------------------------------------------
	// statics
private:

	static std::vector<CityFlavourType> s_cityFlavours;

	static std::unique_ptr<Graphics::Material> s_debugMat;

	static void LoadCityFlavour(const FileSystem::FileInfo &file);
	static void LoadBuildingType(std::string_view key, const Json &buildingDef, BuildingType &out);
	static void GetModelSize(const Aabb &aabb, uint8_t size[2]);
};

#endif /* _CITYONPLANET_H */
