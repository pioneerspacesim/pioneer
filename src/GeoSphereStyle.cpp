#include "GeoSphereStyle.h"
#include "perlin.h"

/* What a bloated waste of space */
const GeoSphereStyle::sbody_valid_styles_t GeoSphereStyle::sbody_valid_styles[SBody::TYPE_MAX] = {
	// TYPE_GRAVPOINT,
	{},
	// TYPE_BROWN_DWARF,
	{},
	// TYPE_STAR_M,
	{},
	// TYPE_STAR_K,
	{},
	// TYPE_WHITE_DWARF,
	{},
	// TYPE_STAR_G,
	{},
	// TYPE_STAR_F,
	{},
	// TYPE_STAR_A,
	{},
	// TYPE_STAR_B,
	{},
	// TYPE_STAR_O,
	{},
	// TYPE_STAR_M_GIANT
	{},
	// TYPE_PLANET_SMALL_GAS_GIANT,
	{ { TERRAIN_GASGIANT },
	  { COLOR_GG_SATURN, COLOR_GG_JUPITER, COLOR_GG_URANUS, COLOR_GG_NEPTUNE } },
	// TYPE_PLANET_MEDIUM_GAS_GIANT,
	{ { TERRAIN_GASGIANT },
	  { COLOR_GG_SATURN, COLOR_GG_JUPITER, COLOR_GG_URANUS, COLOR_GG_NEPTUNE } },
	// TYPE_PLANET_LARGE_GAS_GIANT,
	{ { TERRAIN_GASGIANT },
	  { COLOR_GG_SATURN, COLOR_GG_JUPITER, COLOR_GG_URANUS, COLOR_GG_NEPTUNE } },
	// TYPE_PLANET_VERY_LARGE_GAS_GIANT,
	{ { TERRAIN_GASGIANT },
	  { COLOR_GG_SATURN, COLOR_GG_JUPITER, COLOR_GG_URANUS, COLOR_GG_NEPTUNE } },
	// TYPE_PLANET_ASTEROID,
	{ { TERRAIN_ASTEROID }, { COLOR_ROID } },
	// TYPE_PLANET_LARGE_ASTEROID,
	{ { TERRAIN_ASTEROID }, { COLOR_ROCK } },
	// TYPE_PLANET_DWARF,
	{ { TERRAIN_RUGGED_CRATERED }, { COLOR_ROCK } },
	// TYPE_PLANET_SMALL,
	{ { TERRAIN_RUGGED_CRATERED }, { COLOR_ROCK } },
	// TYPE_PLANET_WATER,
	{ { TERRAIN_H2O_SOLID }, { COLOR_ICEWORLD } },
	// TYPE_PLANET_DESERT,
	{ { TERRAIN_RUGGED_DESERT }, { COLOR_DESERT } },
	// TYPE_PLANET_CO2,
	{ { TERRAIN_RUGGED }, { COLOR_ROCK } },
	// TYPE_PLANET_METHANE,
	{ { TERRAIN_RUGGED }, { COLOR_ROCK } },
	// TYPE_PLANET_WATER_THICK_ATMOS,
	{ { TERRAIN_H2O_LIQUID }, { COLOR_EARTHLIKE } },
	// TYPE_PLANET_CO2_THICK_ATMOS,
	{ { TERRAIN_RUGGED }, { COLOR_ROCK } },
	// TYPE_PLANET_METHANE_THICK_ATMOS,
	{ { TERRAIN_RUGGED_METHANE }, { COLOR_METHANE } },
	// TYPE_PLANET_HIGHLY_VOLCANIC,
	{ { TERRAIN_RUGGED_LAVA }, { COLOR_VOLCANIC } },
	// TYPE_PLANET_INDIGENOUS_LIFE,
	{ { TERRAIN_RUGGED_H2O }, { COLOR_EARTHLIKE } },
	// TYPE_PLANET_TERRAFORMED_POOR,
	{ { TERRAIN_RUGGED_H2O }, { COLOR_TFPOOR } },
	// TYPE_PLANET_TERRAFORMED_GOOD,
	{ { TERRAIN_RUGGED_H2O_MEGAVOLC }, { COLOR_TFGOOD } },
	// TYPE_STARPORT_ORBITAL
	{},
	// TYPE_STARPORT_SURFACE
	{}
};

