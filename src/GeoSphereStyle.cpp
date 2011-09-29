#include "GeoSphereStyle.h"
#include "perlin.h"
#include "Pi.h"


/**
 * All these stinking octavenoise functions return range [0,1] if roughness = 0.5
 */
static inline double octavenoise(fracdef_t &def, double roughness, const vector3d &p);
static inline double river_octavenoise(fracdef_t &def, double roughness, const vector3d &p);
static inline double ridged_octavenoise(fracdef_t &def, double roughness, const vector3d &p);
static inline double billow_octavenoise(fracdef_t &def, double roughness, const vector3d &p);
static inline double voronoiscam_octavenoise(fracdef_t &def, double roughness, const vector3d &p);
static inline double dunes_octavenoise(fracdef_t &def, double roughness, const vector3d &p);
//static inline double dunes_octavenoise(int octaves, double roughness, double lacunarity, const vector3d &p);
static double canyon_ridged_function(const fracdef_t &def, const vector3d &p);
static double canyon2_ridged_function(const fracdef_t &def, const vector3d &p);
static double canyon3_ridged_function(const fracdef_t &def, const vector3d &p);
static double canyon_normal_function(const fracdef_t &def, const vector3d &p);
static double canyon2_normal_function(const fracdef_t &def, const vector3d &p);
static double canyon3_normal_function(const fracdef_t &def, const vector3d &p);
static double canyon_voronoi_function(const fracdef_t &def, const vector3d &p);
static double canyon2_voronoi_function(const fracdef_t &def, const vector3d &p);
static double canyon3_voronoi_function(const fracdef_t &def, const vector3d &p);
static double canyon_billow_function(const fracdef_t &def, const vector3d &p);
static double canyon2_billow_function(const fracdef_t &def, const vector3d &p);
static double canyon3_billow_function(const fracdef_t &def, const vector3d &p);
static double crater_function(const fracdef_t &def, const vector3d &p);
static double impact_crater_function(const fracdef_t &def, const vector3d &p);
static double volcano_function(const fracdef_t &def, const vector3d &p);
static double megavolcano_function(const fracdef_t &def, const vector3d &p);
static double river_function(const fracdef_t &def, const vector3d &p, int style = 0);
//static double cliff_function(const fracdef_t &def, const vector3d &p);

int GeoSphereStyle::GetRawHeightMapVal(int x, int y)
{
	return m_heightMap[Clamp(y, 0, m_heightMapSizeY-1)*m_heightMapSizeX + Clamp(x, 0, m_heightMapSizeX-1)];
}

/*
 * Bicubic interpolation!!!
 */


double GeoSphereStyle::GetHeightMapVal(const vector3d &pt)
{     // This is all used for Earth and Earth alone
	double latitude = -asin(pt.y);
	if (pt.y < -1.0) latitude = -0.5*M_PI;
	if (pt.y > 1.0) latitude = 0.5*M_PI;
//	if (!isfinite(latitude)) {
//		// pt.y is just n of asin domain [-1,1]
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
		
		v = (v<0 ? 0 : v);
		double h = v;
		/*
		if (textures) {
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, 10, rand, 10*m_fracmult);
			SetFracDef(&m_fracdef[1], m_maxHeightInMeters, 25, rand, 10*m_fracmult);
		}
		//small fractal/high detail
		SetFracDef(&m_fracdef[2-m_fracnum], m_maxHeightInMeters*0.05, 50, rand, 10*m_fracmult);//[2]
		//continental/large type fractal
		SetFracDef(&m_fracdef[3-m_fracnum], m_maxHeightInMeters, 1e6, rand, 200*m_fracmult);//[0]
		SetFracDef(&m_fracdef[4-m_fracnum], m_maxHeightInMeters, 1e5, rand, 100*m_fracmult);//[4]
		//medium fractal
		SetFracDef(&m_fracdef[5-m_fracnum], m_maxHeightInMeters, 2e4, rand, 500*m_fracmult);//[5]
		SetFracDef(&m_fracdef[6-m_fracnum], m_maxHeightInMeters*0.2, 5e3.0, rand, 100*m_fracmult);//[3]
		*/
		//Here's where we add some noise over the heightmap so it doesnt look so boring, we scale by height so values are greater high up
		//large mountainous shapes
		double mountains = h*h*0.001*octavenoise(m_fracdef[3-m_fracnum], 0.5*octavenoise(m_fracdef[5-m_fracnum], 0.45, pt),
			pt)*ridged_octavenoise(m_fracdef[4-m_fracnum], 0.475*octavenoise(m_fracdef[6-m_fracnum], 0.4, pt), pt);
		v += mountains;
		//smaller ridged mountains
		if (v < 50.0){
			v += v*v*0.04*ridged_octavenoise(m_fracdef[5-m_fracnum], 0.5, pt);
		} else if (v <100.0){
			v += 100.0*ridged_octavenoise(m_fracdef[5-m_fracnum], 0.5, pt);
		} else {
			v += (100.0/v)*(100.0/v)*(100.0/v)*(100.0/v)*(100.0/v)*
				100.0*ridged_octavenoise(m_fracdef[5-m_fracnum], 0.5, pt);
		}
		//high altitude detail/mountains
		//v += Clamp(h, 0.0, 0.5)*octavenoise(m_fracdef[2-m_fracnum], 0.5, pt);	
		
		//low altitude detail/dunes
		//v += h*0.000003*ridged_octavenoise(m_fracdef[2-m_fracnum], Clamp(1.0-h*0.002, 0.0, 0.5), pt);			
		if (v < 10.0){
			v += 2.0*v*dunes_octavenoise(m_fracdef[6-m_fracnum], 0.5, pt)
				*octavenoise(m_fracdef[6-m_fracnum], 0.5, pt);
		} else if (v <50.0){
			v += 20.0*dunes_octavenoise(m_fracdef[6-m_fracnum], 0.5, pt)
				*octavenoise(m_fracdef[6-m_fracnum], 0.5, pt);
		} else {
			v += (50.0/v)*(50.0/v)*(50.0/v)*(50.0/v)*(50.0/v)
				*20.0*dunes_octavenoise(m_fracdef[6-m_fracnum], 0.5, pt)
				*octavenoise(m_fracdef[6-m_fracnum], 0.5, pt);
		}		
		if (v<40.0) {
			//v = v;
		} else if (v <60.0){
			v += (v-40.0)*billow_octavenoise(m_fracdef[5-m_fracnum], 0.5, pt);
			//printf("V/height: %f\n", Clamp(v-20.0, 0.0, 1.0));
		} else {
			v += (30.0/v)*(30.0/v)*(30.0/v)*20.0*billow_octavenoise(m_fracdef[5-m_fracnum], 0.5, pt);
		}		
		
		//ridges and bumps
		//v += h*0.1*ridged_octavenoise(m_fracdef[6-m_fracnum], Clamp(h*0.0002, 0.3, 0.5), pt) 
		//	* Clamp(h*0.0002, 0.1, 0.5);
		v += h*0.2*voronoiscam_octavenoise(m_fracdef[5-m_fracnum], Clamp(1.0-(h*0.0002), 0.0, 0.6), pt) 
			* Clamp(1.0-(h*0.0006), 0.0, 1.0);
		//polar ice caps with cracks
		if ((m_icyness*0.5)+(fabs(pt.y*pt.y*pt.y*0.38)) > 0.6) {
			h = Clamp(1.0-(v*10.0), 0.0, 1.0)*voronoiscam_octavenoise(m_fracdef[5-m_fracnum], 0.5, pt);
			h *= h*h*2.0;
			h -= 3.0;
			v += h;
		}
		return (v<0 ? 0 : v);
	}
}

static inline vector3d interpolate_color(double n, vector3d start, vector3d end)
{
	n = Clamp(n, 0.0, 1.0);
	return start*(1.0-n) + end*n;
}

void GeoSphereStyle::ChangeDetailLevel()
{

	switch (Pi::detail.textures) {
		case 0: textures = false;
			m_fracnum = 2;break;
		default:
		case 1: textures = true;
			m_fracnum = 0;break;
	}

	switch (Pi::detail.fracmult) {
		case 0: m_fracmult = 100;break;
		case 1: m_fracmult = 10;break;
		case 2: m_fracmult = 1;break;
		case 3: m_fracmult = 0.5;break;
		default:
		case 4: m_fracmult = 0.1;break;
	}

	MTRand rand;
	rand.seed(m_seed);

	PickTerrain(rand);
	InitFractalType(rand);
	//fprintf(stderr, "picked terrain %d, colortype %d for %s\n", (int)m_terrainType, (int)m_colorType, body->name.c_str());
}

void GeoSphereStyle::PickTerrain(MTRand &rand)
{
	/* Pick terrain and color fractals to use */
	if (m_body->type == SBody::TYPE_BROWN_DWARF) {
		m_terrainType = TERRAIN_FLAT;
		m_colorType = COLOR_STAR_BROWN_DWARF;
	} else if (m_body->type == SBody::TYPE_WHITE_DWARF) {
		m_terrainType = TERRAIN_FLAT;
		m_colorType = COLOR_STAR_WHITE_DWARF;
	} else if ((m_body->type == SBody::TYPE_STAR_M) || (m_body->type == SBody::TYPE_STAR_M_GIANT) ||
	(m_body->type == SBody::TYPE_STAR_M_SUPER_GIANT) || (m_body->type == SBody::TYPE_STAR_M_SUPER_GIANT)){ 
		m_terrainType = TERRAIN_FLAT;
		const enum ColorFractal choices[] = {
				COLOR_STAR_M,
				COLOR_STAR_M,
				COLOR_STAR_K,
				COLOR_STAR_G,
			};
			m_colorType = choices[rand.Int32(4)];
	} else if ((m_body->type == SBody::TYPE_STAR_K) || (m_body->type == SBody::TYPE_STAR_K_GIANT) ||
	(m_body->type == SBody::TYPE_STAR_K_SUPER_GIANT) || (m_body->type == SBody::TYPE_STAR_K_SUPER_GIANT)){ 
		m_terrainType = TERRAIN_FLAT;
		const enum ColorFractal choices[] = {
				COLOR_STAR_M,
				COLOR_STAR_K,
				COLOR_STAR_K,
				COLOR_STAR_G,
			};
			m_colorType = choices[rand.Int32(3)];
	} else if ((m_body->type == SBody::TYPE_STAR_G) || (m_body->type == SBody::TYPE_STAR_G_GIANT) ||
	(m_body->type == SBody::TYPE_STAR_G_SUPER_GIANT) || (m_body->type == SBody::TYPE_STAR_G_SUPER_GIANT)){ 
		m_terrainType = TERRAIN_FLAT;
		const enum ColorFractal choices[] = {
				COLOR_STAR_WHITE_DWARF,
				COLOR_STAR_G,
			};
			m_colorType = choices[rand.Int32(2)];
	} else if (m_body->type < SBody::TYPE_PLANET_GAS_GIANT) {
		m_terrainType = TERRAIN_FLAT;
		m_colorType = COLOR_SOLID;
	} else if (m_body->type == SBody::TYPE_PLANET_GAS_GIANT) {
		m_terrainType = TERRAIN_FLAT;
		switch (rand.Int32(5)) {
			case 0: m_colorType = COLOR_GG_SATURN; break;
			case 1: m_colorType = COLOR_GG_SATURN2; break;
			case 2: m_colorType = COLOR_GG_URANUS; break;
			case 3: m_colorType = COLOR_GG_JUPITER; break;
			case 4: m_colorType = COLOR_GG_NEPTUNE; break;
			default: m_colorType = COLOR_GG_NEPTUNE2; break;
		}
	} else if (m_body->type == SBody::TYPE_PLANET_ASTEROID) {
		m_terrainType = TERRAIN_ASTEROID;
		m_colorType = COLOR_ASTEROID;
	} else /* SBody::TYPE_PLANET_TERRESTRIAL */ {
		/* Pick terrain and color fractals for terrestrial planets */
		//Earth-like world
		if ((m_body->m_life > fixed(7,10)) &&  
		   (m_body->m_volatileGas > fixed(2,10))){
			   // There would be no life on the surface without atmosphere
			const enum TerrainFractal choices[] = {
				TERRAIN_HILLS_RIDGED,
				TERRAIN_HILLS_RIVERS,
				TERRAIN_HILLS_DUNES,
				TERRAIN_MOUNTAINS_RIDGED,
				TERRAIN_MOUNTAINS_NORMAL,  // HQ terrain
				TERRAIN_MOUNTAINS_RIVERS,  // HQ terrain
				TERRAIN_MOUNTAINS_VOLCANO,
				TERRAIN_MOUNTAINS_RIVERS_VOLCANO,
			};
			m_terrainType = choices[rand.Int32(8)];
			//m_terrainType = TERRAIN_MOUNTAINS_NORMAL;
			if (m_body->averageTemp > 240) {
				m_colorType = COLOR_EARTHLIKE;
			} else {
				m_colorType = COLOR_DESERT;
			}
			printf("| Earth-like world  temp: %d\n", m_body->averageTemp);
		}//Harsh, habitable world 
		else if ((m_body->m_volatileGas > fixed(2,10)) &&
				  (m_body->m_life > fixed(4,10)) ) {
			const enum TerrainFractal choices[] = {
				TERRAIN_HILLS_RIDGED,
				TERRAIN_HILLS_RIVERS,
				TERRAIN_HILLS_DUNES,
				TERRAIN_HILLS_NORMAL,
				TERRAIN_MOUNTAINS_NORMAL,
				TERRAIN_MOUNTAINS_RIDGED,
				TERRAIN_MOUNTAINS_VOLCANO,
				TERRAIN_MOUNTAINS_RIVERS_VOLCANO,
				TERRAIN_MOUNTAINS_RIVERS,
				TERRAIN_RUGGED_DESERT,
			};
			m_terrainType = choices[rand.Int32(10)];
			//m_terrainType = TERRAIN_MOUNTAINS_RIVERS;
			if (m_body->averageTemp > 240) {
				m_colorType = COLOR_TFGOOD;;
			} else {
				m_colorType = COLOR_ICEWORLD;
			}
			printf("| Harsh, habitable world temp: %d\n", m_body->averageTemp);
		}// Marginally habitable world/ verging on mars like :) 
		else if ((m_body->m_volatileGas > fixed(1,10)) &&
				  (m_body->m_life > fixed(1,10)) ) {
			const enum TerrainFractal choices[] = {
				TERRAIN_HILLS_RIDGED,
				TERRAIN_HILLS_RIVERS,
				TERRAIN_HILLS_DUNES,
				TERRAIN_HILLS_NORMAL,
				TERRAIN_MOUNTAINS_NORMAL,
				TERRAIN_MOUNTAINS_RIDGED,
				TERRAIN_MOUNTAINS_VOLCANO,
				TERRAIN_MOUNTAINS_RIVERS_VOLCANO,
				TERRAIN_MOUNTAINS_RIVERS,
				TERRAIN_RUGGED_DESERT,
			};
			m_terrainType = choices[rand.Int32(10)];
			//m_terrainType = TERRAIN_MOUNTAINS_RIVERS;
			if (m_body->averageTemp > 240) {
				m_colorType = COLOR_TFPOOR;;
			} else {
				m_colorType = COLOR_ICEWORLD;
			}
			printf("| Marginally habitable world temp: %d\n", m_body->averageTemp);
		} // Desert-like world, Mars -like.
		else if ((m_body->m_volatileLiquid < fixed(1,10)) &&
		           (m_body->m_volatileGas > fixed(1,5))) {
			const enum TerrainFractal choices[] = {
				TERRAIN_HILLS_DUNES,
				TERRAIN_H2O_SOLID,
				TERRAIN_RUGGED_DESERT,
				TERRAIN_RUGGED_LAVA,
				TERRAIN_MOUNTAINS_VOLCANO,
				TERRAIN_MOUNTAINS_RIVERS_VOLCANO,
			};
			m_terrainType = choices[rand.Int32(6)];
			//m_terrainType = TERRAIN_MOUNTAINS_NORMAL;
			m_colorType = COLOR_DESERT;
			printf("| Desert-like world. temp: %d\n", m_body->averageTemp);
		} // Frozen world
		else if ((m_body->m_volatileIces > fixed(8,10)) &&  
		           (m_body->averageTemp < 250)) {
			const enum TerrainFractal choices[] = {
				TERRAIN_HILLS_DUNES,
				TERRAIN_HILLS_CRATERS,
				TERRAIN_MOUNTAINS_CRATERS,
				TERRAIN_H2O_SOLID,
				TERRAIN_H2O_SOLID_CANYONS,
				TERRAIN_RUGGED_DESERT,
			};
			m_terrainType = choices[rand.Int32(6)];
			m_colorType = COLOR_ICEWORLD;
			printf("| Frozen world. temp: %d\n", m_body->averageTemp);
		} else if (m_body->m_volcanicity > fixed(7,10)) {
					   // Volcanic world
			m_terrainType = TERRAIN_RUGGED_LAVA;
			if (m_body->m_life > fixed(5,10)) { // life on a volcanic world ;)
				m_colorType = COLOR_TFGOOD;
			} else if (m_body->m_life > fixed(2,10)) {
				m_colorType = COLOR_TFPOOR;
				printf("| Volcanic world with Life. temp: %d\n", m_body->averageTemp);
			} else {
				m_colorType = COLOR_VOLCANIC;
				printf("| Volcanic world. temp: %d\n", m_body->averageTemp);
			}
		//Below might not be needed.
		//Alien life world:
		} else if (m_body->m_life > fixed(1,10))  {
			const enum TerrainFractal choices[] = {
				TERRAIN_HILLS_DUNES,
				TERRAIN_HILLS_RIDGED,
				TERRAIN_HILLS_RIVERS,
				TERRAIN_MOUNTAINS_NORMAL,
				TERRAIN_MOUNTAINS_RIDGED,
				TERRAIN_MOUNTAINS_VOLCANO,
				TERRAIN_MOUNTAINS_RIVERS_VOLCANO,
				TERRAIN_MOUNTAINS_RIVERS,
				TERRAIN_H2O_SOLID,
				TERRAIN_RUGGED_LAVA,
				TERRAIN_RUGGED_DESERT,
			};
			//m_terrainType = TERRAIN_HILLS_DUNES;
			m_terrainType = choices[rand.Int32(11)];
			m_colorType = COLOR_TFPOOR;
			printf("| Alien life world. temp: %d\n", m_body->averageTemp);
		} else if (m_body->m_volatileGas > fixed(1,10)) {
				const enum TerrainFractal choices[] = {
				TERRAIN_HILLS_NORMAL,
				TERRAIN_MOUNTAINS_NORMAL,
				TERRAIN_RUGGED_DESERT,
			};
			m_terrainType = choices[rand.Int32(3)];
			m_colorType = COLOR_ROCK;
			printf("| Rock ball with atmosphere. temp: %d\n", m_body->averageTemp);
		} else if (m_body->m_volatileGas > fixed(1,20)) {
				const enum TerrainFractal choices[] = {
				TERRAIN_HILLS_CRATERS,
				TERRAIN_MOUNTAINS_CRATERS,
				TERRAIN_H2O_SOLID,
			};
			m_terrainType = choices[rand.Int32(3)];
			m_colorType = COLOR_ROCK;
			printf("| Rock ball marginal atmosphere. temp: %d\n", m_body->averageTemp);
		} else {
			const enum TerrainFractal choices[] = {
				TERRAIN_HILLS_CRATERS2,
				TERRAIN_MOUNTAINS_CRATERS2,
			};
			m_terrainType = choices[rand.Int32(2)];
			m_colorType = COLOR_ROCK;
			printf("| Rock ball with craters. temp: %d\n", m_body->averageTemp);
		}
	}
	// XXX override the above so you can test particular fractals XXX

	//m_terrainType = TERRAIN_HILLS_DUNES;
	//m_colorType = COLOR_DESERT;
	printf("%s: \n", m_body->name.c_str());
	printf("|   Terrain: [%d]\n", m_terrainType);
	printf("|    Colour: [%d]\n", m_colorType);

	m_sealevel = Clamp(m_body->m_volatileLiquid.ToDouble(), 0.0, 1.0);
	m_icyness = Clamp(m_body->m_volatileIces.ToDouble(), 0.0, 1.0);
	m_volcanic = Clamp(m_body->m_volcanicity.ToDouble(), 0.0, 1.0); // height scales with volcanicity as well

	const double rad = m_body->GetRadius();
	m_maxHeightInMeters = std::max(100.0, (9000.0*rad*rad*(m_volcanic+0.5)) / (m_body->GetMass() * 6.64e-12));
	if (!isfinite(m_maxHeightInMeters)) m_maxHeightInMeters = rad * 0.5;
	//             ^^^^ max mountain height for earth-like planet (same mass, radius)
	// and then in sphere normalized jizz
	m_maxHeight = std::min(0.5, m_maxHeightInMeters / rad);
	//printf("%s: max terrain height: %fm [%f]\n", m_body->name.c_str(), m_maxHeightInMeters, m_maxHeight);
	m_invMaxHeight = 1.0 / m_maxHeight;
	m_planetRadius = rad;
	m_planetEarthRadii = rad / EARTH_RADIUS;

	// Pick some colors, mainly reds and greens
	for (int i=0; i<12; i++) m_entropy[i] = rand.Double();
	for (int i=0; i<8; i++) {
		double r,g,b;
		r = rand.Double(0.3, 1.0);
		g = rand.Double(0.3, r);
		b = rand.Double(0.3, g);
		r = std::max(b, r * m_body->m_metallicity.ToFloat());
		g = std::max(b, g * m_body->m_metallicity.ToFloat());
		m_rockColor[i] = vector3d(r, g, b);
	}

	// Pick some darker colours mainly reds and greens
	for (int i=0; i<12; i++) m_entropy[i] = rand.Double();
	for (int i=0; i<8; i++) {
		double r,g,b;
		r = rand.Double(0.05, 0.3);
		g = rand.Double(0.05, r);
		b = rand.Double(0.05, g);
		r = std::max(b, r * m_body->m_metallicity.ToFloat());
		g = std::max(b, g * m_body->m_metallicity.ToFloat());
		m_darkrockColor[i] = vector3d(r, g, b);
	}

	// grey colours, in case you simply must have a grey colour on a world with high metallicity
	for (int i=0; i<12; i++) m_entropy[i] = rand.Double();
	for (int i=0; i<8; i++) {
		double g;
		g = rand.Double(0.3, 0.9);
		m_greyrockColor[i] = vector3d(g, g, g);
	}

	// Pick some plant colours, mainly greens
	// TODO take star class into account
	for (int i=0; i<12; i++) m_entropy[i] = rand.Double();
	for (int i=0; i<8; i++) {
		double r,g,b;
		g = rand.Double(0.3, 1.0);
		r = rand.Double(0.3, g);
		b = rand.Double(0.2, r);
		g = std::max(r, g * m_body->m_life.ToFloat());
		b *= (1.0-m_body->m_life.ToFloat());
		m_plantColor[i] = vector3d(r, g, b);
	}

	// Pick some darker plant colours mainly greens
	// TODO take star class into account
	for (int i=0; i<12; i++) m_entropy[i] = rand.Double();
	for (int i=0; i<8; i++) {
		double r,g,b;
		g = rand.Double(0.05, 0.3);
		r = rand.Double(0.00, g);
		b = rand.Double(0.00, r);
		g = std::max(r, g * m_body->m_life.ToFloat());
		b *= (1.0-m_body->m_life.ToFloat());
		m_darkplantColor[i] = vector3d(r, g, b);
	}

	// Pick some sand colours, mainly yellow
	// TODO let some planetary value scale this colour
	for (int i=0; i<12; i++) m_entropy[i] = rand.Double();
	for (int i=0; i<8; i++) {
		double r,g,b;
		r = rand.Double(0.6, 1.0);
		g = rand.Double(0.6, r);
		//b = rand.Double(0.0, g/2.0);
		b = 0;
		m_sandColor[i] = vector3d(r, g, b);
	}

	// Pick some darker sand colours mainly yellow
	// TODO let some planetary value scale this colour
	for (int i=0; i<12; i++) m_entropy[i] = rand.Double();
	for (int i=0; i<8; i++) {
		double r,g,b;
		r = rand.Double(0.05, 0.6);
		g = rand.Double(0.00, r);
		//b = rand.Double(0.00, g/2.0);
		b = 0;
		m_darksandColor[i] = vector3d(r, g, b);
	}

	// Pick some dirt colours, mainly red/brown
	// TODO let some planetary value scale this colour
	for (int i=0; i<12; i++) m_entropy[i] = rand.Double();
	for (int i=0; i<8; i++) {
		double r,g,b;
		r = rand.Double(0.3, 0.7);
		g = rand.Double(r-0.1, 0.75);
		b = rand.Double(0.0, r/2.0);
		m_dirtColor[i] = vector3d(r, g, b);
	}

	// Pick some darker dirt colours mainly red/brown
	// TODO let some planetary value scale this colour
	for (int i=0; i<12; i++) m_entropy[i] = rand.Double();
	for (int i=0; i<8; i++) {
		double r,g,b;
		r = rand.Double(0.05, 0.3);
		g = rand.Double(r-0.05, 0.35);
		b = rand.Double(0.0, r/2.0);
		m_darkdirtColor[i] = vector3d(r, g, b);
	}

	// These are used for gas giant colours, they are more random and *should* really use volatileGasses - TODO
	for (int i=0; i<12; i++) m_entropy[i] = rand.Double();
	for (int i=0; i<8; i++) {
		double r,g,b;
		r = rand.Double(0.0, 0.5);
		g = rand.Double(0.0, 0.5);
		b = rand.Double(0.0, 0.5);
		m_gglightColor[i] = vector3d(r, g, b);
	}
	//darker gas giant colours, more reds and greens
	for (int i=0; i<12; i++) m_entropy[i] = rand.Double();
	for (int i=0; i<8; i++) {
		double r,g,b;
		r = rand.Double(0.0, 0.3);
		g = rand.Double(0.0, r);
		b = rand.Double(0.0, std::min(r, g));
		m_ggdarkColor[i] = vector3d(r, g, b);
	}
}

