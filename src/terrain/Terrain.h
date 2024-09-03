// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _TERRAIN_H
#define _TERRAIN_H

#include "FracDef.h"
#include "../Random.h"
#include "../RefCounted.h"
#include "../vector3.h"
#include "../galaxy/SystemPath.h"

#include <memory>
#include <string>

#ifdef _MSC_VER
#pragma warning(disable : 4250) // workaround for MSVC 2008 multiple inheritance bug
#endif

class SystemBody;

template <typename, typename>
class TerrainGenerator;

class Terrain : public RefCounted {
public:
	// location and intensity of effects are controlled by the colour fractals;
	// it's possible for a Terrain to have a flag set but not actually to exhibit any of that effect
	enum SurfaceEffectFlags {
		EFFECT_LAVA = 1 << 0,
		EFFECT_WATER = 2
		// can add other effect flags here (e.g., water, snow, ice)
	};

	static Terrain *InstanceTerrain(const SystemBody *body);

	virtual ~Terrain();

	void SetFracDef(const unsigned int index, const double featureHeightMeters, const double featureWidthMeters, const double smallestOctaveMeters = 20.0);
	inline const fracdef_t &GetFracDef(const unsigned int index) const
	{
		assert(index < MAX_FRACDEFS);
		return m_fracdef[index];
	}

	virtual double GetHeight(const vector3d &p) const = 0;
	virtual vector3d GetColor(const vector3d &p, double height, const vector3d &norm) const = 0;

	virtual const char *GetHeightFractalName() const = 0;
	virtual const char *GetColorFractalName() const = 0;

	double GetMaxHeight() const { return m_maxHeight; }

	Uint32 GetSurfaceEffects() const { return m_surfaceEffects; }

	double BiCubicInterpolation(const vector3d &p) const;

	void DebugDump() const;

private:
	template <typename HeightFractal, typename ColorFractal>
	static Terrain *InstanceGenerator(const SystemBody *body) { return new TerrainGenerator<HeightFractal, ColorFractal>(body); }

	typedef Terrain *(*GeneratorInstancer)(const SystemBody *);

protected:
	Terrain(const SystemBody *body);

	Uint32 m_seed;
	Random m_rand;

	double m_sealevel; // 0 - no water, 1 - 100% coverage
	double m_icyness; // 0 - 1 (0% to 100% cover)
	double m_volcanic;

	Uint32 m_surfaceEffects;

	// heightmap stuff
	// XXX unify heightmap types
	std::unique_ptr<double[]> m_heightMap;
	double m_heightScaling, m_minh;

	int m_heightMapSizeX;
	int m_heightMapSizeY;

	/** General attributes */
	double m_maxHeight;
	double m_maxHeightInMeters;
	double m_invMaxHeight;
	double m_planetRadius;
	double m_invPlanetRadius;
	double m_planetEarthRadii;

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
	static const Uint32 MAX_FRACDEFS = 10;
	fracdef_t m_fracdef[MAX_FRACDEFS];

	struct MinBodyData {
		MinBodyData(const SystemBody *body);
		double m_radius;
		double m_aspectRatio;
		SystemPath m_path;
		std::string m_name;
	};
	MinBodyData m_minBody;
};

template <typename HeightFractal>
class TerrainHeightFractal : virtual public Terrain {
public:
	TerrainHeightFractal() = delete;
	virtual double GetHeight(const vector3d &p) const;
	virtual const char *GetHeightFractalName() const;

protected:
	TerrainHeightFractal(const SystemBody *body);

private:
};

template <typename ColorFractal>
class TerrainColorFractal : virtual public Terrain {
public:
	TerrainColorFractal() = delete;
	virtual vector3d GetColor(const vector3d &p, double height, const vector3d &norm) const;
	virtual const char *GetColorFractalName() const;

protected:
	TerrainColorFractal(const SystemBody *body);

private:
};

