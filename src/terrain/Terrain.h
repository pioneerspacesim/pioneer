#ifndef _TERRAIN_H
#define _TERRAIN_H

#include "libs.h"
#include "galaxy/StarSystem.h"

#ifdef _MSC_VER
#pragma warning(disable : 4250)			// workaround for MSVC 2008 multiple inheritance bug
#endif

struct fracdef_t {
	fracdef_t() : amplitude(0.0), frequency(0.0), lacunarity(0.0), octaves(0) {}
	double amplitude;
	double frequency;
	double lacunarity;
	int octaves;
};

// data about regions from feature to heightmap code go here
struct RegionType{
	bool Valid;
	double height;
	double heightVariation;
	double inner;
	double outer;
	int Type;
};


template <typename,typename> class TerrainGenerator;


class Terrain {
public:
	static Terrain *InstanceTerrain(const SystemBody *body);

	virtual ~Terrain();

	void SetFracDef(unsigned int index, double featureHeightMeters, double featureWidthMeters, double smallestOctaveMeters = 20.0);
	inline const fracdef_t &GetFracDef(unsigned int index) { return m_fracdef[index]; }

	virtual double GetHeight(const vector3d &p) = 0;
	virtual vector3d GetColor(const vector3d &p, double height, const vector3d &norm) = 0;

	virtual const char *GetHeightFractalName() const = 0;
	virtual const char *GetColorFractalName() const = 0;

	double GetMaxHeight() const { return m_maxHeight; }

	void InitCityRegions();

private:
	template <typename HeightFractal, typename ColorFractal>
	static Terrain *InstanceGenerator(const SystemBody *body) { return new TerrainGenerator<HeightFractal,ColorFractal>(body); }

	typedef Terrain* (*GeneratorInstancer)(const SystemBody *);
	
protected:
	Terrain(const SystemBody *body);
	inline void Terrain::ApplySimpleHeightRegions(double &h,const vector3d &p);

	bool textures;
	int m_fracnum;
	double m_fracmult;

	const SystemBody *m_body;

	Uint32 m_seed;
	MTRand m_rand;

	double m_sealevel; // 0 - no water, 1 - 100% coverage
	double m_icyness; // 0 - 1 (0% to 100% cover)
	double m_volcanic;

	// heightmap stuff
	// XXX unify heightmap types
	// for the earth heightmap
	Sint16 *m_heightMap;
	// For the moon and other bodies (with height scaling)
	Uint16 *m_heightMapScaled;
	double m_heightScaling, m_minh;

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

	// used for region based terrain e.g. cities
	std::vector<vector3d> m_positions;
	std::vector<RegionType> m_regionTypes;
};


template <typename HeightFractal>
class TerrainHeightFractal : virtual public Terrain {
public:
	virtual double GetHeight(const vector3d &p);
	virtual const char *GetHeightFractalName() const;
protected:
	TerrainHeightFractal(const SystemBody *body);
private:
	TerrainHeightFractal() {}
};

template <typename ColorFractal>
class TerrainColorFractal : virtual public Terrain {
public:
	virtual vector3d GetColor(const vector3d &p, double height, const vector3d &norm);
	virtual const char *GetColorFractalName() const;
protected:
	TerrainColorFractal(const SystemBody *body);
private:
	TerrainColorFractal() {}
};


template <typename HeightFractal, typename ColorFractal>
class TerrainGenerator : public TerrainHeightFractal<HeightFractal>, public TerrainColorFractal<ColorFractal> {
public:
	TerrainGenerator(const SystemBody *body) : Terrain(body), TerrainHeightFractal<HeightFractal>(body), TerrainColorFractal<ColorFractal>(body) 
	{
		InitCityRegions();
	}

private:
	TerrainGenerator() {}
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
class TerrainColorDeadWithWater;
class TerrainColorDesert;
/*ColorEarthlike uses features not yet included in all terrain colours
 such as better poles : http://www.spacesimcentral.com/forum/download/file.php?id=1884&mode=view
 http://www.spacesimcentral.com/forum/download/file.php?id=1885&mode=view
and better distribution of snow :  http://www.spacesimcentral.com/forum/download/file.php?id=1879&mode=view  */
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

#ifdef _MSC_VER
#pragma warning(default : 4250)
#endif

// inline member functions must be placed in header
inline void Terrain::ApplySimpleHeightRegions(double &h,const vector3d &p)
{
	for (unsigned int i = 0; i < m_positions.size(); i++) {
		if (m_regionTypes[i].Valid) {
			const vector3d pos = m_positions[i];
			RegionType &rt = m_regionTypes[i];
			double th = rt.height; // target height
			if (pos.Dot(p) > rt.outer) {
				const double outer = rt.outer;
				const double inner = rt.inner;

				// maximum variation in height with respect to target height
				const double dynamicRangeHeight = 60.0/m_planetRadius; //in radii
				const double delta_h = fabs(h-th);
				const double neg = (h-th>0.0) ? 1.0 : -1.0;

				// Make up an expression to compress delta_h: 
				// Compress delta_h between 0 and 1
				//    1.1 use compression of the form c = (delta_h+a)/(a+(delta_h+a)) (eqn. 1)
				//    1.2 this gives c in the interval [0.5, 1] for delta_h [0, +inf] with c=0.5 at delta_h=0.
				//  2.0 Use compressed_h = dynamic range*(sign(h-th)*(c-0.5)) (eqn. 2) to get h between th-0.5*dynamic range, th+0.5*dynamic range
				
				// Choosing a value for a
				//    3.1 c [0.5, 0.8] occurs when delta_h [a to 3a] (3x difference) which is roughly the expressible range (above or below that the function changes slowly)
				//    3.2 Find an expression for the expected variation and divide by around 3
				
				// It may become necessary calculate expected variation based on intermediate quantities generated (e.g. distribution fractals)
				// or to store a per planet estimation of variation when fracdefs are calculated.
				const double variationEstimate = rt.heightVariation;
				const double a = variationEstimate*(1.0/3.0); // point 3.2 
				
				const double c = (delta_h+a)/(2.0*a+delta_h); // point 1.1 
				const double compressed_h = dynamicRangeHeight*(neg*(c-0.5))+th;// point 2.0

				#define blend(a,b,v) a*(1.0-v)+b*v
				h = blend(h, compressed_h, Clamp((pos.Dot(p)-outer)/(inner-outer), 0.0, 1.0)); break; // blends from compressed height-terrain height as pos goes inner to outer
				#undef blend
			}
		}
	}
}


#endif /* TERRAIN_H */