void GeoSphereStyle::PickAtmosphere()
{
	/* Alpha value isn't real alpha. in the shader fog depth is determined
	 * by density*alpha, so that we can have very dense atmospheres
	 * without having them a big stinking solid color obscuring everything

	  These are our atmosphere colours, for terrestrial planets we use m_atmosOxidizing
	  for some variation to atmosphere colours
	 */
	switch (m_body->type) {
		case SBody::TYPE_PLANET_GAS_GIANT:
			m_atmosColor = Color(1.0f, 1.0f, 1.0f, 0.005f);
			m_atmosDensity = 14.0;
			break;
		case SBody::SUPERTYPE_STAR:
		case SBody::TYPE_PLANET_ASTEROID:
			m_atmosColor = Color(0.0f, 0.0f, 0.0f, 0.0f);
			m_atmosDensity = 0.0;
			break;
		default:
		case SBody::TYPE_PLANET_TERRESTRIAL:
			double r = 0, g = 0, b = 0;
			double atmo = m_body->m_atmosOxidizing.ToDouble();
			if (m_body->m_volatileGas.ToDouble() > 0.001) {
				if (atmo > 0.95) {
					// o2
					r = 1.0f + ((0.95f-atmo)*15.0f);
					g = 0.95f + ((0.95f-atmo)*10.0f);
					b = atmo*atmo*atmo*atmo*atmo;
					m_atmosColor = Color(r, g, b, 1.0);
				} else if (atmo > 0.7) {
					// co2
					r = atmo+0.05f;
					g = 1.0f + (0.7f-atmo);
					b = 0.8f;
					m_atmosColor = Color(r, g, b, 1.0f);
				} else if (atmo > 0.65) {
					// co
					r = 1.0f + (0.65f-atmo);
					g = 0.8f;
					b = atmo + 0.25f;
					m_atmosColor = Color(r, g, b, 1.0f);
				} else if (atmo > 0.55) {
					// ch4
					r = 1.0f + ((0.55f-atmo)*5.0);
					g = 0.35f - ((0.55f-atmo)*5.0);
					b = 0.4f;
					m_atmosColor = Color(r, g, b, 1.0f);
				} else if (atmo > 0.3) {
					// h
					r = 1.0f;
					g = 1.0f;
					b = 1.0f;
					m_atmosColor = Color(r, g, b, 1.0f);
				} else if (atmo > 0.2) {
					// he
					r = 1.0f;
					g = 1.0f;
					b = 1.0f;
					m_atmosColor = Color(r, g, b, 1.0f);
				} else if (atmo > 0.15) {
					// ar
					r = 0.5f - ((0.15f-atmo)*5.0);
					g = 0.0f;
					b = 0.5f + ((0.15f-atmo)*5.0);
					m_atmosColor = Color(r, g, b, 1.0f);
				} else if (atmo > 0.1) {
					// s
					r = 0.8f - ((0.1f-atmo)*4.0);
					g = 1.0f;
					b = 0.5f - ((0.1f-atmo)*10.0);
					m_atmosColor = Color(r, g, b, 1.0f);
				} else {
					// n
					r = 1.0f;
					g = 1.0f;
					b = 1.0f;
					m_atmosColor = Color(r, g, b, 1.0f);
				}
			} else {
				m_atmosColor = Color(0.0, 0.0, 0.0, 0.0f);
			}
			m_atmosDensity = m_body->m_volatileGas.ToDouble();
			printf("| Atmosphere :\n|      red   : [%f] \n|      green : [%f] \n|      blue  : [%f] \n", r, g, b);
			printf("-------------------------------\n");
			break;
		/*default:
			m_atmosColor = Color(0.6f, 0.6f, 0.6f, 1.0f);
			m_atmosDensity = m_body->m_volatileGas.ToDouble();
			break;*/
	}
}

void GeoSphereStyle::InitHeightMap()
{
	/* Height map? */
	if (m_body->heightMapFilename) {
		FILE *f;
		f = fopen_or_die(m_body->heightMapFilename, "rb");
		// read size!
		Uint16 v;
		fread_or_die(&v, 2, 1, f); m_heightMapSizeX = v;
		fread_or_die(&v, 2, 1, f); m_heightMapSizeY = v;
		m_heightMap = new Sint16[m_heightMapSizeX * m_heightMapSizeY];
		// XXX TODO XXX what about bigendian archs...
		fread_or_die(m_heightMap, sizeof(Sint16), m_heightMapSizeX * m_heightMapSizeY, f);
		fclose(f);
	} else {
		m_heightMap = 0;
	}	
}

GeoSphereStyle::GeoSphereStyle(const SBody *body) : m_body(body)
{
	m_seed = m_body->seed;

	PickAtmosphere();
	InitHeightMap();

	ChangeDetailLevel();
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
	def->octaves = std::max(1, int(ceil(log(featureWidthMeters / smallestOctaveMeters) / log(2.0))));
	def->lacunarity = 2.0;
	//printf("%d octaves\n", def->octaves); //print
}

