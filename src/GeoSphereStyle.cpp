#include "GeoSphereStyle.h"
#include "perlin.h"


/**
 * All these stinking octavenoise functions return range [0,1] if roughness = 0.5
 */
static inline double octavenoise(fracdef_t &def, double roughness, const vector3d &p);
static inline double river_octavenoise(fracdef_t &def, double roughness, const vector3d &p);
static inline double ridged_octavenoise(fracdef_t &def, double roughness, const vector3d &p);
static double canyon_function(const fracdef_t &def, const vector3d &p);
static double canyon2_function(const fracdef_t &def, const vector3d &p);
static double canyon3_function(const fracdef_t &def, const vector3d &p);
static double canyon3_function(const fracdef_t &def, const vector3d &p);
static double crater_function(const fracdef_t &def, const vector3d &p);
static double volcano_function(const fracdef_t &def, const vector3d &p);
static double megavolcano_function(const fracdef_t &def, const vector3d &p);
static double river_function(const fracdef_t &def, const vector3d &p);
static double river2_function(const fracdef_t &def, const vector3d &p);
static double cliff_function(const fracdef_t &def, const vector3d &p);

int GeoSphereStyle::GetRawHeightMapVal(int x, int y)
{
	return m_heightMap[Clamp(y, 0, m_heightMapSizeY-1)*m_heightMapSizeX + Clamp(x, 0, m_heightMapSizeX-1)];
}

/*
 * Bicubic interpolation!!!
 */
double GeoSphereStyle::GetHeightMapVal(const vector3d &pt)
{
	double latitude = -asin(pt.y);
	if (pt.y < -1.0) latitude = -0.5*M_PI;
	if (pt.y > 1.0) latitude = 0.5*M_PI;
//	if (!isfinite(latitude)) {
//		// pt.y is just out of asin domain [-1,1]
//		latitude = (pt.y < 0 ? -0.5*M_PI : M_PI*0.5);
//	}
	double longitude = atan2(pt.x, pt.z);
	double px = (((m_heightMapSizeX-1) * (longitude + M_PI)) / (2*M_PI));
	double py = ((m_heightMapSizeY-1)*(latitude + 0.5*M_PI)) / M_PI;
	int ix = floor(px);
	int iy = floor(py);
	ix = Clamp(ix, 0, m_heightMapSizeX-1);
	iy = Clamp(iy, 0, m_heightMapSizeY-1);
	double dx = px-ix;
	double dy = py-iy;

	// p0,3 p1,3 p2,3 p3,3
	// p0,2 p1,2 p2,2 p3,2
	// p0,1 p1,1 p2,1 p3,1
	// p0,0 p1,0 p2,0 p3,0
	double p[4][4];
	for (int x=-1; x<3; x++) {
		for (int y=-1; y<3; y++) {
			p[1+x][1+y] = GetRawHeightMapVal(ix+x, iy+y);
		}
	}

	double c[4];
	for (int j=0; j<4; j++) {
		double d0 = p[0][j] - p[1][j];
		double d2 = p[2][j] - p[1][j];
		double d3 = p[3][j] - p[1][j];
		double a0 = p[1][j];
		double a1 = -(1/3.0)*d0 + d2 - (1/6.0)*d3;
		double a2 = 0.5*d0 + 0.5*d2;
		double a3 = -(1/6.0)*d0 - 0.5*d2 + (1/6.0)*d3;
		c[j] = a0 + a1*dx + a2*dx*dx + a3*dx*dx*dx;
	}

	{
		double d0 = c[0] - c[1];
		double d2 = c[2] - c[1];
		double d3 = c[3] - c[1];
		double a0 = c[1];
		double a1 = -(1/3.0)*d0 + d2 - (1/6.0)*d3;
		double a2 = 0.5*d0 + 0.5*d2;
		double a3 = -(1/6.0)*d0 - 0.5*d2 + (1/6.0)*d3;
		double v = a0 + a1*dy + a2*dy*dy + a3*dy*dy*dy;
		return (v<0 ? 0 : v + 1000.0*river_octavenoise(m_fracdef[0], Clamp(v*0.0002, 0.3, 0.5), pt));
	}
}

static inline vector3d interpolate_color(double n, vector3d start, vector3d end)
{
	n = Clamp(n, 0.0, 1.0);
	return start*(1.0-n) + end*n;
}

void GeoSphereStyle::PickAtmosphere(const SBody *sbody)
{
	/* Alpha value isn't real alpha. in the shader fog depth is determined
	 * by density*alpha, so that we can have very dense atmospheres
	 * without having them a big stinking solid color obscuring everything
	 */
	switch (sbody->type) {
		case SBody::TYPE_PLANET_GAS_GIANT:
			m_atmosColor = Color(1.0f, 1.0f, 1.0f, 0.005f);
			m_atmosDensity = 14.0;
			break;
		case SBody::TYPE_PLANET_ASTEROID:
			m_atmosColor = Color(0.0f, 0.0f, 0.0f, 0.0f);
			m_atmosDensity = 0.0;
			break;
		default:
			m_atmosColor = Color(.6f, .6f, .7f, 0.8f);
			m_atmosDensity = sbody->m_volatileGas.ToDouble();
			break;
	}
#if 0
			/*
		case SBody::TYPE_PLANET_DWARF2:
			*outColor = Color(0.0f, 0.0f, 0.0f, 0.0f);
			*outDensity = 0.0f;
			break;
		case SBody::TYPE_PLANET_SMALL:
			*outColor = Color(.2f, .2f, .3f, 1.0f);
			*outDensity = 0.1f;
			break;
		case SBody::TYPE_PLANET_CO2:
			*outColor = Color( .8f, .8f, .8f, 1.0f);
			*outDensity = 2.0f;
			break;
		case SBody::TYPE_PLANET_METHANE:
			*outColor = Color(.2f, .6f, .3f, 2.0f);
			*outDensity = 3.4f;
			break;
		case SBody::TYPE_PLANET_WATER:
			*outColor = Color(.6f, .6f, .7f, 0.8f);
			*outDensity = 0.8f;
			break;
		case SBody::TYPE_PLANET_WATER_THICK_ATMOS:
			*outColor = Color(.5f, .5f, .8f, 2.0f);
			*outDensity = 3.0f;
			break;
		case SBody::TYPE_PLANET_DESERT:
			*outColor = Color(.4f, .3f, .1f, 0.7f);
			*outDensity = 1.0f;
			break;
		case SBody::TYPE_PLANET_CO2_THICK_ATMOS:
			*outColor = Color(.8f, .8f, .8f, 2.0f);
			*outDensity = 7.0f;
			break;
		case SBody::TYPE_PLANET_METHANE_THICK_ATMOS:
			*outColor = Color(0.6f, 0.4f, 0.1f, 3.0f);
			*outDensity = 8.0f;
			break;
		case SBody::TYPE_PLANET_HIGHLY_VOLCANIC:
			*outColor = Color(0.5f, 0.1f, 0.1f, 1.6f);
			*outDensity = 1.8f;
			break;
		case SBody::TYPE_PLANET_INDIGENOUS_LIFE:
			*outColor = Color(.5f, .5f, 1.0f, 1.0f);
			*outDensity = 1.2;
			break;
		case SBody::TYPE_PLANET_TERRAFORMED_POOR:
			*outColor = Color(.7f, .4f, 0.9f, 0.8f);
			*outDensity = 1.0;
			break;
		case SBody::TYPE_PLANET_TERRAFORMED_GOOD:
			*outColor = Color(.5f, .45f, 0.95f, 0.9f);
			*outDensity = 1.1;
			break;
		default:
			*outColor = Color(0.0f, 0.0f, 0.0f, 0.0f);
			*outDensity = 0.0f;
			break;*/
	}
#endif /* 0 */
}

void GeoSphereStyle::InitHeightMap(const SBody *body)
{
	/* Height map? */
	if (body->heightMapFilename) {
		FILE *f;
		f = fopen_or_die(body->heightMapFilename, "r");
		// read size!
		Uint16 v;
		fread(&v, 2, 1, f); m_heightMapSizeX = v;
		fread(&v, 2, 1, f); m_heightMapSizeY = v;
		m_heightMap = new Sint16[m_heightMapSizeX * m_heightMapSizeY];
		// XXX TODO XXX what about bigendian archs...
		fread(m_heightMap, sizeof(Sint16), m_heightMapSizeX * m_heightMapSizeY, f);
		fclose(f);
	} else {
		m_heightMap = 0;
	}	
}

