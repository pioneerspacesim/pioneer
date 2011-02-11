#include "GeoSphereStyle.h"
#include "perlin.h"


/**
 * All these stinking octavenoise functions return range [0,1] if persistence = 0.5
 */
static inline double octavenoise(int octaves, double persistence, double lacunarity, const vector3d &p)
{
	double n = 0;
	double octaveAmplitude = 0.5;
	double jizm = 1.0;
	while (octaves--) {
		n += octaveAmplitude * noise(jizm*p);
		octaveAmplitude *= persistence;
		jizm *= lacunarity;
	}
	return (n+1.0)*0.5;
}
static inline double octavenoise(fracdef_t &def, const vector3d &p)
{
	return octavenoise(def.octaves, 0.5, def.lacunarity, def.frequency * p);
}

static inline double river_octavenoise(int octaves, double persistence, double lacunarity, const vector3d &p)
{
	double n = 0;
	double octaveAmplitude = 0.5;
	double jizm = 1.0;
	while (octaves--) {
		n += octaveAmplitude * fabs(noise(jizm*p));
		octaveAmplitude *= persistence;
		jizm *= lacunarity;
	}
	return n;
}
static inline double river_octavenoise(fracdef_t &def, const vector3d &p)
{
	return river_octavenoise(def.octaves, 0.5, def.lacunarity, def.frequency * p);
}

static inline double ridged_octavenoise(int octaves, double persistence, double lacunarity, const vector3d &p)
{
	double n = 0;
	double octaveAmplitude = 0.5;
	double jizm = 1.0;
	while (octaves--) {
		n += octaveAmplitude * (1.0 - fabs(noise(jizm*p)));
		octaveAmplitude *= persistence;
		jizm *= lacunarity;
	}
	return n;
}
static inline double ridged_octavenoise(fracdef_t &def, const vector3d &p)
{
	return ridged_octavenoise(def.octaves, 0.5, def.lacunarity, def.frequency * p);
}


int GeoSphereStyle::GetRawHeightMapVal(int x, int y)
{
	return m_heightMap[CLAMP(y, 0, m_heightMapSizeY-1)*m_heightMapSizeX + CLAMP(x, 0, m_heightMapSizeX-1)];
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
	ix = CLAMP(ix, 0, m_heightMapSizeX-1);
	iy = CLAMP(iy, 0, m_heightMapSizeY-1);
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
		return (v<0 ? 0 : v + 100*octavenoise(10, 0.5, 2.0, 1000.0*pt) + (50*octavenoise(14, 0.5, 2.0, 1000.0*pt) + 
			50*octavenoise(8, 0.7, 1.0, 1000.0*pt) + 30*octavenoise(7, 0.6, 1.2, 1000.0*pt) + 30*octavenoise(6, 0.4, 1.8, 1000.0*pt)) *
			(v/400) + ((20*octavenoise(2, 0.0, 2.0, 100.0*pt))* ((v/200)*(v/200))));

	}
}

static inline double fractal(fracdef_t &def, int type, const vector3d &p)
{
	double v;
	switch (type) {
		case 0:	
			v = river_octavenoise(def.octaves, 0.5, def.lacunarity, def.frequency * p);
			break;
		case 1:
			v = ridged_octavenoise(def.octaves, 0.5, def.lacunarity, def.frequency * p);
			break;
		default:
			v = octavenoise(def.octaves, 0.5, def.lacunarity, def.frequency * p);
			break;
	}
	return v;
}

static double canyon_function(const vector3d &p)
{
	double h;
	double n = octavenoise(8,0.5,2.0,p);
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

static double crater_function(const vector3d &p)  // makes large and small craters across the entire planet.
{
	double crater = 0.0;
	double sz = 1.0;
	double max_h = 0.5;
	for (int i=0; i<14; i++) {
		crater_function_1pass(sz*p, crater, max_h);
		sz *= 2.0;
		max_h *= 0.5;
	}
	return 4.0 * crater;
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
		//out *= (outer-n)/-(ejecta_outer-outer);
	}
}

static double volcano_function(const vector3d &p)
{
	double acrater = 0.0;
	double asz = 1;
	double amax_h = 0.7;
	for (int i=0; i<14; i++) {
		volcano_function_1pass(asz*p, acrater, amax_h);
		asz *= 1.0;  //...
		amax_h *= 0.4; // .
	}
	return 3.0 * acrater;
}