// Fracdef is used to define the fractals width/area, height and detail
void GeoSphereStyle::InitFractalType(MTRand &rand)
{
	//Earth uses these fracdef settings
	if (m_heightMap) {	
		//textures
		if (textures) {
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, 10, rand, 10*m_fracmult);
			SetFracDef(&m_fracdef[1], m_maxHeightInMeters, 25, rand, 10*m_fracmult);
		}
		//small fractal/high detail
		SetFracDef(&m_fracdef[2-m_fracnum], m_maxHeightInMeters*0.0000005, 50, rand, 20*m_fracmult);//[2]
		//continental/large type fractal
		SetFracDef(&m_fracdef[3-m_fracnum], m_maxHeightInMeters*0.00005, 1e6, rand, 800*m_fracmult);//[0]
		SetFracDef(&m_fracdef[4-m_fracnum], m_maxHeightInMeters*0.00005, 1e5, rand, 400*m_fracmult);//[4]
		//medium fractal
		SetFracDef(&m_fracdef[5-m_fracnum], m_maxHeightInMeters*0.000005, 2e4, rand, 200*m_fracmult);//[5]
		SetFracDef(&m_fracdef[6-m_fracnum], m_maxHeightInMeters*0.0000005, 5e3, rand, 100*m_fracmult);//[3]
		return;
	}

	switch (m_terrainType) {
		case TERRAIN_ASTEROID:
		{
			//m_maxHeight = rand.Double(0.2,0.4);
			//m_invMaxHeight = 1.0 / m_maxHeight;
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, m_planetRadius, rand);
			// craters
			SetFracDef(&m_fracdef[1], 5000.0, 1000000.0, rand, 1000.0*m_fracmult);
			break;
		}
		case TERRAIN_HILLS_NORMAL: //1
		{
			//textures
			if (textures) {
				SetFracDef(&m_fracdef[0], m_maxHeightInMeters, rand.Double(5, 15), rand, 10*m_fracmult);
				SetFracDef(&m_fracdef[1], m_maxHeightInMeters, rand.Double(20, 40), rand, 10*m_fracmult);
			}
			//small fractal/high detail
			SetFracDef(&m_fracdef[2-m_fracnum], m_maxHeightInMeters*0.000000005, 500, rand, 20*m_fracmult);
			//continental:
			SetFracDef(&m_fracdef[3-m_fracnum], m_maxHeightInMeters*0.00001, 1e7, rand, 1000*m_fracmult);
			//large fractal:
			SetFracDef(&m_fracdef[4-m_fracnum], m_maxHeightInMeters, 1e5, rand, 200*m_fracmult);
			//medium fractal:
			SetFracDef(&m_fracdef[5-m_fracnum], m_maxHeightInMeters*0.00005, 2e4, rand, 200*m_fracmult);
			SetFracDef(&m_fracdef[6-m_fracnum], m_maxHeightInMeters*0.000000005, 1000, rand, 20*m_fracmult);
			break;
		}
		case TERRAIN_HILLS_DUNES: //2
		{
			//textures
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, rand.Double(50, 100), rand, 10*m_fracmult);
			SetFracDef(&m_fracdef[1], m_maxHeightInMeters, rand.Double(300, 500), rand, 10*m_fracmult);
			//small fractal/high detail
			SetFracDef(&m_fracdef[2], m_maxHeightInMeters*0.00000000001, 50, rand, 50*m_fracmult);
			//continental:
			SetFracDef(&m_fracdef[3], m_maxHeightInMeters*0.00001, 1e7, rand, 1000*m_fracmult); 
			//large fractal:
			SetFracDef(&m_fracdef[4], m_maxHeightInMeters*0.00001, 1e5, rand, 200*m_fracmult); 
			SetFracDef(&m_fracdef[5], m_maxHeightInMeters*0.000001, 5e4, rand, 100*m_fracmult); 
			SetFracDef(&m_fracdef[6], m_maxHeightInMeters*0.0000001, 1e4, rand, 50*m_fracmult); 
			//medium fractal:
			SetFracDef(&m_fracdef[7], m_maxHeightInMeters*0.0000000002, 1e3, rand, 20*m_fracmult); 
			break;
		}
		case TERRAIN_HILLS_RIDGED: //3
		{
			//textures:
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, rand.Double(5, 15), rand, 10*m_fracmult);
			SetFracDef(&m_fracdef[1], m_maxHeightInMeters, rand.Double(20, 40), rand, 10*m_fracmult);
			//small fractal/high detail:
			SetFracDef(&m_fracdef[2], m_maxHeightInMeters*0.000000005, rand.Double(40, 80), rand, 10*m_fracmult);
			//continental:
			SetFracDef(&m_fracdef[3], m_maxHeightInMeters*0.00001, rand.Double(1e6, 2e7), rand, 1000*m_fracmult);
			//large fractal:
			SetFracDef(&m_fracdef[4], m_maxHeightInMeters, rand.Double(1e5, 5e6), rand, 200*m_fracmult);
			//medium fractal:
			SetFracDef(&m_fracdef[5], m_maxHeightInMeters*0.00005, rand.Double(1e3, 5e4), rand, 100*m_fracmult);
			SetFracDef(&m_fracdef[6], m_maxHeightInMeters*0.00000002, rand.Double(250, 1e3), rand, 50*m_fracmult);
			break;
		}
		case TERRAIN_HILLS_RIVERS: //4
		{
			//textures
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, rand.Double(5, 15), rand, 10*m_fracmult);
			SetFracDef(&m_fracdef[1], m_maxHeightInMeters, rand.Double(20, 40), rand, 10*m_fracmult);
			//small fractal/high detail
			SetFracDef(&m_fracdef[2], m_maxHeightInMeters*0.000000008, rand.Double(5, 70), rand, 10*m_fracmult);
			//continental:
			SetFracDef(&m_fracdef[3], m_maxHeightInMeters, rand.Double(1e6, 2e7), rand, 10000*m_fracmult); 
			//large fractal:
			SetFracDef(&m_fracdef[4], m_maxHeightInMeters*0.00001, 1e5, rand, 1000*m_fracmult); 
			SetFracDef(&m_fracdef[5], m_maxHeightInMeters*0.000001, rand.Double(1e5, 1e6), rand, 100*m_fracmult); 
			//medium fractal:
			SetFracDef(&m_fracdef[6], m_maxHeightInMeters*0.0000002, rand.Double(500, 2e4), rand, 50*m_fracmult); 
			break;
		}
		case TERRAIN_HILLS_CRATERS: //5
		{
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, rand.Double(1e6,1e7), rand);
			double height = m_maxHeightInMeters*0.3;
			SetFracDef(&m_fracdef[1], height, rand.Double(4.0, 20.0)*height, rand);
			SetFracDef(&m_fracdef[2], m_maxHeightInMeters, rand.Double(50.0, 100.0)*m_maxHeightInMeters, rand);
			SetFracDef(&m_fracdef[3], m_maxHeightInMeters*0.07, 1e6, rand, 100.0*m_fracmult);
			SetFracDef(&m_fracdef[4], m_maxHeightInMeters*0.05, 8e5, rand, 100.0*m_fracmult);
			break;
		}
		case TERRAIN_HILLS_CRATERS2: //6
		{
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, rand.Double(1e6,1e7), rand);
			double height = m_maxHeightInMeters*0.6;
			SetFracDef(&m_fracdef[1], height, rand.Double(4.0, 20.0)*height, rand);
			SetFracDef(&m_fracdef[2], m_maxHeightInMeters, rand.Double(50.0, 100.0)*m_maxHeightInMeters, rand);
			SetFracDef(&m_fracdef[3], m_maxHeightInMeters*0.07, 11e5, rand, 1000.0*m_fracmult);
			SetFracDef(&m_fracdef[4], m_maxHeightInMeters*0.05, 98e4, rand, 800.0*m_fracmult);
			SetFracDef(&m_fracdef[5], m_maxHeightInMeters*0.05, 1e6, rand, 400.0*m_fracmult);
			SetFracDef(&m_fracdef[6], m_maxHeightInMeters*0.04, 99e4, rand, 200.0*m_fracmult);
			SetFracDef(&m_fracdef[7], m_maxHeightInMeters*0.05, 12e5, rand, 100.0*m_fracmult);
			SetFracDef(&m_fracdef[8], m_maxHeightInMeters*0.04, 9e5, rand, 100.0*m_fracmult);
			break;
		}
		case TERRAIN_MOUNTAINS_NORMAL: //7
		{
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, rand.Double(1e6, 1e7), rand, 10000*m_fracmult);
			SetFracDef(&m_fracdef[1], m_maxHeightInMeters*0.00000000001, 100.0, rand, 10*m_fracmult);
			SetFracDef(&m_fracdef[2], m_maxHeightInMeters*0.0000001, rand.Double(500, 2e3), rand, 1000*m_fracmult);
			SetFracDef(&m_fracdef[3], m_maxHeightInMeters*0.00002, rand.Double(1500, 1e4), rand, 100*m_fracmult);
			SetFracDef(&m_fracdef[4], m_maxHeightInMeters*0.08, 1e4, rand, 100*m_fracmult);
			SetFracDef(&m_fracdef[5], m_maxHeightInMeters*0.2, 1e5, rand, 100*m_fracmult);
			SetFracDef(&m_fracdef[6], m_maxHeightInMeters*0.5, 1e6, rand, 1000*m_fracmult);
			SetFracDef(&m_fracdef[7], m_maxHeightInMeters*0.5, rand.Double(1e6,1e7), rand, 1000*m_fracmult);
			SetFracDef(&m_fracdef[8], m_maxHeightInMeters, rand.Double(3e6, 1e7), rand, 1000*m_fracmult);
			break;
		}
		case TERRAIN_MOUNTAINS_RIDGED: //8
		{
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, rand.Double(1e6,1e7), rand);
			double height = m_maxHeightInMeters*0.9;
			SetFracDef(&m_fracdef[1], m_maxHeightInMeters, rand.Double(50.0, 100.0)*m_maxHeightInMeters, rand, 8*m_fracmult);
			SetFracDef(&m_fracdef[2], height, rand.Double(4.0, 200.0)*height, rand, 10*m_fracmult);
			SetFracDef(&m_fracdef[3], m_maxHeightInMeters, rand.Double(120.0, 2000.0)*m_maxHeightInMeters, rand, 1000*m_fracmult);

			height = m_maxHeightInMeters*0.4;
			SetFracDef(&m_fracdef[4], m_maxHeightInMeters, rand.Double(100.0, 200.0)*m_maxHeightInMeters, rand);
			SetFracDef(&m_fracdef[5], height*0.4, rand.Double(2.5,30.5)*height, rand);
			SetFracDef(&m_fracdef[6], height*0.2, rand.Double(20.5,350.5)*height, rand, 10000*m_fracmult);

			SetFracDef(&m_fracdef[7], m_maxHeightInMeters, rand.Double(100.0, 2000.0)*m_maxHeightInMeters, rand, 100*m_fracmult);
			SetFracDef(&m_fracdef[8], height*0.3, rand.Double(2.5,300.5)*height, rand, 500*m_fracmult);
			SetFracDef(&m_fracdef[9], height*0.2, rand.Double(2.5,300.5)*height, rand, 20*m_fracmult);
			break;
		}
		case TERRAIN_MOUNTAINS_RIVERS: //9
		{
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, rand.Double(1e6, 2e6), rand, 10*m_fracmult);
			SetFracDef(&m_fracdef[1], m_maxHeightInMeters, 15e6, rand, 100.0*m_fracmult);
			SetFracDef(&m_fracdef[2], m_maxHeightInMeters*0.0000001, rand.Double(500, 2e3), rand, 10*m_fracmult);
			SetFracDef(&m_fracdef[3], m_maxHeightInMeters*0.00002, rand.Double(1500, 1e4), rand, 10*m_fracmult);
			SetFracDef(&m_fracdef[4], m_maxHeightInMeters*0.08, 1e4, rand, 10*m_fracmult);
			SetFracDef(&m_fracdef[5], m_maxHeightInMeters*0.2, 1e5, rand, 10*m_fracmult);
			SetFracDef(&m_fracdef[6], m_maxHeightInMeters*0.5, 1e6, rand, 100*m_fracmult);
			SetFracDef(&m_fracdef[7], m_maxHeightInMeters*0.5, rand.Double(1e6,5e6), rand, 100*m_fracmult);
			SetFracDef(&m_fracdef[8], m_maxHeightInMeters, rand.Double(12e5, 22e5), rand, 10*m_fracmult);
			SetFracDef(&m_fracdef[9], m_maxHeightInMeters, 1e7, rand, 100.0*m_fracmult);
			break;
		}
		case TERRAIN_MOUNTAINS_CRATERS: //10
		{
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, rand.Double(1e6,1e7), rand);
			double height = m_maxHeightInMeters*0.3;
			SetFracDef(&m_fracdef[1], height, rand.Double(4.0, 20.0)*height, rand);
			SetFracDef(&m_fracdef[2], m_maxHeightInMeters, rand.Double(50.0, 100.0)*m_maxHeightInMeters, rand);

			height = m_maxHeightInMeters*0.3;
			SetFracDef(&m_fracdef[4], m_maxHeightInMeters, rand.Double(100.0, 200.0)*m_maxHeightInMeters, rand);
			SetFracDef(&m_fracdef[3], height, rand.Double(2.5,3.5)*height, rand);

			SetFracDef(&m_fracdef[5], m_maxHeightInMeters*0.05, 8e5, rand, 1000.0*m_fracmult);
			SetFracDef(&m_fracdef[6], m_maxHeightInMeters*0.05, 1e6, rand, 10000.0*m_fracmult);
			break;
		}
		case TERRAIN_MOUNTAINS_CRATERS2: //11
		{
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, rand.Double(1e6,1e7), rand);
			double height = m_maxHeightInMeters*0.5;
			SetFracDef(&m_fracdef[1], height, rand.Double(50.0, 200.0)*height, rand, 10*m_fracmult);
			SetFracDef(&m_fracdef[2], m_maxHeightInMeters, rand.Double(500.0, 5000.0)*m_maxHeightInMeters, rand);

			height = m_maxHeightInMeters*0.4;
			SetFracDef(&m_fracdef[3], height, rand.Double(2.5,3.5)*height, rand);
			SetFracDef(&m_fracdef[4], m_maxHeightInMeters, rand.Double(100.0, 200.0)*m_maxHeightInMeters, rand);
			SetFracDef(&m_fracdef[5], m_maxHeightInMeters*0.05, 1e6, rand, 10000.0*m_fracmult);
			SetFracDef(&m_fracdef[6], m_maxHeightInMeters*0.04, 9e5, rand, 10000.0*m_fracmult);
			SetFracDef(&m_fracdef[7], m_maxHeightInMeters*0.05, 8e5, rand, 10000.0*m_fracmult);
			SetFracDef(&m_fracdef[8], m_maxHeightInMeters*0.04, 11e5, rand, 10000.0*m_fracmult);
			SetFracDef(&m_fracdef[9], m_maxHeightInMeters*0.07, 12e5, rand, 10000.0*m_fracmult);
			break;
		}
		case TERRAIN_MOUNTAINS_VOLCANO:  //12
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
			SetFracDef(&m_fracdef[7], 20000.0, 5000000.0, rand, 1000.0*m_fracmult);

			// canyons 
			SetFracDef(&m_fracdef[8], m_maxHeightInMeters*0.5, 2e6, rand, 100.0*m_fracmult);
			//SetFracDef(&m_fracdef[9], m_maxHeightInMeters*0.1, 1.5e6, rand, 100.0*m_fracmult);
			//SetFracDef(&m_fracdef[10], m_maxHeightInMeters*0.1, 2e6, rand, 100.0*m_fracmult);
			break;
		}
		case TERRAIN_MOUNTAINS_RIVERS_VOLCANO:  //old terraformed mars terrain  //13
		{
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, rand.Double(1e6,1e7), rand);
			double height = m_maxHeightInMeters*0.6;
			SetFracDef(&m_fracdef[1], m_maxHeightInMeters, rand.Double(50.0, 100.0)*m_maxHeightInMeters, rand);
			SetFracDef(&m_fracdef[2], height, rand.Double(4.0, 20.0)*height, rand);
			SetFracDef(&m_fracdef[3], m_maxHeightInMeters, rand.Double(120.0, 2000.0)*m_maxHeightInMeters, rand, 20*m_fracmult);

			height = m_maxHeightInMeters*0.3;
			SetFracDef(&m_fracdef[4], m_maxHeightInMeters, rand.Double(100.0, 200.0)*m_maxHeightInMeters, rand);
			SetFracDef(&m_fracdef[5], height, rand.Double(2.5,3.5)*height, rand);
			SetFracDef(&m_fracdef[6], height, rand.Double(2.5,3.5)*height, rand);
			// volcano
			SetFracDef(&m_fracdef[7], 20000.0, 5000000.0, rand, 100.0*m_fracmult);

			// canyons and rivers
			SetFracDef(&m_fracdef[8], m_maxHeightInMeters*1.0, 4e6, rand, 100.0*m_fracmult);
			SetFracDef(&m_fracdef[9], m_maxHeightInMeters*1.0, 5e6, rand, 100.0*m_fracmult);
			//SetFracDef(&m_fracdef[10], m_maxHeightInMeters*0.5, 2e6, rand, 100.0);
			break;
		}
		case TERRAIN_RUGGED_LAVA: //14
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
			SetFracDef(&m_fracdef[6], height, 6e6, rand, 100000.0*m_fracmult);
			SetFracDef(&m_fracdef[7], height, 3e6, rand, 1000.0*m_fracmult);

			// canyon
			SetFracDef(&m_fracdef[8], m_maxHeightInMeters*0.4, 4e6, rand, 100.0*m_fracmult);
			// bumps/rocks
			SetFracDef(&m_fracdef[9], height*0.001, rand.Double(10,100), rand, 2.0*m_fracmult);
			break;
		}
		case TERRAIN_H2O_SOLID: //15
		{
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, rand.Double(5e6,1e8), rand);
			double height = m_maxHeightInMeters*0.3;
			SetFracDef(&m_fracdef[1], height, rand.Double(4.0, 20.0)*height, rand);
			SetFracDef(&m_fracdef[2], m_maxHeightInMeters, rand.Double(200.0, 1000.0)*m_maxHeightInMeters, rand);

			// mountains with some canyons
			SetFracDef(&m_fracdef[3], m_maxHeightInMeters*0.4, 4e6, rand);
			SetFracDef(&m_fracdef[4], m_maxHeightInMeters*0.4, 5e6, rand);
			//crater
			SetFracDef(&m_fracdef[5], m_maxHeightInMeters*0.4, 1.5e7, rand, 50000.0*m_fracmult);
			break;
			break;
		}
		case TERRAIN_H2O_SOLID_CANYONS: //16
		{
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, rand.Double(5e6,1e8), rand);
			double height = m_maxHeightInMeters*0.3;
			SetFracDef(&m_fracdef[1], height, rand.Double(4.0, 20.0)*height, rand);
			SetFracDef(&m_fracdef[2], m_maxHeightInMeters, rand.Double(200.0, 1000.0)*m_maxHeightInMeters, rand);

			// mountains with some canyons
			SetFracDef(&m_fracdef[3], m_maxHeightInMeters*0.4, 4e6, rand);
			SetFracDef(&m_fracdef[4], m_maxHeightInMeters*0.4, 5e6, rand);
			//crater
			SetFracDef(&m_fracdef[5], m_maxHeightInMeters*0.4, 15e6, rand, 50000.0*m_fracmult);
			//canyons
			//SetFracDef(&m_fracdef[6], m_maxHeightInMeters*0.4, 12e6, rand, 50000.0);
			//SetFracDef(&m_fracdef[7], m_maxHeightInMeters*0.4, 9e6, rand, 50000.0);
			break;
		}
		case TERRAIN_RUGGED_DESERT: //17
		{
			SetFracDef(&m_fracdef[0], 0.1*m_maxHeightInMeters, 2e6, rand, 180e3*m_fracmult);
			double height = m_maxHeightInMeters*0.9;
			SetFracDef(&m_fracdef[1], height, rand.Double(120.0, 10000.0)*height, rand, 100*m_fracmult);
			SetFracDef(&m_fracdef[2], m_maxHeightInMeters, rand.Double(1.0, 2.0)*m_maxHeightInMeters, rand);

			height = m_maxHeightInMeters*0.3;
			SetFracDef(&m_fracdef[3], height, rand.Double(20.0,240.0)*height, rand);
			SetFracDef(&m_fracdef[4], m_maxHeightInMeters, rand.Double(1.0, 2.0)*m_maxHeightInMeters, rand);
			// dunes
			height = m_maxHeightInMeters*0.2;
			SetFracDef(&m_fracdef[5], height*0.1, rand.Double(5,75)*height, rand, 10000.0*m_fracmult);
			// canyon
			SetFracDef(&m_fracdef[6], m_maxHeightInMeters*0.2, 1e6, rand, 200.0*m_fracmult);
			SetFracDef(&m_fracdef[7], m_maxHeightInMeters*0.35, 1.5e6, rand, 100.0*m_fracmult);
			SetFracDef(&m_fracdef[8], m_maxHeightInMeters*0.2, 3e6, rand, 100.0*m_fracmult);

			//SetFracDef(&m_fracdef[9], m_maxHeightInMeters*0.1, 100, rand, 10.0);
			// adds bumps to the landscape
			SetFracDef(&m_fracdef[9], height*0.0025, rand.Double(1,100), rand, 100.0*m_fracmult);
            break;
		}
        case TERRAIN_FLAT:
        case TERRAIN_NONE:
            // Added in to prevent compiler warnings
            break;
	}

// We set some fracdefs here for colours if we need them:
	switch (m_colorType) {
		case COLOR_NONE:
		case COLOR_STAR_BROWN_DWARF:
			{
				double height = m_maxHeightInMeters*0.1;
				SetFracDef(&m_fracdef[0], height, 5e6, rand, 10.0*m_fracmult);
				SetFracDef(&m_fracdef[1], height, 3e6, rand, 10.0*m_fracmult);
				SetFracDef(&m_fracdef[2], height, 1e5, rand, 10.0*m_fracmult);
				SetFracDef(&m_fracdef[3], height, 1e2, rand, 10.0*m_fracmult);
			}
		case COLOR_STAR_WHITE_DWARF:
			{
				double height = m_maxHeightInMeters*0.1;
				SetFracDef(&m_fracdef[0], height, 3e5, rand, 10.0*m_fracmult);
				SetFracDef(&m_fracdef[1], height, 1e5, rand, 10.0*m_fracmult);
				//SetFracDef(&m_fracdef[2], height, 1e6, rand, 10.0*m_fracmult);
				//SetFracDef(&m_fracdef[3], height, 1e2, rand, 10.0*m_fracmult);
			}
		case COLOR_STAR_M:
			{
				double height = m_maxHeightInMeters*0.1;
				SetFracDef(&m_fracdef[0], height, 1e8, rand, 10.0*m_fracmult);
				SetFracDef(&m_fracdef[1], height, 7e7, rand, 10.0*m_fracmult);
				SetFracDef(&m_fracdef[2], height, 3e6, rand, 10.0*m_fracmult);
				SetFracDef(&m_fracdef[3], height, 2e5, rand, 10.0*m_fracmult);
			}
		case COLOR_STAR_K:
			{
				double height = m_maxHeightInMeters*0.1;
				SetFracDef(&m_fracdef[0], height, 2e8, rand, 10.0*m_fracmult);
				SetFracDef(&m_fracdef[1], height, 7e7, rand, 10.0*m_fracmult);
				SetFracDef(&m_fracdef[2], height, 1e6, rand, 10.0*m_fracmult);
				SetFracDef(&m_fracdef[3], height, 1e3, rand, 10.0*m_fracmult);
			}
		case COLOR_STAR_G:
			{
				double height = m_maxHeightInMeters*0.1;
				SetFracDef(&m_fracdef[0], height, 8e8, rand, 10.0*m_fracmult);
				SetFracDef(&m_fracdef[1], height, 7e6, rand, 10.0*m_fracmult);
				SetFracDef(&m_fracdef[2], height, 4e6, rand, 10.0*m_fracmult);
				SetFracDef(&m_fracdef[3], height, 2e6, rand, 10.0*m_fracmult);
			}
		case COLOR_GG_JUPITER: 
			{
				// spots
				double height = m_maxHeightInMeters*0.1;
				SetFracDef(&m_fracdef[0], height, 1e8, rand, 1000.0*m_fracmult);
				SetFracDef(&m_fracdef[1], height, 8e7, rand, 1000.0*m_fracmult);
				SetFracDef(&m_fracdef[2], height, 4e7, rand, 1000.0*m_fracmult);
				SetFracDef(&m_fracdef[3], height, 1e7, rand, 100.0*m_fracmult);
				break;
			}
		case COLOR_GG_SATURN:
			{
				double height = m_maxHeightInMeters*0.1;
				//spot + clouds
				SetFracDef(&m_fracdef[0], height, 3e7, rand, 10.0*m_fracmult);
				SetFracDef(&m_fracdef[1], height, 9e7, rand, 1000.0*m_fracmult);
				SetFracDef(&m_fracdef[2], height, 8e7, rand, 100.0*m_fracmult);
				//spot boundary
				SetFracDef(&m_fracdef[3], height, 3e7, rand, 10000000.0*m_fracmult);
				break;
			}
		case COLOR_GG_SATURN2:
			{
				double height = m_maxHeightInMeters*0.1;
				//spot + clouds
				SetFracDef(&m_fracdef[0], height, 3e7, rand, 10.0*m_fracmult);
				SetFracDef(&m_fracdef[1], height, 9e7, rand, 1000.0*m_fracmult);
				SetFracDef(&m_fracdef[2], height, 8e7, rand, 100.0*m_fracmult);
				//spot boundary
				SetFracDef(&m_fracdef[3], height, 3e7, rand, 10000000.0*m_fracmult);
				break;
			}
		case COLOR_GG_URANUS: 
			{
				double height = m_maxHeightInMeters*0.1;
				SetFracDef(&m_fracdef[0], height, 3e7, rand, 1000.0*m_fracmult);
				SetFracDef(&m_fracdef[1], height, 9e7, rand, 1000.0*m_fracmult);
				SetFracDef(&m_fracdef[2], height, 8e7, rand, 1000.0*m_fracmult);
				break;
			}
		case COLOR_GG_NEPTUNE:
			{
				double height = m_maxHeightInMeters*0.1;
				//spot boundary
				SetFracDef(&m_fracdef[0], height, 3e7, rand, 10000000.0*m_fracmult);
				//spot
				SetFracDef(&m_fracdef[1], height, 9e7, rand, 100.0*m_fracmult);
				//bands
				SetFracDef(&m_fracdef[2], height, 8e7, rand, 1000.0*m_fracmult);
				SetFracDef(&m_fracdef[3], height, 1e8, rand, 1000.0*m_fracmult);
				break;
			}
		case COLOR_GG_NEPTUNE2:
			{
				// spots
				double height = m_maxHeightInMeters*0.1;
				SetFracDef(&m_fracdef[0], height, 2e8, rand, 1000.0*m_fracmult);
				SetFracDef(&m_fracdef[1], height, 9e7, rand, 1000.0*m_fracmult);
				SetFracDef(&m_fracdef[2], height, 6e7, rand, 1000.0*m_fracmult);
				SetFracDef(&m_fracdef[3], height, 1e8, rand, 100.0*m_fracmult);
				break;
			}
		case COLOR_EARTHLIKE: 
			{
				// crappy water
				//double height = m_maxHeightInMeters*0.5;
				//SetFracDef(&m_fracdef[3], m_maxHeightInMeters, 1e8, rand, 50.0);
				//SetFracDef(&m_fracdef[2], m_maxHeightInMeters, 10, rand, 10.0);
				break;
			}
		case COLOR_DEAD_WITH_H2O:  
		case COLOR_ICEWORLD: 
		case COLOR_DESERT:
		case COLOR_ROCK:
		case COLOR_ROCK2:
		case COLOR_ASTEROID:
		case COLOR_VOLCANIC:
		case COLOR_METHANE:
		case COLOR_TFGOOD:
		case COLOR_TFPOOR:
		case COLOR_BANDED_ROCK:
		default:
			{
				break;
			}

	}
}