GeoSphereStyle::GeoSphereStyle(const SBody *body)
{
	MTRand rand;
	rand.seed(body->seed);
	m_seed = body->seed;

	/* Pick terrain and color fractals to use */
	if (body->type == SBody::TYPE_PLANET_GAS_GIANT) {
		m_terrainType = TERRAIN_GASGIANT;
		switch (rand.Int32(4)) {
			case 0: m_colorType = COLOR_GG_SATURN; break;
			case 1: m_colorType = COLOR_GG_URANUS; break;
			case 2: m_colorType = COLOR_GG_JUPITER; break;
			default: m_colorType = COLOR_GG_NEPTUNE; break;
		}
	} else if (body->type == SBody::TYPE_PLANET_ASTEROID) {
		m_terrainType = TERRAIN_ASTEROID;
		m_colorType = COLOR_ASTEROID;
	} else /* SBody::TYPE_PLANET_TERRESTRIAL */ {
		/* Pick terrain and color fractals for terrestrial planets */
		if ((body->m_life > fixed(1,2)) &&  
		   (body->m_volatileGas > fixed(2,10))){
			   // There would be no life on the surface without atmosphere
			const enum TerrainFractal choices[] = {
				TERRAIN_HILLS_RIDGED,
				TERRAIN_HILLS_RIVERS,
				TERRAIN_MOUNTAINS_RIDGED,
				TERRAIN_MOUNTAINS_RIVERS,
				TERRAIN_MOUNTAINS_VOLCANO,
				TERRAIN_MOUNTAINS_RIVERS_VOLCANO,
			};
			//m_terrainType = TERRAIN_MOUNTAINS_VOLCANO;
			m_terrainType = choices[rand.Int32(6)];
			m_colorType = COLOR_EARTHLIKE;
		} else if ((body->m_volatileGas > fixed(2,10)) &&
				  (body->m_life > fixed(1,10)) ) {
			const enum TerrainFractal choices[] = {
				TERRAIN_HILLS_RIDGED,
				TERRAIN_HILLS_RIVERS,
				TERRAIN_MOUNTAINS_RIDGED,
				TERRAIN_MOUNTAINS_VOLCANO,
				TERRAIN_MOUNTAINS_RIVERS_VOLCANO,
				TERRAIN_MOUNTAINS_RIVERS,
				TERRAIN_RUGGED_DESERT,
			};
			//m_terrainType = TERRAIN_MOUNTAINS_RIVERS_VOLCANO;
			m_terrainType = choices[rand.Int32(7)];
			m_colorType = COLOR_TFGOOD;
		} else if (body->m_life > fixed(1,10))  {
			const enum TerrainFractal choices[] = {
				TERRAIN_HILLS_RIDGED,
				TERRAIN_HILLS_RIVERS,
				TERRAIN_MOUNTAINS_RIDGED,
				TERRAIN_MOUNTAINS_VOLCANO,
				TERRAIN_MOUNTAINS_RIVERS_VOLCANO,
				TERRAIN_MOUNTAINS_RIVERS,
				TERRAIN_H2O_SOLID,
				TERRAIN_RUGGED_LAVA,
				TERRAIN_RUGGED_DESERT,
			};
			//m_terrainType = TERRAIN_MOUNTAINS_RIVERS_VOLCANO;
			m_terrainType = choices[rand.Int32(9)];
			m_colorType = COLOR_TFPOOR;
		} else if ((body->m_volatileLiquid < fixed(1,10)) &&
		           (body->m_volatileGas > fixed(1,5))) {
			const enum TerrainFractal choices[] = {
				TERRAIN_H2O_SOLID,
				TERRAIN_RUGGED_DESERT,
				TERRAIN_RUGGED_LAVA,
				TERRAIN_MOUNTAINS_VOLCANO,
				TERRAIN_MOUNTAINS_RIVERS_VOLCANO,
			};
			m_terrainType = choices[rand.Int32(5)];
			m_colorType = COLOR_DESERT;
		} else if ((body->m_volatileLiquid > fixed(1,10)) &&  
		           (body->m_volatileGas < fixed(1,10))) {
					   // Planet with no atmosphere should not have liquid water.
			const enum TerrainFractal choices[] = {
				TERRAIN_HILLS_CRATERS,
				TERRAIN_MOUNTAINS_CRATERS,
				TERRAIN_H2O_SOLID,
				TERRAIN_RUGGED_DESERT,
			};
			m_terrainType = choices[rand.Int32(4)];
			m_colorType = COLOR_ICEWORLD;
		} else if (body->m_volcanicity > fixed(7,10)) {
					   // Volcanic world
			m_terrainType = TERRAIN_RUGGED_LAVA;
			m_colorType = COLOR_VOLCANIC;
		} else if (body->m_volatileGas > fixed(1,10)) {
				const enum TerrainFractal choices[] = {
				TERRAIN_HILLS_NORMAL,
				TERRAIN_MOUNTAINS_NORMAL,
				TERRAIN_RUGGED_DESERT,
			};
			m_terrainType = choices[rand.Int32(3)];
			m_colorType = COLOR_ROCK;
		} else if (body->m_volatileGas > fixed(1,20)) {
				const enum TerrainFractal choices[] = {
				TERRAIN_HILLS_CRATERS,
				TERRAIN_MOUNTAINS_CRATERS,
				TERRAIN_H2O_SOLID,
			};
			m_terrainType = choices[rand.Int32(3)];
			m_colorType = COLOR_ROCK;
		} else {
			const enum TerrainFractal choices[] = {
				TERRAIN_HILLS_CRATERS2,
				TERRAIN_MOUNTAINS_CRATERS2,
			};
			m_terrainType = choices[rand.Int32(2)];
			m_colorType = COLOR_ROCK;
		}
	}
	// XXX override the above so you can test particular fractals XXX

	//m_terrainType = TERRAIN_RUGGED_DESERT;
	//m_colorType = COLOR_DESERT;

	m_sealevel = Clamp(body->m_volatileLiquid.ToDouble(), 0.0, 1.0);
	m_icyness = Clamp(body->m_volatileIces.ToDouble(), 0.0, 1.0);
	m_volcanic = Clamp(body->m_volcanicity.ToDouble(), 0.0, 1.0); // height scales with volcanicity as well

	const double rad = body->GetRadius();
	m_maxHeightInMeters = std::max(100.0, (9000.0*rad*rad*(m_volcanic+0.5)) / (body->GetMass() * 6.64e-12));
	if (!isfinite(m_maxHeightInMeters)) m_maxHeightInMeters = rad * 0.5;
	//             ^^^^ max mountain height for earth-like planet (same mass, radius)
	// and then in sphere normalized jizz
	m_maxHeight = std::min(0.5, m_maxHeightInMeters / rad);
	//printf("%s: max terrain height: %fm [%f]\n", body->name.c_str(), m_maxHeightInMeters, m_maxHeight);
	m_invMaxHeight = 1.0 / m_maxHeight;
	m_planetRadius = rad;
	m_planetEarthRadii = rad / EARTH_RADIUS;

	// Pick some colors
	for (int i=0; i<12; i++) m_entropy[i] = rand.Double();
	for (int i=0; i<8; i++) {
		double r,g,b;
		r = rand.Double(0.5, 1.0);
		g = rand.Double(0.5, r);
		b = rand.Double(0.5, std::min(r, g));
		//r = std::min(1.0, r + body->m_metallicity.ToFloat());
		r = std::max(b, r * body->m_metallicity.ToFloat());
		g = std::max(b, g * body->m_metallicity.ToFloat());
		m_rockColor[i] = vector3d(r, g, b);
	}

	// Pick some darker colours
	for (int i=0; i<12; i++) m_entropy[i] = rand.Double();
	for (int i=0; i<8; i++) {
		double r,g,b;
		r = rand.Double(0.05, 0.3);
		g = rand.Double(0.05, r);
		b = rand.Double(0.05, std::min(r, g));
		//r = std::min(1.0, r + body->m_metallicity.ToFloat());
		r = std::max(b, r * body->m_metallicity.ToFloat());
		g = std::max(b, g * body->m_metallicity.ToFloat());
		m_darkrockColor[i] = vector3d(r, g, b);
	}

	// might not be needed now
	for (int i=0; i<12; i++) m_entropy[i] = rand.Double();
	for (int i=0; i<8; i++) {
		double g;
		g = rand.Double(0.3, 0.8);
		m_greyrockColor[i] = vector3d(g, g, g);
	}

	PickAtmosphere(body);
	InitHeightMap(body);
	//fprintf(stderr, "picked terrain %d, colortype %d for %s\n", (int)m_terrainType, (int)m_colorType, body->name.c_str());
	InitFractalType(rand);
}

/**
 * Feature width means roughly one perlin noise blob or grain.
 * This will end up being one hill, mountain or continent, roughly.
 */
void GeoSphereStyle::SetFracDef(struct fracdef_t *def, double featureHeightMeters, double featureWidthMeters, MTRand &rand, double smallestOctaveMeters)
{
	// feature 
	def->amplitude = featureHeightMeters / (m_maxHeight * m_planetRadius);
	def->frequency = m_planetRadius / featureWidthMeters;
	def->octaves = std::max(1, (int)ceil(log(featureWidthMeters / smallestOctaveMeters) / log(2.0)));
	def->lacunarity = 2.0;
	//printf("%d octaves\n", def->octaves);
}

