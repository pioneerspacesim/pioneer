#ifndef _TERRAIN_H
#define _TERRAIN_H

#include "libs.h"
#include "StarSystem.h"

struct fracdef_t {
	fracdef_t() : amplitude(0.0), frequency(0.0), lacunarity(0.0), octaves(0) {}
	double amplitude;
	double frequency;
	double lacunarity;
	int octaves;
};


template <typename,typename> class TerrainGenerator;


class Terrain {
public:
	static Terrain *InstanceTerrain(const SBody *body);

	virtual ~Terrain();

	void SetFracDef(unsigned int index, double featureHeightMeters, double featureWidthMeters, double smallestOctaveMeters = 20.0);
	inline const fracdef_t &GetFracDef(unsigned int index) { return m_fracdef[index]; }

	virtual double GetHeight(const vector3d &p) = 0;
	virtual vector3d GetColor(const vector3d &p, double height, const vector3d &norm) = 0;

	virtual const char *GetHeightFractalName() const = 0;
	virtual const char *GetColorFractalName() const = 0;

	double GetMaxHeight() const { return m_maxHeight; }

private:
	template <typename HeightFractal, typename ColorFractal>
	static Terrain *InstanceGenerator(const SBody *body) { return new TerrainGenerator<HeightFractal,ColorFractal>(body); }

	typedef Terrain* (*GeneratorInstancer)(const SBody *);


protected:
	Terrain(const SBody *body);

	bool textures;
	int m_fracnum;
	double m_fracmult;

	const SBody *m_body;

	Uint32 m_seed;
	MTRand m_rand;

	double m_sealevel; // 0 - no water, 1 - 100% coverage
	double m_icyness; // 0 - 1 (0% to 100% cover)
	double m_volcanic;
	
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
	double m_noise1;
	double m_noise2;
	double m_noise3;

	double m_entropy[12];

	vector3d m_rockColor[8];
	vector3d m_darkrockColor[8];
	vector3d m_greyrockColor[8];
	vector3d m_plantColor[8];
	vector3d m_darkplantColor[8];
	vector3d m_sandColor[8];
	vector3d m_darksandColor[8];
	vector3d m_dirtColor[8];
	vector3d m_darkdirtColor[8];
	vector3d m_gglightColor[8];
	vector3d m_ggdarkColor[8];

	/* XXX you probably shouldn't increase this. If you are
	   using more than 10 then things will be slow as hell */
	fracdef_t m_fracdef[10];
};


template <typename HeightFractal>
class TerrainHeightFractal : virtual public Terrain {
public:
	virtual double GetHeight(const vector3d &p);
	virtual const char *GetHeightFractalName() const;
protected:
	TerrainHeightFractal(const SBody *body);
private:
	TerrainHeightFractal() {}
};

template <typename ColorFractal>
class TerrainColorFractal : virtual public Terrain {
public:
	virtual vector3d GetColor(const vector3d &p, double height, const vector3d &norm);
	virtual const char *GetColorFractalName() const;
protected:
	TerrainColorFractal(const SBody *body);
private:
	TerrainColorFractal() {}
};


template <typename HeightFractal, typename ColorFractal>
class TerrainGenerator : public TerrainHeightFractal<HeightFractal>, public TerrainColorFractal<ColorFractal> {
public:
	TerrainGenerator(const SBody *body) : Terrain(body), TerrainHeightFractal<HeightFractal>(body), TerrainColorFractal<ColorFractal>(body) {}

private:
	TerrainGenerator() {}
};


class TerrainHeightAsteroid;
class TerrainHeightFlat;
class TerrainHeightHillsCraters2;
class TerrainHeightHillsCraters;
class TerrainHeightHillsDunes;
class TerrainHeightHillsNormal;
class TerrainHeightHillsRidged;
class TerrainHeightHillsRivers;
class TerrainHeightMapped;
class TerrainHeightMountainsCraters2;
class TerrainHeightMountainsCraters;
class TerrainHeightMountainsNormal;
class TerrainHeightMountainsRidged;
class TerrainHeightMountainsRivers;
class TerrainHeightMountainsRiversVolcano;
class TerrainHeightMountainsVolcano;
class TerrainHeightRuggedDesert;
class TerrainHeightRuggedLava;
class TerrainHeightWaterSolidCanyons;
class TerrainHeightWaterSolid;

class TerrainColorAsteroid;
class TerrainColorBandedRock;
class TerrainColorDeadWithWater;
class TerrainColorDesert;
class TerrainColorEarthLike;
class TerrainColorGGJupiter;
class TerrainColorGGNeptune2;
class TerrainColorGGNeptune;
class TerrainColorGGSaturn2;
class TerrainColorGGSaturn;
class TerrainColorGGUranus;
class TerrainColorIce;
class TerrainColorMethane;
class TerrainColorRock2;
class TerrainColorRock;
class TerrainColorSolid;
class TerrainColorStarBrownDwarf;
class TerrainColorStarG;
class TerrainColorStarK;
class TerrainColorStarM;
class TerrainColorStarWhiteDwarf;
class TerrainColorTFGood;
class TerrainColorTFPoor;
class TerrainColorVolcanic;

#endif /* TERRAIN_H */