/**
 * All these stinking octavenoise functions return range [0,1] if persistence = 0.5
 */
inline double octavenoise(int octaves, double persistence, double lacunarity, vector3d p)
{
	double n = 0;
	double octaveAmplitude = 1.0;
	double jizm = 1.0;
	while (octaves--) {
		n += octaveAmplitude * noise(jizm*p);
		octaveAmplitude *= persistence;
		jizm *= lacunarity;
	}
	return 0.5 + n*0.5;
}
inline double river_octavenoise(int octaves, double persistence, double lacunarity, vector3d p)
{
	double n = 0;
	double octaveAmplitude = 1.0;
	double jizm = 1.0;
	while (octaves--) {
		n += octaveAmplitude * fabs(noise(jizm*p));
		octaveAmplitude *= persistence;
		jizm *= lacunarity;
	}
	return n;
}
inline double ridged_octavenoise(int octaves, double persistence, double lacunarity, vector3d p)
{
	double n = 0;
	double octaveAmplitude = 1.0;
	double jizm = 1.0;
	while (octaves--) {
		n += octaveAmplitude * (1.0 - fabs(noise(jizm*p)));
		octaveAmplitude *= persistence;
		jizm *= lacunarity;
	}
	return n;
}

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
		/*if (v > 1.0) {
			return (v<0 ? 0 : v + 100*octavenoise(24, 0.5, 2.0, 1000.0*pt));
		} else if (v < 0.8) {
			return (v<0 ? 0 : v + 100*octavenoise(18, 0.5, 2.0, 1000.0*pt));
		} else if (v < 0.6) {
			return (v<0 ? 0 : v + 100*octavenoise(16, 0.5, 2.0, 1000.0*pt));
		} else if (v < 0.4) {
			return (v<0 ? 0 : v + 100*octavenoise(14, 0.5, 2.0, 1000.0*pt));
		} else if (v < 0.2) {
			return (v<0 ? 0 : v + 100*octavenoise(12, 0.5, 2.0, 1000.0*pt));
		} else return (v<0 ? 0 : v + 100*octavenoise(10, 0.5, 2.0, 1000.0*pt));
		*/
		return (v<0 ? 0 : v + 100*octavenoise(10, 0.5, 2.0, 1000.0*pt) + (50*octavenoise(14, 0.5, 2.0, 1000.0*pt) + 
			50*octavenoise(8, 0.7, 1.0, 1000.0*pt) + 30*octavenoise(7, 0.6, 1.2, 1000.0*pt) + 30*octavenoise(6, 0.4, 1.8, 1000.0*pt)) *
			(v/400) + ((20*octavenoise(2, 0.0, 2.0, 100.0*pt))* ((v/200)*(v/200))));

	}
}

static inline double fractal(int octaves, fracdef_t &def, int type, const vector3d &p)
{
	switch (type) {
		case 0:	
			return river_octavenoise(octaves, 0.5, def.lacunarity, def.frequency * p);
		case 1:
			return ridged_octavenoise(octaves, 0.5, def.lacunarity, def.frequency * p);
		default:
			return octavenoise(octaves, 0.5, def.lacunarity, def.frequency * p);
	}
}

double canyon_function(const vector3d &p)
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

void crater_function_1pass(const vector3d &p, double &out, const double height)
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