void GeoSphereStyle::InitFractalType(MTRand &rand)
{
	if (m_heightMap) {
		SetFracDef(&m_fracdef[0], m_maxHeightInMeters, 10000.0, rand);
		return;
	}
/*	
		case CONTINENT_VOLCANIC_MARE:
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, rand.Double(5e5,1.5e6), rand, 1e5);
			break;
*/
	switch (m_terrainType) {
		case TERRAIN_ASTEROID:
			//m_maxHeight = rand.Double(0.2,0.4);
			//m_invMaxHeight = 1.0 / m_maxHeight;
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, m_planetRadius, rand);
			// craters
			SetFracDef(&m_fracdef[1], 5000.0, 1000000.0, rand, 1000.0);
			break;
		case TERRAIN_HILLS_NORMAL:
		case TERRAIN_HILLS_RIDGED:
		case TERRAIN_HILLS_RIVERS:
		{
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, rand.Double(1e6,1e7), rand);
			double height = m_maxHeightInMeters*0.7;
			SetFracDef(&m_fracdef[1], height, rand.Double(4.0, 20.0)*height, rand);
			SetFracDef(&m_fracdef[2], m_maxHeightInMeters, rand.Double(50.0, 100.0)*m_maxHeightInMeters, rand);
			break;
		}
		case TERRAIN_HILLS_CRATERS:
		{
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, rand.Double(1e6,1e7), rand);
			double height = m_maxHeightInMeters*0.3;
			SetFracDef(&m_fracdef[1], height, rand.Double(4.0, 20.0)*height, rand);
			SetFracDef(&m_fracdef[2], m_maxHeightInMeters, rand.Double(50.0, 100.0)*m_maxHeightInMeters, rand);
			SetFracDef(&m_fracdef[3], m_maxHeightInMeters*0.07, 1e6, rand, 20.0);
			SetFracDef(&m_fracdef[4], m_maxHeightInMeters*0.05, 8e5, rand, 10.0);
			break;
		}
		case TERRAIN_HILLS_CRATERS2:
		{
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, rand.Double(1e6,1e7), rand);
			double height = m_maxHeightInMeters*0.6;
			SetFracDef(&m_fracdef[1], height, rand.Double(4.0, 20.0)*height, rand);
			SetFracDef(&m_fracdef[2], m_maxHeightInMeters, rand.Double(50.0, 100.0)*m_maxHeightInMeters, rand);
			SetFracDef(&m_fracdef[3], m_maxHeightInMeters*0.07, 11e5, rand, 100.0);
			SetFracDef(&m_fracdef[4], m_maxHeightInMeters*0.05, 98e4, rand, 80.0);
			SetFracDef(&m_fracdef[5], m_maxHeightInMeters*0.05, 1e6, rand, 40.0);
			SetFracDef(&m_fracdef[6], m_maxHeightInMeters*0.04, 99e4, rand, 20.0);
			SetFracDef(&m_fracdef[7], m_maxHeightInMeters*0.05, 12e5, rand, 10.0);
			SetFracDef(&m_fracdef[8], m_maxHeightInMeters*0.04, 9e5, rand, 10.0);
			break;
		}
		case TERRAIN_MOUNTAINS_NORMAL:
		{
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, rand.Double(1e6,1e7), rand);
			double height = m_maxHeightInMeters*0.15;
			SetFracDef(&m_fracdef[1], height, rand.Double(4.0, 20.0)*height, rand);
			SetFracDef(&m_fracdef[2], m_maxHeightInMeters, rand.Double(50.0, 100.0)*m_maxHeightInMeters, rand);

			height = m_maxHeightInMeters*0.3;
			SetFracDef(&m_fracdef[4], m_maxHeightInMeters, rand.Double(100.0, 200.0)*m_maxHeightInMeters, rand);
			SetFracDef(&m_fracdef[3], height, rand.Double(2.5,3.5)*height, rand);
			break;
		}
		case TERRAIN_MOUNTAINS_RIDGED:
		{
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, rand.Double(1e6,1e7), rand);
			double height = m_maxHeightInMeters*0.15;
			SetFracDef(&m_fracdef[1], height, rand.Double(4.0, 20.0)*height, rand);
			SetFracDef(&m_fracdef[2], m_maxHeightInMeters, rand.Double(50.0, 100.0)*m_maxHeightInMeters, rand);

			height = m_maxHeightInMeters*0.4;
			SetFracDef(&m_fracdef[4], m_maxHeightInMeters, rand.Double(100.0, 200.0)*m_maxHeightInMeters, rand);
			SetFracDef(&m_fracdef[3], height, rand.Double(6.0,8.0)*height, rand);
			break;
		}
		case TERRAIN_MOUNTAINS_RIVERS:
		{
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, rand.Double(1e6,1e7), rand);
			// XXX looks too flat and crappy
			double height = m_maxHeightInMeters*0.2;
			SetFracDef(&m_fracdef[1], height, rand.Double(4.0, 20.0)*height, rand);
			SetFracDef(&m_fracdef[2], m_maxHeightInMeters, rand.Double(50.0, 100.0)*m_maxHeightInMeters, rand);

			height = m_maxHeightInMeters*0.5;
			SetFracDef(&m_fracdef[4], m_maxHeightInMeters, rand.Double(100.0, 200.0)*m_maxHeightInMeters, rand);
			SetFracDef(&m_fracdef[3], height, rand.Double(6.0,8.0)*height, rand);
			break;
		}
		case TERRAIN_MOUNTAINS_CRATERS:
		{
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, rand.Double(1e6,1e7), rand);
			double height = m_maxHeightInMeters*0.3;
			SetFracDef(&m_fracdef[1], height, rand.Double(4.0, 20.0)*height, rand);
			SetFracDef(&m_fracdef[2], m_maxHeightInMeters, rand.Double(50.0, 100.0)*m_maxHeightInMeters, rand);

			height = m_maxHeightInMeters*0.3;
			SetFracDef(&m_fracdef[4], m_maxHeightInMeters, rand.Double(100.0, 200.0)*m_maxHeightInMeters, rand);
			SetFracDef(&m_fracdef[3], height, rand.Double(2.5,3.5)*height, rand);

			SetFracDef(&m_fracdef[5], m_maxHeightInMeters*0.05, 8e5, rand, 10.0);
			SetFracDef(&m_fracdef[6], m_maxHeightInMeters*0.05, 1e6, rand, 10.0);
			break;
		}
		case TERRAIN_MOUNTAINS_CRATERS2:
		{
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, rand.Double(1e6,1e7), rand);
			double height = m_maxHeightInMeters*0.5;
			SetFracDef(&m_fracdef[1], height, rand.Double(4.0, 200.0)*height, rand, 10);
			SetFracDef(&m_fracdef[2], m_maxHeightInMeters, rand.Double(50.0, 100.0)*m_maxHeightInMeters, rand);

			height = m_maxHeightInMeters*0.4;
			SetFracDef(&m_fracdef[4], m_maxHeightInMeters, rand.Double(100.0, 200.0)*m_maxHeightInMeters, rand);
			SetFracDef(&m_fracdef[3], height, rand.Double(2.5,3.5)*height, rand);
			SetFracDef(&m_fracdef[5], m_maxHeightInMeters*0.05, 1e6, rand, 100.0);
			SetFracDef(&m_fracdef[6], m_maxHeightInMeters*0.04, 9e5, rand, 50.0);
			SetFracDef(&m_fracdef[7], m_maxHeightInMeters*0.05, 8e5, rand, 20.0);
			SetFracDef(&m_fracdef[8], m_maxHeightInMeters*0.04, 11e5, rand, 10.0);
			SetFracDef(&m_fracdef[9], m_maxHeightInMeters*0.07, 12e5, rand, 10.0);
			break;
		}
		case TERRAIN_MOUNTAINS_VOLCANO:  
		{
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, rand.Double(1e6,1e7), rand);
			double height = m_maxHeightInMeters*0.8;
			SetFracDef(&m_fracdef[1], m_maxHeightInMeters, rand.Double(50.0, 100.0)*m_maxHeightInMeters, rand);
			SetFracDef(&m_fracdef[2], height, rand.Double(4.0, 20.0)*height, rand);
			SetFracDef(&m_fracdef[3], height, rand.Double(12.0, 200.0)*height, rand);

			height = m_maxHeightInMeters*0.7;
			SetFracDef(&m_fracdef[4], m_maxHeightInMeters, rand.Double(100.0, 200.0)*m_maxHeightInMeters, rand);
			SetFracDef(&m_fracdef[5], height, rand.Double(2.5,3.5)*height, rand);
			SetFracDef(&m_fracdef[6], height, rand.Double(2.5,3.5)*height, rand);
			// volcano
			SetFracDef(&m_fracdef[7], 20000.0, 5000000.0, rand, 10.0);

			// canyons 
			SetFracDef(&m_fracdef[8], m_maxHeightInMeters*0.5, 2e6, rand, 10.0);
			//SetFracDef(&m_fracdef[9], m_maxHeightInMeters*0.1, 1.5e6, rand, 100.0);
			//SetFracDef(&m_fracdef[10], m_maxHeightInMeters*0.1, 2e6, rand, 100.0);
			break;
		}
		case TERRAIN_MOUNTAINS_RIVERS_VOLCANO:  //old terraformed mars terrain 
		{
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, rand.Double(1e6,1e7), rand);
			double height = m_maxHeightInMeters*0.6;
			SetFracDef(&m_fracdef[1], m_maxHeightInMeters, rand.Double(50.0, 100.0)*m_maxHeightInMeters, rand);
			SetFracDef(&m_fracdef[2], height, rand.Double(4.0, 20.0)*height, rand);
			SetFracDef(&m_fracdef[3], m_maxHeightInMeters, rand.Double(120.0, 2000.0)*m_maxHeightInMeters, rand, 20);

			height = m_maxHeightInMeters*0.3;
			SetFracDef(&m_fracdef[4], m_maxHeightInMeters, rand.Double(100.0, 200.0)*m_maxHeightInMeters, rand);
			SetFracDef(&m_fracdef[5], height, rand.Double(2.5,3.5)*height, rand);
			SetFracDef(&m_fracdef[6], height, rand.Double(2.5,3.5)*height, rand);
			// volcano
			SetFracDef(&m_fracdef[7], 20000.0, 5000000.0, rand, 10.0);

			// canyons and rivers
			SetFracDef(&m_fracdef[8], m_maxHeightInMeters*1.0, 4e6, rand, 10.0);
			SetFracDef(&m_fracdef[9], m_maxHeightInMeters*1.0, 5e6, rand, 10.0);
			//SetFracDef(&m_fracdef[10], m_maxHeightInMeters*0.5, 2e6, rand, 100.0);
			break;
		}
		case TERRAIN_RUGGED_LAVA:
		{
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, rand.Double(1e6,1e7), rand);
			double height = m_maxHeightInMeters*1.0;
			SetFracDef(&m_fracdef[1], m_maxHeightInMeters, rand.Double(50.0, 100.0)*m_maxHeightInMeters, rand);
			SetFracDef(&m_fracdef[2], height, rand.Double(4.0, 20.0)*height, rand);
			SetFracDef(&m_fracdef[3], height, rand.Double(12.0, 200.0)*height, rand);

			height = m_maxHeightInMeters*0.3;
			SetFracDef(&m_fracdef[4], m_maxHeightInMeters, rand.Double(100.0, 200.0)*m_maxHeightInMeters, rand);
			SetFracDef(&m_fracdef[5], height, rand.Double(2.5,3.5)*height, rand);

			// volcanoes
			SetFracDef(&m_fracdef[6], 20000.0, 5e6, rand, 5.0);
			SetFracDef(&m_fracdef[7], m_maxHeightInMeters, 3e6, rand, 5.0);

			// canyon
			SetFracDef(&m_fracdef[8], m_maxHeightInMeters*0.4, 4e6, rand, 10.0);
			// bumps/rocks
			SetFracDef(&m_fracdef[9], height*0.015, rand.Double(1,20), rand, 8.0);
			break;
		}
		case TERRAIN_H2O_SOLID:
		{
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, rand.Double(1e6,1e7), rand);
			double height = m_maxHeightInMeters*0.3;
			SetFracDef(&m_fracdef[1], height, rand.Double(4.0, 20.0)*height, rand, 10);
			SetFracDef(&m_fracdef[2], m_maxHeightInMeters, rand.Double(50.0, 100.0)*m_maxHeightInMeters, rand);

			height = m_maxHeightInMeters*0.5;
			SetFracDef(&m_fracdef[4], m_maxHeightInMeters, rand.Double(100.0, 200.0)*m_maxHeightInMeters, rand);
			SetFracDef(&m_fracdef[3], height, rand.Double(2.5,3.5)*height, rand);
			// craters
			SetFracDef(&m_fracdef[5], m_maxHeightInMeters*0.05, 1e6, rand, 50.0);
			SetFracDef(&m_fracdef[6], m_maxHeightInMeters*0.04, 8e5, rand, 20.0);

			// canyon
			SetFracDef(&m_fracdef[7], m_maxHeightInMeters*0.2, 1e5, rand, 10.0);
			SetFracDef(&m_fracdef[8], m_maxHeightInMeters*0.3, 2.5e6, rand, 10.0);
			SetFracDef(&m_fracdef[9], m_maxHeightInMeters*0.4, 3e6, rand, 10.0);
			// so dan, the above means: make a fractal with 40% of the planet maximum
			// terrain height, feature size of around 2000km, and detail down to
			// 50.0 meters. SetFracDef will choose the right p multiplier and octaves - Tom
			break;
		}
		case TERRAIN_RUGGED_DESERT:
		{
			SetFracDef(&m_fracdef[0], 0.1*m_maxHeightInMeters, 2e6, rand, 180e3);
			double height = m_maxHeightInMeters*0.9;
			SetFracDef(&m_fracdef[1], height, rand.Double(120.0, 10000.0)*height, rand, 10);
			SetFracDef(&m_fracdef[2], m_maxHeightInMeters, rand.Double(1.0, 2.0)*m_maxHeightInMeters, rand);

			height = m_maxHeightInMeters*0.3;
			SetFracDef(&m_fracdef[3], height, rand.Double(20.0,240.0)*height, rand);
			SetFracDef(&m_fracdef[4], m_maxHeightInMeters, rand.Double(1.0, 2.0)*m_maxHeightInMeters, rand);
			// dunes
			height = m_maxHeightInMeters*0.2;
			SetFracDef(&m_fracdef[5], height*0.1, rand.Double(5,75)*height, rand, 1000.0);
			// canyon
			SetFracDef(&m_fracdef[6], m_maxHeightInMeters*0.2, 1e6, rand, 20.0);
			SetFracDef(&m_fracdef[7], m_maxHeightInMeters*0.35, 1.5e6, rand, 10.0);
			SetFracDef(&m_fracdef[8], m_maxHeightInMeters*0.2, 3e6, rand, 10.0);

			//SetFracDef(&m_fracdef[9], m_maxHeightInMeters*0.1, 100, rand, 10.0);
			// adds bumps to the landscape
			SetFracDef(&m_fracdef[9], height*0.005, rand.Double(1,10), rand, 10.0);
		}
	}
}