/*
 * Must return >= 0.0
  Here we create the noise used to generate the landscape, the noise should use the fracdef[] settings that were defined earlier.
 */
double GeoSphereStyle::GetHeight(const vector3d &p)
{
	if (m_heightMap) return GetHeightMapVal(p) / m_planetRadius;

	switch (m_terrainType) {
		case TERRAIN_NONE:
		case TERRAIN_FLAT:
			return 0;
		case TERRAIN_ASTEROID:
		{
			return std::max(0.0, m_maxHeight * (octavenoise(m_fracdef[0], 0.5, p) + 
					m_fracdef[1].amplitude * crater_function(m_fracdef[1], p)));
		}
		case TERRAIN_HILLS_NORMAL:
		{
			double continents = octavenoise(m_fracdef[3-m_fracnum], 0.65, p) * (1.0-m_sealevel) - (m_sealevel*0.1);
			if (continents < 0) return 0;
			double n = continents;
			double distrib = octavenoise(m_fracdef[4-m_fracnum], 0.5, p);
			distrib *= distrib;
			double m = 0.5*m_fracdef[3-m_fracnum].amplitude * octavenoise(m_fracdef[4-m_fracnum], 0.55*distrib, p) 
				* m_fracdef[5-m_fracnum].amplitude;
			m += 0.25*billow_octavenoise(m_fracdef[5-m_fracnum], 0.55*distrib, p);
			//hill footings
			m -= octavenoise(m_fracdef[2-m_fracnum], 0.6*(1.0-distrib), p) 
				* Clamp(0.05-m, 0.0, 0.05) * Clamp(0.05-m, 0.0, 0.05);
			//hill footings
			m += voronoiscam_octavenoise(m_fracdef[6-m_fracnum], 0.765*distrib, p) 
				* Clamp(0.025-m, 0.0, 0.025) * Clamp(0.025-m, 0.0, 0.025);
			// cliffs at shore
			if (continents < 0.01) n += m * continents * 100.0f;
			else n += m;
			return (n > 0.0 ? n*m_maxHeight : 0.0); 
		}
		case TERRAIN_HILLS_DUNES:
		{
			/*
			//textures
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, rand.Double(5, 15), rand, 10*m_fracmult);
			SetFracDef(&m_fracdef[1], m_maxHeightInMeters, rand.Double(20, 40), rand, 10*m_fracmult);
			//small fractal/high detail
			SetFracDef(&m_fracdef[2], m_maxHeightInMeters*0.000000008, 500, rand, 10*m_fracmult);
			//continental:
			SetFracDef(&m_fracdef[3], m_maxHeightInMeters, 1e7, rand, 1000*m_fracmult); 
			//large fractal:
			SetFracDef(&m_fracdef[4], m_maxHeightInMeters*0.00001, 1e5, rand, 200*m_fracmult); 
			SetFracDef(&m_fracdef[5], m_maxHeightInMeters*0.00001, 5e4, rand, 200*m_fracmult); 
			SetFracDef(&m_fracdef[6], m_maxHeightInMeters*0.000001, 1e4, rand, 100*m_fracmult); 
			//medium fractal:
			SetFracDef(&m_fracdef[7], m_maxHeightInMeters*0.0000002, 1e3, rand, 20*m_fracmult); 
			*/
			double continents = ridged_octavenoise(m_fracdef[3], 0.65, p) * (1.0-m_sealevel) - (m_sealevel*0.1);
			if (continents < 0) return 0;
			double n = continents;
			double distrib = dunes_octavenoise(m_fracdef[4], 0.4, p);
			distrib *= distrib * distrib;
			double m = octavenoise(m_fracdef[7], 0.5, p) * dunes_octavenoise(m_fracdef[7], 0.5, p)
				* Clamp(0.2-distrib, 0.0, 0.05);
			m += octavenoise(m_fracdef[2], 0.5, p) * dunes_octavenoise(m_fracdef[2], 0.5
			*octavenoise(m_fracdef[6], 0.5*distrib, p), p) * Clamp(1.0-distrib, 0.0, 0.0005);
			double mountains = ridged_octavenoise(m_fracdef[5], 0.5*distrib, p) 
				* octavenoise(m_fracdef[4], 0.5*distrib, p) * octavenoise(m_fracdef[6], 0.5, p) * distrib;
			mountains *= mountains;
			m += mountains;
			//detail for mountains, stops them looking smooth.
			//m += mountains*mountains*0.02*octavenoise(m_fracdef[2], 0.6*mountains*mountains*distrib, p);
			//m *= m*m*m*10.0;
			// smooth cliffs at shore
			if (continents < 0.01) n += m * continents * 100.0f;
			else n += m;
			//n += continents*Clamp(0.5-m, 0.0, 0.5)*0.2*dunes_octavenoise(m_fracdef[6], 0.6*distrib, p);
			//n += continents*Clamp(0.05-n, 0.0, 0.01)*0.2*dunes_octavenoise(m_fracdef[2], Clamp(0.5-n, 0.0, 0.5), p);
			return (n > 0.0 ? n*m_maxHeight : 0.0); 
		}
		case TERRAIN_HILLS_RIDGED:
		{
			double continents = ridged_octavenoise(m_fracdef[3], 0.65, p) * (1.0-m_sealevel) - (m_sealevel*0.1);
			if (continents < 0) return 0;
			double n = continents;
			double distrib = river_octavenoise(m_fracdef[4], 0.5, p);
			double m = 0.5* ridged_octavenoise(m_fracdef[4], 0.55*distrib, p);
			m += continents*0.25*ridged_octavenoise(m_fracdef[5], 0.58*distrib, p);
			// **
			m += 0.001*ridged_octavenoise(m_fracdef[6], 0.55*distrib*m, p);
			// cliffs at shore
			if (continents < 0.01) n += m * continents * 100.0f;
			else n += m;
			// was n -= 0.001*ridged_octavenoise(m_fracdef[6], 0.55*distrib*m, p);
			//n += 0.001*ridged_octavenoise(m_fracdef[6], 0.55*distrib*m, p);
			return (n > 0.0 ? n*m_maxHeight : 0.0);
		}
		case TERRAIN_HILLS_RIVERS:
		{
			double continents = river_octavenoise(m_fracdef[3], 0.65, p) * (1.0-m_sealevel) - (m_sealevel*0.1);
			if (continents < 0) return 0;
			double n = continents;
			double distrib = voronoiscam_octavenoise(m_fracdef[4], 0.5*m_fracdef[5].amplitude, p);
			double m = 0.1 * m_fracdef[4].amplitude * river_octavenoise(m_fracdef[5], 0.5*distrib, p);
			double mountains = ridged_octavenoise(m_fracdef[5], 0.5*distrib, p) * billow_octavenoise(m_fracdef[5], 0.5, p) *
				voronoiscam_octavenoise(m_fracdef[4], 0.5*distrib, p) * distrib;
			m += mountains;
			//detail for mountains, stops them looking smooth.
			m += mountains*mountains*0.02*ridged_octavenoise(m_fracdef[2], 0.6*mountains*mountains*distrib, p);
			m *= m*m*m*10.0;
			// smooth cliffs at shore
			if (continents < 0.01) n += m * continents * 100.0f;
			else n += m;
			n += continents*Clamp(0.5-m, 0.0, 0.5)*0.2*river_octavenoise(m_fracdef[6], 0.6*distrib, p);
			n += continents*Clamp(0.05-n, 0.0, 0.01)*0.2*dunes_octavenoise(m_fracdef[2], Clamp(0.5-n, 0.0, 0.5), p);
			n *= m_maxHeight;
			return (n > 0.0 ? n : 0.0);
		}
		case TERRAIN_HILLS_CRATERS:
		{
			double continents = octavenoise(m_fracdef[0], 0.5, p) - m_sealevel;
			if (continents < 0) return 0;
			// == TERRAIN_HILLS_NORMAL except river_octavenoise
			double n = 0.3 * continents;
			double distrib = river_octavenoise(m_fracdef[2], 0.5, p);
			double m = m_fracdef[1].amplitude * river_octavenoise(m_fracdef[1], 0.5*distrib, p);
			// cliffs at shore
			if (continents < 0.001) n += m * continents * 1000.0f;
			else n += m;
			n += crater_function(m_fracdef[3], p);
			n += crater_function(m_fracdef[4], p);
			n *= m_maxHeight;
			return (n > 0.0 ? n : 0.0);
		}
		case TERRAIN_HILLS_CRATERS2:
		{
			double continents = octavenoise(m_fracdef[0], 0.5, p) - m_sealevel;
			if (continents < 0) return 0;
			// == TERRAIN_HILLS_NORMAL except river_octavenoise
			double n = 0.3 * continents;
			double distrib = river_octavenoise(m_fracdef[2], 0.5, p);
			double m = m_fracdef[1].amplitude * river_octavenoise(m_fracdef[1], 0.5*distrib, p);
			// cliffs at shore
			if (continents < 0.001) n += m * continents * 1000.0f;
			else n += m;
			n += crater_function(m_fracdef[3], p);
			n += crater_function(m_fracdef[4], p);
			n += crater_function(m_fracdef[5], p);
			n += crater_function(m_fracdef[6], p);
			n += crater_function(m_fracdef[7], p);
			n += crater_function(m_fracdef[8], p);
			n *= m_maxHeight;
			return (n > 0.0 ? n : 0.0);
		}
		case TERRAIN_MOUNTAINS_NORMAL:
			//This is among the most complex of terrains, so I'll use this as an example:
		{
			//We need a continental pattern to place our noise onto, the 0.7*ridged_octavnoise..... is important here
			// for making 'broken up' coast lines, as opposed to circular land masses, it will reduce the frequency of our
			// continents depending on the ridged noise value, we subtract sealevel so that sea level will have an effect on the continents size
			double continents = octavenoise(m_fracdef[0], 0.7*
				ridged_octavenoise(m_fracdef[8], 0.58, p), p) - m_sealevel*0.65;
			// if there are no continents on an area, we want it to be sea level
			if (continents < 0) return 0;
			double n = continents - (m_fracdef[0].amplitude*m_sealevel*0.5);
			// we save the height n now as a constant h
			const double h = n;
		/*  Definitions here for easy referral
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, rand.Double(1e6, 1e8), rand, 100);
			SetFracDef(&m_fracdef[1], m_maxHeightInMeters*0.00001, 20.0, rand, 100);
			SetFracDef(&m_fracdef[2], m_maxHeightInMeters*0.00001, rand.Double(500, 2e3), rand, 10);
			SetFracDef(&m_fracdef[3], m_maxHeightInMeters*0.00002, rand.Double(1500, 1e4), rand, 10);
			SetFracDef(&m_fracdef[4], m_maxHeightInMeters*0.08, 1e4, rand, 10);
			SetFracDef(&m_fracdef[5], m_maxHeightInMeters*0.3, 1e5, rand, 10);
			SetFracDef(&m_fracdef[6], m_maxHeightInMeters*0.8, 1e6, rand, 10);
			SetFracDef(&m_fracdef[7], m_maxHeightInMeters, 1e7, rand, 10);
		*/
			//We don't want to apply noise to sea level n=0
			if (n > 0.0) {
				//large mountainous shapes
				n += h*0.2*ridged_octavenoise(m_fracdef[7], 
					0.5*octavenoise(m_fracdef[6], 0.5, p), p);

				// This smoothes edges near the coast, we cant have vertical terrain its not handled correctly.
				if (n < 0.4){
					n += n*1.25*ridged_octavenoise(m_fracdef[6], 
						Clamp(h*0.00002, 0.3, 0.7)*
						ridged_octavenoise(m_fracdef[5], 0.5, p), p);
				} else {
					n += 0.5*ridged_octavenoise(m_fracdef[6], 
						Clamp(h*0.00002, 0.3, 0.7)*
						ridged_octavenoise(m_fracdef[5], 0.5, p), p);
				}

				if (n < 0.2){
					n += n*15.0*river_octavenoise(m_fracdef[6], 
						Clamp(h*0.00002, 0.5, 0.7), p);
				} else {
					n += 3.0*river_octavenoise(m_fracdef[6], 
						Clamp(h*0.00002, 0.5, 0.7), p);
				}

				if (n < 0.4){
					n += n*billow_octavenoise(m_fracdef[6], 
						0.5*octavenoise(m_fracdef[5], 0.5, p), p);
				} else {
					n += (0.16/n)*billow_octavenoise(m_fracdef[6], 
						0.5*octavenoise(m_fracdef[5], 0.5, p), p);
				}

				if (n < 0.2){
					n += n*billow_octavenoise(m_fracdef[5], 
						0.5*octavenoise(m_fracdef[5], 0.5, p), p);
				} else {
					n += (0.04/n)*billow_octavenoise(m_fracdef[5], 
						0.5*octavenoise(m_fracdef[5], 0.5, p), p);
				}
				//smaller ridged mountains
				n += n*0.7*ridged_octavenoise(m_fracdef[5], 
					0.5*octavenoise(m_fracdef[6], 0.5, p), p);

				n = (n/2)+(n*n);
				
				//jagged surface for mountains
				//This is probably using far too much noise, some of it is just not needed
				// More specifically this: Clamp(h*0.0002*octavenoise(m_fracdef[5], 0.5, p),
				//		 0.5*octavenoise(m_fracdef[3], 0.5, p), 
				//		 0.5*octavenoise(m_fracdef[3], 0.5, p))
				//should probably be: Clamp(h*0.0002*octavenoise(m_fracdef[5], 0.5, p),
				//		 0.1, 
				//		 0.5)  But I have no time for testing
				if (n > 0.25) {
					n += (n-0.25)*0.1*octavenoise(m_fracdef[3], 
						Clamp(h*0.0002*octavenoise(m_fracdef[5], 0.5, p),
						 0.5*octavenoise(m_fracdef[3], 0.5, p), 
						 0.5*octavenoise(m_fracdef[3], 0.5, p)), p); //[4]?
				} 
				
				if (n > 0.25) {
					n = n;
				} else if (n > 0.2) {
					n += (0.25-n)*0.2*ridged_octavenoise(m_fracdef[3], 
						Clamp(h*0.0002*octavenoise(m_fracdef[5], 0.5, p),
						 0.5*octavenoise(m_fracdef[3], 0.5, p), 
						 0.5*octavenoise(m_fracdef[4], 0.5, p)), p);
				} else if (n > 0.05) {
					n += ((n-0.05)/15)*ridged_octavenoise(m_fracdef[3], 
						Clamp(h*0.0002*octavenoise(m_fracdef[5], 0.5, p),
						 0.5*octavenoise(m_fracdef[3], 0.5, p), 
						 0.5*octavenoise(m_fracdef[4], 0.5, p)), p);
				}
				n = n*0.2;

				if (n < 0.01){
					n += n*voronoiscam_octavenoise(m_fracdef[3], 
						Clamp(h*0.00002, 0.5, 0.5), p);
				} else if (n <0.02){
					n += 0.01*voronoiscam_octavenoise(m_fracdef[3], 
						Clamp(h*0.00002, 0.5, 0.5), p);
				} else {
					n += (0.02/n)*0.01*voronoiscam_octavenoise(m_fracdef[3], 
						Clamp(h*0.00002, 0.5, 0.5), p);
				}

				if (n < 0.001){
					n += n*3*dunes_octavenoise(m_fracdef[2], 
						1.0*octavenoise(m_fracdef[2], 0.5, p), p);
				} else if (n <0.01){
					n += 0.003*dunes_octavenoise(m_fracdef[2], 
						1.0*octavenoise(m_fracdef[2], 0.5, p), p);
				} else {
					n += (0.01/n)*0.003*dunes_octavenoise(m_fracdef[2], 
						1.0*octavenoise(m_fracdef[2], 0.5, p), p);
				}

				if (n < 0.001){
					n += n*0.2*ridged_octavenoise(m_fracdef[1], 
						0.5*octavenoise(m_fracdef[2], 0.5, p), p);
				} else if (n <0.01){
					n += 0.0002*ridged_octavenoise(m_fracdef[1], 
						0.5*octavenoise(m_fracdef[2], 0.5, p), p);
				} else {
					n += (0.01/n)*0.0002*ridged_octavenoise(m_fracdef[1], 
						0.5*octavenoise(m_fracdef[2], 0.5, p), p);
				}

				if (n < 0.1){
					n += n*0.05*dunes_octavenoise(m_fracdef[2], 
						n*river_octavenoise(m_fracdef[2], 0.5, p), p);
				} else if (n <0.2){
					n += 0.005*dunes_octavenoise(m_fracdef[2], 
						((n*n*10.0)+(3*(n-0.1)))*
						river_octavenoise(m_fracdef[2], 0.5, p), p);
				} else {
					n += (0.2/n)*0.005*dunes_octavenoise(m_fracdef[2], 
						Clamp(0.7-(1-(5*n)), 0.0, 0.7)*
						river_octavenoise(m_fracdef[2], 0.5, p), p);
				}
 
				//terrain is too mountainous, so we reduce the height
				n *= 0.3;

			}
			
			n = m_maxHeight*n;
			return (n > 0.0 ? n : 0.0); 
		}
		case TERRAIN_MOUNTAINS_RIDGED:
		{
			double continents = octavenoise(m_fracdef[0], 0.5, p) - m_sealevel;
			if (continents < 0) return 0;
			// unused variable \\ double mountain_distrib = octavenoise(m_fracdef[1], 0.5, p);
			double mountains = octavenoise(m_fracdef[2], 0.5, p);
			double mountains2 = ridged_octavenoise(m_fracdef[3], 0.5, p);

			double hill_distrib = octavenoise(m_fracdef[4], 0.5, p);
			double hills = hill_distrib * m_fracdef[5].amplitude * ridged_octavenoise(m_fracdef[5], 0.5, p);
			double hills2 = hill_distrib * m_fracdef[6].amplitude * octavenoise(m_fracdef[6], 0.5, p);

			double hill2_distrib = octavenoise(m_fracdef[7], 0.5, p);
			double hills3 = hill2_distrib * m_fracdef[8].amplitude * ridged_octavenoise(m_fracdef[8], 0.5, p);
			double hills4 = hill2_distrib * m_fracdef[9].amplitude * ridged_octavenoise(m_fracdef[9], 0.5, p);

			double n = continents - (m_fracdef[0].amplitude*m_sealevel);

			if (n > 0.0) {
				// smooth in hills at shore edges
				if (n < 0.1) n += hills * n * 10.0f;
				else n += hills;
				if (n < 0.05) n += hills2 * n * 20.0f;
				else n += hills2 ;

				if (n < 0.1) n += hills3 * n * 10.0f;
				else n += hills3;
				if (n < 0.05) n += hills4 * n * 20.0f;
				else n += hills4 ;

				mountains  = octavenoise(m_fracdef[1], 0.5, p) *
					m_fracdef[2].amplitude * mountains*mountains*mountains;
				mountains2 = octavenoise(m_fracdef[4], 0.5, p) *
					m_fracdef[3].amplitude * mountains2*mountains2*mountains2*mountains2;
				if (n > 0.2) n += mountains2 * (n - 0.2) ;
				if (n < 0.2) n += mountains * n * 5.0f ;
				else n += mountains  ; 
			}
			
			n = m_maxHeight*n;
			return (n > 0.0 ? n : 0.0); 
		}
		case TERRAIN_MOUNTAINS_RIVERS:
		{
			double continents = octavenoise(m_fracdef[0], 0.7*
				ridged_octavenoise(m_fracdef[8], 0.58, p), p) - m_sealevel*0.65;
			if (continents < 0) return 0;
			double n = (river_function(m_fracdef[9], p)*
				river_function(m_fracdef[7], p)*
				river_function(m_fracdef[6], p)*
				canyon3_normal_function(m_fracdef[1], p)*continents) -
				(m_fracdef[0].amplitude*m_sealevel*0.1);
			n *= 0.5;

			double h = n;
		/*  Definitions here for easy referral
			SetFracDef(&m_fracdef[0], m_maxHeightInMeters, rand.Double(1e6, 2e6), rand, 10);
			SetFracDef(&m_fracdef[1], m_maxHeightInMeters, 11e6, rand, 10);
			SetFracDef(&m_fracdef[2], m_maxHeightInMeters*0.0000001, rand.Double(500, 2e3), rand, 10);
			SetFracDef(&m_fracdef[3], m_maxHeightInMeters*0.00002, rand.Double(1500, 1e4), rand, 10);
			SetFracDef(&m_fracdef[4], m_maxHeightInMeters*0.08, 1e4, rand, 10);
			SetFracDef(&m_fracdef[5], m_maxHeightInMeters*0.2, 1e5, rand, 10);
			SetFracDef(&m_fracdef[6], m_maxHeightInMeters*0.5, 1e6, rand, 10);
			SetFracDef(&m_fracdef[7], m_maxHeightInMeters*0.5, rand.Double(1e6,5e6), rand, 10);
			SetFracDef(&m_fracdef[8], m_maxHeightInMeters, rand.Double(3e6, 1e7), rand, 10);
			SetFracDef(&m_fracdef[9], m_maxHeightInMeters, 1e7, rand, 10.0);
		*/

			if (n > 0.0) {
				// smooth in hills at shore edges
				//large mountainous shapes
				n += h*river_octavenoise(m_fracdef[7], 
					0.5*octavenoise(m_fracdef[6], 0.5, p), p);

				//if (n < 0.2) n += canyon3_billow_function(m_fracdef[9], p) * n * 5;
				//else if (n < 0.4) n += canyon3_billow_function(m_fracdef[9], p);
				//else n += canyon3_billow_function(m_fracdef[9], p) * (0.4/n);
				//n += -0.5;
			}

			if (n > 0.0) {
				if (n < 0.4){
					n += n*2.5*river_octavenoise(m_fracdef[6], 
						Clamp(h*0.00002, 0.3, 0.7)*
						ridged_octavenoise(m_fracdef[5], 0.5, p), p);
				} else {
					n += 1.0*river_octavenoise(m_fracdef[6], 
						Clamp(h*0.00002, 0.3, 0.7)*
						ridged_octavenoise(m_fracdef[5], 0.5, p), p);
				}
			}

			if (n > 0.0) {
				if (n < 0.2){
					n += n*5.0*billow_octavenoise(m_fracdef[6], 
						Clamp(h*0.00002, 0.5, 0.7), p);
				} else {
					n += billow_octavenoise(m_fracdef[6], 
						Clamp(h*0.00002, 0.5, 0.7), p);
				}
			}

			if (n > 0.0) {
				if (n < 0.4){
					n += n*2.0*river_octavenoise(m_fracdef[6], 
						0.5*octavenoise(m_fracdef[5], 0.5, p), p);
				} else {
					n += (0.32/n)*river_octavenoise(m_fracdef[6], 
						0.5*octavenoise(m_fracdef[5], 0.5, p), p);
				}

				if (n < 0.2){
					n += n*ridged_octavenoise(m_fracdef[5], 
						0.5*octavenoise(m_fracdef[5], 0.5, p), p);
				} else {
					n += (0.04/n)*ridged_octavenoise(m_fracdef[5], 
						0.5*octavenoise(m_fracdef[5], 0.5, p), p);
				}
				//smaller ridged mountains
				n += n*0.7*ridged_octavenoise(m_fracdef[5], 
					0.7*octavenoise(m_fracdef[6], 0.6, p), p);

				//n += n*0.7*voronoiscam_octavenoise(m_fracdef[5], 
				//	0.7*octavenoise(m_fracdef[6], 0.6, p), p);

				//n = n*0.6667;
				
				//jagged surface for mountains
				if (n > 0.25) {
					n += (n-0.25)*0.1*octavenoise(m_fracdef[3], 
						Clamp(h*0.0002*octavenoise(m_fracdef[5], 0.6, p),
						 0.5*octavenoise(m_fracdef[3], 0.5, p), 
						 0.6*octavenoise(m_fracdef[4], 0.6, p)), p);
				} 
				
				if (n > 0.25) {
					n = n;
				} else if (n > 0.2) {
					n += (0.25-n)*0.2*ridged_octavenoise(m_fracdef[3], 
						Clamp(h*0.0002*octavenoise(m_fracdef[5], 0.5, p),
						 0.5*octavenoise(m_fracdef[3], 0.5, p), 
						 0.5*octavenoise(m_fracdef[4], 0.5, p)), p);
				} else if (n > 0.05) {
					n += ((n-0.05)/15)*ridged_octavenoise(m_fracdef[3], 
						Clamp(h*0.0002*octavenoise(m_fracdef[5], 0.5, p),
						 0.5*octavenoise(m_fracdef[3], 0.5, p), 
						 0.5*octavenoise(m_fracdef[4], 0.5, p)), p);
				}
				//n = n*0.2;

				if (n < 0.01){
					n += n*voronoiscam_octavenoise(m_fracdef[3], 
						Clamp(h*0.00002, 0.5, 0.5), p);
				} else if (n <0.02){
					n += 0.01*voronoiscam_octavenoise(m_fracdef[3], 
						Clamp(h*0.00002, 0.5, 0.5), p);
				} else {
					n += (0.02/n)*0.01*voronoiscam_octavenoise(m_fracdef[3], 
						Clamp(h*0.00002, 0.5, 0.5), p);
				}

				if (n < 0.001){
					n += n*3*dunes_octavenoise(m_fracdef[2], 
						1.0*octavenoise(m_fracdef[2], 0.5, p), p);
				} else if (n <0.01){
					n += 0.003*dunes_octavenoise(m_fracdef[2], 
						1.0*octavenoise(m_fracdef[2], 0.5, p), p);
				} else {
					n += (0.01/n)*0.003*dunes_octavenoise(m_fracdef[2], 
						1.0*octavenoise(m_fracdef[2], 0.5, p), p);
				}

				//if (n < 0.001){
				//	n += n*0.2*ridged_octavenoise(m_fracdef[2], 
				//		0.5*octavenoise(m_fracdef[2], 0.5, p), p);
				//} else if (n <0.01){
				//	n += 0.0002*ridged_octavenoise(m_fracdef[2], 
				//		0.5*octavenoise(m_fracdef[2], 0.5, p), p);
				//} else {
				//	n += (0.01/n)*0.0002*ridged_octavenoise(m_fracdef[2], 
				//		0.5*octavenoise(m_fracdef[2], 0.5, p), p);
				//}

				if (n < 0.1){
					n += n*0.05*dunes_octavenoise(m_fracdef[2], 
						n*river_octavenoise(m_fracdef[2], 0.5, p), p);
				} else if (n <0.2){
					n += 0.005*dunes_octavenoise(m_fracdef[2], 
						((n*n*10.0)+(3*(n-0.1)))*
						river_octavenoise(m_fracdef[2], 0.5, p), p);
				} else {
					n += (0.2/n)*0.005*dunes_octavenoise(m_fracdef[2], 
						Clamp(0.7-(1-(5*n)), 0.0, 0.7)*
						river_octavenoise(m_fracdef[2], 0.5, p), p);
				}
 
				n *= 0.3;

			}
			
			n = m_maxHeight*n;
			return (n > 0.0 ? n : 0.0); 
		}
		case TERRAIN_MOUNTAINS_CRATERS:
		{
			double continents = octavenoise(m_fracdef[0], 0.5, p) - m_sealevel;
			if (continents < 0) return 0;
			double n = 0.3 * continents;
			double m = m_fracdef[1].amplitude * ridged_octavenoise(m_fracdef[1], 0.5, p);
			double distrib = ridged_octavenoise(m_fracdef[4], 0.5, p);
			if (distrib > 0.5) m += 2.0 * (distrib-0.5) * m_fracdef[3].amplitude * ridged_octavenoise(m_fracdef[3], 0.5*distrib, p);
			// cliffs at shore
			if (continents < 0.001) n += m * continents * 1000.0f;
			else n += m;
			n += crater_function(m_fracdef[5], p);
			n += crater_function(m_fracdef[6], p);
			n *= m_maxHeight;
			return (n > 0.0 ? n : 0.0);
		}
		case TERRAIN_MOUNTAINS_CRATERS2:
		{
			double continents = octavenoise(m_fracdef[0], 0.5, p) - m_sealevel;
			if (continents < 0) return 0;
			double n = 0.3 * continents;
			double m = 0;//m_fracdef[1].amplitude * octavenoise(m_fracdef[1], 0.5, p);
			double distrib = 0.5*ridged_octavenoise(m_fracdef[1], 0.5*octavenoise(m_fracdef[2], 0.5, p), p);
			distrib += 0.7*billow_octavenoise(m_fracdef[2], 0.5*ridged_octavenoise(m_fracdef[1], 0.5, p), p) +
				0.1*octavenoise(m_fracdef[3], 0.5*ridged_octavenoise(m_fracdef[2], 0.5, p), p);

			if (distrib > 0.5) m += 2.0 * (distrib-0.5) * m_fracdef[3].amplitude * octavenoise(m_fracdef[4], 0.5*distrib, p);
			// cliffs at shore
			if (continents < 0.001) n += m * continents * 1000.0f;
			else n += m;
			n += crater_function(m_fracdef[5], p);
			n += crater_function(m_fracdef[6], p);
			n += crater_function(m_fracdef[7], p);
			n += crater_function(m_fracdef[8], p);
			n += crater_function(m_fracdef[9], p);
			n *= m_maxHeight;
			return (n > 0.0 ? n : 0.0);
		}
		case TERRAIN_MOUNTAINS_VOLCANO:
		{
			double continents = octavenoise(m_fracdef[0], 0.5, p) - m_sealevel;
			if (continents < 0) return 0;
			// unused variable \\ double mountain_distrib = octavenoise(m_fracdef[1], 0.5, p);
			double mountains = octavenoise(m_fracdef[2], 0.5, p);
			double mountains2 = octavenoise(m_fracdef[3], 0.5, p);
			double hill_distrib = octavenoise(m_fracdef[4], 0.5, p);
			double hills = hill_distrib * m_fracdef[5].amplitude * octavenoise(m_fracdef[5], 0.5, p);
			double hills2 = hill_distrib * m_fracdef[6].amplitude * octavenoise(m_fracdef[6], 0.5, p);



			double n = continents - (m_fracdef[0].amplitude*m_sealevel);

			
			if (n < 0.01) n += megavolcano_function(m_fracdef[7], p) * n * 3000.0f;
			else n += megavolcano_function(m_fracdef[7], p) * 30.0f;

			n = (n > 0.0 ? n : 0.0); 

			if ((m_seed>>2)%3 > 2) {
				if (n < .2f) n += canyon3_ridged_function(m_fracdef[8], p) * n * 2;
				else if (n < .4f) n += canyon3_ridged_function(m_fracdef[8], p) * .4;
				else n += canyon3_ridged_function(m_fracdef[8], p) * (.4/n) * .4;
			} else if ((m_seed>>2)%3 > 1) {
				if (n < .2f) n += canyon3_billow_function(m_fracdef[8], p) * n * 2;
				else if (n < .4f) n += canyon3_billow_function(m_fracdef[8], p) * .4;
				else n += canyon3_billow_function(m_fracdef[8], p) * (.4/n) * .4;
			} else {
				if (n < .2f) n += canyon3_voronoi_function(m_fracdef[8], p) * n * 2;
				else if (n < .4f) n += canyon3_voronoi_function(m_fracdef[8], p) * .4;
				else n += canyon3_voronoi_function(m_fracdef[8], p) * (.4/n) * .4;
			}

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
			// unused variable \\ double mountain_distrib = octavenoise(m_fracdef[1], 0.5, p);
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

			// BEWARE THE WALL OF TEXT
			if ((m_seed>>2) %3 > 2) {

				if (n < .2f) n += canyon3_ridged_function(m_fracdef[8], p) * n * 2;
				else if (n < .4f) n += canyon3_ridged_function(m_fracdef[8], p) * .4;
				else n += canyon3_ridged_function(m_fracdef[8], p) * (.4/n) * .4;

				if (n < .2f) n += canyon2_ridged_function(m_fracdef[8], p) * n * 2;
				else if (n < .4f) n += canyon2_ridged_function(m_fracdef[8], p) * .4;
				else n += canyon2_ridged_function(m_fracdef[8], p) * (.4/n) * .4;

				if (n < .2f) n += canyon_ridged_function(m_fracdef[8], p) * n * 2;
				else if (n < .4f) n += canyon_ridged_function(m_fracdef[8], p) * .4;
				else n += canyon_ridged_function(m_fracdef[8], p) * (.4/n) * .4;

				if (n < .2f) n += canyon3_ridged_function(m_fracdef[9], p) * n * 2;
				else if (n < .4f) n += canyon3_ridged_function(m_fracdef[9], p) * .4;
				else n += canyon3_ridged_function(m_fracdef[9], p) * (.4/n) * .4;

				if (n < .2f) n += canyon2_ridged_function(m_fracdef[9], p) * n * 2;
				else if (n < .4f) n += canyon2_ridged_function(m_fracdef[9], p) * .4;
				else n += canyon2_ridged_function(m_fracdef[9], p) * (.4/n) * .4;

				if (n < .2f) n += canyon_ridged_function(m_fracdef[9], p) * n * 2;
				else if (n < .4f) n += canyon_ridged_function(m_fracdef[9], p) * .4;
				else n += canyon_ridged_function(m_fracdef[9], p) * (.4/n) * .4;

			} else if ((m_seed>>2) %3 > 1) {

				if (n < .2f) n += canyon3_billow_function(m_fracdef[8], p) * n * 2;
				else if (n < .4f) n += canyon3_billow_function(m_fracdef[8], p) * .4;
				else n += canyon3_billow_function(m_fracdef[8], p) * (.4/n) * .4;

				if (n < .2f) n += canyon2_billow_function(m_fracdef[8], p) * n * 2;
				else if (n < .4f) n += canyon2_billow_function(m_fracdef[8], p) * .4;
				else n += canyon2_billow_function(m_fracdef[8], p) * (.4/n) * .4;

				if (n < .2f) n += canyon_billow_function(m_fracdef[8], p) * n * 2;
				else if (n < .4f) n += canyon_billow_function(m_fracdef[8], p) * .4;
				else n += canyon_billow_function(m_fracdef[8], p) * (.4/n) * .4;

				if (n < .2f) n += canyon3_billow_function(m_fracdef[9], p) * n * 2;
				else if (n < .4f) n += canyon3_billow_function(m_fracdef[9], p) * .4;
				else n += canyon3_billow_function(m_fracdef[9], p) * (.4/n) * .4;

				if (n < .2f) n += canyon2_billow_function(m_fracdef[9], p) * n * 2;
				else if (n < .4f) n += canyon2_billow_function(m_fracdef[9], p) * .4;
				else n += canyon2_billow_function(m_fracdef[9], p) * (.4/n) * .4;

				if (n < .2f) n += canyon_billow_function(m_fracdef[9], p) * n * 2;
				else if (n < .4f) n += canyon_billow_function(m_fracdef[9], p) * .4;
				else n += canyon_billow_function(m_fracdef[9], p) * (.4/n) * .4;

			} else {

				if (n < .2f) n += canyon3_voronoi_function(m_fracdef[8], p) * n * 2;
				else if (n < .4f) n += canyon3_voronoi_function(m_fracdef[8], p) * .4;
				else n += canyon3_voronoi_function(m_fracdef[8], p) * (.4/n) * .4;

				if (n < .2f) n += canyon2_voronoi_function(m_fracdef[8], p) * n * 2;
				else if (n < .4f) n += canyon2_voronoi_function(m_fracdef[8], p) * .4;
				else n += canyon2_voronoi_function(m_fracdef[8], p) * (.4/n) * .4;

				if (n < .2f) n += canyon_voronoi_function(m_fracdef[8], p) * n * 2;
				else if (n < .4f) n += canyon_voronoi_function(m_fracdef[8], p) * .4;
				else n += canyon_voronoi_function(m_fracdef[8], p) * (.4/n) * .4;

				if (n < .2f) n += canyon3_voronoi_function(m_fracdef[9], p) * n * 2;
				else if (n < .4f) n += canyon3_voronoi_function(m_fracdef[9], p) * .4;
				else n += canyon3_voronoi_function(m_fracdef[9], p) * (.4/n) * .4;

				if (n < .2f) n += canyon2_voronoi_function(m_fracdef[9], p) * n * 2;
				else if (n < .4f) n += canyon2_voronoi_function(m_fracdef[9], p) * .4;
				else n += canyon2_voronoi_function(m_fracdef[9], p) * (.4/n) * .4;

				if (n < .2f) n += canyon_voronoi_function(m_fracdef[9], p) * n * 2;
				else if (n < .4f) n += canyon_voronoi_function(m_fracdef[9], p) * .4;
				else n += canyon_voronoi_function(m_fracdef[9], p) * (.4/n) * .4;

			}

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
			double continents = octavenoise(m_fracdef[0], Clamp(0.725-(m_sealevel/2), 0.1, 0.725), p) - m_sealevel;
			if (continents < 0) return 0;
			double mountain_distrib = octavenoise(m_fracdef[1], 0.55, p);
			double mountains = octavenoise(m_fracdef[2], 0.5, p) * ridged_octavenoise(m_fracdef[2], 0.575, p);
			double mountains2 = octavenoise(m_fracdef[3], 0.5, p);
			double hill_distrib = octavenoise(m_fracdef[4], 0.5, p);
			double hills = hill_distrib * m_fracdef[5].amplitude * octavenoise(m_fracdef[5], 0.5, p);
			double rocks = octavenoise(m_fracdef[9], 0.5, p);

			
			double n = continents - (m_fracdef[0].amplitude*m_sealevel);
			//double n = (megavolcano_function(p) + volcano_function(p) + smlvolcano_function(p));
			n += mountains*mountains2*5.0*megavolcano_function(m_fracdef[6], p);
			n += 2.5*megavolcano_function(m_fracdef[6], p);
			n += mountains*mountains2*5.0*volcano_function(m_fracdef[6], p)*volcano_function(m_fracdef[6], p);
			n += 2.5*volcano_function(m_fracdef[6], p);
			
			n += mountains*mountains2*7.5*megavolcano_function(m_fracdef[7], p);
			n += 2.5*megavolcano_function(m_fracdef[7], p);
			n += mountains*mountains2*7.5*volcano_function(m_fracdef[7], p)*volcano_function(m_fracdef[7], p);
			n += 2.5*volcano_function(m_fracdef[7], p);

			
			//n += 1.4*(continents - targ.continents.amplitude*targ.sealevel + (volcano_function(p)*1)) ;
			//smooth canyon transitions and limit height of canyon placement
			if (n < .01) n += n * 100.0f * canyon3_ridged_function(m_fracdef[8], p);
			else if (n < .7) n += canyon3_ridged_function(m_fracdef[8], p);
			else n += canyon3_ridged_function(m_fracdef[8], p);

			if (n < .01) n += n * 100.0f * canyon2_ridged_function(m_fracdef[8], p);
			else if (n < .7) n += canyon2_ridged_function(m_fracdef[8], p);
			else n += canyon2_ridged_function(m_fracdef[8], p);
			n *= 0.5;

			n += continents*hills*hill_distrib*mountain_distrib;

			mountains  = octavenoise(m_fracdef[1], 0.5, p) *
					m_fracdef[2].amplitude * mountains*mountains*mountains;
			mountains2 = octavenoise(m_fracdef[4], 0.5, p) *
					m_fracdef[3].amplitude * mountains2*mountains2*mountains2;
			/*mountains = fractal(2, targ.mountainDistrib, (m_seed>>2)&3, p) *
				targ.mountains.amplitude * mountains*mountains*mountains;
			mountains2 = fractal(24, targ.mountainDistrib, (m_seed>>2)&3, p) *
				targ.mountains.amplitude * mountains*mountains*mountains;*/
				
			n += continents*mountains*hill_distrib ;
			if (n < 0.01) n += continents*mountains2 * n * 40.0f ;
			else n += continents*mountains2*.4f ;
			n *= 0.2;
			n += mountains*mountains2*mountains2*hills*hills*hill_distrib*mountain_distrib*20.0;

			rocks = continents * mountain_distrib * m_fracdef[9].amplitude * rocks*rocks*rocks * 2.0;
			n += rocks;
		
			n = (n<0.0 ? 0.0 : m_maxHeight*n);
			return n;
		}
		case TERRAIN_H2O_SOLID:
		{
			double continents = 0.7*river_octavenoise(m_fracdef[2], 0.5, p)-m_sealevel;
			continents = m_fracdef[0].amplitude * ridged_octavenoise(m_fracdef[0], 
				Clamp(continents, 0.0, 0.6), p);
			double mountains = ridged_octavenoise(m_fracdef[2], 0.5, p);
			double hills = octavenoise(m_fracdef[2], 0.5, p) *
				m_fracdef[1].amplitude * river_octavenoise(m_fracdef[1], 0.5, p);
			double n = continents - (m_fracdef[0].amplitude*m_sealevel);
			// craters
			n += crater_function(m_fracdef[5], p);	
			if (n > 0.0) {
				// smooth in hills at shore edges 
				if (n < 0.05) {
					n += hills * n * 4.0 ;
					n += n * 20.0 * (billow_octavenoise(m_fracdef[3], 0.5*
						ridged_octavenoise(m_fracdef[2], 0.5, p), p) +
						river_octavenoise(m_fracdef[4], 0.5*
						ridged_octavenoise(m_fracdef[3], 0.5, p), p) +
						billow_octavenoise(m_fracdef[3], 0.6*
						ridged_octavenoise(m_fracdef[4], 0.55, p), p));
				} else {
					n += hills * .2f ;
					n += billow_octavenoise(m_fracdef[3], 0.5*
						ridged_octavenoise(m_fracdef[2], 0.5, p), p) +
						river_octavenoise(m_fracdef[4], 0.5*
						ridged_octavenoise(m_fracdef[3], 0.5, p), p) +
						billow_octavenoise(m_fracdef[3], 0.6*
						ridged_octavenoise(m_fracdef[4], 0.55, p), p);
				}
				// adds mountains hills craters 
				mountains = octavenoise(m_fracdef[3], 0.5, p) *
					m_fracdef[2].amplitude * mountains*mountains*mountains;
				if (n < 0.4) n += 2.0 * n * mountains;
				else n += mountains * .8f;
			}			
			n = m_maxHeight*n;
			n = (n<0.0 ? -n : n);
			n = (n>1.0 ? 2.0-n : n);
			return n;
		}
		case TERRAIN_H2O_SOLID_CANYONS:
		{
			double continents = 0.7*river_octavenoise(m_fracdef[2], 0.5, p)-m_sealevel;
			continents = m_fracdef[0].amplitude * ridged_octavenoise(m_fracdef[0], 
				Clamp(continents, 0.0, 0.6), p);
			double mountains = ridged_octavenoise(m_fracdef[2], 0.5, p);
			double hills = octavenoise(m_fracdef[2], 0.5, p) *
				m_fracdef[1].amplitude * river_octavenoise(m_fracdef[1], 0.5, p);
			double n = continents - (m_fracdef[0].amplitude*m_sealevel);
			if (n > 0.0) {
				// smooth in hills at shore edges 
				if (n < 0.05) {
					n += hills * n * 4.0 ;
					n += n * 20.0 * (billow_octavenoise(m_fracdef[3], 0.5*
						ridged_octavenoise(m_fracdef[2], 0.5, p), p) +
						river_octavenoise(m_fracdef[4], 0.5*
						ridged_octavenoise(m_fracdef[3], 0.5, p), p) +
						billow_octavenoise(m_fracdef[3], 0.6*
						ridged_octavenoise(m_fracdef[4], 0.55, p), p));
				} else {
					n += hills * .2f ;
					n += billow_octavenoise(m_fracdef[3], 0.5*
						ridged_octavenoise(m_fracdef[2], 0.5, p), p) +
						river_octavenoise(m_fracdef[4], 0.5*
						ridged_octavenoise(m_fracdef[3], 0.5, p), p) +
						billow_octavenoise(m_fracdef[3], 0.6*
						ridged_octavenoise(m_fracdef[4], 0.55, p), p);
				}
				// adds mountains hills craters 
				mountains = octavenoise(m_fracdef[3], 0.5, p) *
					m_fracdef[2].amplitude * mountains*mountains*mountains;
				if (n < 0.4) n += 2.0 * n * mountains;
				else n += mountains * .8f;
			}	
			// craters
			n += 3.0*impact_crater_function(m_fracdef[5], p);	
			n = m_maxHeight*n;
			n = (n<0.0 ? 0 : n);
			n = (n>1.0 ? 2.0-n : n);
			return n;
		}
		case TERRAIN_RUGGED_DESERT:
		{
			double continents = octavenoise(m_fracdef[0], 0.5, p) - m_sealevel;// + (cliff_function(m_fracdef[7], p)*0.5);
			if (continents < 0) return 0;
			double mountain_distrib = octavenoise(m_fracdef[2], 0.5, p);
			double mountains = ridged_octavenoise(m_fracdef[1], 0.5, p);
			//double rocks = octavenoise(m_fracdef[9], 0.5, p);
			double hill_distrib = octavenoise(m_fracdef[4], 0.5, p);
			double hills = hill_distrib * m_fracdef[3].amplitude * billow_octavenoise(m_fracdef[3], 0.5, p);
			double dunes = hill_distrib * m_fracdef[5].amplitude * dunes_octavenoise(m_fracdef[5], 0.5, p);
			double n = continents * m_fracdef[0].amplitude * 2 ;//+ (cliff_function(m_fracdef[6], p)*0.5);
			double m = canyon_normal_function(m_fracdef[6], p);
			m += canyon2_normal_function(m_fracdef[6], p);
			m += canyon3_ridged_function(m_fracdef[6], p);
			m = (n<1 ? n : 1/n ); //sometimes creates some interesting features
			m += canyon_billow_function(m_fracdef[7], p);
			m += canyon2_ridged_function(m_fracdef[7], p);
			m += canyon3_normal_function(m_fracdef[7], p);
			m += canyon_normal_function(m_fracdef[8], p);
			m += canyon2_ridged_function(m_fracdef[8], p);
			m += canyon3_voronoi_function(m_fracdef[8], p);
			m += -0.5;
			m = n * 0.5;
			m = (n<0.0 ? 0.0 : n);
			n += m;

			// makes larger dunes at lower altitudes, flat ones at high altitude.
			mountains = mountain_distrib * m_fracdef[3].amplitude * mountains*mountains*mountains;
			// smoothes edges of mountains and places them only above a set altitude
			if (n < 0.1) n += n * 10.0f * hills;
			else n += hills;
			if (n > 0.2) n += dunes * (0.2/n);  
			else n += dunes;
			if (n < 0.1) n += n * 10.0f * mountains;
			else n += mountains;	
			
			
			//rocks = mountain_distrib * m_fracdef[9].amplitude * rocks*rocks*rocks;
			//n += rocks ;
			
			
			//n = (n<0.0 ? 0.0 : m_maxHeight*n);
			n = m_maxHeight*n;
			return n;
		}
	}
    return 0;
}