void bigcrater_function_1pass(const vector3d &p, double &out, const double height)
{
	double n = fabs(noise(p));
	const double ejecta_outer = 0.6;
	const double outer = 0.88;
	const double inner = 0.955;
	const double midrim = 0.925;
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

double crater_function(const vector3d &p)  // makes large and small craters across the entire planet.
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

double bigcrater_function(const vector3d &p)  //should make a few very large craters like on Earth.
{
	double acrater = 0.0;
	double asz = 0.7;
	double amax_h = 0.5;
	for (int i=0; i<14; i++) {
		bigcrater_function_1pass(asz*p, acrater, amax_h);
		asz *= 1.0;
		amax_h *= 0.5;
	}
	return 4.5 * acrater;
}

void volcano_function_1pass(const vector3d &p, double &out, const double height)
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

double volcano_function(const vector3d &p)
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
void smlvolcano_function_1pass(const vector3d &p, double &out, const double height)
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

double smlvolcano_function(const vector3d &p)
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
void megavolcano_function_1pass(const vector3d &p, double &out, const double height)
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

double megavolcano_function(const vector3d &p)
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
	n = Clamp(n, 0.0, 1.0);
	return start*(1.0-n) + end*n;
}


GeoSphereStyle::GeoSphereStyle(const SBody *body)
{
	MTRand rand;
	rand.seed(body->seed);
	m_seed = body->seed;

	const sbody_valid_styles_t &styles = sbody_valid_styles[body->type];

	int numTerrain;
	int numColor;

	for (numTerrain=0; numTerrain<16; numTerrain++) {
		if (!styles.terrainType[numTerrain]) break;
	}
	for (numColor=0; numColor<16; numColor++) {
		if (!styles.colorType[numColor]) break;
	}
	printf("%d possible terrain and %d possible color for %s\n", numTerrain, numColor, body->name.c_str());

	if (numTerrain) {
		m_terrainType = styles.terrainType[ rand.Int32(numTerrain) ];
	} else {
		m_terrainType = TERRAIN_NONE;
	}
	if (numColor) {
		m_colorType = styles.colorType[ rand.Int32(numColor) ];
	} else {
		m_colorType = COLOR_NONE;
	}

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

	fprintf(stderr, "picked terrain %d, colortype %d for %s\n", (int)m_terrainType, (int)m_colorType, body->name.c_str());
	Init(m_terrainType, m_colorType, body->GetRadius(), (double)body->averageTemp, rand);
}

