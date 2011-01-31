#ifndef GEOSPHERESTYLE_H
#define GEOSPHERESTYLE_H
#include "libs.h"
#include "StarSystem.h"

struct fracdef_t {
	double amplitude;
	double frequency;
	double lacunarity;
};

class GeoSphereStyle {
	public:
	enum TerrainType {
		TERRAIN_NONE,
		TERRAIN_GASGIANT,
		TERRAIN_ASTEROID,
		TERRAIN_RUGGED,
		TERRAIN_RUGGED_CRATERED,
		TERRAIN_RUGGED_LAVA,
		TERRAIN_RUGGED_METHANE,
		TERRAIN_H2O_SOLID,
		TERRAIN_H2O_LIQUID,
		TERRAIN_RUGGED_DESERT,
		TERRAIN_RUGGED_H2O,
		TERRAIN_RUGGED_H2O_MEGAVOLC,
		TERRAIN_MAX = TERRAIN_RUGGED_H2O_MEGAVOLC
	};

	enum ColorType {
		COLOR_NONE,
		COLOR_GG_SATURN,
		COLOR_GG_JUPITER,
		COLOR_GG_URANUS,
		COLOR_GG_NEPTUNE,
		COLOR_EARTHLIKE,
		COLOR_DEAD_WITH_H2O,
		COLOR_ICEWORLD,
		COLOR_DESERT,
		COLOR_ROCK,
		COLOR_ROID,
		COLOR_VOLCANIC,
		COLOR_METHANE,
		COLOR_TFGOOD,
		COLOR_TFPOOR,
		COLOR_BANDED_ROCK,
		COLOR_MAX = COLOR_BANDED_ROCK
	};

	~GeoSphereStyle() {
		if (m_heightMap) delete [] m_heightMap;
		m_heightMap = 0;
	}
	GeoSphereStyle(): m_terrainType(TERRAIN_NONE), m_colorType(COLOR_NONE) {
		m_heightMap = 0;
	}
	GeoSphereStyle(const SBody *body);
	GeoSphereStyle(TerrainType t, ColorType c, Uint32 random_seed) {
		MTRand rand;
		rand.seed(random_seed);
		m_seed = random_seed;
		m_heightMap = 0;
		Init(t, c, EARTH_RADIUS, 273.0, rand);
	}
	double GetHeight(const vector3d &p);
	vector3d GetColor(const vector3d &p, double height, const vector3d &norm);
	double GetMaxHeight() const { return m_maxHeight; }

	// so the object viewer can play with planet types
#ifdef DEBUG
	friend class ObjectViewerView;
#endif /* DEBUG */
	
	private:
	void Init(TerrainType t, ColorType c, double planetRadius, double averageTemp, MTRand &rand);
	int GetRawHeightMapVal(int x, int y);
	double GetHeightMapVal(const vector3d &pt);

	TerrainType m_terrainType;
	ColorType m_colorType;
	Uint32 m_seed;
	
	/** for sbodies with a heightMap we load this turd
	 * and use it instead of perlin height function */
	Sint16 *m_heightMap;
	int m_heightMapSizeX;
	int m_heightMapSizeY;

	/** General attributes */
	double m_maxHeight;
	double m_invMaxHeight;
	double m_planetRadius;
	double m_planetEarthRadii;
	double noise1;
	double noise2;
	double noise3;

	double m_icyness;
	double m_entropy[12];

	vector3d m_rockColor[8];
	vector3d m_greyrockColor[8];

	struct {
		fracdef_t continents;
		fracdef_t mountains;
		fracdef_t hills;
		fracdef_t hillDistrib;
		fracdef_t mountainDistrib;
		double sealevel;
	} targ;

	struct sbody_valid_styles_t {
		TerrainType terrainType[16];
		ColorType colorType[16];
	};
	static const sbody_valid_styles_t sbody_valid_styles[SBody::TYPE_MAX];
};

#endif /* GEOSPHERESTYLE_H */