// The small Volcano function, unlike the others is not necesarilly used to create volcanoes,  
// but can be effectively used to bunch mountains together in ranges generally caused by volcanic activity.
static void smlvolcano_function_1pass(const vector3d &p, double &out, const double height)
{
	double n = fabs(noise(p));
	const double ejecta_outer = 0.69;
	const double outer = 0.7;  //Radius
	const double inner = 0.981;
	const double midrim = 0.961;
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
		//out *= (outer-n)/-(ejecta_outer-outer);
	}
}

static double smlvolcano_function(const vector3d &p)
{
	double acrater = 0.0;
	double asz = 0.7;
	double amax_h = 0.4;
	for (int i=0; i<14; i++) {
		smlvolcano_function_1pass(asz*p, acrater, amax_h);
		asz *= 1.0;  //...
		amax_h *= 0.15; // .
	}
	return 2.0 * acrater;
}

// Creates a nice mega-volcano based on the crater function. Multiply the function by mountains or hills for best effect.
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

static double megavolcano_function(const vector3d &p)
{
	double crater = 0.0;
	double sz = 0.58;
	double max_h = 0.5;
	for (int i=0; i<14; i++) {
		megavolcano_function_1pass(sz*p, crater, max_h);
		sz *= 1.0;  //frequency?
		max_h *= 0.15; // height??
	}
	return 4.0 * crater;
}

static inline vector3d interpolate_color(double n, vector3d start, vector3d end)
{
	n = CLAMP(n, 0.0f, 1.0f);
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
			m_atmosDensity = 14.0f;
			break;
		case SBody::TYPE_PLANET_ASTEROID:
			m_atmosColor = Color(0.0f, 0.0f, 0.0f, 0.0f);
			m_atmosDensity = 0.0f;
			break;
		default:
			m_atmosColor = Color(.6f, .6f, .7f, 0.8f);
			m_atmosDensity = sbody->m_volatileGas.ToFloat();
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
	}
	m_continentType = CONTINENT_SIMPLE;
	m_terrainType = TERRAIN_MOUNTAINS_RIVERS;
	m_colorType = COLOR_EARTHLIKE;

	m_sealevel = CLAMP(body->m_volatileLiquid.ToDouble(), 0.0, 1.0);
	m_icyness = CLAMP(body->m_volatileIces.ToDouble(), 0.0, 1.0);

	const double rad = body->GetRadius();
	m_maxHeightInMeters = std::max(100.0, (9000.0*rad*rad) / (body->GetMass() * 6.64e-12));
	//             ^^^^ max mountain height for earth-like planet (same mass, radius)
	// and then in sphere normalized jizz
	m_maxHeight = std::min(0.5, m_maxHeightInMeters / rad);
	printf("%s: max terrain height: %fm [%f]\n", body->name.c_str(), m_maxHeightInMeters, m_maxHeight);
	m_invMaxHeight = 1.0 / m_maxHeight;
	m_planetRadius = rad;
	m_planetEarthRadii = rad / EARTH_RADIUS;

	// Pick some colors
	for (int i=0; i<12; i++) m_entropy[i] = rand.Double();
	for (int i=0; i<8; i++) {
		double r,g,b;
		r = rand.Double(0.5, 1.0);
		g = rand.Double(0.5, r);
		b = rand.Double(0.5, MIN(r, g));
		r = std::min(1.0, r + body->m_metallicity.ToFloat());
		m_rockColor[i] = vector3d(r, g, b);
	}

	for (int i=0; i<12; i++) m_entropy[i] = rand.Double();
	for (int i=0; i<8; i++) {
		double g;
		g = rand.Double(0.3, 0.8);
		m_greyrockColor[i] = vector3d(g, g, g);
	}

	PickAtmosphere(body);
	InitHeightMap(body);
	fprintf(stderr, "picked continent %d, terrain %d, colortype %d for %s\n", (int)m_continentType, (int)m_terrainType, (int)m_colorType, body->name.c_str());
	InitFractalType(rand);
}

/**
 * Feature width means roughly one perlin noise blob or grain.
 * This will end up being one hill, mountain or continent, roughly.
 */