/*
 * Must return >= 0.0
 */
double GeoSphereStyle::GetHeight(const vector3d &p)
{
	if (m_heightMap) return GetHeightMapVal(p) / m_planetRadius;

	double continents;

	switch (m_terrainType) {
		case TERRAIN_NONE:
		case TERRAIN_GASGIANT:
			return 0;
		case TERRAIN_ASTEROID:
		{
			return std::max(0.0, m_maxHeight * (octavenoise(m_fracdef[0], 0.5, p) + 
					m_fracdef[1].amplitude * crater_function(m_fracdef[1], p)));
		}
		case TERRAIN_HILLS_NORMAL:
		{
			continents = octavenoise(m_fracdef[0], 0.5, p) - m_sealevel;
			if (continents < 0) return 0;
			double out = 0.3 * continents;
			double distrib = octavenoise(m_fracdef[2], 0.5, p);
			double m = m_fracdef[1].amplitude * octavenoise(m_fracdef[1], 0.5*distrib, p);
			// cliffs at shore
			if (continents < 0.001) out += m * continents * 1000.0f;
			else out += m;
			return m_maxHeight * out;
		}
		case TERRAIN_HILLS_RIDGED:
		{
			continents = octavenoise(m_fracdef[0], 0.5, p) - m_sealevel;
			if (continents < 0) return 0;
			// == TERRAIN_HILLS_NORMAL except ridged_octavenoise
			double out = 0.3 * continents;
			double distrib = ridged_octavenoise(m_fracdef[2], 0.5, p);
			double m = m_fracdef[1].amplitude * ridged_octavenoise(m_fracdef[1], 0.5*distrib, p);
			// cliffs at shore
			if (continents < 0.001) out += m * continents * 1000.0f;
			else out += m;
			return m_maxHeight * out;
		}
		case TERRAIN_HILLS_RIVERS:
		{
			continents = octavenoise(m_fracdef[0], 0.5, p) - m_sealevel;
			if (continents < 0) return 0;
			// == TERRAIN_HILLS_NORMAL except river_octavenoise
			double out = 0.3 * continents;
			double distrib = river_octavenoise(m_fracdef[2], 0.5, p);
			double m = m_fracdef[1].amplitude * river_octavenoise(m_fracdef[1], 0.5*distrib, p);
			// cliffs at shore
			if (continents < 0.001) out += m * continents * 1000.0f;
			else out += m;
			return m_maxHeight * out;
		}
		case TERRAIN_HILLS_CRATERS:
		{
			continents = octavenoise(m_fracdef[0], 0.5, p) - m_sealevel;
			if (continents < 0) return 0;
			// == TERRAIN_HILLS_NORMAL except river_octavenoise
			double out = 0.3 * continents;
			double distrib = river_octavenoise(m_fracdef[2], 0.5, p);
			double m = m_fracdef[1].amplitude * river_octavenoise(m_fracdef[1], 0.5*distrib, p);
			// cliffs at shore
			if (continents < 0.001) out += m * continents * 1000.0f;
			else out += m;
			out += crater_function(m_fracdef[3], p);
			out += crater_function(m_fracdef[4], p);
			return m_maxHeight * out;
		}
		case TERRAIN_HILLS_CRATERS2:
		{
			continents = octavenoise(m_fracdef[0], 0.5, p) - m_sealevel;
			if (continents < 0) return 0;
			// == TERRAIN_HILLS_NORMAL except river_octavenoise
			double out = 0.3 * continents;
			double distrib = river_octavenoise(m_fracdef[2], 0.5, p);
			double m = m_fracdef[1].amplitude * river_octavenoise(m_fracdef[1], 0.5*distrib, p);
			// cliffs at shore
			if (continents < 0.001) out += m * continents * 1000.0f;
			else out += m;
			out += crater_function(m_fracdef[3], p);
			out += crater_function(m_fracdef[4], p);
			out += crater_function(m_fracdef[5], p);
			out += crater_function(m_fracdef[6], p);
			out += crater_function(m_fracdef[7], p);
			out += crater_function(m_fracdef[8], p);
			return m_maxHeight * out;
		}
		case TERRAIN_MOUNTAINS_NORMAL:
		{
			continents = octavenoise(m_fracdef[0], 0.5, p) - m_sealevel;
			if (continents < 0) return 0;
			double out = 0.3 * continents;
			double m = 0;//m_fracdef[1].amplitude * octavenoise(m_fracdef[1], 0.5, p);
			double distrib = ridged_octavenoise(m_fracdef[3], 0.5, p);
			m += m_fracdef[3].amplitude * octavenoise(m_fracdef[4], 0.5*distrib, p);
			// cliffs at shore
			if (continents < 0.001) out += m * continents * 1000.0f;
			else out += m;
			return m_maxHeight * out;
		}
		case TERRAIN_MOUNTAINS_RIDGED:
		{
			continents = octavenoise(m_fracdef[0], 0.5, p) - m_sealevel;
			if (continents < 0) return 0;
			double out = 0.3 * continents;
			double m = m_fracdef[1].amplitude * ridged_octavenoise(m_fracdef[1], 0.5, p);
			double distrib = ridged_octavenoise(m_fracdef[4], 0.5, p);
			m += m_fracdef[3].amplitude * ridged_octavenoise(m_fracdef[3], 0.5*distrib, p);
			// cliffs at shore
			if (continents < 0.001) out += m * continents * 1000.0f;
			else out += m;
			return m_maxHeight * out;
		}
		case TERRAIN_MOUNTAINS_RIVERS:
		{
			continents = octavenoise(m_fracdef[0], 0.5, p) - m_sealevel;
			if (continents < 0) return 0;
			double out = 0.3 * continents;
			double m = m_fracdef[1].amplitude * river_octavenoise(m_fracdef[1], 0.5, p);
			double distrib = river_octavenoise(m_fracdef[4], 0.5, p);
			if (distrib > 0.5) m += 2.0 * (distrib-0.5) * m_fracdef[3].amplitude * river_octavenoise(m_fracdef[3], 0.5, p);
			// cliffs at shore
			if (continents < 0.001) out += m * continents * 1000.0f;
			else out += m;
			return m_maxHeight * out;
		}
		case TERRAIN_MOUNTAINS_CRATERS:
		{
			continents = octavenoise(m_fracdef[0], 0.5, p) - m_sealevel;
			if (continents < 0) return 0;
			double out = 0.3 * continents;
			double m = m_fracdef[1].amplitude * ridged_octavenoise(m_fracdef[1], 0.5, p);
			double distrib = ridged_octavenoise(m_fracdef[4], 0.5, p);
			if (distrib > 0.5) m += 2.0 * (distrib-0.5) * m_fracdef[3].amplitude * ridged_octavenoise(m_fracdef[3], 0.5*distrib, p);
			// cliffs at shore
			if (continents < 0.001) out += m * continents * 1000.0f;
			else out += m;
			out += crater_function(m_fracdef[5], p);
			out += crater_function(m_fracdef[6], p);
			return m_maxHeight * out;
		}
		case TERRAIN_MOUNTAINS_CRATERS2:
		{
			continents = octavenoise(m_fracdef[0], 0.5, p) - m_sealevel;
			if (continents < 0) return 0;
			double out = 0.3 * continents;
			double m = 0;//m_fracdef[1].amplitude * octavenoise(m_fracdef[1], 0.5, p);
			double distrib = ridged_octavenoise(m_fracdef[3], 0.5, p);
			if (distrib > 0.5) m += 2.0 * (distrib-0.5) * m_fracdef[3].amplitude * octavenoise(m_fracdef[4], 0.5*distrib, p);
			// cliffs at shore
			if (continents < 0.001) out += m * continents * 1000.0f;
			else out += m;
			out += crater_function(m_fracdef[5], p);
			out += crater_function(m_fracdef[6], p);
			out += crater_function(m_fracdef[7], p);
			out += crater_function(m_fracdef[8], p);
			out += crater_function(m_fracdef[9], p);
			return m_maxHeight * out;
		}
		case TERRAIN_MOUNTAINS_VOLCANO:
		{
			double continents = octavenoise(m_fracdef[0], 0.5, p) - m_sealevel;
			if (continents < 0) return 0;
			double mountain_distrib = octavenoise(m_fracdef[1], 0.5, p);
			double mountains = octavenoise(m_fracdef[2], 0.5, p);
			double mountains2 = octavenoise(m_fracdef[3], 0.5, p);
			double hill_distrib = octavenoise(m_fracdef[4], 0.5, p);
			double hills = hill_distrib * m_fracdef[5].amplitude * octavenoise(m_fracdef[5], 0.5, p);
			double hills2 = hill_distrib * m_fracdef[6].amplitude * octavenoise(m_fracdef[6], 0.5, p);



			double n = continents - (m_fracdef[0].amplitude*m_sealevel);

			
			if (n < 0.01) n += megavolcano_function(m_fracdef[7], p) * n * 3000.0f;
			else n += megavolcano_function(m_fracdef[7], p) * 30.0f;

			n = (n > 0.0 ? n : 0.0); 

			if (n < .2f) n += canyon3_function(m_fracdef[8], p) * n * 2;
			else if (n < .4f) n += canyon3_function(m_fracdef[8], p) * .4;
			else n += canyon3_function(m_fracdef[8], p) * (.4/n) * .4;

			//if (n < .2) n += canyon2_function(m_fracdef[9], p) * n ;
			//else if (n < .4) n += canyon2_function(m_fracdef[9], p) * .2f;
			//else n += canyon2_function(m_fracdef[9], p) * (.4f/n) * .2f;

			//if (n < .2) n += river_function(m_fracdef[9], p) * n * 5.0f;
			//else if (n < .4) n += river_function(m_fracdef[9], p);
			//else n += river_function(m_fracdef[9], p) * (.4f/n);

			n += -0.05f;
			n = (n > 0.0 ? n : 0.0); 

			n = n*.01f;

			if (n > 0.0) {
				// smooth in hills at shore edges
				if (n < 0.01) n += hills * n * 100.0f;
				else n += hills;
				if (n < 0.02) n += hills2 * n * 50.0f;
				else n += hills2 * (0.02f/n);

				mountains  = octavenoise(m_fracdef[1], 0.5, p) *
					m_fracdef[2].amplitude * mountains*mountains*mountains;
				mountains2 = octavenoise(m_fracdef[4], 0.5, p) *
					m_fracdef[3].amplitude * mountains2*mountains2*mountains2;
				if (n > 2.5) n += mountains2 * (n - 2.5) * 0.6f;
				if (n < 0.01) n += mountains * n * 60.0f ;
				else n += mountains * 0.6f ; 
			}
			
			n = m_maxHeight*n;
			return (n > 0.0 ? n : 0.0); 
		}
		case TERRAIN_MOUNTAINS_RIVERS_VOLCANO:
		{
			double continents = octavenoise(m_fracdef[0], 0.5, p) - m_sealevel;
			if (continents < 0) return 0;
			double mountain_distrib = octavenoise(m_fracdef[1], 0.5, p);
			double mountains = octavenoise(m_fracdef[2], 0.5, p);
			double mountains2 = octavenoise(m_fracdef[3], 0.5, p);
			double hill_distrib = octavenoise(m_fracdef[4], 0.5, p);
			double hills = hill_distrib * m_fracdef[5].amplitude * octavenoise(m_fracdef[5], 0.5, p);
			double hills2 = hill_distrib * m_fracdef[6].amplitude * octavenoise(m_fracdef[6], 0.5, p);



			double n = continents - (m_fracdef[0].amplitude*m_sealevel);

			
			if (n < 0.01) n += megavolcano_function(m_fracdef[7], p) * n * 800.0f;
			else n += megavolcano_function(m_fracdef[7], p) * 8.0f;

			//n = (n > 0.0 ? n : 0.0); 
			//n = n*.1f;

			if (n < .2f) n += canyon3_function(m_fracdef[8], p) * n * 5.0f;
			else if (n < .4f) n += canyon3_function(m_fracdef[8], p) ;
			else n += canyon3_function(m_fracdef[8], p) * (.4f/n) ;

			if (n < .2f) n += canyon2_function(m_fracdef[8], p) * n * 5.0f;
			else if (n < .4f) n += canyon2_function(m_fracdef[8], p) ;
			else n += canyon2_function(m_fracdef[8], p) * (.4f/n) ;

			if (n < .2f) n += river_function(m_fracdef[8], p) * n * 5.0f;
			else if (n < .4f) n += river_function(m_fracdef[8], p);
			else n += river_function(m_fracdef[8], p) * (.4f/n);

			if (n < .2f) n += canyon3_function(m_fracdef[9], p) * n * 5.0f;
			else if (n < .4f) n += canyon3_function(m_fracdef[9], p) ;
			else n += canyon3_function(m_fracdef[9], p) * (.4f/n) ;

			if (n < .2f) n += canyon2_function(m_fracdef[9], p) * n * 5.0f;
			else if (n < .4f) n += canyon2_function(m_fracdef[9], p) ;
			else n += canyon2_function(m_fracdef[9], p) * (.4f/n) ;

			if (n < .2f) n += river_function(m_fracdef[9], p) * n * 5.0f;
			else if (n < .4f) n += river_function(m_fracdef[9], p);
			else n += river_function(m_fracdef[9], p) * (.4f/n);

			n += -1.0f;
			n = (n > 0.0 ? n : 0.0); 

			n = n*.03f;

			//n += continents - (m_fracdef[0].amplitude*m_sealevel);

			if (n > 0.0) {
				// smooth in hills at shore edges
				if (n < 0.1) n += hills * n * 10.0f;
				else n += hills;
				if (n < 0.05) n += hills2 * n * 20.0f;
				else n += hills2 ;

				mountains  = octavenoise(m_fracdef[1], 0.5, p) *
					m_fracdef[2].amplitude * mountains*mountains*mountains;
				mountains2 = octavenoise(m_fracdef[4], 0.5, p) *
					m_fracdef[3].amplitude * mountains2*mountains2*mountains2;
				if (n > 0.5) n += mountains2 * (n - 0.5) ;
				if (n < 0.2) n += mountains * n * 5.0f ;
				else n += mountains  ; 
			}
			
			n = m_maxHeight*n;
			return (n > 0.0 ? n : 0.0); 
		}
		case TERRAIN_RUGGED_LAVA:
		{
			double continents = octavenoise(m_fracdef[0], 0.5, p) - m_sealevel;
			if (continents < 0) return 0;
			double mountain_distrib = octavenoise(m_fracdef[1], 0.5, p);
			double mountains = octavenoise(m_fracdef[2], 0.5, p);
			double mountains2 = octavenoise(m_fracdef[3], 0.5, p);
			double hill_distrib = octavenoise(m_fracdef[4], 0.5, p);
			double hills = hill_distrib * m_fracdef[5].amplitude * octavenoise(m_fracdef[5], 0.5, p);
			double rocks = octavenoise(m_fracdef[9], 0.5, p);

			
			double n = continents - (m_fracdef[0].amplitude*m_sealevel);
			//double n = (megavolcano_function(p) + volcano_function(p) + smlvolcano_function(p));
			n += megavolcano_function(m_fracdef[6], p);
			n += volcano_function(m_fracdef[6], p);
			
			n += megavolcano_function(m_fracdef[7], p);
			n += volcano_function(m_fracdef[7], p);

			
			//n += 1.4*(continents - targ.continents.amplitude*targ.sealevel + (volcano_function(p)*1)) ;
			//smooth canyon transitions and limit height of canyon placement
			if (n < .01) n += n * 100.0f * canyon3_function(m_fracdef[8], p);
			else if (n < .7) n += canyon3_function(m_fracdef[8], p);
			else n += canyon3_function(m_fracdef[8], p);

			if (n < .01) n += n * 100.0f * canyon2_function(m_fracdef[8], p);
			else if (n < .7) n += canyon2_function(m_fracdef[8], p);
			else n += canyon2_function(m_fracdef[8], p);
			n = n*.3f;

			n += hills ;

			mountains  = octavenoise(m_fracdef[1], 0.5, p) *
					m_fracdef[2].amplitude * mountains*mountains*mountains;
			mountains2 = octavenoise(m_fracdef[4], 0.5, p) *
					m_fracdef[3].amplitude * mountains2*mountains2*mountains2;
			/*mountains = fractal(2, targ.mountainDistrib, (m_seed>>2)&3, p) *
				targ.mountains.amplitude * mountains*mountains*mountains;
			mountains2 = fractal(24, targ.mountainDistrib, (m_seed>>2)&3, p) *
				targ.mountains.amplitude * mountains*mountains*mountains;*/
				
			n += mountains ;
			if (n < 0.01) n += mountains2 * n * 40.0f ;
			else n += mountains2*.4f ;

			rocks = mountain_distrib * m_fracdef[9].amplitude * rocks*rocks*rocks;
			n += rocks ;
		
			n = (n<0.0 ? 0.0 : m_maxHeight*n);
			return n;
		}
		case TERRAIN_H2O_SOLID:
		{
			double continents = m_fracdef[0].amplitude * octavenoise(m_fracdef[0], 0.5, p);
			double mountains = octavenoise(m_fracdef[3], 0.5, p);
			double hills = octavenoise(m_fracdef[2], 0.5, p) *
				m_fracdef[1].amplitude * octavenoise(m_fracdef[1], 0.5, p);

			double n = continents - (m_fracdef[0].amplitude*m_sealevel);
			
			//canyons
			n += canyon_function(m_fracdef[7], p);
			n += canyon2_function(m_fracdef[7], p);
			n += canyon3_function(m_fracdef[7], p);
			n += canyon_function(m_fracdef[8], p);
			n += canyon2_function(m_fracdef[8], p);
			n += canyon3_function(m_fracdef[8], p);
			n += canyon_function(m_fracdef[9], p);
			n += canyon2_function(m_fracdef[9], p);
			n += canyon3_function(m_fracdef[9], p);
			n += -0.5;
			n = n*0.1;
			n = (n<0.0 ? 0.0 : n);

			// craters
			n += crater_function(m_fracdef[5], p);
			n += crater_function(m_fracdef[6], p);


			if (n > 0.0) {
				// smooth in hills at shore edges 
				if (n < 0.01) n += hills * n * 50.0f ;
				else n += hills * .5f ;
				// adds mountains hills craters 
				mountains = octavenoise(m_fracdef[4], 0.5, p) *
					m_fracdef[3].amplitude * mountains*mountains*mountains;
				if (n < 0.01) n += n * 100.0f * mountains;
				else n += mountains;
			}

			//n += (hills * .5f) +  crater_function(m_fracdef[5], p);
			
			n = m_maxHeight*n;
			return (n > 0.0 ? n : 0.0);
		}
		case TERRAIN_RUGGED_DESERT:
		{
			double continents = octavenoise(m_fracdef[0], 0.5, p) - m_sealevel + (cliff_function(m_fracdef[7], p)*0.5);
			if (continents < 0) return 0;
			double mountain_distrib = octavenoise(m_fracdef[2], 0.5, p);
			double mountains = octavenoise(m_fracdef[1], 0.5, p);
			double rocks = octavenoise(m_fracdef[9], 0.5, p);
			double hill_distrib = octavenoise(m_fracdef[4], 0.5, p);
			double hills = hill_distrib * m_fracdef[3].amplitude * octavenoise(m_fracdef[3], 0.5, p);
			double dunes = hill_distrib * m_fracdef[5].amplitude * octavenoise(m_fracdef[5], 0.5, p);

			double n = continents * m_fracdef[0].amplitude * 2;//+ (cliff_function(m_fracdef[7], p)*0.5);
			n += canyon_function(m_fracdef[6], p);
			n += canyon2_function(m_fracdef[6], p);
			n += canyon3_function(m_fracdef[6], p);
			n = (n<1 ? n : 1/n ); 
			n += canyon_function(m_fracdef[7], p);
			n += canyon2_function(m_fracdef[7], p);
			n += canyon3_function(m_fracdef[7], p);
			n += canyon_function(m_fracdef[8], p);
			n += canyon2_function(m_fracdef[8], p);
			n += canyon3_function(m_fracdef[8], p);
			n += -0.5;
			n = n * 0.5;
			n = (n<0.0 ? 0.0 : n);

			// makes larger dunes at lower altitudes, flat ones at high altitude.
			mountains = mountain_distrib * m_fracdef[1].amplitude * mountains*mountains*mountains;
			// smoothes edges of mountains and places them only above a set altitude
			if (n < 0.1) n += n * 10.0f * hills;
			else n += hills;
			if (n > 0.2) n += dunes * (0.2/n);  
			else n += dunes;
			if (n < 0.1) n += n * 10.0f * mountains;
			else n += mountains;	
			
			
			rocks = mountain_distrib * m_fracdef[9].amplitude * rocks*rocks*rocks;
			n += rocks ;
			
			
			//n = (n<0.0 ? 0.0 : m_maxHeight*n);
			n = m_maxHeight*n;
			return n;
		}
	}
}