/* These fuctions should not be used by GeoSphereStyle::GetHeight, so don't move these definitions
   to above that function. GetHeight should use the versions of these functions that take fracdef_t
   objects, ensuring that the resulting terrains have the desired scale */
static inline double octavenoise(int octaves, double roughness, double lacunarity, const vector3d &p);
static inline double river_octavenoise(int octaves, double roughness, double lacunarity, const vector3d &p);
static inline double ridged_octavenoise(int octaves, double roughness, double lacunarity, const vector3d &p);
static inline double billow_octavenoise(int octaves, double roughness, double lacunarity, const vector3d &p);
static inline double voronoiscam_octavenoise(int octaves, double roughness, double lacunarity, const vector3d &p);

#define rock   octavenoise(m_fracdef[0], 0.65, p);
#define rock2  octavenoise(m_fracdef[1], 0.6, p)*ridged_octavenoise(m_fracdef[0], 0.55, p);
#define rock3  0.5*ridged_octavenoise(m_fracdef[0], 0.5, p)*voronoiscam_octavenoise(m_fracdef[0], 0.5, p)*ridged_octavenoise(m_fracdef[1], 0.5, p);
#define rock4  ridged_octavenoise(m_fracdef[1], 0.5, p)*octavenoise(m_fracdef[1], 0.5, p)*octavenoise(m_fracdef[5], 0.5, p);
#define mud    0.1*voronoiscam_octavenoise(m_fracdef[1], 0.5, p)*octavenoise(m_fracdef[1], 0.5, p) * m_fracdef[5].amplitude;
#define sand   ridged_octavenoise(m_fracdef[0], 0.4, p)*dunes_octavenoise(m_fracdef[2], 0.4, p) + 0.1*dunes_octavenoise(m_fracdef[1], 0.5, p);
#define sand2  dunes_octavenoise(m_fracdef[0], 0.6, p)*octavenoise(m_fracdef[4], 0.6, p);
#define sand3  dunes_octavenoise(m_fracdef[2], 0.6, p)*dunes_octavenoise(m_fracdef[6], 0.6, p);
#define grass  billow_octavenoise(m_fracdef[1], 0.8, p);
#define grass2 billow_octavenoise(m_fracdef[3], 0.6, p)*voronoiscam_octavenoise(m_fracdef[4], 0.6, p)*river_octavenoise(m_fracdef[5], 0.6, p);
#define forest octavenoise(m_fracdef[1], 0.65, p)*voronoiscam_octavenoise(m_fracdef[2], 0.65, p);
#define water  dunes_octavenoise(m_fracdef[6], 0.6, p);