void GeoSphereStyle::Init(TerrainType t, ColorType c, double planetRadius, double averageTemp, MTRand &rand)
{
	m_terrainType = t;
	m_colorType = c;

	m_planetRadius = planetRadius;
	m_planetEarthRadii = planetRadius / EARTH_RADIUS;
	m_maxHeight = 3.61/sqrt(planetRadius);
	m_invMaxHeight = 1.0 / m_maxHeight;

	// Bunch of random calculated attributes
	m_icyness = 38.0 / (std::max(1.0, averageTemp-250.0));

	if (t == TERRAIN_ASTEROID) {
		m_maxHeight = rand.Double(0.2,0.4);
		m_invMaxHeight = 1.0 / m_maxHeight;
	}

	// make special arguments
	if ((t == TERRAIN_RUGGED) ||
	    (t == TERRAIN_RUGGED_CRATERED) ||
		(t == TERRAIN_RUGGED_METHANE) ||
	    (t == TERRAIN_RUGGED_LAVA)) {
		double totalAmp = 1.0;
		targ.mountains.amplitude = rand.Double(totalAmp);
		targ.mountains.frequency = 20.0 + 40.0*rand.Double();
		targ.mountains.lacunarity = rand.Double(1.8,2.2);
		totalAmp -= targ.mountains.amplitude;

		targ.continents.amplitude = rand.Double(totalAmp);
		targ.continents.frequency = 1.0 + rand.Double();
		targ.continents.lacunarity = rand.Double(1.8,2.2);
		totalAmp -= targ.continents.amplitude;

		targ.hills.amplitude = totalAmp;
		targ.hills.frequency = 64.0 + 128.0*rand.Double();
		targ.hills.lacunarity = rand.Double(1.8,2.2);

		targ.hillDistrib.amplitude = 1.0;
		targ.hillDistrib.frequency = pow(2.0, rand.Double(4.0));
		targ.hillDistrib.lacunarity = rand.Double(1.8, 2.2);

		targ.mountainDistrib.amplitude = 1.0;
		targ.mountainDistrib.frequency = pow(2.0, rand.Double(3.0));
		targ.mountainDistrib.lacunarity = rand.Double(1.8, 2.2);
	}
	if  ((t == TERRAIN_RUGGED_H2O) ||
		(t == TERRAIN_RUGGED_H2O_MEGAVOLC) ||
		(t == TERRAIN_H2O_SOLID) ||
		(t == TERRAIN_RUGGED_DESERT) ||
		(t == TERRAIN_H2O_LIQUID))  {
		double totalAmp = 1.0;
		targ.mountains.amplitude = rand.Double(totalAmp);
		targ.mountains.frequency = 4.0 + 32.0*rand.Double();
		targ.mountains.lacunarity = rand.Double(1.8,2.2);
		totalAmp -= targ.mountains.amplitude;

		targ.continents.amplitude = rand.Double(totalAmp);
		targ.continents.frequency = 1.0 + rand.Double();
		targ.continents.lacunarity = rand.Double(1.8,2.2);
		totalAmp -= targ.continents.amplitude;

		targ.hills.amplitude = totalAmp;
		targ.hills.frequency = 8.0 + 64.0*rand.Double();
		targ.hills.lacunarity = rand.Double(1.8,2.2);

		targ.hillDistrib.amplitude = 1.0;
		targ.hillDistrib.frequency = pow(3.0, rand.Double(6.0));
		targ.hillDistrib.lacunarity = rand.Double(1.8, 2.2);

		targ.mountainDistrib.amplitude = 0.01;
		targ.mountainDistrib.frequency = pow(0.0001, rand.Double(0.0004));
		targ.mountainDistrib.lacunarity = rand.Double(1.8, 2.2);
	}
	if (t == TERRAIN_RUGGED_LAVA) {
		targ.sealevel = rand.Double(0,0.3);
	}
	if (t == TERRAIN_RUGGED_METHANE) {
		targ.sealevel = rand.Double(0,0.2);
	}
	if ((t == TERRAIN_RUGGED_H2O) ||
		(t == TERRAIN_RUGGED_H2O_MEGAVOLC)) {
		targ.sealevel = rand.Double();
	}
	if (t == TERRAIN_H2O_LIQUID) {
		targ.sealevel = rand.Double(0.6,1);
	}
	if (t == TERRAIN_H2O_SOLID) {
		targ.sealevel = rand.Double(0.3,.8);
	}
	if (t == TERRAIN_ASTEROID) {    //simple randomness to asteroid noise levels.
		noise1 = rand.Double(2,14);
		noise2 = rand.Double(0.1,0.75);
		noise3 = rand.Double(0.1,2);
		targ.sealevel = rand.Double(1,4);
	}

	for (int i=0; i<12; i++) m_entropy[i] = rand.Double();
	for (int i=0; i<8; i++) {
		double r,g,b;
		r = rand.Double(0.0, 0.5);
		g = rand.Double(0.1, r);
		b = rand.Double(0.05, std::min(r, g));
		m_rockColor[i] = vector3d(r, g, b);
	}

	for (int i=0; i<12; i++) m_entropy[i] = rand.Double();
	for (int i=0; i<8; i++) {
		double g;
		g = rand.Double(0.0, 0.8);
		m_greyrockColor[i] = vector3d(g, g, g);
	}
}