/* These fuctions should not be used by GeoSphereStyle::GetHeight, so don't move these definitions
   to above that function. GetHeight should use the versions of these functions that take fracdef_t
   objects, ensuring that the resulting terrains have the desired scale */
static inline double octavenoise(int octaves, double roughness, double lacunarity, const vector3d &p);
static inline double river_octavenoise(int octaves, double roughness, double lacunarity, const vector3d &p);
static inline double ridged_octavenoise(int octaves, double roughness, double lacunarity, const vector3d &p);

/**
 * Height: 0.0 would be sea-level. 1.0 would be an extra elevation of 1 radius (huge)
 */
vector3d GeoSphereStyle::GetColor(const vector3d &p, double height, const vector3d &norm)
{
	switch (m_colorType) {
	case COLOR_NONE:
		return vector3d(1.0);
	case COLOR_GG_JUPITER: {
		double n = octavenoise(12, 0.5f*m_entropy[0] + 0.25f, 2.0, noise(vector3d(p.x, p.y*m_planetEarthRadii, p.z))*p);
		n = (1.0 + n)*0.5;
		return interpolate_color(n, vector3d(.50,.22,.18), vector3d(.99,.76,.62));
		}
	case COLOR_GG_SATURN: {
		double n = octavenoise(12, 0.5f*m_entropy[0] + 0.25f, 2.0, noise(vector3d(p.x, p.y*m_planetEarthRadii, p.z))*p);
		n = (1.0 + n)*0.5;
		return interpolate_color(n, vector3d(.69, .53, .43), vector3d(.99, .76, .62));
		}
	case COLOR_GG_URANUS: {
		double n = octavenoise(12, 0.5f*m_entropy[0] + 0.25f, 2.0, noise(vector3d(p.x, p.y*m_planetEarthRadii, p.z))*p);
		n = (1.0 + n)*0.5;
		return interpolate_color(n, vector3d(.63, .76, .77), vector3d(.70,.85,.86));
		}
	case COLOR_GG_NEPTUNE: {
		double n = octavenoise(12, 0.5f*m_entropy[0] + 0.25f, 2.0, noise(vector3d(p.x, p.y*m_planetEarthRadii, p.z))*p);
		n = (1.0 + n)*0.5;
		return interpolate_color(n*n, vector3d(.21, .34, .54), vector3d(.31, .44, .73)); 
		}
	case COLOR_EARTHLIKE:
	{
		double n = m_invMaxHeight*height;
		// water
		if (n <= 0) return vector3d(0,0,0.5);

		const double flatness = pow(p.Dot(norm), 6.0);
		const vector3d color_cliffs = m_rockColor[1];
		// ice on mountains and poles
		if (fabs(m_icyness*p.y) + m_icyness*n > 1) {
			return interpolate_color(flatness, color_cliffs, vector3d(1,1,1));
		}

		double equatorial_desert = (2.0-m_icyness)*(-1.0+2.0*octavenoise(12, 0.5, 2.0, (n*2.0)*p)) *
				1.0*(2.0-m_icyness)*(1.0-p.y*p.y);

		// More sensitive height detection for application of colours
		vector3d col;
		if (n > 0.48) {
		col = interpolate_color(equatorial_desert, m_rockColor[3], vector3d(.86, .75, .48));
		col = interpolate_color(n, col, m_rockColor[7]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.3) {     
		col = interpolate_color(equatorial_desert, vector3d(.36, .29, .2), vector3d(.55,.5,.375));
		col = interpolate_color(n, col, m_rockColor[5]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.18) {                                 
		col = interpolate_color(equatorial_desert, vector3d(.59,.57,.4), vector3d(.86, .75, .38));
		col = interpolate_color(n, col, m_rockColor[2]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.12) {     
		col = interpolate_color(equatorial_desert, vector3d(.15,.1,.0), vector3d(0.6, 0.75, .44));
		col = interpolate_color(n, col, m_rockColor[6]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.08) {      
		col = interpolate_color(equatorial_desert, vector3d(0.27,0.265,0.23), vector3d(0.45, 0.425, .35));
		col = interpolate_color(n, col, m_rockColor[5]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.05) {      
		col = interpolate_color(equatorial_desert, vector3d(0.08,0.06,.0), vector3d(0.55, 0.5, .35));
		col = interpolate_color(n, col, m_rockColor[4]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.02) {       //                       Dark Green         Light Green
		col = interpolate_color(equatorial_desert, vector3d(-1.8,-1.3,-5), vector3d(-1.9, -1.2, -5));
		col = interpolate_color(n, col, vector3d(54,39.75,100));
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.01) {       //                       Brown/yellow
		col = interpolate_color(equatorial_desert, vector3d(1.52, 1.5, 0), vector3d(1.5, 1.52, 0));
		col = interpolate_color(n, col, vector3d(-80,-76,0));
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.005) {       //                       Brown
		col = interpolate_color(equatorial_desert, vector3d(0.11,.2,.0), vector3d(.1, .25, .0));
		col = interpolate_color(n, col, vector3d(42,50.8,0));
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else {                    //                         Yellow
		col = interpolate_color(equatorial_desert, vector3d(0.9,0.84,0), vector3d(.9, .6, .4));
		col = interpolate_color(n, col, vector3d(-125,-98,-159));
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
	}
	case COLOR_DEAD_WITH_H2O: {
		double n = m_invMaxHeight*height;
		if (n <= 0) return vector3d(0.0,0.0,0.5);
		else return interpolate_color(n, vector3d(.2,.2,.2), vector3d(.6,.6,.6));
	}
	case COLOR_ICEWORLD:
	{
		double n = m_invMaxHeight*height;

		if (n <= 0.0) return vector3d(0.96,0.96,0.96);

		const double flatness = pow(p.Dot(norm), 6.0);
		const vector3d color_cliffs = m_rockColor[2];
		// ice on mountains and poles
		//if (fabs(m_icyness*p.y) + m_icyness*n > 18) {
		//	return interpolate_color(flatness, color_cliffs, vector3d(.99,.99,.99));
		//}

		double equatorial_desert = (2.0-m_icyness)*(-1.0+2.0*octavenoise(12, 0.5, 2.0, (n*2.0)*p)) *
				1.0*(2.0-m_icyness)*(1.0-p.y*p.y);
	
		vector3d col;
		if (n > .7) {     
		col = interpolate_color(equatorial_desert, vector3d(1, 1, 1), vector3d(.96, .95, .94));
		col = interpolate_color(n, m_rockColor[1], col);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.3) {  
		col = interpolate_color(equatorial_desert, vector3d(0.19,0.18,.0), vector3d(.2, .2, .1));
		col = interpolate_color(n, col, vector3d(2, 2, 2));
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else {      
		col = interpolate_color(equatorial_desert, m_rockColor[7], m_rockColor[6]);
		col = interpolate_color(n, col, vector3d(0.95,0.85,0));
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
	}
	case COLOR_DESERT:
	{
		double n = m_invMaxHeight*height/2;
		

		const double flatness = pow(p.Dot(norm), 6.0);
		const vector3d color_cliffs = m_rockColor[1];
		// Ice has been left as is so the occasional desert world will have polar ice-caps like mars
		if (fabs(m_icyness*p.y) + m_icyness*n > 1) {
			return interpolate_color(flatness, color_cliffs, vector3d(1,1,1));
		}

		double equatorial_desert = (2.0-m_icyness)*(-1.0+2.0*octavenoise(12, 0.5, 2.0, (n*2.0)*p)) *
				1.0*(2.0-m_icyness)*(1.0-p.y*p.y);

		vector3d col;
		if (n > .4) {
			n = n*n;
			col = interpolate_color(equatorial_desert, vector3d(.8,.75,.5), vector3d(.52, .5, .3));
			col = interpolate_color(n, col, vector3d(.1, .0, .0));
			col = interpolate_color(flatness, color_cliffs, col);
			return col;
		} else if (n > .3) {
			n = n*n;
			col = interpolate_color(equatorial_desert, vector3d(.81, .68, .3), vector3d(.85, .7, 0));
			col = interpolate_color(n, col, vector3d(-1.2,-.84,.35));
			col = interpolate_color(flatness, color_cliffs, col);
			return col;
		} else if (n > .2) {
			col = interpolate_color(equatorial_desert, vector3d(-0.4, -0.47, -0.6), vector3d(-.6, -.7, -2));
			col = interpolate_color(n, col, vector3d(4, 3.95, 3.94));
			col = interpolate_color(flatness, color_cliffs, col);
			return col;
		} else {
			col = interpolate_color(equatorial_desert, vector3d(.78, .73, .68), vector3d(.8, .77, .5));
			col = interpolate_color(n, col, vector3d(-2.0, -2.3, -2.4));
			col = interpolate_color(flatness, color_cliffs, col);
			return col;
		}
	
	}
	case COLOR_ROCK:
		{
		double n = m_invMaxHeight*height/2;

		if (n <= 0) return m_rockColor[1];		

		const double flatness = pow(p.Dot(norm), 6.0);
		const vector3d color_cliffs = m_rockColor[0];

		double equatorial_desert = (2.0-m_icyness)*(-1.0+2.0*octavenoise(12, 0.5, 2.0, (n*2.0)*p)) *
				1.0*(2.0-m_icyness)*(1.0-p.y*p.y);


		// Below is to do with variable colours for different heights, it gives a nice effect.
		// n is height.
		vector3d col;
		col = interpolate_color(equatorial_desert, m_rockColor[2], m_rockColor[3]);
		if (n > 0.45) {
		col = interpolate_color(n, col, m_rockColor[6]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.4) {
		col = interpolate_color(n, col, m_rockColor[5]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.35) {
		col = interpolate_color(n, m_rockColor[7], col);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.3) {
		col = interpolate_color(n, m_darkrockColor[0], col);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.25) {
		col = interpolate_color(n, col, m_rockColor[0]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.2) {
		col = interpolate_color(n, col, m_darkrockColor[2]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		if (n > 0.15) {
		col = interpolate_color(n, m_darkrockColor[3], col);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.1) {
		col = interpolate_color(n, col, m_rockColor[3]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.05) {
		col = interpolate_color(n, col, m_darkrockColor[1]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else {
		col = interpolate_color(n, m_darkrockColor[0], col);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}

	}
	case COLOR_ROCK2:
		{
		double n = m_invMaxHeight*height/2;

		if (n <= 0) return m_greyrockColor[1];		

		const double flatness = pow(p.Dot(norm), 6.0);
		const vector3d color_cliffs = m_greyrockColor[1];

		double equatorial_desert = (2.0-m_icyness)*(-1.0+2.0*octavenoise(12, 0.5, 2.0, (n*2.0)*p)) *
				1.0*(2.0-m_icyness)*(1.0-p.y*p.y);


		// Below is to do with variable colours for different heights, it gives a nice effect.
		// n is height.
		vector3d col;
		col = interpolate_color(equatorial_desert, m_greyrockColor[2], m_greyrockColor[3]);
		if (n > 0.45) {
		col = interpolate_color(n, col, m_greyrockColor[6]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.4) {
		col = interpolate_color(n, col, m_greyrockColor[5]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.35) {
		col = interpolate_color(n, m_greyrockColor[7], col);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.3) {
		col = interpolate_color(n, m_greyrockColor[0], col);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.25) {
		col = interpolate_color(n, col, m_greyrockColor[0]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.2) {
		col = interpolate_color(n, m_greyrockColor[2], col);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.15) {
		col = interpolate_color(n, m_greyrockColor[3], col);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.1) {
		col = interpolate_color(n, col, m_greyrockColor[3]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.05) {
		col = interpolate_color(n, col, m_greyrockColor[1]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else {
		col = interpolate_color(n, m_greyrockColor[0], col);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}

	}
	case COLOR_ASTEROID:
		{
		double n = m_invMaxHeight*height/2;

		if (n <= 0.02) {
			const double flatness = pow(p.Dot(norm), 6.0);
			const vector3d color_cliffs = m_rockColor[1];

			double equatorial_desert = (2.0)*(-1.0+2.0*octavenoise(12, 0.5, 2.0, (n*2.0)*p)) *
				1.0*(2.0)*(1.0-p.y*p.y);

			vector3d col;
			col = interpolate_color(equatorial_desert, m_rockColor[0], m_greyrockColor[3]);
			col = interpolate_color(n, col, vector3d(1.5,1.35,1.3));
			col = interpolate_color(flatness, color_cliffs, col);
			return col;
		} else {
			const double flatness = pow(p.Dot(norm), 6.0);
			const vector3d color_cliffs = m_greyrockColor[1];

			double equatorial_desert = (2.0)*(-1.0+2.0*octavenoise(12, 0.5, 2.0, (n*2.0)*p)) *
				1.0*(2.0)*(1.0-p.y*p.y);

			vector3d col;
			col = interpolate_color(equatorial_desert, m_greyrockColor[0], m_greyrockColor[2]);
			col = interpolate_color(n, col, m_rockColor[3]);
			col = interpolate_color(flatness, color_cliffs, col);
			return col;
		}


	}
	case COLOR_VOLCANIC:
	{
		double n = m_invMaxHeight*height;
		const double flatness = pow(p.Dot(norm), 6.0);
		const vector3d color_cliffs = m_rockColor[2];		
		double equatorial_desert = (-1.0+2.0*octavenoise(12, 0.5, 2.0, (n*2.0)*p)) *
				1.0*(1.0-p.y*p.y);

		vector3d col;

		if (n > 0.4){
		col = interpolate_color(equatorial_desert, vector3d(.3,.2,0), vector3d(.3, .1, .0));
		col = interpolate_color(n, col, vector3d(.1, .0, .0));
		col = interpolate_color(flatness, color_cliffs, col);
		} else if (n > 0.2){
		col = interpolate_color(equatorial_desert, vector3d(1.2,1,0), vector3d(.9, .3, .0));
		col = interpolate_color(n, col, vector3d(-1.1, -1, .0));
		col = interpolate_color(flatness, color_cliffs, col);
		} else if (n > 0.1){
		col = interpolate_color(equatorial_desert, vector3d(.2,.1,0), vector3d(.1, .05, .0));
		col = interpolate_color(n, col, vector3d(2.5, 2, .0));
		col = interpolate_color(flatness, color_cliffs, col);
		} else {
		col = interpolate_color(equatorial_desert, vector3d(.75,.6,0), vector3d(.75, .2, .0));
		col = interpolate_color(n, col, vector3d(-2, -2.2, .0));
		col = interpolate_color(flatness, color_cliffs, col);
		}
		return col;
	}
	case COLOR_METHANE: {
		double n = m_invMaxHeight*height;
		if (n <= 0) return vector3d(.3,.0,.0);
		else return interpolate_color(n, vector3d(.3,.2,.0), vector3d(.6,.3,.0));
	}
	case COLOR_TFGOOD:
	{
		double n = m_invMaxHeight*height;
		// water
		if (n <= 0) return vector3d(0,0.1,0.4);

		const double flatness = pow(p.Dot(norm), 6.0);
		const vector3d color_cliffs = m_rockColor[1];
		// ice on mountains and poles
		if (fabs(m_icyness*p.y) + m_icyness*n > 1) {
			return interpolate_color(flatness, color_cliffs, vector3d(0.98,0.98,0.98));
		}

		double equatorial_desert = (2.0-m_icyness)*(-1.0+2.0*octavenoise(12, 0.5, 2.0, (n*2.0)*p)) *
				1.0*(2.0-m_icyness)*(1.0-p.y*p.y);

		vector3d col;
		if (n >= 0.2) {
		col = interpolate_color(equatorial_desert, m_darkrockColor[0], m_darkrockColor[3]);
		col = interpolate_color(n, col, m_rockColor[2]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n >= 0.04) {      
		col = interpolate_color(equatorial_desert, vector3d(.45,.43, .2), vector3d(.4, .43, .2));
		col = interpolate_color(n, col, vector3d(-1.66,-2.3, -1.75));
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n >= 0.02) {      
		col = interpolate_color(equatorial_desert, vector3d(.15,.5, -.1), vector3d(.2, .6, -.1));
		col = interpolate_color(n, col, vector3d(5,-5, 5));
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n >= 0.01) {      
		col = interpolate_color(equatorial_desert, vector3d(.45,.6,0), vector3d(.5, .6, .0));
		col = interpolate_color(n, col, vector3d(-10,-10,0));
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else {                                      
		col = interpolate_color(equatorial_desert, vector3d(.35,.3,0), vector3d(.4, .3, .0));
		col = interpolate_color(n, col, vector3d(0,20,0));
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
	}
	case COLOR_TFPOOR:
	{
		double n = m_invMaxHeight*height;
		// water
		if (n <= 0) return vector3d(0,0.1,0.4);

		const double flatness = pow(p.Dot(norm), 6.0);
		const vector3d color_cliffs = m_rockColor[1];
		// ice on mountains and poles
		if (fabs(m_icyness*p.y) + m_icyness*n > 1) {
			return interpolate_color(flatness, color_cliffs, vector3d(0.98,0.98,0.98));
		}

		double equatorial_desert = (2.0-m_icyness)*(-1.0+2.0*octavenoise(12, 0.5, 2.0, (n*2.0)*p)) *
				1.0*(2.0-m_icyness)*(1.0-p.y*p.y);

		vector3d col;
		if (n >= 0.2) {
		col = interpolate_color(equatorial_desert, m_darkrockColor[0], m_darkrockColor[3]);
		col = interpolate_color(n, col, m_rockColor[2]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n >= 0.04) {      
		col = interpolate_color(equatorial_desert, m_rockColor[5], m_rockColor[6]);
		col = interpolate_color(n, col, m_darkrockColor[5]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n >= 0.02) {      
		col = interpolate_color(equatorial_desert, m_darkrockColor[2], m_rockColor[2]);
		col = interpolate_color(n, col, m_rockColor[3]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n >= 0.01) {      
		col = interpolate_color(equatorial_desert, vector3d(.45,.6,0), vector3d(.5, .6, .0));
		col = interpolate_color(n, col, vector3d(-10,-10,0));
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else {                                      
		col = interpolate_color(equatorial_desert, vector3d(.35,.3,0), vector3d(.4, .3, .0));
		col = interpolate_color(n, col, vector3d(0,20,0));
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
	}
	case COLOR_BANDED_ROCK: {
		const double flatness = pow(p.Dot(norm), 6.0);
		double n = fabs(noise(vector3d(height*10000.0,0.0,0.0)));
		vector3d col = interpolate_color(n, m_rockColor[0], m_rockColor[1]);
		return interpolate_color(flatness, col, m_rockColor[2]);
	}
	}

}

static inline double octavenoise(int octaves, double roughness, double lacunarity, const vector3d &p)
{
	double n = 0;
	double octaveAmplitude = roughness;
	double jizm = 1.0;
	while (octaves--) {
		n += octaveAmplitude * noise(jizm*p);
		octaveAmplitude *= roughness;
		jizm *= lacunarity;
	}
	return (n+1.0)*0.5;
}
static inline double river_octavenoise(int octaves, double roughness, double lacunarity, const vector3d &p)
{
	double n = 0;
	double octaveAmplitude = roughness;
	double jizm = 1.0;
	while (octaves--) {
		n += octaveAmplitude * fabs(noise(jizm*p));
		octaveAmplitude *= roughness;
		jizm *= lacunarity;
	}
	return n;
}
static inline double ridged_octavenoise(int octaves, double roughness, double lacunarity, const vector3d &p)
{
	double n = 0;
	double octaveAmplitude = roughness;
	double jizm = 1.0;
	while (octaves--) {
		n += octaveAmplitude * noise(jizm*p);
		octaveAmplitude *= roughness;
		jizm *= lacunarity;
	}
	return 1.0 - fabs(n);
}
static inline double octavenoise(fracdef_t &def, double roughness, const vector3d &p)
{
	double n = 0;
	double octaveAmplitude = roughness;
	double jizm = def.frequency;
	for (int i=0; i<def.octaves; i++) {
		n += octaveAmplitude * noise(jizm*p);
		octaveAmplitude *= roughness;
		jizm *= def.lacunarity;
	}
	return (n+1.0)*0.5;
}

static inline double river_octavenoise(fracdef_t &def, double roughness, const vector3d &p)
{
	double n = 0;
	double octaveAmplitude = roughness;
	double jizm = def.frequency;
	for (int i=0; i<def.octaves; i++) {
		n += octaveAmplitude * fabs(noise(jizm*p));
		octaveAmplitude *= roughness;
		jizm *= def.lacunarity;
	}
	return fabs(n);
}

static inline double ridged_octavenoise(fracdef_t &def, double roughness, const vector3d &p)
{
	double n = 0;
	double octaveAmplitude = roughness;
	double jizm = def.frequency;
	for (int i=0; i<def.octaves; i++) {
		n += octaveAmplitude * noise(jizm*p);
		octaveAmplitude *= roughness;
		jizm *= def.lacunarity;
	}
	return 1.0 - fabs(n);
}

// Creates small canyons.
static double canyon_function(const fracdef_t &def, const vector3d &p)
{
	double h;
	double n = octavenoise(def.octaves, 0.54, 2.0, def.frequency*p);
	const double outer = 0.71;
	const double inner = 0.715;
	const double inner2 = 0.715;
	const double outer2 = 0.72;
	if (n > outer2) {
		h = 1;
	} else if (n > inner2) {
		h = 0.0+1.0*(n-inner2)*(1.0/(outer2-inner2));
	} else if (n > inner) {
		h = 0;
	} else if (n > outer) {
		h = 1.0-1.0*(n-outer)*(1.0/(inner-outer));
	} else {
		h = 1.0;
	}
	return h * def.amplitude;
}

// Larger canyon.
double canyon2_function(const fracdef_t &def, const vector3d &p)
{
	double h;
	double n = octavenoise(def.octaves, 0.56, 2.0, def.frequency*p);
	const double outer = 0.7;
	const double inner = 0.71;
	const double inner2 = 0.72;
	const double outer2 = 0.73;
	if (n > outer2) {
		h = 1;
	} else if (n > inner2) {
		h = 0.0+1.0*(n-inner2)*(1.0/(outer2-inner2));
	} else if (n > inner) {
		h = 0;
	} else if (n > outer) {
		h = 1.0-1.0*(n-outer)*(1.0/(inner-outer));
	} else {
		h = 1.0;
	}
	return h * def.amplitude;
}

// Largest and best looking canyon, combine them together for best results.
double canyon3_function(const fracdef_t &def, const vector3d &p)
{
	double h;
	double n = octavenoise(def.octaves, 0.585, 2.0, def.frequency*p);
	const double outer = 0.7;
	const double inner = 0.71;
	const double inner2 = 0.72;
	const double outer2 = 0.73;
	if (n > outer2) {
		h = 1.0;
	} else if (n > inner2) {
		h = 0.0+1.0*(n-inner2)*(1.0/(outer2-inner2));
	} else if (n > inner) {
		h = 0.0;
	} else if (n > outer) {
		h = 1.0-1.0*(n-outer)*(1.0/(inner-outer));
	} else {
		h = 1.0;
	}
	return h * def.amplitude;
}

/*double rock_function(const fracdef_t &def, const vector3d &p)
{
	double h;
	double n = octavenoise(def.octaves, 0.788, 2.0, def.frequency*p);
	const double outer = 0.1;
	const double inner = 0.3;
	const double inner2 = 0.7;
	const double outer2 = 0.9;
	if (n > outer2) {
		h = 0.0;
	} else if (n > inner2) {
		h = 1.0 - ((n-inner2)*(1.0/(outer2-inner2)));
	} else if (n > inner) {
		h = 1.0;
	} else if (n > outer) {
		h = 0.0+1.0*(n-outer)*(1.0/(inner-outer));
	} else {
		h = 0.0;
	}
	return h * def.amplitude;
}*/

static void crater_function_1pass(const vector3d &p, double &out, const double height)
{
	double n = fabs(noise(p));
	const double ejecta_outer = 0.6;
	const double outer = 0.9;
	const double inner = 0.94;
	const double midrim = 0.93;
	if (n > inner) {
		//out = 0;
	} else if (n > midrim) {
		double hrim = inner - midrim;
		double descent = (hrim-(n-midrim))/hrim;
		out += height * descent * descent;
	} else if (n > outer) {
		double hrim = midrim - outer;
		double ascent = (n-outer)/hrim;
		out += height * ascent * ascent;
	} else if (n > ejecta_outer) {
		// blow down walls of other craters too near this one,
		// so we don't have sharp transition
		//out *= (outer-n)/-(ejecta_outer-outer);
	}
}

// makes large and small craters across the entire planet.
static double crater_function(const fracdef_t &def, const vector3d &p) 
{
	double crater = 0.0;
	double sz = def.frequency;
	double max_h = def.amplitude;
	for (int i=0; i<def.octaves; i++) {
		crater_function_1pass(sz*p, crater, max_h);
		sz *= 2.0;
		max_h *= 0.5;
	}
	return crater;
}

static void volcano_function_1pass(const vector3d &p, double &out, const double height)
{
	double n = fabs(noise(p));
	const double ejecta_outer = 0.6;
	const double outer = 0.9;  //Radius
	const double inner = 0.951;
	const double midrim = 0.941;
	if (n > inner) {
		//out = 0;
	} else if (n > midrim) {
		double hrim = inner - midrim;
		double descent = (hrim-(n-midrim))/hrim;
		out += height * descent;
	} else if (n > outer) {
		double hrim = midrim - outer;
		double ascent = (n-outer)/hrim;
		out += height * ascent * ascent;
	} else if (n > ejecta_outer) {
		// blow down walls of other craters too near this one,
		// so we don't have sharp transition
		out *= (outer-n)/-(ejecta_outer-outer);
	}
}

static double volcano_function(const fracdef_t &def, const vector3d &p)
{
	double crater = 0.0;
	double sz = def.frequency;
	double max_h = def.amplitude;
	for (int i=0; i<def.octaves; i++) {
		volcano_function_1pass(sz*p, crater, max_h);
		sz *= 1.0;  //frequency?
		max_h *= 0.4; // height??
	}
	return 3.0 * crater;
}

static void megavolcano_function_1pass(const vector3d &p, double &out, const double height)
{
	double n = fabs(noise(p));
	const double ejecta_outer = 0.6;
	const double outer = 0.76;  //Radius
	const double inner = 0.97;
	const double midrim = 0.925;
	if (n > inner) {
		//out = 0;
	} else if (n > midrim) {
		double hrim = inner - midrim;
		double descent = (hrim-(n-midrim))/hrim;
		out += height * descent;
	} else if (n > outer) {
		double hrim = midrim - outer;
		double ascent = (n-outer)/hrim;
		out += height * ascent * ascent;
	} else if (n > ejecta_outer) {
		// blow down walls of other craters too near this one,
		// so we don't have sharp transition
		out *= (outer-n)/-(ejecta_outer-outer);
	}
}

static double megavolcano_function(const fracdef_t &def, const vector3d &p)
{
	double crater = 0.0;
	double sz = def.frequency;
	double max_h = def.amplitude;
	for (int i=0; i<def.octaves; i++) {
		megavolcano_function_1pass(sz*p, crater, max_h);
		sz *= 1.0;  //frequency?
		max_h *= 0.15; // height??
	}
	return 4.0 * crater;
}

double river_function(const fracdef_t &def, const vector3d &p)
{
	double h;
	double n = octavenoise(def.octaves, 0.585, 2.0, def.frequency*p);
	const double outer = 0.67;
	const double inner = 0.715;
	const double inner2 = 0.715;
	const double outer2 = 0.76;
	if (n > outer2) {
		h = 1;
	} else if (n > inner2) {
		h = 0.0+1.0*(n-inner2)*(1.0/(outer2-inner2));
	} else if (n > inner) {
		h = 0;
	} else if (n > outer) {
		h = 1.0-1.0*(n-outer)*(1.0/(inner-outer));
	} else {
		h = 1.0;
	}
	return h * def.amplitude;
}

// Wider river.
double river2_function(const fracdef_t &def, const vector3d &p)
{
	double h;
	double n = octavenoise(def.octaves, 0.585, 2.0, def.frequency*p);
	const double outer = 0.01;
	const double inner = 0.49;
	const double inner2 = 0.51;
	const double outer2 = 0.99;
	if (n > outer2) {
		h = 1;
	} else if (n > inner2) {
		h = 0.0+1.0*(n-inner2)*(1.0/(outer2-inner2));
	} else if (n > inner) {
		h = 0;
	} else if (n > outer) {
		h = 1.0-1.0*(n-outer)*(1.0/(inner-outer));
	} else {
		h = 1.0;
	}
	return h * def.amplitude;
}

// Original canyon function, But really it generates cliffs.
double cliff_function(const fracdef_t &def, const vector3d &p)
{
	double h;
	double n = octavenoise(def.octaves, 0.54, 2.0, def.frequency*p);
	const double outer = 0.7;
	const double inner = 0.71;
	if (n > inner) {
		h = 0;
	} else if (n > outer) {
		h = 1.0-1.0*(n-outer)*(1.0/(inner-outer));
	} else {
		h = 1.0;
	}
	return h;
}