/**
 * Height: 0.0 would be sea-level. 1.0 would be an extra elevation of 1 radius (huge)
 */
vector3d GeoSphereStyle::GetColor(const vector3d &p, double height, const vector3d &norm)
{
	switch (m_colorType) {
	case COLOR_NONE:
	case COLOR_SOLID: 
		return vector3d(1.0);
	case COLOR_STAR_BROWN_DWARF: {
		double n;
		vector3d col;
			n = voronoiscam_octavenoise(m_fracdef[0], 0.6, p) * 0.5;
			//n += ridged_octavenoise(m_fracdef[1], 0.7, p) * 0.5;
			//n += voronoiscam_octavenoise(m_fracdef[0], 0.8, p) * ridged_octavenoise(m_fracdef[1], 0.8, p);
			//n *= n * n;
			//n += voronoiscam_octavenoise(m_fracdef[2], 0.6, p) * 0.5;
			//n += ridged_octavenoise(m_fracdef[3], 0.6, p) * 0.5;
			//n += 0.8*billow_octavenoise(m_fracdef[0], 0.8, noise(p*3.142)*p);
			//n *= n;
			if (n > 0.666) {
				n -= 0.666; n *= 3.0;
				col = interpolate_color(n, vector3d(.25, .2, .2), vector3d(.1, .0, .0) );
				return col;
			} else if (n > 0.333) {
				n -= 0.333; n *= 3.0;
				col = interpolate_color(n, vector3d(.2, .25, .1), vector3d(.25, .2, .2) );
				return col;
			} else {
				n *= 3.0;
				col = interpolate_color(n, vector3d(1.5, 1.0, 1.0), vector3d(.2, .25, .1) );
				return col;
			}
		}
	case COLOR_STAR_WHITE_DWARF: {
		double n;
		vector3d col;
			n = ridged_octavenoise(m_fracdef[0], 0.8, p*p.x);
			n += ridged_octavenoise(m_fracdef[1], 0.8, p);
			n += voronoiscam_octavenoise(m_fracdef[0], 0.8 * octavenoise(m_fracdef[1], 0.6, p), p);
			n *= n*n;
			if (n > 0.666) {
				n -= 0.666; n *= 3.0;
				col = interpolate_color(n, vector3d(.8, .8, 1.0), vector3d(1.0, 1.0, 1.0));
				return col;
			} else if (n > 0.333) {
				n -= 0.333; n *= 3.0;
				col = interpolate_color(n, vector3d(.6, .8, .8), vector3d(.8, .8, 1.0));
				return col;
			} else {
				n *= 3.0;
				col = interpolate_color(n, vector3d(.0, .0, .9), vector3d(.6, .8, .8));
				return col;
			}
		}
	case COLOR_STAR_M: {
		double n;
		vector3d col;
			n = ridged_octavenoise(m_fracdef[0], 0.6, p) * 0.5;
			n += ridged_octavenoise(m_fracdef[1], 0.7, p) * 0.5;
			n += ridged_octavenoise(m_fracdef[0], 0.8, p) * ridged_octavenoise(m_fracdef[1], 0.8, p);
			n *= n * n;
			n += ridged_octavenoise(m_fracdef[2], 0.6, p) * 0.5;
			n += ridged_octavenoise(m_fracdef[3], 0.6, p) * 0.5;
			n += 0.8*billow_octavenoise(m_fracdef[0], 0.8, noise(p*3.142)*p)*
			 megavolcano_function(m_fracdef[1], p);
			n *= 0.5;
			if (n > 0.666) {
				n -= 0.666; n *= 3.0;
				col = interpolate_color(n, vector3d(.65, .5, .25), vector3d(1.0, 1.0, 1.0) );
				return col;
			} else if (n > 0.333) {
				n -= 0.333; n *= 3.0;
				col = interpolate_color(n, vector3d(.3, .1, .0), vector3d(.65, .5, .25) );
				return col;
			} else {
				n *= 3.0;
				col = interpolate_color(n, vector3d(.2, .0, .0), vector3d(.3, .1, .0) );
				return col;
			}
		}
	case COLOR_STAR_K: {
		double n;
		vector3d col;
			n = octavenoise(m_fracdef[0], 0.6, p) * 0.5;
			n += ridged_octavenoise(m_fracdef[1], 0.7, p) * 0.5;
			n += billow_octavenoise(m_fracdef[0], 0.8, p) * octavenoise(m_fracdef[1], 0.8, p);
			n -= dunes_octavenoise(m_fracdef[2], 0.6, p) * 0.5;
			n += octavenoise(m_fracdef[3], 0.6, p) * 0.5;
			n *= n * 0.2;
			if (n > 0.666) {
				n -= 0.666; n *= 3.0;
				col = interpolate_color(n, vector3d(.95, .7, .25), vector3d(1.0, 1.0, 1.0) );
				return col;
			} else if (n > 0.333) {
				n -= 0.333; n *= 3.0;
				col = interpolate_color(n, vector3d(.4, .25, .0), vector3d(.95, .7, .25) );
				return col;
			} else {
				n *= 3.0;
				col = interpolate_color(n, vector3d(.2, .05, .0), vector3d(.4, .25, .0) );
				return col;
			}
		}
   case COLOR_STAR_G: {
		double n;
		vector3d col;
			n = voronoiscam_octavenoise(m_fracdef[0], 0.5, p) * 0.5;
			n += voronoiscam_octavenoise(m_fracdef[1], 0.5, p) * 0.5;
			n += river_octavenoise(m_fracdef[0], 0.5, p) * voronoiscam_octavenoise(m_fracdef[1], 0.5, p);
			n += river_octavenoise(m_fracdef[2], 0.5, p) * 0.5;
			n += voronoiscam_octavenoise(m_fracdef[3], 0.5, p) * 0.5;
			n *= n * n * n * 0.1;
			if (n > 0.666) {
				n -= 0.666; n *= 3.0;
				col = interpolate_color(n, vector3d(.9, .9, .05), vector3d(1.0, 1.0, 1.0) );
				return col;
			} else if (n > 0.333) {
				n -= 0.333; n *= 3.0;
				col = interpolate_color(n, vector3d(.6, .6, .0), vector3d(.9, .9, .05) );
				return col;
			} else {
				n *= 3.0;
				col = interpolate_color(n, vector3d(.8, .8, .0), vector3d(.6, .6, .0) );
				return col;
			}
		}
	case COLOR_GG_JUPITER: {
		double n;
		double h = river_octavenoise(m_fracdef[0], 0.5*m_entropy[0] + 
			0.25f, noise(vector3d(p.x*8, p.y*32, p.z*8)))*.125;
		double equatorial_region_1 = billow_octavenoise(m_fracdef[0], 0.7, p) * p.y * p.x;
		double equatorial_region_2 = octavenoise(m_fracdef[1], 0.8, p) * p.x * p.x;
		vector3d col;
		col = interpolate_color(equatorial_region_1, m_ggdarkColor[0], m_ggdarkColor[1]);
		col = interpolate_color(equatorial_region_2, col, vector3d(.45, .3, .0));
		//top stripe
		if (p.y < 0.5 && p.y > 0.1) {
			for(float i=-1 ; i < 1; i+=0.6){
				double temp = p.y - i;
				if ( temp < .15+h && temp > -.15+h ){
					n = billow_octavenoise(m_fracdef[2], 0.7*m_entropy[0], 
						noise(vector3d(p.x, p.y*m_planetEarthRadii*0.3, p.z))*p);
					n += 0.5*octavenoise(m_fracdef[1], 0.6*m_entropy[0],
						noise(vector3d(p.x, p.y*m_planetEarthRadii, p.z))*p);
					n += ridged_octavenoise(m_fracdef[1], 0.6*m_entropy[0], 
						noise(vector3d(p.x, p.y*m_planetEarthRadii*0.3, p.z))*p);
					//n += 0.5;
					n *= n;
					n = (n<0.0 ? -n : n);
					n = (n>1.0 ? 2.0-n : n);
					if (n >0.8) {
						n -= 0.8; n *= 5.0;
						col = interpolate_color(n, col, m_ggdarkColor[7] );
						return col;
					} else if (n>0.6) {
						n -= 0.6; n*= 5.0;
						col = interpolate_color(n, m_gglightColor[4], col );
						return col;
					} else if (n>0.4) {
						n -= 0.4; n*= 5.0;
						col = interpolate_color(n, vector3d(.9, .89, .85), m_gglightColor[4] );
						return col;
					} else if (n>0.2) {
						n -= 0.2; n*= 5.0;
						col = interpolate_color(n, m_ggdarkColor[2], vector3d(.9, .89, .85) );
						return col;
					} else {
						n *= 5.0;
						col = interpolate_color(n, col, m_ggdarkColor[2] );
						return col;
					}
				}
			} // bottom stripe
		} else if (p.y < -0.1 && p.y > -0.5) { 
			for(float i=-1 ; i < 1; i+=0.6){
				double temp = p.y - i;
				if ( temp < .15+h && temp > -.15+h ){
					n = billow_octavenoise(m_fracdef[2], 0.6*m_entropy[0], 
						noise(vector3d(p.x, p.y*m_planetEarthRadii*0.3, p.z))*p);
					n += 0.5*octavenoise(m_fracdef[1], 0.7*m_entropy[0],
						noise(vector3d(p.x, p.y*m_planetEarthRadii, p.z))*p);
					n += ridged_octavenoise(m_fracdef[1], 0.6*m_entropy[0], 
						noise(vector3d(p.x, p.y*m_planetEarthRadii*0.3, p.z))*p);
					//n += 0.5;
					//n *= n;
					n = (n<0.0 ? -n : n);
					n = (n>1.0 ? 2.0-n : n);
					if (n >0.8) {
						n -= 0.8; n *= 5.0;
						col = interpolate_color(n, col, m_ggdarkColor[7] );
						return col;
					} else if (n>0.6) {
						n -= 0.6; n*= 5.0;
						col = interpolate_color(n, m_gglightColor[4], col );
						return col;
					} else if (n>0.4) {
						n -= 0.4; n*= 5.0;
						col = interpolate_color(n, vector3d(.9, .89, .85), m_gglightColor[4] );
						return col;
					} else if (n>0.2) {
						n -= 0.2; n*= 5.0;
						col = interpolate_color(n, m_ggdarkColor[2], vector3d(.9, .89, .85) );
						return col;
					} else {
						n *= 5.0;
						col = interpolate_color(n, col, m_ggdarkColor[2] );
						return col;
					}
				}
			}
		} else {  //small stripes
			for(float i=-1 ; i < 1; i+=0.3){
				double temp = p.y - i;
				if ( temp < .1+h && temp > -.0+h ){
					n = billow_octavenoise(m_fracdef[2], 0.6*m_entropy[0], 
						noise(vector3d(p.x, p.y*m_planetEarthRadii*0.3, p.z))*p);
					n += 0.5*octavenoise(m_fracdef[1], 0.6*m_entropy[0],
						noise(vector3d(p.x, p.y*m_planetEarthRadii, p.z))*p);
					n += ridged_octavenoise(m_fracdef[1], 0.7*m_entropy[0], 
						noise(vector3d(p.x, p.y*m_planetEarthRadii*0.3, p.z))*p);
					//n += 0.5;
					//n *= n;
					n = (n<0.0 ? -n : n);
					n = (n>1.0 ? 2.0-n : n);
					if (n >0.8) {
						n -= 0.8; n *= 5.0;
						col = interpolate_color(n, col, m_ggdarkColor[7] );
						return col;
					} else if (n>0.6) {
						n -= 0.6; n*= 5.0;
						col = interpolate_color(n, m_gglightColor[4], col );
						return col;
					} else if (n>0.4) {
						n -= 0.4; n*= 5.0;
						col = interpolate_color(n, vector3d(.9, .89, .85), m_gglightColor[4] );
						return col;
					} else if (n>0.2) {
						n -= 0.2; n*= 5.0;
						col = interpolate_color(n, m_ggdarkColor[2], vector3d(.9, .89, .85) );
						return col;
					} else {
						n *= 5.0;
						col = interpolate_color(n, col, m_ggdarkColor[2] );
						return col;
					}
				}
			}
		}
		//if is not a stripe.
		n = octavenoise(m_fracdef[1], 0.6*m_entropy[0] + 
			0.25f,noise(vector3d(p.x, p.y*m_planetEarthRadii*3, p.z))*p);
		n *= n*n;
		n = (n<0.0 ? -n : n);
		n = (n>1.0 ? 2.0-n : n);
	
		if (n>0.5) {
			n -= 0.5; n*= 2.0;
			col = interpolate_color(n, col, m_gglightColor[2] );
			return col;
		} else {
			n *= 2.0;
			col = interpolate_color(n, vector3d(.9, .89, .85), col );
			return col;
		}
		}
	case COLOR_GG_SATURN: {
		double n = 0.4*ridged_octavenoise(m_fracdef[0], 0.7, 3.142*p.y*p.y);
		n += 0.4*octavenoise(m_fracdef[1], 0.6, 3.142*p.y*p.y);
		n += 0.3*octavenoise(m_fracdef[2], 0.5, 3.142*p.y*p.y);
		n += 0.8*octavenoise(m_fracdef[0], 0.7, p*p.y*p.y);
		n += 0.5*ridged_octavenoise(m_fracdef[1], 0.7, p*p.y*p.y);
		n /= 2.0;
		n *= n*n;
		n += billow_octavenoise(m_fracdef[0], 0.8, noise(p*3.142)*p)*
			 megavolcano_function(m_fracdef[3], p);
		return interpolate_color(n, vector3d(.69, .53, .43), vector3d(.99, .76, .62));
		}
	case COLOR_GG_SATURN2: {
		double n = 0.2*billow_octavenoise(m_fracdef[0], 0.8, p*p.y*p.y);
		n += 0.5*ridged_octavenoise(m_fracdef[1], 0.7, p*p.y*p.y);
		n += 0.25*octavenoise(m_fracdef[2], 0.7, p*p.y*p.y);
		//spot
		n *= n*n*0.5;
		n += billow_octavenoise(m_fracdef[0], 0.8, noise(p*3.142)*p)*
			 megavolcano_function(m_fracdef[3], p);
		vector3d col;
		//col = interpolate_color(octavenoise(m_fracdef[2], 0.7, noise(p*3.142)*p), vector3d(.05, .0, .0), vector3d(.4,.0,.35));
		if (n > 1.0) {
			n -= 1.0;// n *= 5.0;
			col = interpolate_color(n, vector3d(.25, .3, .4), vector3d(.0, .2, .0) );
		} else if (n >0.8) {
			n -= 0.8; n *= 5.0;
			col = interpolate_color(n, vector3d(.0, .0, .15), vector3d(.25, .3, .4) );
			return col;
		} else if (n>0.6) {
			n -= 0.6; n*= 5.0;
			col = interpolate_color(n, vector3d(.0, .0, .1), vector3d(.0, .0, .15) );
			return col;
		} else if (n>0.4) {
			n -= 0.4; n*= 5.0;
			col = interpolate_color(n, vector3d(.05, .0, .05), vector3d(.0, .0, .1) );
			return col;
		} else if (n>0.2) {
			n -= 0.2; n*= 5.0;
			col = interpolate_color(n, vector3d(.0, .0, .1), vector3d(.05, .0, .05) );
			return col;
		} else {
			n *= 5.0;
			col = interpolate_color(n, vector3d(.0, .0, .0), vector3d(.0, .0, .1) );
			return col;
		}
		}
	case COLOR_GG_URANUS: {
		double n = 0.5*ridged_octavenoise(m_fracdef[0], 0.7, 3.142*p.y*p.y);
		n += 0.5*octavenoise(m_fracdef[1], 0.6, 3.142*p.y*p.y);
		n += 0.2*octavenoise(m_fracdef[2], 0.5, 3.142*p.y*p.y);
		n /= 2.0;
		n *= n*n;
		return interpolate_color(n, vector3d(.4, .5, .55), vector3d(.85,.95,.96));
		}
	case COLOR_GG_NEPTUNE: {
		double n = 0.8*octavenoise(m_fracdef[2], 0.6, 3.142*p.y*p.y);
		n += 0.25*ridged_octavenoise(m_fracdef[3], 0.55, 3.142*p.y*p.y);
		n += 0.2*octavenoise(m_fracdef[3], 0.5, 3.142*p.y*p.y);
		//spot
		n += 0.8*billow_octavenoise(m_fracdef[1], 0.8, noise(p*3.142)*p)*
			 megavolcano_function(m_fracdef[0], p);
		n /= 2.0;
		n *= n*n;
		return interpolate_color(n, vector3d(.04, .05, .15), vector3d(.80,.94,.96));
		}
	case COLOR_GG_NEPTUNE2: {
				double n;
		double h = billow_octavenoise(m_fracdef[0], 0.5*m_entropy[0] + 0.25f, noise(vector3d(p.x*8, p.y*32, p.z*8)))*.125;
		double equatorial_region_1 = billow_octavenoise(m_fracdef[0], 0.54, p) * p.y * p.x;
		double equatorial_region_2 = octavenoise(m_fracdef[1], 0.58, p) * p.x * p.x;
		vector3d col;
		col = interpolate_color(equatorial_region_1, vector3d(.01, .01, .1), m_ggdarkColor[0]);
		col = interpolate_color(equatorial_region_2, col, vector3d(0, 0, .2));
		//stripes
		if (p.y < 0.5 && p.y > -0.5) {
			for(float i=-1 ; i < 1; i+=0.6){
				double temp = p.y - i;
				if ( temp < .07+h && temp > -.07+h ){
					n = 2.0*billow_octavenoise(m_fracdef[2], 0.5*m_entropy[0], 
						noise(vector3d(p.x, p.y*m_planetEarthRadii*0.3, p.z))*p);
					n += 0.8*octavenoise(m_fracdef[1], 0.5*m_entropy[0],
						noise(vector3d(p.x, p.y*m_planetEarthRadii, p.z))*p);
					n += 0.5*billow_octavenoise(m_fracdef[3], 0.6, p);
					n *= n;
					n = (n<0.0 ? -n : n);
					n = (n>1.0 ? 2.0-n : n);
					if (n >0.8) {
						n -= 0.8; n *= 5.0;
						col = interpolate_color(n, col, m_ggdarkColor[2] );
						return col;
					} else if (n>0.6) {
						n -= 0.6; n*= 5.0;
						col = interpolate_color(n, vector3d(.03, .03, .15), col );
						return col;
					} else if (n>0.4) {
						n -= 0.4; n*= 5.0;
						col = interpolate_color(n, vector3d(.0, .0, .05), vector3d(.03, .03, .15) );
						return col;
					} else if (n>0.2) {
						n -= 0.2; n*= 5.0;
						col = interpolate_color(n, m_ggdarkColor[2], vector3d(.0, .0, .05) );
						return col;
					} else {
						n *= 5.0;
						col = interpolate_color(n, col, m_ggdarkColor[2] );
						return col;
					}
				}
			} 
		} 
		//if is not a stripe.
		n = octavenoise(m_fracdef[1], 0.5*m_entropy[0] + 
			0.25f,noise(vector3d(p.x*0.2, p.y*m_planetEarthRadii*10, p.z))*p);
		//n += 0.5;
		//n += octavenoise(m_fracdef[0], 0.6*m_entropy[0], 3.142*p.z*p.z);
		n *= n*n*n;
		n = (n<0.0 ? -n : n);
		n = (n>1.0 ? 2.0-n : n);
	
		if (n>0.5) {
			n -= 0.5; n*= 2.0;
			col = interpolate_color(n, col, m_ggdarkColor[2] );
			return col;
		} else {
			n *= 2.0;
			col = interpolate_color(n, vector3d(.0, .0, .0), col );
			return col;
		}
		}
	case COLOR_EARTHLIKE:
	{
		double n = m_invMaxHeight*height;
		double flatness = pow(p.Dot(norm), 8.0);
		//textures:
		double tex_rock, tex_sand, tex_grass, tex_forest;

		if (textures) {
			tex_rock   =   rock;
			tex_sand   =   sand;
			tex_grass  =  grass;
			tex_forest = forest;
		}
		//textures end
		double continents = 0;
		double equatorial_desert = (2.0-m_icyness)*(-1.0+2.0*octavenoise(12, 0.5, 2.0, (n*2.0)*p)) *
				1.0*(2.0-m_icyness)*(1.0-p.y*p.y);
		vector3d color_cliffs = m_darkrockColor[5];
		vector3d col, tex1, tex2;
		
		if (m_heightMap) {
			if (n > 0) {
				// ice on mountains
				if (flatness > 0.6/Clamp(n*m_icyness+(m_icyness*0.5)+(fabs(p.y*p.y*p.y*0.38)), 0.1, 1.0)) {
					if (textures) {
						col = interpolate_color(tex_rock, color_cliffs, m_rockColor[5]);
						col = interpolate_color(flatness, col, vector3d(1,1,1));
					} else col = interpolate_color(flatness, color_cliffs, vector3d(1,1,1));
					return col;
				}
				//polar ice-caps
				if ((m_icyness*0.5)+(fabs(p.y*p.y*p.y*0.38)) > 0.6) {
					//if (flatness > 0.5/Clamp(fabs(p.y*m_icyness), 0.1, 1.0)) {
					if (textures) {
						col = interpolate_color(tex_rock, color_cliffs, m_rockColor[5]);
						col = interpolate_color(flatness, col, vector3d(1,1,1));
					} else col = interpolate_color(flatness, color_cliffs, vector3d(1,1,1));
					return col;
				}
			}
		} else {
			// ice on mountains
			//printf("flatness : %d", flatness);
			if (flatness > 0.6/Clamp(n*m_icyness+(m_icyness*0.5)+(fabs(p.y*p.y*p.y*0.38)), 0.1, 1.0)) {
				if (textures) {
					col = interpolate_color(tex_rock, color_cliffs, m_rockColor[5]);
					col = interpolate_color(flatness, col, vector3d(1,1,1));
				} else col = interpolate_color(flatness, color_cliffs, vector3d(1,1,1));
				return col;
			}
			//polar ice-caps
			if ((m_icyness*0.5)+(fabs(p.y*p.y*p.y*0.38)) > 0.6) {
				//if (flatness > 0.5/Clamp(fabs(p.y*m_icyness), 0.1, 1.0)) {
				if (textures) {
					col = interpolate_color(tex_rock, color_cliffs, m_rockColor[5]);
					col = interpolate_color(flatness, col, vector3d(1,1,1));
				} else col = interpolate_color(flatness, color_cliffs, vector3d(1,1,1));
				return col;
			}
		}
		

		// This is for fake ocean depth by the coast.
			if (m_heightMap) {
				continents = 0;
			} else {
				continents = ridged_octavenoise(m_fracdef[3-m_fracnum], 0.55, p) * (1.0-m_sealevel) - ((m_sealevel*0.1)-0.1);
			}
		// water
		if (n <= 0) {
			if (m_heightMap) {	
				// waves
				if (textures) {
					n += water;
					n *= 0.1;
				}
			} else {
			// Oooh, pretty coastal regions with shading based on underwater depth.
				n += continents;// - (m_fracdef[3].amplitude*m_sealevel*0.49);
				n *= n*10.0;
				//n = (n>0.3 ? 0.3-(n*n*n-0.027) : n);
			}
			col = interpolate_color(equatorial_desert, vector3d(0,0,0.15), vector3d(0,0,0.25));
			col = interpolate_color(n, col, vector3d(0,0.8,0.6));
			return col;
		}
		flatness = pow(p.Dot(norm), 16.0);
		// More sensitive height detection for application of colours	
		if (n > 0.5) {
			n -= 0.5; n *= 2.0;
			color_cliffs = interpolate_color(n, m_darkrockColor[2], m_rockColor[4]);
			col = interpolate_color(equatorial_desert, m_rockColor[2], m_rockColor[4]);
			col = interpolate_color(n, col, m_darkrockColor[6]);
			if (textures) {
				tex1 = interpolate_color(tex_rock, col, color_cliffs);
				tex2 = interpolate_color(tex_sand, col, m_darkdirtColor[3]);
				col = interpolate_color(flatness, tex1, tex2);
			} else col = interpolate_color(flatness, color_cliffs, col);
			return col;
		}
		else if (n > 0.25) { 
			n -= 0.25; n *= 4.0;
			color_cliffs = interpolate_color(n, m_rockColor[3], m_darkplantColor[4]);
			col = interpolate_color(equatorial_desert, m_darkrockColor[3], m_darksandColor[1]);
			col = interpolate_color(n, col, m_rockColor[2]);
			if (textures) {
				tex1 = interpolate_color(tex_rock, col, color_cliffs);
				tex2 = interpolate_color(tex_sand, col, m_darkdirtColor[3]);
				col = interpolate_color(flatness, tex1, tex2);
			} else col = interpolate_color(flatness, color_cliffs, col);
			return col;
		}
		else if (n > 0.05) {  
			n -= 0.05; n *= 5.0;
			color_cliffs = interpolate_color(equatorial_desert, m_darkrockColor[5], m_darksandColor[7]);			
			col = interpolate_color(equatorial_desert, m_darkplantColor[2], m_sandColor[2]);
			col = interpolate_color(n, col, m_darkrockColor[3]);
			if (textures) {
				tex1 = interpolate_color(tex_rock, col, color_cliffs);
				tex2 = interpolate_color(tex_forest, col, color_cliffs);
				col = interpolate_color(flatness, tex1, tex2);
			} else col = interpolate_color(flatness, color_cliffs, col);
			return col;
		}
		else if (n > 0.01) {
			n -= 0.01; n *= 25.0;
			color_cliffs = m_darkdirtColor[7];
			col = interpolate_color(equatorial_desert, m_plantColor[1], m_plantColor[0]);
			col = interpolate_color(n, col, m_darkplantColor[2]);
			if (textures) {
				tex1 = interpolate_color(tex_rock, col, color_cliffs);
				tex2 = interpolate_color(tex_grass, color_cliffs, col);
				col = interpolate_color(flatness, tex1, tex2);
			} else col = interpolate_color(flatness, color_cliffs, col);
			return col;
		}
		else if (n > 0.005) {   
			n -= 0.005; n *= 200.0;
			color_cliffs = m_dirtColor[2];
			col = interpolate_color(equatorial_desert, m_darkplantColor[0], m_sandColor[1]);
			col = interpolate_color(n, col, m_plantColor[0]);
			if (textures) {
				tex1 = interpolate_color(tex_rock, col, color_cliffs);
				tex2 = interpolate_color(tex_grass, color_cliffs, col);
				col = interpolate_color(flatness, tex1, tex2);
			} else col = interpolate_color(flatness, color_cliffs, col);
			return col;
		}
		else { 
			n *= 200.0;
			color_cliffs = m_darksandColor[0];
			col = interpolate_color(equatorial_desert, m_sandColor[0], m_sandColor[1]);
			col = interpolate_color(n, col, m_darkplantColor[0]);
			if (textures) {
				tex1 = interpolate_color(tex_rock, col, color_cliffs);
				tex2 = interpolate_color(tex_sand, col, color_cliffs);
				return col = interpolate_color(flatness, tex1, tex2);
			} else { 
				return col = interpolate_color(flatness, color_cliffs, col);
			}
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

		const double flatness = pow(p.Dot(norm), 24.0);
		double equatorial_desert = (2.0-m_icyness)*(-1.0+2.0*octavenoise(12, 0.5, 2.0, (n*2.0)*p)) *
				1.0*(2.0-m_icyness)*(1.0-p.y*p.y);
		double equatorial_region_1 = billow_octavenoise(m_fracdef[0], 0.5, p) * p.y * p.y;
		double equatorial_region_2 = ridged_octavenoise(m_fracdef[5], 0.5, p) * p.x * p.x;
		// cliff colours
		vector3d color_cliffs;
		// adds some variation
		color_cliffs = interpolate_color(equatorial_region_1, m_rockColor[3],  m_rockColor[0] );
		color_cliffs = interpolate_color(equatorial_region_2, color_cliffs,  m_rockColor[2] );
		// main colours
		vector3d col;
		// start by interpolating between noise values for variation
		col = interpolate_color(equatorial_region_1, m_darkrockColor[0], vector3d(1, 1, 1) );
		col = interpolate_color(equatorial_region_2, m_darkrockColor[1], col );
		col = interpolate_color(equatorial_desert, col, vector3d(.96, .95, .94));
		// scale by different colours depending on height for more variation
		if (n > .666) {  
			n -= 0.666; n*= 3.0;
			col = interpolate_color(n, vector3d(.96, .95, .94), col);
			col = interpolate_color(flatness, color_cliffs, col);
			return col;
		}
		else if (n > 0.333) {
			n -= 0.333; n*= 3.0;
			col = interpolate_color(n, col, vector3d(.96, .95, .94));
			col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else {   
			n *= 3.0;
			col = interpolate_color(n, vector3d(.96, .95, .94), col);
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
		double equatorial_region = octavenoise(m_fracdef[0], 0.54, p) * p.y * p.x;
		double equatorial_region_2 = ridged_octavenoise(m_fracdef[1], 0.58, p) * p.x * p.x;
		// Below is to do with variable colours for different heights, it gives a nice effect.
		// n is height.
		vector3d col;
		col = interpolate_color(equatorial_desert, m_rockColor[2], m_rockColor[3]);
		col = interpolate_color(equatorial_region, col, m_darkrockColor[4]);
		col = interpolate_color(equatorial_region_2, m_rockColor[1], col);
		if (n > 0.9) {
			n -= 0.9; n *= 10.0;
			col = interpolate_color(n, m_rockColor[6], col );
			col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.8) {
			n -= 0.8; n *= 10.0;
			col = interpolate_color(n, col, m_rockColor[5]);
			col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.7) {
			n -= 0.7; n *= 10.0;
			col = interpolate_color(n, m_rockColor[4], col);
			col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.6) {
			n -= 0.6; n *= 10.0;
			col = interpolate_color(n, m_rockColor[0], m_rockColor[4]);
			col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.5) {
			n -= 0.5; n *= 10.0;
			col = interpolate_color(n, col, m_rockColor[0]);
			col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.4) {
			n -= 0.4; n *= 10.0;
			col = interpolate_color(n, m_darkrockColor[3], col);
			col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		if (n > 0.3) {
			n -= 0.3; n *= 10.0;
			col = interpolate_color(n, col, m_darkrockColor[3]);
			col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.2) {
			n -= 0.2; n *= 10.0;
			col = interpolate_color(n, m_darkrockColor[1], col);
			col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.1) {
			n -= 0.1; n *= 10.0;
			col = interpolate_color(n, col, m_darkrockColor[1]);
			col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else {
			n *= 10.0;
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
		const double flatness = pow(p.Dot(norm), 8.0);
		vector3d color_cliffs = m_rockColor[5];
		// ice on mountains and poles
			if (fabs(m_icyness*p.y) + m_icyness*n > 1) {
				return interpolate_color(flatness, color_cliffs, vector3d(1,1,1));
			}

		double equatorial_desert = (2.0-m_icyness)*(-1.0+2.0*octavenoise(12, 0.5, 2.0, (n*2.0)*p)) *
				1.0*(2.0-m_icyness)*(1.0-p.y*p.y);
		// This is for fake ocean depth by the coast.
		double continents = octavenoise(m_fracdef[0], 0.7*
					ridged_octavenoise(m_fracdef[8], 0.58, p), p) - m_sealevel*0.6;

		vector3d col;
		//we don't want water on the poles if there are ice-caps
		if (fabs(m_icyness*p.y) > 0.75) {
			col = interpolate_color(equatorial_desert, vector3d(0.42, 0.46, 0), vector3d(0.5, 0.3, 0));
			col = interpolate_color(flatness, col, vector3d(1,1,1));
			return col;
		}
		// water
		if (n <= 0) {
				// Oooh, pretty coastal regions with shading based on underwater depth.
			n += continents - (m_fracdef[0].amplitude*m_sealevel*0.49);
			n *= 10.0;
			n = (n>0.3 ? 0.3-(n*n*n-0.027) : n);
			col = interpolate_color(equatorial_desert, vector3d(0,0,0.15), vector3d(0,0,0.25));
			col = interpolate_color(n, col, vector3d(0,0.8,0.6));
			return col;
		}

		// More sensitive height detection for application of colours
		
		if (n > 0.5) {
		col = interpolate_color(equatorial_desert, m_rockColor[2], m_rockColor[4]);
		col = interpolate_color(n, col, m_darkrockColor[6]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.25) { 
		color_cliffs = m_darkrockColor[1];
		col = interpolate_color(equatorial_desert, m_darkrockColor[5], m_darkrockColor[7]);
		col = interpolate_color(n, col, m_rockColor[1]);
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.05) {  
		col = interpolate_color(equatorial_desert, m_darkrockColor[5], m_darkrockColor[7]);
		color_cliffs = col;
		col = interpolate_color(equatorial_desert, vector3d(.45,.43, .2), vector3d(.4, .43, .2));
		col = interpolate_color(n, col, vector3d(-1.66,-2.3, -1.75));
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.01) { 
		color_cliffs = vector3d(0.2,0.28,0.2);
		col = interpolate_color(equatorial_desert, vector3d(.15,.5, -.1), vector3d(.2, .6, -.1));
		col = interpolate_color(n, col, vector3d(5,-5, 5));
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else if (n > 0.005) {   
		color_cliffs = vector3d(0.25,0.28,0.2);
		col = interpolate_color(equatorial_desert, vector3d(.45,.6,0), vector3d(.5, .6, .0));
		col = interpolate_color(n, col, vector3d(-10,-10,0));
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
		else { 
		color_cliffs = vector3d(0.3,0.1,0.0);
		col = interpolate_color(equatorial_desert, vector3d(.35,.3,0), vector3d(.4, .3, .0));
		col = interpolate_color(n, col, vector3d(0,20,0));
		col = interpolate_color(flatness, color_cliffs, col);
		return col;
		}
	}
	case COLOR_TFPOOR:
	{
		double n = m_invMaxHeight*height;
		double flatness = pow(p.Dot(norm), 8.0);
		//textures:
		double tex_rock, tex_rock2, tex_mud, tex_sand, tex_sand2, tex_grass, tex_grass2;
		if (textures) {
			tex_rock   = rock;
			tex_rock2  = rock2;
			tex_mud    = mud;
			tex_sand   = sand;
			tex_sand2  = sand2;
			tex_grass  = grass;
			tex_grass2 = grass2;
		}
		//textures end
		double continents = 0;
		double equatorial_desert = (2.0-m_icyness)*(-1.0+2.0*octavenoise(12, 0.5, 2.0, (n*2.0)*p)) *
				1.0*(2.0-m_icyness)*(1.0-p.y*p.y);
		vector3d color_cliffs = m_darkrockColor[5];
		vector3d col, tex1, tex2;
		// ice on mountains and poles
		if (fabs(m_icyness*p.y) + m_icyness*n > 1) {
			if (textures) {
				col = interpolate_color(tex_rock2, color_cliffs, vector3d(.9,.9,.9));
				col = interpolate_color(flatness, col, vector3d(1,1,1));
			} else col = interpolate_color(flatness, color_cliffs, vector3d(1,1,1));
			return col;
		}
		//we don't want water on the poles if there are ice-caps
		if (fabs(m_icyness*p.y) > 0.67) {
			col = interpolate_color(equatorial_desert, m_sandColor[2], m_darksandColor[5]);
			col = interpolate_color(flatness, col, vector3d(1,1,1));
			return col;
		}
		// This is for fake ocean depth by the coast.
			if (m_heightMap) {
				continents = 0;
			} else {
				continents = ridged_octavenoise(m_fracdef[3-m_fracnum], 0.55, p) * (1.0-m_sealevel) - ((m_sealevel*0.1)-0.1);
			}
		// water
		if (n <= 0) {
			if (m_heightMap) {	
				// waves
				if (textures) {
					n += water;
					n *= 0.1;
				}
			} else {
			// Oooh, pretty coastal regions with shading based on underwater depth.
				n += continents;// - (m_fracdef[3].amplitude*m_sealevel*0.49);
				n *= n*10.0;
				//n = (n>0.3 ? 0.3-(n*n*n-0.027) : n);
			}
			col = interpolate_color(n, vector3d(0,0.0,0.1), vector3d(0,0.5,0.5));
			return col;
		}
		// More sensitive height detection for application of colours	
		if (n > 0.5) {
			n -= 0.5; n *= 2.0;
			//color_cliffs = m_rockColor[1];
			col = interpolate_color(equatorial_desert, m_rockColor[2], m_rockColor[6]);
			col = interpolate_color(n, col, m_darkrockColor[6]);
			if (textures) {
				tex1 = interpolate_color(tex_rock, col, color_cliffs);
				tex2 = interpolate_color(tex_rock2, col, color_cliffs);
				col = interpolate_color(flatness, tex1, tex2);
			} else col = interpolate_color(flatness, color_cliffs, col);
			return col;
		}
		else if (n > 0.25) { 
			n -= 0.25; n *= 4.0;
			color_cliffs = m_rockColor[3];
			col = interpolate_color(equatorial_desert, m_darkrockColor[4], m_darksandColor[6]);
			col = interpolate_color(n, col, m_rockColor[2]);
			if (textures) {
				tex1 = interpolate_color(tex_rock, col, color_cliffs);
				tex2 = interpolate_color(tex_mud, col, color_cliffs);
				col = interpolate_color(flatness, tex1, tex2);
			} else col = interpolate_color(flatness, color_cliffs, col);
			return col;
		}
		else if (n > 0.05) {  
			n -= 0.05; n *= 5.0;
			col = interpolate_color(equatorial_desert, m_darkrockColor[5], m_darksandColor[7]);
			color_cliffs = col;
			col = interpolate_color(equatorial_desert, m_darksandColor[2], m_sandColor[2]);
			col = interpolate_color(n, col, m_darkrockColor[3]);
			if (textures) {
				tex1 = interpolate_color(tex_mud, col, color_cliffs);
				tex2 = interpolate_color(tex_grass, col, color_cliffs);
				col = interpolate_color(flatness, tex1, tex2);
			} else col = interpolate_color(flatness, color_cliffs, col);
			return col;
		}
		else if (n > 0.01) {
			n -= 0.01; n *= 25.0;
			color_cliffs = m_darkplantColor[0];
			col = interpolate_color(equatorial_desert, m_sandColor[1], m_sandColor[0]);
			col = interpolate_color(n, col, m_darksandColor[2]);
			if (textures) {
				tex1 = interpolate_color(tex_grass, col, color_cliffs);
				tex2 = interpolate_color(tex_grass2, col, color_cliffs);
				col = interpolate_color(flatness, tex1, tex2);
			} else col = interpolate_color(flatness, color_cliffs, col);
			return col;
		}
		else if (n > 0.005) {   
			n -= 0.005; n *= 200.0;
			color_cliffs = m_plantColor[0];
			col = interpolate_color(equatorial_desert, m_darkplantColor[0], m_sandColor[1]);
			col = interpolate_color(n, col, m_plantColor[0]);
			if (textures) {
				tex1 = interpolate_color(tex_sand2, col, color_cliffs);
				tex2 = interpolate_color(tex_grass, col, color_cliffs);
				col = interpolate_color(flatness, tex1, tex2);
			} else col = interpolate_color(flatness, color_cliffs, col);
			return col;
		}
		else { 
			n *= 200.0;
			color_cliffs = m_darksandColor[0];
			col = interpolate_color(equatorial_desert, m_sandColor[0], m_sandColor[1]);
			col = interpolate_color(n, col, m_darkplantColor[0]);
			if (textures) {
				tex1 = interpolate_color(tex_sand, col, color_cliffs);
				//tex2 = interpolate_color(sand2, col, color_cliffs);
				col = interpolate_color(flatness, tex1, col);
			} else col = interpolate_color(flatness, color_cliffs, col);
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
    return vector3d(1.0);
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
	n = 1.0 - fabs(n);
	n *= n;
	return n;
}

static inline double billow_octavenoise(int octaves, double roughness, double lacunarity, const vector3d &p)
{
	double n = 0;
	double octaveAmplitude = roughness;
	double jizm = 1.0;
	while (octaves--) {
		n += octaveAmplitude * noise(jizm*p);
		octaveAmplitude *= roughness;
		jizm *= lacunarity;
	}
	return (2.0 * fabs(n) - 1.0)+1.0;
}

static inline double voronoiscam_octavenoise(int octaves, double roughness, double lacunarity, const vector3d &p)
{
	double n = 0;
	double octaveAmplitude = roughness;
	double jizm = 1.0;
	while (octaves--) {
		n += octaveAmplitude * noise(jizm*p);
		octaveAmplitude *= roughness;
		jizm *= lacunarity;
	}
	return sqrt(10.0 * fabs(n));
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
	n = 1.0 - fabs(n);
	n *= n;
	return n;
	//return 1.0 - fabs(n);
}

static inline double billow_octavenoise(fracdef_t &def, double roughness, const vector3d &p)
{
	double n = 0;
	double octaveAmplitude = roughness;
	double jizm = def.frequency;
	for (int i=0; i<def.octaves; i++) {
		n += octaveAmplitude * noise(jizm*p);
		octaveAmplitude *= roughness;
		jizm *= def.lacunarity;
	}
	return (2.0 * fabs(n) - 1.0)+1.0;
}

static inline double voronoiscam_octavenoise(fracdef_t &def, double roughness, const vector3d &p)
{
	double n = 0;
	double octaveAmplitude = roughness;
	double jizm = def.frequency;
	for (int i=0; i<def.octaves; i++) {
		n += octaveAmplitude * noise(jizm*p);
		octaveAmplitude *= roughness;
		jizm *= def.lacunarity;
	}
	return sqrt(10.0 * fabs(n));
}


static inline double dunes_octavenoise(fracdef_t &def, double roughness, const vector3d &p)
{
	double n = 0;
	double octaveAmplitude = roughness;
	double jizm = def.frequency;
	for (int i=0; i<3; i++) {
		n += octaveAmplitude * noise(jizm*p);
		octaveAmplitude *= roughness;
		jizm *= def.lacunarity;
	}
	return 1.0 - fabs(n);
}

// Creates small canyons.
static double canyon_ridged_function(const fracdef_t &def, const vector3d &p)
{
	double h;
	double n = 0;
	n = ridged_octavenoise(def.octaves, 0.54, 2.0, def.frequency*p);
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
double canyon2_ridged_function(const fracdef_t &def, const vector3d &p)
{
	double h;
	double n = 0; //octavenoise(def.octaves, 0.56, 2.0, def.frequency*p);
	n = ridged_octavenoise(def.octaves, 0.56, 2.0, def.frequency*p);
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
double canyon3_ridged_function(const fracdef_t &def, const vector3d &p)
{
	double h;
	double n = 0; //octavenoise(def.octaves, 0.585, 2.0, def.frequency*p);
	n = ridged_octavenoise(def.octaves, 0.585, 2.0, def.frequency*p);
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

static double canyon_normal_function(const fracdef_t &def, const vector3d &p)
{
	double h;
	double n = 0;
	n = octavenoise(def.octaves, 0.54, 2.0, def.frequency*p);
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

double canyon2_normal_function(const fracdef_t &def, const vector3d &p)
{
	double h;
	double n = 0;
	n = octavenoise(def.octaves, 0.56, 2.0, def.frequency*p);
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

double canyon3_normal_function(const fracdef_t &def, const vector3d &p)
{
	double h;
	double n = 0;
	n = octavenoise(def.octaves, 0.585, 2.0, def.frequency*p);
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

static double canyon_voronoi_function(const fracdef_t &def, const vector3d &p)
{
	double h;
	double n = 0;
	n = octavenoise(def.octaves, 0.54, 2.0, def.frequency*p);
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

double canyon2_voronoi_function(const fracdef_t &def, const vector3d &p)
{
	double h;
	double n = 0;
	n = octavenoise(def.octaves, 0.56, 2.0, def.frequency*p);
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

double canyon3_voronoi_function(const fracdef_t &def, const vector3d &p)
{
	double h;
	double n = 0;
	n = octavenoise(def.octaves, 0.585, 2.0, def.frequency*p);
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

static double canyon_billow_function(const fracdef_t &def, const vector3d &p)
{
	double h;
	double n = 0;
	n = octavenoise(def.octaves, 0.54, 2.0, def.frequency*p);
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

double canyon2_billow_function(const fracdef_t &def, const vector3d &p)
{
	double h;
	double n = 0;
	n = octavenoise(def.octaves, 0.56, 2.0, def.frequency*p);
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

double canyon3_billow_function(const fracdef_t &def, const vector3d &p)
{
	double h;
	double n = 0;
	n = octavenoise(def.octaves, 0.585, 2.0, def.frequency*p);
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

static void impact_crater_function_1pass(const vector3d &p, double &out, const double height)
{
	double n = fabs(noise(p));
	const double ejecta_outer = 0.6;
	const double outer = 0.9; 
	const double midrim = 0.93;
	double hrim;
	double descent;
	if (n > midrim) {
		out -= height;
	} else if (n > outer) {
		hrim = midrim - outer;
		descent = (n-outer)/hrim;
		out -= height * descent * descent;
	} else if (n > ejecta_outer) {
		// blow down walls of other craters too near this one,
		// so we don't have sharp transition
		//out *= (outer-n)/-(ejecta_outer-outer);
	}
}

// makes large and small craters across the entire planet.
static double impact_crater_function(const fracdef_t &def, const vector3d &p) 
{
	double crater = 0.0;
	double sz = def.frequency;
	double max_h = def.amplitude;
	for (int i=0; i<def.octaves; i++) {
		impact_crater_function_1pass(sz*p, crater, max_h);
		sz *= 2.0;
		max_h *= 0.5;
	}
	return crater;
}

static void volcano_function_1pass(const vector3d &p, double &out, const double height)
{
	double n = fabs(noise(p));
	const double ejecta_outer = 0.6;
	const double outer = 0.9; 
	const double inner = 0.975;
	const double midrim = 0.971;
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
	const double inner = 0.98;
	const double midrim = 0.964;
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

double river_function(const fracdef_t &def, const vector3d &p, int style)
{
	assert(style >= 0 && style < 2);
	double h;
	double n = octavenoise(def.octaves, 0.585, 2.0, def.frequency*p*0.5);
	const double outer[] = {0.67, 0.01};
	const double inner[] = {0.715, 0.49};
	const double inner2[] = {0.715, 0.51};
	const double outer2[] = {0.76, 0.99};
	if (n > outer2[style]) {
		h = 1;
	} else if (n > inner2[style]) {
		h = 0.0+1.0*(n-inner2[style])*(1.0/(outer2[style]-inner2[style]));
	} else if (n > inner[style]) {
		h = 0;
	} else if (n > outer[style]) {
		h = 1.0-1.0*(n-outer[style])*(1.0/(inner[style]-outer[style]));
	} else {
		h = 1.0;
	}
	return h * def.amplitude;
}

// Original canyon function, But really it generates cliffs.
#if 0
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
#endif