double GeoSphereStyle::GetHeight(const vector3d &p)
{
	if (m_heightMap) return GetHeightMapVal(p) / m_planetRadius;

	switch (m_terrainType) {
		case TERRAIN_NONE:
		case TERRAIN_GASGIANT:
			return 0;
		case TERRAIN_ASTEROID:
			{
			
			//return m_maxHeight*(octavenoise(14, 0.5, 2.0, p)*0.3) + 0.005*crater_function(p);
			return m_maxHeight*(octavenoise(14, 0.5, 2.0, p)*0.6)*(octavenoise(noise1, noise2, noise3, p)*targ.sealevel) + 0.011*crater_function(p);
			}
		case TERRAIN_RUGGED_CRATERED:
		{
			double continents = targ.continents.amplitude * fractal(14, targ.continents, (m_seed)&3, p);
			double mountains = targ.mountains.amplitude * fractal(10, targ.mountains, (m_seed>>2)&3, p);
			double hills = fractal(6, targ.hillDistrib, (m_seed>>4)&3, p) *
				       targ.hills.amplitude * fractal(10, targ.hills, (m_seed>>6)&3, p);

			//return m_maxHeight * (continents + hills + mountains + crater_function(p)) * (crater_function(p) + 1);
			double n = (bigcrater_function(p) /2) + (continents * 4) - (targ.continents.amplitude * 2);// + canyon_function(p);
			n += continents + hills + mountains + crater_function(p);
			//n += n * n;
			//n += bigcrater_function(p);
			n = n * m_maxHeight;
			return n;
		}
		case TERRAIN_RUGGED:
		// same as above but no craters because of atmosphere
		{
			double continents = targ.continents.amplitude * fractal(14, targ.continents, (m_seed)&3, p);
			double mountains = targ.mountains.amplitude * fractal(10, targ.mountains, (m_seed>>2)&3, p);
			double hills = fractal(2, targ.hillDistrib, (m_seed>>4)&3, p) *
				       targ.hills.amplitude * fractal(10, targ.hills, (m_seed>>6)&3, p);

			return m_maxHeight * (continents + hills + mountains + volcano_function(p));
		}
		case TERRAIN_RUGGED_METHANE:
		// same as above but no craters because of atmosphere
		{
			double continents = targ.continents.amplitude * fractal(14, targ.continents, (m_seed)&3, p);
			double mountains = targ.mountains.amplitude * fractal(10, targ.mountains, (m_seed>>2)&3, p);
			double hills = fractal(4, targ.hillDistrib, (m_seed>>4)&3, p) *
				       targ.hills.amplitude * fractal(10, targ.hills, (m_seed>>6)&3, p);

			double n = continents - targ.continents.amplitude*targ.sealevel;
			if (n > 0.0) {
				// smooth in hills at shore edges
				if (n < 0.01) n += hills * n * 100.0f;
				else n += hills;

				mountains = fractal(6, targ.mountainDistrib, (m_seed>>8)&3, p) *
					targ.mountains.amplitude * mountains*mountains*mountains*mountains*2;
				if (n < 0.01) n += mountains * n * 100.0f;
				else n += mountains;
			}
			n = (n<0.0 ? 0.0 : m_maxHeight*n);
			return n;
		}
		case TERRAIN_RUGGED_LAVA:
		{
			double continents = targ.continents.amplitude * fractal(14, targ.continents, (m_seed)&3, p);
			double mountains = fractal(13, targ.mountains, (m_seed>>2)&3, p);
			double hills = fractal(14, targ.hillDistrib, (m_seed>>4)&3, p) *
				       targ.hills.amplitude * fractal(13, targ.hills, (m_seed>>6)&3, p);

			double n = continents - targ.continents.amplitude*targ.sealevel + volcano_function(p) ;//+ bigcrater_function(p);
			if (n > 0.0) {
				// smooth in hills at shore edges
				if (n < 0.01) n += hills * n * 100.0f ;
				else n += hills ;

				mountains = fractal(2, targ.mountainDistrib, (m_seed>>2)&3, p) *
					targ.mountains.amplitude * mountains*mountains*mountains;
				if (n < 0.01) n += mountains * n * 100.0f ;
				else n += mountains ;
			}
			n = (n<0.0 ? 0.0 : m_maxHeight*n);
			return n;
		}
		case TERRAIN_H2O_SOLID:
		{
			double continents = targ.continents.amplitude * fractal(14, targ.continents, (m_seed)&2, p);
			double mountains = fractal(13, targ.mountains, (m_seed>>2)&3, p);
			double hills = fractal(4, targ.hillDistrib, (m_seed>>4)&3, p) *
				       targ.hills.amplitude * fractal(2, targ.hills, (m_seed>>8)&3, p);

			double n = continents - targ.continents.amplitude*targ.sealevel;
			
			if (n > 0.0) {
				// smooth in hills at shore edges 
				if (n < 0.01) n += hills * n * 100.0f ;
				else n += hills ;
				// adds mountains hills craters 
				mountains = fractal(3, targ.mountainDistrib, (m_seed>>8)&3, p) *
					targ.mountains.amplitude * mountains*mountains*mountains;
				if (n < 0.01) n += n * 100.0f * mountains;
				else n += mountains;
				
				//n += (continents * targ.sealevel) * 10;

			}
			
			//n += hills  +  crater_function(p) +  smlvolcano_function(p) ;
			n += hills + (smlvolcano_function(p) * 5) + (crater_function(p) * 0.35) ;
				
				//n += n + 0.1;
			
			n = (n<0.0 ? 0.0 : m_maxHeight*n);
			return n;
		}
		case TERRAIN_H2O_LIQUID:
		{
			double continents = targ.continents.amplitude * fractal(14, targ.continents, (m_seed)&3, p);
			double mountains = fractal(13, targ.mountains, (m_seed>>2)&3, p);
			double hills = fractal(6, targ.hillDistrib, (m_seed>>4)&3, p) *
				       targ.hills.amplitude * fractal(13, targ.hills, (m_seed>>8)&4, p);

			double n = continents * volcano_function(p) - (targ.continents.amplitude*targ.sealevel * 1.7) ;
			if (n > 0.0) {
				
				
				if (n < 0.01) n += hills * n * 100.0f;
				else n += hills;

				mountains = fractal(12, targ.mountainDistrib, (m_seed>>8)&3, p) *
					targ.mountains.amplitude * mountains*mountains*mountains*mountains*mountains;
				if (n < 0.01) n += mountains * n * 200.0f;
				else n += mountains * 2.0f;
				
			}
			n = (n<0.0 ? 0.0 : m_maxHeight*n);
			return n;
		}
		case TERRAIN_RUGGED_DESERT:
		{
			double continents = targ.continents.amplitude * fractal(6, targ.continents, (m_seed>>6)&3, p);
			double mountains = fractal(14, targ.mountains, (m_seed>>2)&3, p);
			double dunes = fractal(8, targ.hillDistrib, (m_seed>>6)&3, p) *
				       targ.hills.amplitude * fractal(4, targ.hills, (m_seed>>6)&3, p);  //If you can find a better seed for creating dunes, please place it here
			
			double n = continents + (canyon_function(p) / 3) + (smlvolcano_function(p)*1);
				n += continents * targ.continents.amplitude; 
				n += n/2;
				
				// makes larger dunes at lower altitudes, flat ones at high altitude.
				if (n > 0.2) n += dunes * (0.5/n);  
				if (n < 0.2) n += dunes * 2.5;
				//n += dunes;
				
				

					
				mountains = fractal(9, targ.mountainDistrib, (m_seed>>8)&3, p) *
					targ.mountains.amplitude * mountains*mountains*mountains;
				double mountains2 = fractal(4, targ.mountainDistrib, (m_seed>>8)&3, p) *
					targ.mountains.amplitude * mountains*mountains*mountains;
				// smoothes edges of mountains and places them only above a set altitude
				if (n > 0.55) n += mountains * (n - 0.55) * (1/n);
				if (n < 0.10) n += mountains2 * n * 5.0f;
				else n += mountains2 * 0.5f;

				
			
			n = (n<0.0 ? 0.0 : m_maxHeight*n);
			return n;
		}
		case TERRAIN_RUGGED_H2O:
		{
			double continents = targ.continents.amplitude * fractal(14, targ.continents, (m_seed)&3, p);
			double mountains = fractal(13, targ.mountains, (m_seed>>2)&3, p);
			double hills = fractal(6, targ.hillDistrib, (m_seed>>4)&3, p) *
				       targ.hills.amplitude * fractal(13, targ.hills, (m_seed>>6)&3, p);

			double n = continents - targ.continents.amplitude*targ.sealevel;
			if (n > 0.0) {
				// smooth in hills at shore edges
				if (n < 0.01) n += hills * n * 100.0f;
				else n += hills;

				mountains = fractal(6, targ.mountainDistrib, (m_seed>>8)&3, p) *
					targ.mountains.amplitude * mountains*mountains;
				if (n < 0.01) n += mountains * n * 100.0f + (smlvolcano_function(p) * 3 * smlvolcano_function(p));
				else n += mountains + (smlvolcano_function(p) * 3 * smlvolcano_function(p));
			}
			n = (n<0.0 ? 0.0 : m_maxHeight*n);
			return n;
		}
		case TERRAIN_RUGGED_H2O_MEGAVOLC:
		{
			double continents = targ.continents.amplitude * fractal(14, targ.continents, (m_seed)&2, p);
			double mountains = fractal(13, targ.mountains, (m_seed>>2)&3, p);
			double mountains2 = fractal(13, targ.mountains, (m_seed>>8)&3, p);
			double hills = fractal(6, targ.hillDistrib, (m_seed>>4)&3, p) *
				       targ.hills.amplitude * fractal(13, targ.hills, (m_seed>>6)&3, p);

			double n = continents - targ.continents.amplitude*targ.sealevel*1;
			if (n < 0.01) n += 3*megavolcano_function(p) * n * 100.0f;
			else n += 3*megavolcano_function(p);
			if (n > 0.0) {
				// smooth in hills at shore edges
				if (n < 0.01) n += hills * n * 100.0f;
				else n += hills;

				mountains = fractal(1, targ.mountainDistrib, (m_seed>>8)&3, p) *
					targ.mountains.amplitude * mountains*mountains*mountains;
				mountains2 = fractal(36, targ.mountainDistrib, (m_seed>>8)&3, p) *
					targ.mountains.amplitude * mountains2;
				if (n > 2.2) n += mountains2 * (n - 2.2) * 0.6;
				if (n < 0.01) n += mountains * n * 40.0f ;
				else n += mountains * 0.4f ; 
			}
			
			n = (n<0.0 ? 0.0 : m_maxHeight*n);
			return n;
		}
		default:
			return 0;
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
		col = interpolate_color(n, col, m_greyrockColor[2]);
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
		col = interpolate_color(equatorial_desert, vector3d(0.22,0.12,.0), vector3d(0.4, 0.7, 0.2));
		col = interpolate_color(n, col, m_rockColor[3]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.01) {       //                       Brown/yellow
		col = interpolate_color(equatorial_desert, vector3d(0.1,0.28,0), vector3d(.04, .2, 0));
		col = interpolate_color(n, col, m_rockColor[1]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.005) {       //                       Brown
		col = interpolate_color(equatorial_desert, vector3d(0.2,0.18,.0), vector3d(.6, .8, .0));
		col = interpolate_color(n, col, m_rockColor[0]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else {                    //                         Yellow
		col = interpolate_color(equatorial_desert, vector3d(0.9,0.84,0), vector3d(.2, .1, .0));
		col = interpolate_color(n, col, m_rockColor[2]);
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

		const double flatness = pow(vector3d::Dot(p, norm), 6.0);
		const vector3d color_cliffs = m_rockColor[2];
		// ice on mountains and poles
		if (fabs(m_icyness*p.y) + m_icyness*n > 18) {
			return interpolate_color(flatness, color_cliffs, vector3d(.99,.99,.99));
		}

		double equatorial_desert = (2.0-m_icyness)*(-1.0+2.0*octavenoise(12, 0.5, 2.0, (n*2.0)*p)) *
				1.0*(2.0-m_icyness)*(1.0-p.y*p.y);

		vector3d col;
		// latitude: high ~col, low orange
		col = interpolate_color(equatorial_desert, m_greyrockColor[4], vector3d(.99,.99,.99));
		// height: dark grey, dark brown
		col = interpolate_color(n, col, m_greyrockColor[0]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
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
		// latitude: high ~col, low brown/orange
		col = interpolate_color(equatorial_desert, vector3d(.12,.1,0), vector3d(.86, .75, .48));
		// height: brown, light yellow/brown
		col = interpolate_color(n, col, m_rockColor[0]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
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
		col = interpolate_color(equatorial_desert, m_rockColor[2], m_rockColor[3]);
		if (n > 0.45) {
		col = interpolate_color(n, col, m_greyrockColor[3]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.4) {
		col = interpolate_color(n, col, m_greyrockColor[5]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.35) {
		col = interpolate_color(n, col, m_greyrockColor[6]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else {
		col = interpolate_color(n, col, m_greyrockColor[7]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		if (n > 0.3) {
		col = interpolate_color(n, col, m_rockColor[4]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.25) {
		col = interpolate_color(n, col, m_greyrockColor[0]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.2) {
		col = interpolate_color(n, col, m_greyrockColor[1]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else {
		col = interpolate_color(n, col, m_greyrockColor[2]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		if (n > 0.15) {
		col = interpolate_color(n, col, m_greyrockColor[4]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.1) {
		col = interpolate_color(n, col, m_rockColor[5]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.05) {
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
	case COLOR_ROID:
		{
		double n = m_invMaxHeight*height/2;

		if (n <= 0.02) {
			const double flatness = pow(vector3d::Dot(p, norm), 6.0);
			const vector3d color_cliffs = m_rockColor[1];

			double equatorial_desert = (2.0)*(-1.0+2.0*octavenoise(12, 0.5, 2.0, (n*2.0)*p)) *
				1.0*(2.0)*(1.0-p.y*p.y);

			vector3d col;
			col = interpolate_color(equatorial_desert, m_rockColor[0], m_greyrockColor[3]);
			col = interpolate_color(n, col, m_rockColor[2]);
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
		else if (n > 0.03) {       //                       Dark Green         Light Green
		col = interpolate_color(equatorial_desert, vector3d(0,.2,0), vector3d(.0, .6, .0));
		col = interpolate_color(n, col, m_rockColor[0]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.01) {       //                       Brown
		col = interpolate_color(equatorial_desert, vector3d(.2,.6,0), vector3d(.1, .2, .0));
		col = interpolate_color(n, col, m_rockColor[1]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else {                    //                         
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
		const vector3d color_cliffs = m_rockColor[1];
		// ice on mountains and poles
		if (fabs(m_icyness*p.y) + m_icyness*n > 1) {
			return interpolate_color(flatness, color_cliffs, vector3d(0.6,0.6,0.6));
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
		else if (n > 0.4) {
		col = interpolate_color(n, col, m_greyrockColor[5]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.35) {
		col = interpolate_color(n, col, m_greyrockColor[6]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else {
		col = interpolate_color(n, col, m_greyrockColor[7]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		if (n > 0.3) {
		col = interpolate_color(n, col, m_rockColor[4]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.25) {
		col = interpolate_color(n, col, m_greyrockColor[0]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.2) {
		col = interpolate_color(n, col, m_greyrockColor[1]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else {
		col = interpolate_color(n, col, m_greyrockColor[2]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		if (n > 0.15) {
		col = interpolate_color(n, col, m_greyrockColor[4]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.1) {
		col = interpolate_color(n, col, m_rockColor[5]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.05) {
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