template <typename HeightFractal, typename ColorFractal>
class TerrainGenerator : public TerrainHeightFractal<HeightFractal>, public TerrainColorFractal<ColorFractal> {
public:
	TerrainGenerator() = delete;
	TerrainGenerator(const SystemBody *body) :
		Terrain(body),
		TerrainHeightFractal<HeightFractal>(body),
		TerrainColorFractal<ColorFractal>(body) {}

private:
};

//This is the most complex and insanely crazy terrain you will ever see :
class TerrainHeightFlat;

//New terrains with less noise :
class TerrainHeightAsteroid;
class TerrainHeightAsteroid2;
class TerrainHeightAsteroid3;
class TerrainHeightAsteroid4;
class TerrainHeightBarrenRock;
class TerrainHeightBarrenRock2;
class TerrainHeightBarrenRock3;
/* Pictures of the above terrains:
 http://i.imgur.com/cJO4E.jpg
 http://i.imgur.com/BtB0g.png
 http://i.imgur.com/qeEuS.png
 */

class TerrainHeightEllipsoid;

// Newish terrains, 6 months or so :
class TerrainHeightHillsCraters2;
class TerrainHeightHillsCraters;
class TerrainHeightHillsDunes;
//   This terrain or the following one should have terragen style ridged mountains :
//   (As seen in an ancient clip of Mars http://www.youtube.com/watch?v=WeO28VBTWxs )
class TerrainHeightHillsNormal;
class TerrainHeightHillsRidged;
class TerrainHeightHillsRivers;

class TerrainHeightMapped;
class TerrainHeightMapped2;
class TerrainHeightMountainsCraters2;
class TerrainHeightMountainsCraters;

//Probably the best looking terrain due to variety, but among the most costly too :
//(It was also used for mars at some point : http://www.youtube.com/watch?feature=player_embedded&v=4-DcyQm0zE4 )
/// and http://www.youtube.com/watch?v=gPtxUUunSWg&t=5m15s
class TerrainHeightMountainsNormal;
// Based on TerrainHeightMountainsNormal :
class TerrainHeightMountainsRivers;
/*Pictures from the above two terrains generating Earth-like worlds:
 http://www.spacesimcentral.com/forum/download/file.php?id=1533&mode=view
 http://www.spacesimcentral.com/forum/download/file.php?id=1544&mode=view
 http://www.spacesimcentral.com/forum/download/file.php?id=1550&mode=view
 http://www.spacesimcentral.com/forum/download/file.php?id=1540&mode=view
 */

// Older terrains:
class TerrainHeightMountainsRidged;
class TerrainHeightMountainsRiversVolcano;
//   Used to be used for mars since it has a megavolcano:
class TerrainHeightMountainsVolcano;

//Oldest terrains, from before fracdefs :
class TerrainHeightRuggedDesert;
//   lava terrain should look like this http://www.spacesimcentral.com/forum/download/file.php?id=1778&mode=view
class TerrainHeightRuggedLava;

/*Terrains used for Iceworlds,
only terrain to use the much neglected impact crater function
(basically I forgot about it;) ) **It makes cool looking sunken craters** */
class TerrainHeightWaterSolidCanyons;
class TerrainHeightWaterSolid;

class TerrainColorAsteroid;
class TerrainColorBandedRock;
class TerrainColorBlack;
class TerrainColorDeadWithWater;
class TerrainColorDesert;
/*ColorEarthlike uses features not yet included in all terrain colours
 such as better poles : http://www.spacesimcentral.com/forum/download/file.php?id=1884&mode=view
 http://www.spacesimcentral.com/forum/download/file.php?id=1885&mode=view
and better distribution of snow :  http://www.spacesimcentral.com/forum/download/file.php?id=1879&mode=view  */
class TerrainColorEarthLike;
class TerrainColorEarthLikeHeightmapped;
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
class TerrainColorWhite;
class TerrainColorStarBrownDwarf;
class TerrainColorStarG;
class TerrainColorStarK;
class TerrainColorStarM;
class TerrainColorStarWhiteDwarf;
class TerrainColorTFGood;
class TerrainColorTFPoor;
class TerrainColorVolcanic;

#ifdef _MSC_VER
#pragma warning(default : 4250)
#endif

#endif /* TERRAIN_H */