void GeoSphereStyle::SetFracDef(struct fracdef_t *def, double featureHeightMeters, double featureWidthMeters, double lacunarity, double smallestOctaveMeters)
{
	// feature 
	def->amplitude = featureHeightMeters / (m_maxHeight * m_planetRadius);
	def->frequency = m_planetRadius / featureWidthMeters;
	def->lacunarity = lacunarity;
	def->octaves = std::max(1, (int)ceil(log(featureWidthMeters / smallestOctaveMeters) / log(2.0)));
	printf("%d octaves\n", def->octaves);
}

void GeoSphereStyle::InitFractalType(MTRand &rand)
{
	switch (m_continentType) {
		case CONTINENT_FLAT:
			break;
		case CONTINENT_VOLCANIC_MARE:
			SetFracDef(&targ.continents, m_maxHeightInMeters, rand.Double(5e5,1.5e6), rand.Double(1.8, 2.2), 1e5);
			break;
		case CONTINENT_SIMPLE:
			/* Continent sizes between 1000km and 10,000km */
			SetFracDef(&targ.continents, m_maxHeightInMeters, rand.Double(1e6,1e7), rand.Double(1.8, 2.2));
			break;
	}

	switch (m_terrainType) {
		case TERRAIN_ASTEROID:
			m_maxHeight = rand.Double(0.2,0.4);
			m_invMaxHeight = 1.0 / m_maxHeight;
			noise1 = rand.Double(2,14);
			noise2 = rand.Double(0.1,0.75);
			noise3 = rand.Double(0.1,2);
			break;
		case TERRAIN_HILLS_NORMAL:
		case TERRAIN_HILLS_RIDGED:
		case TERRAIN_HILLS_RIVERS:
		{
			double height = m_maxHeightInMeters*0.7;
			SetFracDef(&targ.midTerrain, height, rand.Double(4.0, 20.0)*height, rand.Double(1.8, 2.2));
			SetFracDef(&targ.midDistrib, m_maxHeightInMeters, rand.Double(50.0, 100.0)*m_maxHeightInMeters, rand.Double(1.8, 2.2));
			break;
		}
		case TERRAIN_MOUNTAINS_NORMAL:
		{
			double height = m_maxHeightInMeters*0.2;
			SetFracDef(&targ.midTerrain, height, rand.Double(4.0, 20.0)*height, rand.Double(1.8, 2.2));
			SetFracDef(&targ.midDistrib, m_maxHeightInMeters, rand.Double(50.0, 100.0)*m_maxHeightInMeters, rand.Double(1.8, 2.2));

			height = m_maxHeightInMeters*0.5;
			SetFracDef(&targ.localDistrib, m_maxHeightInMeters, rand.Double(100.0, 200.0)*m_maxHeightInMeters, rand.Double(1.8, 2.2));
			SetFracDef(&targ.localTerrain, height, rand.Double(2.5,3.5)*height, rand.Double(1.8, 2.2));
			break;
		}
		case TERRAIN_MOUNTAINS_RIDGED:
		{
			double height = m_maxHeightInMeters*0.2;
			SetFracDef(&targ.midTerrain, height, rand.Double(4.0, 20.0)*height, rand.Double(1.8, 2.2));
			SetFracDef(&targ.midDistrib, m_maxHeightInMeters, rand.Double(50.0, 100.0)*m_maxHeightInMeters, rand.Double(1.8, 2.2));

			height = m_maxHeightInMeters*0.5;
			SetFracDef(&targ.localDistrib, m_maxHeightInMeters, rand.Double(100.0, 200.0)*m_maxHeightInMeters, rand.Double(1.8, 2.2));
			SetFracDef(&targ.localTerrain, height, rand.Double(6.0,8.0)*height, rand.Double(1.8, 2.2));
			break;
		}
		case TERRAIN_MOUNTAINS_RIVERS:
		{
			// XXX looks too flat and crappy
			double height = m_maxHeightInMeters*0.2;
			SetFracDef(&targ.midTerrain, height, rand.Double(4.0, 20.0)*height, rand.Double(1.8, 2.2));
			SetFracDef(&targ.midDistrib, m_maxHeightInMeters, rand.Double(50.0, 100.0)*m_maxHeightInMeters, rand.Double(1.8, 2.2));

			height = m_maxHeightInMeters*0.5;
			SetFracDef(&targ.localDistrib, m_maxHeightInMeters, rand.Double(100.0, 200.0)*m_maxHeightInMeters, rand.Double(1.8, 2.2));
			SetFracDef(&targ.localTerrain, height, rand.Double(6.0,8.0)*height, rand.Double(1.8, 2.2));
			break;
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

	switch (m_continentType) {
		case CONTINENT_FLAT:
			continents = 0;
			break;
		case CONTINENT_VOLCANIC_MARE:
		case CONTINENT_SIMPLE:
			continents = octavenoise(targ.continents, p) - m_sealevel;
			break;
	}

	switch (m_terrainType) {
		case TERRAIN_NONE:
		case TERRAIN_GASGIANT:
			return 0;
		case TERRAIN_ASTEROID:
		{
			//return m_maxHeight*(octavenoise(14, 0.5, 2.0, p)*0.3) + 0.005*crater_function(p);
			return m_maxHeight*(octavenoise(14, 0.5, 2.0, p)*0.6)*(octavenoise(noise1, noise2, noise3, p)*m_sealevel) 
				+ (0.005*crater_function(p));
		}
		case TERRAIN_HILLS_NORMAL:
		{
			if (continents < 0) return 0;
			double out = 0.3 * continents;
			double distrib = octavenoise(targ.midDistrib, p);
			double m = 0;
			if (distrib > 0.5) m += 2.0 * (distrib-0.5) * targ.midTerrain.amplitude * octavenoise(targ.midTerrain, p);
			// cliffs at shore
			if (continents < 0.001) out += m * continents * 1000.0f;
			else out += m;
			return m_maxHeight * out;
		}
		case TERRAIN_HILLS_RIDGED:
		{
			if (continents < 0) return 0;
			// == TERRAIN_HILLS_NORMAL except ridged_octavenoise
			double out = 0.3 * continents;
			double distrib = ridged_octavenoise(targ.midDistrib, p);
			double m = 0;
			if (distrib > 0.5) m += 2.0 * (distrib-0.5) * targ.midTerrain.amplitude * ridged_octavenoise(targ.midTerrain, p);
			// cliffs at shore
			if (continents < 0.001) out += m * continents * 1000.0f;
			else out += m;
			return m_maxHeight * out;
		}
		case TERRAIN_HILLS_RIVERS:
		{
			if (continents < 0) return 0;
			// == TERRAIN_HILLS_NORMAL except river_octavenoise
			double out = 0.3 * continents;
			double distrib = river_octavenoise(targ.midDistrib, p);
			double m = 0;
			if (distrib > 0.5) m += 2.0 * (distrib-0.5) * targ.midTerrain.amplitude * river_octavenoise(targ.midTerrain, p);
			// cliffs at shore
			if (continents < 0.001) out += m * continents * 1000.0f;
			else out += m;
			return m_maxHeight * out;
		}
		case TERRAIN_MOUNTAINS_NORMAL:
		{
			if (continents < 0) return 0;
			double out = 0.3 * continents;
			double m = targ.midTerrain.amplitude * octavenoise(targ.midTerrain, p);
			double distrib = octavenoise(targ.localTerrain, p);
			if (distrib > 0.5) m += 2.0 * (distrib-0.5) * targ.localTerrain.amplitude * octavenoise(targ.localTerrain, p);
			// cliffs at shore
			if (continents < 0.001) out += m * continents * 1000.0f;
			else out += m;
			return m_maxHeight * out;
		}
		case TERRAIN_MOUNTAINS_RIDGED:
		{
			if (continents < 0) return 0;
			double out = 0.3 * continents;
			double m = targ.midTerrain.amplitude * ridged_octavenoise(targ.midTerrain, p);
			double distrib = ridged_octavenoise(targ.localTerrain, p);
			if (distrib > 0.5) m += 2.0 * (distrib-0.5) * targ.localTerrain.amplitude * ridged_octavenoise(targ.localTerrain, p);
			// cliffs at shore
			if (continents < 0.001) out += m * continents * 1000.0f;
			else out += m;
			return m_maxHeight * out;
		}
		case TERRAIN_MOUNTAINS_RIVERS:
		{
			if (continents < 0) return 0;
			double out = 0.3 * continents;
			double m = targ.midTerrain.amplitude * river_octavenoise(targ.midTerrain, p);
			double distrib = river_octavenoise(targ.localTerrain, p);
			if (distrib > 0.5) m += 2.0 * (distrib-0.5) * targ.localTerrain.amplitude * river_octavenoise(targ.localTerrain, p);
			// cliffs at shore
			if (continents < 0.001) out += m * continents * 1000.0f;
			else out += m;
			return m_maxHeight * out;
		}
	}
}

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

		const double flatness = pow(vector3d::Dot(p, norm), 6.0);
		const vector3d color_cliffs = m_rockColor[1];
		// ice on mountains and poles
		if (fabs(m_icyness*p.y) + m_icyness*n > 1) {
			return interpolate_color(flatness, color_cliffs, vector3d(1,1,1));
		}

		double equatorial_desert = (2.0-m_icyness)*(-1.0+2.0*octavenoise(12, 0.5, 2.0, (n*2.0)*p)) *
				1.0*(2.0-m_icyness)*(1.0-p.y*p.y);

		vector3d col;
		// latitude: high ~col, low orange
		col = interpolate_color(equatorial_desert, vector3d(0,.5,0), vector3d(.86, .75, .48));
		// height: low green, high grey
		col = interpolate_color(n, col, m_rockColor[0]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
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

		const double flatness = pow(vector3d::Dot(p, norm), 6.0);
		const vector3d color_cliffs = m_rockColor[2];
		// ice on mountains and poles
		//if (fabs(m_icyness*p.y) + m_icyness*n > 18) {
		//	return interpolate_color(flatness, color_cliffs, vector3d(.99,.99,.99));
		//}

		double equatorial_desert = (2.0-m_icyness)*(-1.0+2.0*octavenoise(12, 0.5, 2.0, (n*2.0)*p)) *
				1.0*(2.0-m_icyness)*(1.0-p.y*p.y);

		vector3d col;
		if (n > 0.8) {     
		col = interpolate_color(equatorial_desert, m_rockColor[4], m_rockColor[5]);
		col = interpolate_color(n, col, vector3d(1, 1, 1));
		col = interpolate_color(flatness, m_rockColor[3], col);
		return col;
		}
		else if (n > 0.5) {  
		col = interpolate_color(equatorial_desert, vector3d(0.19,0.18,.17), vector3d(.2, .2, .2));
		col = interpolate_color(n, col, m_rockColor[4]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else {      
		col = interpolate_color(equatorial_desert, vector3d(1.9,1.89,1.888), vector3d(2, 2, 2));
		col = interpolate_color(n, col, vector3d(-1.5,-1.49,-1.488));
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
	}
	case COLOR_DESERT:
	{
		double n = m_invMaxHeight*height/2;
		

		const double flatness = pow(vector3d::Dot(p, norm), 6.0);
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

		const double flatness = pow(vector3d::Dot(p, norm), 6.0);
		const vector3d color_cliffs = m_rockColor[0];

		double equatorial_desert = (2.0-m_icyness)*(-1.0+2.0*octavenoise(12, 0.5, 2.0, (n*2.0)*p)) *
				1.0*(2.0-m_icyness)*(1.0-p.y*p.y);


		// Below is to do with variable colours for different heights, it gives a nice effect.
		// n is height.
		vector3d col;
		col = interpolate_color(equatorial_desert, m_rockColor[2], m_greyrockColor[3]);
		if (n > 0.45) {
		col = interpolate_color(n, col, m_greyrockColor[6]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.4) {
		col = interpolate_color(n, col, m_rockColor[5]);
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
		col = interpolate_color(n, col, m_greyrockColor[2]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		if (n > 0.15) {
		col = interpolate_color(n, m_rockColor[3], col);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.1) {
		col = interpolate_color(n, col, m_rockColor[3]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.05) {
		col = interpolate_color(n, col, m_rockColor[1]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else {
		col = interpolate_color(n, m_rockColor[0], col);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}

	}
	case COLOR_ROCK2:
		{
		double n = m_invMaxHeight*height/2;

		if (n <= 0) return m_greyrockColor[1];		

		const double flatness = pow(vector3d::Dot(p, norm), 6.0);
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
			const double flatness = pow(vector3d::Dot(p, norm), 6.0);
			const vector3d color_cliffs = m_rockColor[1];

			double equatorial_desert = (2.0)*(-1.0+2.0*octavenoise(12, 0.5, 2.0, (n*2.0)*p)) *
				1.0*(2.0)*(1.0-p.y*p.y);

			vector3d col;
			col = interpolate_color(equatorial_desert, m_rockColor[0], m_greyrockColor[3]);
			col = interpolate_color(n, col, vector3d(1.5,1.35,1.3));
			col = interpolate_color(flatness, color_cliffs, col);
			return col;
		} else {
			const double flatness = pow(vector3d::Dot(p, norm), 6.0);
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
		// water
		if (n <= 0) return vector3d(.75,.6,0);

		const double flatness = pow(vector3d::Dot(p, norm), 6.0);
		const vector3d color_cliffs = vector3d(.05, .02, .0);
		
		double equatorial_desert = (-1.0+2.0*octavenoise(12, 0.5, 2.0, (n*2.0)*p)) *
				1.0*(1.0-p.y*p.y);

		vector3d col;
		// latitude: high ~col, low orange
		col = interpolate_color(equatorial_desert, vector3d(.5,.4,0), vector3d(.2, .1, .0));
		// height: low yellowbrown, high darkred
		col = interpolate_color(n, col, vector3d(.1, .0, .0));
		col = interpolate_color(flatness, color_cliffs, col);
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

		const double flatness = pow(vector3d::Dot(p, norm), 6.0);
		const vector3d color_cliffs = m_rockColor[1];
		// ice on mountains and poles
		if (fabs(m_icyness*p.y) + m_icyness*n > 1) {
			return interpolate_color(flatness, color_cliffs, vector3d(0.9,0.9,0.9));
		}

		double equatorial_desert = (2.0-m_icyness)*(-1.0+2.0*octavenoise(12, 0.5, 2.0, (n*2.0)*p)) *
				1.0*(2.0-m_icyness)*(1.0-p.y*p.y);

		vector3d col;
		if (n > 0.05) {
		col = interpolate_color(equatorial_desert, m_rockColor[0], vector3d(.86, .75, .48));
		col = interpolate_color(n, col, m_rockColor[2]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.03) {      
		col = interpolate_color(equatorial_desert, vector3d(0,.2,0), vector3d(.0, .6, .0));
		col = interpolate_color(n, col, m_rockColor[0]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.01) {      
		col = interpolate_color(equatorial_desert, vector3d(.2,.6,0), vector3d(.1, .2, .0));
		col = interpolate_color(n, col, m_rockColor[1]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else {                                      
		col = interpolate_color(equatorial_desert, vector3d(.45,.7,0), vector3d(.2, .6, .0));
		col = interpolate_color(n, col, m_rockColor[0]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
	}
	case COLOR_TFPOOR:
	{
		double n = m_invMaxHeight*height;
		// water
		if (n <= 0) return vector3d(0.1,0.2,0.4);

		const double flatness = pow(vector3d::Dot(p, norm), 6.0);
		const vector3d color_cliffs = m_greyrockColor[1];
		// ice on mountains and poles
		if (fabs(m_icyness*p.y) + m_icyness*n > 1) {
			return interpolate_color(flatness, color_cliffs, vector3d(0.9,0.9,0.9));
		}

		double equatorial_desert = (2.0-m_icyness)*(-1.0+2.0*octavenoise(12, 0.5, 2.0, (n*2.0)*p)) *
				1.0*(2.0-m_icyness)*(1.0-p.y*p.y);

		vector3d col;
		col = interpolate_color(equatorial_desert, m_rockColor[2], m_rockColor[3]);
		if (n > 0.45) {
		col = interpolate_color(n, col, m_greyrockColor[3]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.35) {
		col = interpolate_color(n, col, m_greyrockColor[5]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.25) {
		col = interpolate_color(n, col, m_greyrockColor[6]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.2) {
		col = interpolate_color(n, col, m_greyrockColor[4]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.15) {
		col = interpolate_color(n, col, m_greyrockColor[0]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.1) {
		col = interpolate_color(n, col, m_greyrockColor[1]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.07) {
		col = interpolate_color(n, col, m_greyrockColor[4]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.04) {
		col = interpolate_color(n, col, m_greyrockColor[5]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.02) {
		col = interpolate_color(n, col, m_rockColor[6]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else {
		col = interpolate_color(n, col, m_rockColor[7]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
	}
	case COLOR_BANDED_ROCK: {
		const double flatness = pow(vector3d::Dot(p, norm), 6.0);
		double n = fabs(noise(vector3d(height*10000.0,0.0,0.0)));
		vector3d col = interpolate_color(n, m_rockColor[0], m_rockColor[1]);
		return interpolate_color(flatness, col, m_rockColor[2]);
	}
	}
}
