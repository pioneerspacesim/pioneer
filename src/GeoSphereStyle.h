#ifndef GEOSPHERESTYLE_H
#define GEOSPHERESTYLE_H
#include "libs.h"
#include "StarSystem.h"

struct fracdef_t {
	double amplitude;
	double frequency;
	double lacunarity;
	int octaves;
};

class GeoSphereStyle {
	public:
	enum TerrainType {
		TERRAIN_NONE,
		TERRAIN_ROLLING_HILLS,
		TERRAIN_BIG_MOUNTAINS,
		TERRAIN_RUGGED,
		TERRAIN_ASTEROID,
		TERRAIN_GASGIANT,
		TERRAIN_MAX = TERRAIN_GASGIANT
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
		COLOR_ROCK2,
		COLOR_ASTEROID,
		COLOR_VOLCANIC,
		COLOR_METHANE,
		COLOR_TFGOOD,
		COLOR_TFPOOR,
		COLOR_BANDED_ROCK,
		COLOR_MAX = COLOR_BANDED_ROCK
	};

	GeoSphereStyle(const SBody *body);
	~GeoSphereStyle() {
		if (m_heightMap) delete [] m_heightMap;
		m_heightMap = 0;
	}
	GeoSphereStyle(): m_terrainType(TERRAIN_NONE), m_colorType(COLOR_NONE) {
		m_heightMap = 0;
	}
	double GetHeight(const vector3d &p);
	vector3d GetColor(const vector3d &p, double height, const vector3d &norm);
	void GetAtmosphereFlavor(Color *outColor, float *outDensity) const {
		*outColor = m_atmosColor;
		*outDensity = m_atmosDensity;
	}
	double GetMaxHeight() const { return m_maxHeight; }

	private:
	void PickAtmosphere(const SBody *sbody);
	void InitFractalType(TerrainType t, ColorType c, double averageTemp, MTRand &rand);
	int GetRawHeightMapVal(int x, int y);
	double GetHeightMapVal(const vector3d &pt);
	void InitHeightMap(const SBody *sbody);
	void SetFracDef(struct fracdef_t *def, double featureHeightMeters, double featureWidthMeters, double lacunarity, double smallestOctaveMeters = 20.0);

	TerrainType m_terrainType;
	ColorType m_colorType;
	Uint32 m_seed;

	Color m_atmosColor;
	float m_atmosDensity;
	double m_sealevel; // 0 - no water, 1 - 100% coverage
	double m_icyness; // 0 - 1 (0% to 100% cover)
	
	/** for sbodies with a heightMap we load this turd
	 * and use it instead of perlin height function */
	Sint16 *m_heightMap;
	int m_heightMapSizeX;
	int m_heightMapSizeY;

	/** General attributes */
	double m_maxHeight;
	double m_maxHeightInMeters;
	double m_invMaxHeight;
	double m_planetRadius;
	double m_planetEarthRadii;
	double noise1;
	double noise2;
	double noise3;

	double m_entropy[12];

	vector3d m_rockColor[8];
	vector3d m_greyrockColor[8];

	struct {
		fracdef_t continents;
		fracdef_t midTerrain;
		fracdef_t midDistrib;
		fracdef_t localTerrain;
		fracdef_t localDistrib;
	} targ;
};

#endif /* GEOSPHERESTYLE_H */
