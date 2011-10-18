#include "Terrain.h"
#include "perlin.h"
#include "Pi.h"


int Terrain::GetRawHeightMapVal(int x, int y)
{
	return m_heightMap[Clamp(y, 0, m_heightMapSizeY-1)*m_heightMapSizeX + Clamp(x, 0, m_heightMapSizeX-1)];
}

/*
 * Bicubic interpolation!!!
 */


double Terrain::GetHeightMapVal(const vector3d &pt)
{     // This is all used for Earth and Earth alone
#if 0
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
	int ix = int(floor(px));
	int iy = int(floor(py));
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
			SetFracDef(0, m_maxHeightInMeters, 10, rand, 10*m_fracmult);
			SetFracDef(1, m_maxHeightInMeters, 25, rand, 10*m_fracmult);
		}
		//small fractal/high detail
		SetFracDef(2-m_fracnum, m_maxHeightInMeters*0.05, 50, rand, 10*m_fracmult);//[2]
		//continental/large type fractal
		SetFracDef(3-m_fracnum, m_maxHeightInMeters, 1e6, rand, 200*m_fracmult);//[0]
		SetFracDef(4-m_fracnum, m_maxHeightInMeters, 1e5, rand, 100*m_fracmult);//[4]
		//medium fractal
		SetFracDef(5-m_fracnum, m_maxHeightInMeters, 2e4, rand, 500*m_fracmult);//[5]
		SetFracDef(6-m_fracnum, m_maxHeightInMeters*0.2, 5e3.0, rand, 100*m_fracmult);//[3]
		*/
		//Here's where we add some noise over the heightmap so it doesnt look so boring, we scale by height so values are greater high up
		//large mountainous shapes
		double mountains = h*h*0.001*octavenoise(GetFracDef(3-m_fracnum), 0.5*octavenoise(GetFracDef(5-m_fracnum), 0.45, pt),
			pt)*ridged_octavenoise(GetFracDef(4-m_fracnum), 0.475*octavenoise(GetFracDef(6-m_fracnum), 0.4, pt), pt);
		v += mountains;
		//smaller ridged mountains
		if (v < 50.0){
			v += v*v*0.04*ridged_octavenoise(GetFracDef(5-m_fracnum), 0.5, pt);
		} else if (v <100.0){
			v += 100.0*ridged_octavenoise(GetFracDef(5-m_fracnum), 0.5, pt);
		} else {
			v += (100.0/v)*(100.0/v)*(100.0/v)*(100.0/v)*(100.0/v)*
				100.0*ridged_octavenoise(GetFracDef(5-m_fracnum), 0.5, pt);
		}
		//high altitude detail/mountains
		//v += Clamp(h, 0.0, 0.5)*octavenoise(GetFracDef(2-m_fracnum), 0.5, pt);	
		
		//low altitude detail/dunes
		//v += h*0.000003*ridged_octavenoise(GetFracDef(2-m_fracnum), Clamp(1.0-h*0.002, 0.0, 0.5), pt);			
		if (v < 10.0){
			v += 2.0*v*dunes_octavenoise(GetFracDef(6-m_fracnum), 0.5, pt)
				*octavenoise(GetFracDef(6-m_fracnum), 0.5, pt);
		} else if (v <50.0){
			v += 20.0*dunes_octavenoise(GetFracDef(6-m_fracnum), 0.5, pt)
				*octavenoise(GetFracDef(6-m_fracnum), 0.5, pt);
		} else {
			v += (50.0/v)*(50.0/v)*(50.0/v)*(50.0/v)*(50.0/v)
				*20.0*dunes_octavenoise(GetFracDef(6-m_fracnum), 0.5, pt)
				*octavenoise(GetFracDef(6-m_fracnum), 0.5, pt);
		}		
		if (v<40.0) {
			//v = v;
		} else if (v <60.0){
			v += (v-40.0)*billow_octavenoise(GetFracDef(5-m_fracnum), 0.5, pt);
			//printf("V/height: %f\n", Clamp(v-20.0, 0.0, 1.0));
		} else {
			v += (30.0/v)*(30.0/v)*(30.0/v)*20.0*billow_octavenoise(GetFracDef(5-m_fracnum), 0.5, pt);
		}		
		
		//ridges and bumps
		//v += h*0.1*ridged_octavenoise(GetFracDef(6-m_fracnum), Clamp(h*0.0002, 0.3, 0.5), pt) 
		//	* Clamp(h*0.0002, 0.1, 0.5);
		v += h*0.2*voronoiscam_octavenoise(GetFracDef(5-m_fracnum), Clamp(1.0-(h*0.0002), 0.0, 0.6), pt) 
			* Clamp(1.0-(h*0.0006), 0.0, 1.0);
		//polar ice caps with cracks
		if ((m_icyness*0.5)+(fabs(pt.y*pt.y*pt.y*0.38)) > 0.6) {
			h = Clamp(1.0-(v*10.0), 0.0, 1.0)*voronoiscam_octavenoise(GetFracDef(5-m_fracnum), 0.5, pt);
			h *= h*h*2.0;
			h -= 3.0;
			v += h;
		}
		return (v<0 ? 0 : v);
	}
#endif
}

void Terrain::ChangeDetailLevel()
{
#if 0
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
#endif
}

void Terrain::PickTerrain(MTRand &rand)
{
#if 0
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
#endif
}

void Terrain::PickAtmosphere()
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

void Terrain::InitHeightMap()
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

Terrain *Terrain::InstanceTerrain(const SBody *body)
{
	Terrain *t = new TerrainGenerator<TerrainHeightFlat,TerrainColorSolid>();

	t->m_body = body;
	t->m_seed = t->m_body->seed;
}

#if 0
Terrain::Terrain(const SBody *body) : m_body(body)
{
	m_seed = m_body->seed;

	PickAtmosphere();
	InitHeightMap();

	ChangeDetailLevel();
}
#endif

/**
 * Feature width means roughly one perlin noise blob or grain.
 * This will end up being one hill, mountain or continent, roughly.
 */
void Terrain::SetFracDef(unsigned int index, double featureHeightMeters, double featureWidthMeters, MTRand &rand, double smallestOctaveMeters)
{
	// feature 
	m_fracdef[index].amplitude = featureHeightMeters / (m_maxHeight * m_planetRadius);
	m_fracdef[index].frequency = m_planetRadius / featureWidthMeters;
	m_fracdef[index].octaves = std::max(1, int(ceil(log(featureWidthMeters / smallestOctaveMeters) / log(2.0))));
	m_fracdef[index].lacunarity = 2.0;
	//printf("%d octaves\n", m_fracdef[index].octaves); //print
}

// Fracdef is used to define the fractals width/area, height and detail
void Terrain::InitFractalType(MTRand &rand)
{
#if 0
	//Earth uses these fracdef settings
	if (m_heightMap) {	
		//textures
		if (textures) {
			SetFracDef(0, m_maxHeightInMeters, 10, rand, 10*m_fracmult);
			SetFracDef(1, m_maxHeightInMeters, 25, rand, 10*m_fracmult);
		}
		//small fractal/high detail
		SetFracDef(2-m_fracnum, m_maxHeightInMeters*0.0000005, 50, rand, 20*m_fracmult);//[2]
		//continental/large type fractal
		SetFracDef(3-m_fracnum, m_maxHeightInMeters*0.00005, 1e6, rand, 800*m_fracmult);//[0]
		SetFracDef(4-m_fracnum, m_maxHeightInMeters*0.00005, 1e5, rand, 400*m_fracmult);//[4]
		//medium fractal
		SetFracDef(5-m_fracnum, m_maxHeightInMeters*0.000005, 2e4, rand, 200*m_fracmult);//[5]
		SetFracDef(6-m_fracnum, m_maxHeightInMeters*0.0000005, 5e3, rand, 100*m_fracmult);//[3]
		return;
	}

	switch (m_terrainType) {
		case TERRAIN_ASTEROID:
		{
			//m_maxHeight = rand.Double(0.2,0.4);
			//m_invMaxHeight = 1.0 / m_maxHeight;
			SetFracDef(0, m_maxHeightInMeters, m_planetRadius, rand);
			// craters
			SetFracDef(1, 5000.0, 1000000.0, rand, 1000.0*m_fracmult);
			break;
		}
		case TERRAIN_HILLS_NORMAL: //1
		{
			//textures
			if (textures) {
				SetFracDef(0, m_maxHeightInMeters, rand.Double(5, 15), rand, 10*m_fracmult);
				SetFracDef(1, m_maxHeightInMeters, rand.Double(20, 40), rand, 10*m_fracmult);
			}
			//small fractal/high detail
			SetFracDef(2-m_fracnum, m_maxHeightInMeters*0.000000005, 500, rand, 20*m_fracmult);
			//continental:
			SetFracDef(3-m_fracnum, m_maxHeightInMeters*0.00001, 1e7, rand, 1000*m_fracmult);
			//large fractal:
			SetFracDef(4-m_fracnum, m_maxHeightInMeters, 1e5, rand, 200*m_fracmult);
			//medium fractal:
			SetFracDef(5-m_fracnum, m_maxHeightInMeters*0.00005, 2e4, rand, 200*m_fracmult);
			SetFracDef(6-m_fracnum, m_maxHeightInMeters*0.000000005, 1000, rand, 20*m_fracmult);
			break;
		}
		case TERRAIN_HILLS_DUNES: //2
		{
			//textures
			SetFracDef(0, m_maxHeightInMeters, rand.Double(50, 100), rand, 10*m_fracmult);
			SetFracDef(1, m_maxHeightInMeters, rand.Double(300, 500), rand, 10*m_fracmult);
			//small fractal/high detail
			SetFracDef(2, m_maxHeightInMeters*0.00000000001, 50, rand, 50*m_fracmult);
			//continental:
			SetFracDef(3, m_maxHeightInMeters*0.00001, 1e7, rand, 1000*m_fracmult); 
			//large fractal:
			SetFracDef(4, m_maxHeightInMeters*0.00001, 1e5, rand, 200*m_fracmult); 
			SetFracDef(5, m_maxHeightInMeters*0.000001, 5e4, rand, 100*m_fracmult); 
			SetFracDef(6, m_maxHeightInMeters*0.0000001, 1e4, rand, 50*m_fracmult); 
			//medium fractal:
			SetFracDef(7, m_maxHeightInMeters*0.0000000002, 1e3, rand, 20*m_fracmult); 
			break;
		}
		case TERRAIN_HILLS_RIDGED: //3
		{
			//textures:
			SetFracDef(0, m_maxHeightInMeters, rand.Double(5, 15), rand, 10*m_fracmult);
			SetFracDef(1, m_maxHeightInMeters, rand.Double(20, 40), rand, 10*m_fracmult);
			//small fractal/high detail:
			SetFracDef(2, m_maxHeightInMeters*0.000000005, rand.Double(40, 80), rand, 10*m_fracmult);
			//continental:
			SetFracDef(3, m_maxHeightInMeters*0.00001, rand.Double(1e6, 2e7), rand, 1000*m_fracmult);
			//large fractal:
			SetFracDef(4, m_maxHeightInMeters, rand.Double(1e5, 5e6), rand, 200*m_fracmult);
			//medium fractal:
			SetFracDef(5, m_maxHeightInMeters*0.00005, rand.Double(1e3, 5e4), rand, 100*m_fracmult);
			SetFracDef(6, m_maxHeightInMeters*0.00000002, rand.Double(250, 1e3), rand, 50*m_fracmult);
			break;
		}
		case TERRAIN_HILLS_RIVERS: //4
		{
			//textures
			SetFracDef(0, m_maxHeightInMeters, rand.Double(5, 15), rand, 10*m_fracmult);
			SetFracDef(1, m_maxHeightInMeters, rand.Double(20, 40), rand, 10*m_fracmult);
			//small fractal/high detail
			SetFracDef(2, m_maxHeightInMeters*0.000000008, rand.Double(5, 70), rand, 10*m_fracmult);
			//continental:
			SetFracDef(3, m_maxHeightInMeters, rand.Double(1e6, 2e7), rand, 10000*m_fracmult); 
			//large fractal:
			SetFracDef(4, m_maxHeightInMeters*0.00001, 1e5, rand, 1000*m_fracmult); 
			SetFracDef(5, m_maxHeightInMeters*0.000001, rand.Double(1e5, 1e6), rand, 100*m_fracmult); 
			//medium fractal:
			SetFracDef(6, m_maxHeightInMeters*0.0000002, rand.Double(500, 2e4), rand, 50*m_fracmult); 
			break;
		}
		case TERRAIN_HILLS_CRATERS: //5
		{
			SetFracDef(0, m_maxHeightInMeters, rand.Double(1e6,1e7), rand);
			double height = m_maxHeightInMeters*0.3;
			SetFracDef(1, height, rand.Double(4.0, 20.0)*height, rand);
			SetFracDef(2, m_maxHeightInMeters, rand.Double(50.0, 100.0)*m_maxHeightInMeters, rand);
			SetFracDef(3, m_maxHeightInMeters*0.07, 1e6, rand, 100.0*m_fracmult);
			SetFracDef(4, m_maxHeightInMeters*0.05, 8e5, rand, 100.0*m_fracmult);
			break;
		}
		case TERRAIN_HILLS_CRATERS2: //6
		{
			SetFracDef(0, m_maxHeightInMeters, rand.Double(1e6,1e7), rand);
			double height = m_maxHeightInMeters*0.6;
			SetFracDef(1, height, rand.Double(4.0, 20.0)*height, rand);
			SetFracDef(2, m_maxHeightInMeters, rand.Double(50.0, 100.0)*m_maxHeightInMeters, rand);
			SetFracDef(3, m_maxHeightInMeters*0.07, 11e5, rand, 1000.0*m_fracmult);
			SetFracDef(4, m_maxHeightInMeters*0.05, 98e4, rand, 800.0*m_fracmult);
			SetFracDef(5, m_maxHeightInMeters*0.05, 1e6, rand, 400.0*m_fracmult);
			SetFracDef(6, m_maxHeightInMeters*0.04, 99e4, rand, 200.0*m_fracmult);
			SetFracDef(7, m_maxHeightInMeters*0.05, 12e5, rand, 100.0*m_fracmult);
			SetFracDef(8, m_maxHeightInMeters*0.04, 9e5, rand, 100.0*m_fracmult);
			break;
		}
		case TERRAIN_MOUNTAINS_NORMAL: //7
		{
			SetFracDef(0, m_maxHeightInMeters, rand.Double(1e6, 1e7), rand, 10000*m_fracmult);
			SetFracDef(1, m_maxHeightInMeters*0.00000000001, 100.0, rand, 10*m_fracmult);
			SetFracDef(2, m_maxHeightInMeters*0.0000001, rand.Double(500, 2e3), rand, 1000*m_fracmult);
			SetFracDef(3, m_maxHeightInMeters*0.00002, rand.Double(1500, 1e4), rand, 100*m_fracmult);
			SetFracDef(4, m_maxHeightInMeters*0.08, 1e4, rand, 100*m_fracmult);
			SetFracDef(5, m_maxHeightInMeters*0.2, 1e5, rand, 100*m_fracmult);
			SetFracDef(6, m_maxHeightInMeters*0.5, 1e6, rand, 1000*m_fracmult);
			SetFracDef(7, m_maxHeightInMeters*0.5, rand.Double(1e6,1e7), rand, 1000*m_fracmult);
			SetFracDef(8, m_maxHeightInMeters, rand.Double(3e6, 1e7), rand, 1000*m_fracmult);
			break;
		}
		case TERRAIN_MOUNTAINS_RIDGED: //8
		{
			SetFracDef(0, m_maxHeightInMeters, rand.Double(1e6,1e7), rand);
			double height = m_maxHeightInMeters*0.9;
			SetFracDef(1, m_maxHeightInMeters, rand.Double(50.0, 100.0)*m_maxHeightInMeters, rand, 8*m_fracmult);
			SetFracDef(2, height, rand.Double(4.0, 200.0)*height, rand, 10*m_fracmult);
			SetFracDef(3, m_maxHeightInMeters, rand.Double(120.0, 2000.0)*m_maxHeightInMeters, rand, 1000*m_fracmult);

			height = m_maxHeightInMeters*0.4;
			SetFracDef(4, m_maxHeightInMeters, rand.Double(100.0, 200.0)*m_maxHeightInMeters, rand);
			SetFracDef(5, height*0.4, rand.Double(2.5,30.5)*height, rand);
			SetFracDef(6, height*0.2, rand.Double(20.5,350.5)*height, rand, 10000*m_fracmult);

			SetFracDef(7, m_maxHeightInMeters, rand.Double(100.0, 2000.0)*m_maxHeightInMeters, rand, 100*m_fracmult);
			SetFracDef(8, height*0.3, rand.Double(2.5,300.5)*height, rand, 500*m_fracmult);
			SetFracDef(9, height*0.2, rand.Double(2.5,300.5)*height, rand, 20*m_fracmult);
			break;
		}
		case TERRAIN_MOUNTAINS_RIVERS: //9
		{
			SetFracDef(0, m_maxHeightInMeters, rand.Double(1e6, 2e6), rand, 10*m_fracmult);
			SetFracDef(1, m_maxHeightInMeters, 15e6, rand, 100.0*m_fracmult);
			SetFracDef(2, m_maxHeightInMeters*0.0000001, rand.Double(500, 2e3), rand, 10*m_fracmult);
			SetFracDef(3, m_maxHeightInMeters*0.00002, rand.Double(1500, 1e4), rand, 10*m_fracmult);
			SetFracDef(4, m_maxHeightInMeters*0.08, 1e4, rand, 10*m_fracmult);
			SetFracDef(5, m_maxHeightInMeters*0.2, 1e5, rand, 10*m_fracmult);
			SetFracDef(6, m_maxHeightInMeters*0.5, 1e6, rand, 100*m_fracmult);
			SetFracDef(7, m_maxHeightInMeters*0.5, rand.Double(1e6,5e6), rand, 100*m_fracmult);
			SetFracDef(8, m_maxHeightInMeters, rand.Double(12e5, 22e5), rand, 10*m_fracmult);
			SetFracDef(9, m_maxHeightInMeters, 1e7, rand, 100.0*m_fracmult);
			break;
		}
		case TERRAIN_MOUNTAINS_CRATERS: //10
		{
			SetFracDef(0, m_maxHeightInMeters, rand.Double(1e6,1e7), rand);
			double height = m_maxHeightInMeters*0.3;
			SetFracDef(1, height, rand.Double(4.0, 20.0)*height, rand);
			SetFracDef(2, m_maxHeightInMeters, rand.Double(50.0, 100.0)*m_maxHeightInMeters, rand);

			height = m_maxHeightInMeters*0.3;
			SetFracDef(4, m_maxHeightInMeters, rand.Double(100.0, 200.0)*m_maxHeightInMeters, rand);
			SetFracDef(3, height, rand.Double(2.5,3.5)*height, rand);

			SetFracDef(5, m_maxHeightInMeters*0.05, 8e5, rand, 1000.0*m_fracmult);
			SetFracDef(6, m_maxHeightInMeters*0.05, 1e6, rand, 10000.0*m_fracmult);
			break;
		}
		case TERRAIN_MOUNTAINS_CRATERS2: //11
		{
			SetFracDef(0, m_maxHeightInMeters, rand.Double(1e6,1e7), rand);
			double height = m_maxHeightInMeters*0.5;
			SetFracDef(1, height, rand.Double(50.0, 200.0)*height, rand, 10*m_fracmult);
			SetFracDef(2, m_maxHeightInMeters, rand.Double(500.0, 5000.0)*m_maxHeightInMeters, rand);

			height = m_maxHeightInMeters*0.4;
			SetFracDef(3, height, rand.Double(2.5,3.5)*height, rand);
			SetFracDef(4, m_maxHeightInMeters, rand.Double(100.0, 200.0)*m_maxHeightInMeters, rand);
			SetFracDef(5, m_maxHeightInMeters*0.05, 1e6, rand, 10000.0*m_fracmult);
			SetFracDef(6, m_maxHeightInMeters*0.04, 9e5, rand, 10000.0*m_fracmult);
			SetFracDef(7, m_maxHeightInMeters*0.05, 8e5, rand, 10000.0*m_fracmult);
			SetFracDef(8, m_maxHeightInMeters*0.04, 11e5, rand, 10000.0*m_fracmult);
			SetFracDef(9, m_maxHeightInMeters*0.07, 12e5, rand, 10000.0*m_fracmult);
			break;
		}
		case TERRAIN_MOUNTAINS_VOLCANO:  //12
		{
			SetFracDef(0, m_maxHeightInMeters, rand.Double(1e6,1e7), rand);
			double height = m_maxHeightInMeters*0.8;
			SetFracDef(1, m_maxHeightInMeters, rand.Double(50.0, 100.0)*m_maxHeightInMeters, rand);
			SetFracDef(2, height, rand.Double(4.0, 20.0)*height, rand);
			SetFracDef(3, height, rand.Double(12.0, 200.0)*height, rand);

			height = m_maxHeightInMeters*0.7;
			SetFracDef(4, m_maxHeightInMeters, rand.Double(100.0, 200.0)*m_maxHeightInMeters, rand);
			SetFracDef(5, height, rand.Double(2.5,3.5)*height, rand);
			SetFracDef(6, height, rand.Double(2.5,3.5)*height, rand);
			// volcano
			SetFracDef(7, 20000.0, 5000000.0, rand, 1000.0*m_fracmult);

			// canyons 
			SetFracDef(8, m_maxHeightInMeters*0.5, 2e6, rand, 100.0*m_fracmult);
			//SetFracDef(9, m_maxHeightInMeters*0.1, 1.5e6, rand, 100.0*m_fracmult);
			//SetFracDef(10, m_maxHeightInMeters*0.1, 2e6, rand, 100.0*m_fracmult);
			break;
		}
		case TERRAIN_MOUNTAINS_RIVERS_VOLCANO:  //old terraformed mars terrain  //13
		{
			SetFracDef(0, m_maxHeightInMeters, rand.Double(1e6,1e7), rand);
			double height = m_maxHeightInMeters*0.6;
			SetFracDef(1, m_maxHeightInMeters, rand.Double(50.0, 100.0)*m_maxHeightInMeters, rand);
			SetFracDef(2, height, rand.Double(4.0, 20.0)*height, rand);
			SetFracDef(3, m_maxHeightInMeters, rand.Double(120.0, 2000.0)*m_maxHeightInMeters, rand, 20*m_fracmult);

			height = m_maxHeightInMeters*0.3;
			SetFracDef(4, m_maxHeightInMeters, rand.Double(100.0, 200.0)*m_maxHeightInMeters, rand);
			SetFracDef(5, height, rand.Double(2.5,3.5)*height, rand);
			SetFracDef(6, height, rand.Double(2.5,3.5)*height, rand);
			// volcano
			SetFracDef(7, 20000.0, 5000000.0, rand, 100.0*m_fracmult);

			// canyons and rivers
			SetFracDef(8, m_maxHeightInMeters*1.0, 4e6, rand, 100.0*m_fracmult);
			SetFracDef(9, m_maxHeightInMeters*1.0, 5e6, rand, 100.0*m_fracmult);
			//SetFracDef(10, m_maxHeightInMeters*0.5, 2e6, rand, 100.0);
			break;
		}
		case TERRAIN_RUGGED_LAVA: //14
		{
			SetFracDef(0, m_maxHeightInMeters, rand.Double(1e6,1e7), rand);
			double height = m_maxHeightInMeters*1.0;
			SetFracDef(1, m_maxHeightInMeters, rand.Double(50.0, 100.0)*m_maxHeightInMeters, rand);
			SetFracDef(2, height, rand.Double(4.0, 20.0)*height, rand);
			SetFracDef(3, height, rand.Double(12.0, 200.0)*height, rand);

			height = m_maxHeightInMeters*0.3;
			SetFracDef(4, m_maxHeightInMeters, rand.Double(100.0, 200.0)*m_maxHeightInMeters, rand);
			SetFracDef(5, height, rand.Double(2.5,3.5)*height, rand);

			// volcanoes
			SetFracDef(6, height, 6e6, rand, 100000.0*m_fracmult);
			SetFracDef(7, height, 3e6, rand, 1000.0*m_fracmult);

			// canyon
			SetFracDef(8, m_maxHeightInMeters*0.4, 4e6, rand, 100.0*m_fracmult);
			// bumps/rocks
			SetFracDef(9, height*0.001, rand.Double(10,100), rand, 2.0*m_fracmult);
			break;
		}
		case TERRAIN_H2O_SOLID: //15
		{
			SetFracDef(0, m_maxHeightInMeters, rand.Double(5e6,1e8), rand);
			double height = m_maxHeightInMeters*0.3;
			SetFracDef(1, height, rand.Double(4.0, 20.0)*height, rand);
			SetFracDef(2, m_maxHeightInMeters, rand.Double(200.0, 1000.0)*m_maxHeightInMeters, rand);

			// mountains with some canyons
			SetFracDef(3, m_maxHeightInMeters*0.4, 4e6, rand);
			SetFracDef(4, m_maxHeightInMeters*0.4, 5e6, rand);
			//crater
			SetFracDef(5, m_maxHeightInMeters*0.4, 1.5e7, rand, 50000.0*m_fracmult);
			break;
			break;
		}
		case TERRAIN_H2O_SOLID_CANYONS: //16
		{
			SetFracDef(0, m_maxHeightInMeters, rand.Double(5e6,1e8), rand);
			double height = m_maxHeightInMeters*0.3;
			SetFracDef(1, height, rand.Double(4.0, 20.0)*height, rand);
			SetFracDef(2, m_maxHeightInMeters, rand.Double(200.0, 1000.0)*m_maxHeightInMeters, rand);

			// mountains with some canyons
			SetFracDef(3, m_maxHeightInMeters*0.4, 4e6, rand);
			SetFracDef(4, m_maxHeightInMeters*0.4, 5e6, rand);
			//crater
			SetFracDef(5, m_maxHeightInMeters*0.4, 15e6, rand, 50000.0*m_fracmult);
			//canyons
			//SetFracDef(6, m_maxHeightInMeters*0.4, 12e6, rand, 50000.0);
			//SetFracDef(7, m_maxHeightInMeters*0.4, 9e6, rand, 50000.0);
			break;
		}
		case TERRAIN_RUGGED_DESERT: //17
		{
			SetFracDef(0, 0.1*m_maxHeightInMeters, 2e6, rand, 180e3*m_fracmult);
			double height = m_maxHeightInMeters*0.9;
			SetFracDef(1, height, rand.Double(120.0, 10000.0)*height, rand, 100*m_fracmult);
			SetFracDef(2, m_maxHeightInMeters, rand.Double(1.0, 2.0)*m_maxHeightInMeters, rand);

			height = m_maxHeightInMeters*0.3;
			SetFracDef(3, height, rand.Double(20.0,240.0)*height, rand);
			SetFracDef(4, m_maxHeightInMeters, rand.Double(1.0, 2.0)*m_maxHeightInMeters, rand);
			// dunes
			height = m_maxHeightInMeters*0.2;
			SetFracDef(5, height*0.1, rand.Double(5,75)*height, rand, 10000.0*m_fracmult);
			// canyon
			SetFracDef(6, m_maxHeightInMeters*0.2, 1e6, rand, 200.0*m_fracmult);
			SetFracDef(7, m_maxHeightInMeters*0.35, 1.5e6, rand, 100.0*m_fracmult);
			SetFracDef(8, m_maxHeightInMeters*0.2, 3e6, rand, 100.0*m_fracmult);

			//SetFracDef(9, m_maxHeightInMeters*0.1, 100, rand, 10.0);
			// adds bumps to the landscape
			SetFracDef(9, height*0.0025, rand.Double(1,100), rand, 100.0*m_fracmult);
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
				SetFracDef(0, height, 5e6, rand, 10.0*m_fracmult);
				SetFracDef(1, height, 3e6, rand, 10.0*m_fracmult);
				SetFracDef(2, height, 1e5, rand, 10.0*m_fracmult);
				SetFracDef(3, height, 1e2, rand, 10.0*m_fracmult);
			}
		case COLOR_STAR_WHITE_DWARF:
			{
				double height = m_maxHeightInMeters*0.1;
				SetFracDef(0, height, 3e5, rand, 10.0*m_fracmult);
				SetFracDef(1, height, 1e5, rand, 10.0*m_fracmult);
				//SetFracDef(2, height, 1e6, rand, 10.0*m_fracmult);
				//SetFracDef(3, height, 1e2, rand, 10.0*m_fracmult);
			}
		case COLOR_STAR_M:
			{
				double height = m_maxHeightInMeters*0.1;
				SetFracDef(0, height, 1e8, rand, 10.0*m_fracmult);
				SetFracDef(1, height, 7e7, rand, 10.0*m_fracmult);
				SetFracDef(2, height, 3e6, rand, 10.0*m_fracmult);
				SetFracDef(3, height, 2e5, rand, 10.0*m_fracmult);
			}
		case COLOR_STAR_K:
			{
				double height = m_maxHeightInMeters*0.1;
				SetFracDef(0, height, 2e8, rand, 10.0*m_fracmult);
				SetFracDef(1, height, 7e7, rand, 10.0*m_fracmult);
				SetFracDef(2, height, 1e6, rand, 10.0*m_fracmult);
				SetFracDef(3, height, 1e3, rand, 10.0*m_fracmult);
			}
		case COLOR_STAR_G:
			{
				double height = m_maxHeightInMeters*0.1;
				SetFracDef(0, height, 8e8, rand, 10.0*m_fracmult);
				SetFracDef(1, height, 7e6, rand, 10.0*m_fracmult);
				SetFracDef(2, height, 4e6, rand, 10.0*m_fracmult);
				SetFracDef(3, height, 2e6, rand, 10.0*m_fracmult);
			}
		case COLOR_GG_JUPITER: 
			{
				// spots
				double height = m_maxHeightInMeters*0.1;
				SetFracDef(0, height, 1e8, rand, 1000.0*m_fracmult);
				SetFracDef(1, height, 8e7, rand, 1000.0*m_fracmult);
				SetFracDef(2, height, 4e7, rand, 1000.0*m_fracmult);
				SetFracDef(3, height, 1e7, rand, 100.0*m_fracmult);
				break;
			}
		case COLOR_GG_SATURN:
			{
				double height = m_maxHeightInMeters*0.1;
				//spot + clouds
				SetFracDef(0, height, 3e7, rand, 10.0*m_fracmult);
				SetFracDef(1, height, 9e7, rand, 1000.0*m_fracmult);
				SetFracDef(2, height, 8e7, rand, 100.0*m_fracmult);
				//spot boundary
				SetFracDef(3, height, 3e7, rand, 10000000.0*m_fracmult);
				break;
			}
		case COLOR_GG_SATURN2:
			{
				double height = m_maxHeightInMeters*0.1;
				//spot + clouds
				SetFracDef(0, height, 3e7, rand, 10.0*m_fracmult);
				SetFracDef(1, height, 9e7, rand, 1000.0*m_fracmult);
				SetFracDef(2, height, 8e7, rand, 100.0*m_fracmult);
				//spot boundary
				SetFracDef(3, height, 3e7, rand, 10000000.0*m_fracmult);
				break;
			}
		case COLOR_GG_URANUS: 
			{
				double height = m_maxHeightInMeters*0.1;
				SetFracDef(0, height, 3e7, rand, 1000.0*m_fracmult);
				SetFracDef(1, height, 9e7, rand, 1000.0*m_fracmult);
				SetFracDef(2, height, 8e7, rand, 1000.0*m_fracmult);
				break;
			}
		case COLOR_GG_NEPTUNE:
			{
				double height = m_maxHeightInMeters*0.1;
				//spot boundary
				SetFracDef(0, height, 3e7, rand, 10000000.0*m_fracmult);
				//spot
				SetFracDef(1, height, 9e7, rand, 100.0*m_fracmult);
				//bands
				SetFracDef(2, height, 8e7, rand, 1000.0*m_fracmult);
				SetFracDef(3, height, 1e8, rand, 1000.0*m_fracmult);
				break;
			}
		case COLOR_GG_NEPTUNE2:
			{
				// spots
				double height = m_maxHeightInMeters*0.1;
				SetFracDef(0, height, 2e8, rand, 1000.0*m_fracmult);
				SetFracDef(1, height, 9e7, rand, 1000.0*m_fracmult);
				SetFracDef(2, height, 6e7, rand, 1000.0*m_fracmult);
				SetFracDef(3, height, 1e8, rand, 100.0*m_fracmult);
				break;
			}
		case COLOR_EARTHLIKE: 
			{
				// crappy water
				//double height = m_maxHeightInMeters*0.5;
				//SetFracDef(3, m_maxHeightInMeters, 1e8, rand, 50.0);
				//SetFracDef(2, m_maxHeightInMeters, 10, rand, 10.0);
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
#endif
}


#if 0
/*
 * Must return >= 0.0
  Here we create the noise used to generate the landscape, the noise should use the fracdef[] settings that were defined earlier.
 */
double Terrain::GetHeight(const vector3d &p)
{
	if (m_heightMap) return GetHeightMapVal(p) / m_planetRadius;

	switch (m_terrainType) {
		case TERRAIN_HILLS_NORMAL:
			return GetHeightHillsNormal(p);

		case TERRAIN_HILLS_DUNES:
			return GetHeightHillsDunes(p);

		case TERRAIN_HILLS_RIDGED:
			return GetHeightHillsRidged(p);

		case TERRAIN_HILLS_RIVERS:
			return GetHeightHillsRivers(p);

		case TERRAIN_HILLS_CRATERS:
			return GetHeightHillsCraters(p);

		case TERRAIN_HILLS_CRATERS2:
			return GetHeightHillsCraters2(p);

		case TERRAIN_MOUNTAINS_NORMAL:
			return GetHeightMountainsNormal(p);

		case TERRAIN_MOUNTAINS_RIDGED:
			return GetHeightMountainsRidged(p);

		case TERRAIN_MOUNTAINS_RIVERS:
			return GetHeightMountainsRivers(p);

		case TERRAIN_MOUNTAINS_CRATERS:
			return GetHeightMountainsCraters(p);

		case TERRAIN_MOUNTAINS_CRATERS2:
			return GetHeightMountainsCraters2(p);

		case TERRAIN_MOUNTAINS_VOLCANO:
			return GetHeightMountainsVolcano(p);

		case TERRAIN_MOUNTAINS_RIVERS_VOLCANO:
			return GetHeightMountainsRiversVolcano(p);

		case TERRAIN_RUGGED_LAVA:
			return GetHeightRuggedLava(p);

		case TERRAIN_H2O_SOLID:
			return GetHeightWaterSolid(p);

		case TERRAIN_H2O_SOLID_CANYONS:
			return GetHeightWaterSolidCanyons(p);

		case TERRAIN_RUGGED_DESERT:
			return GetHeightRuggedDesert(p);

		case TERRAIN_ASTEROID:
			return GetHeightAsteroid(p);

		case TERRAIN_NONE:
		case TERRAIN_FLAT:
			return GetHeightFlat(p);
	}

	assert(0 && "unknown terrain type");
}




/**
 * Height: 0.0 would be sea-level. 1.0 would be an extra elevation of 1 radius (huge)
 */
vector3d Terrain::GetColor(const vector3d &p, double height, const vector3d &norm)
{
	switch (m_colorType) {
		case COLOR_STAR_BROWN_DWARF:
			return GetColorStarBrownDwarf(p, height, norm);

		case COLOR_STAR_WHITE_DWARF:
			return GetColorStarWhiteDwarf(p, height, norm);

		case COLOR_STAR_M:
			return GetColorStarM(p, height, norm);

		case COLOR_STAR_K:
			return GetColorStarK(p, height, norm);

		case COLOR_STAR_G:
			return GetColorStarG(p, height, norm);

		case COLOR_GG_JUPITER:
			return GetColorGGJupiter(p, height, norm);

		case COLOR_GG_SATURN:
			return GetColorGGSaturn(p, height, norm);

		case COLOR_GG_SATURN2:
			return GetColorGGSaturn2(p, height, norm);

		case COLOR_GG_URANUS:
			return GetColorGGUranus(p, height, norm);

		case COLOR_GG_NEPTUNE:
			return GetColorGGNeptune(p, height, norm);

		case COLOR_GG_NEPTUNE2:
			return GetColorGGNeptune2(p, height, norm);

		case COLOR_EARTHLIKE:
			return GetColorEarthlike(p, height, norm);

		case COLOR_DEAD_WITH_H2O:
			return GetColorDeadWithWater(p, height, norm);

		case COLOR_ICEWORLD:
			return GetColorIce(p, height, norm);

		case COLOR_DESERT:
			return GetColorDesert(p, height, norm);

		case COLOR_ROCK:
			return GetColorRock(p, height, norm);

		case COLOR_ROCK2:
			return GetColorRock2(p, height, norm);

		case COLOR_ASTEROID:
			return GetColorAsteroid(p, height, norm);

		case COLOR_VOLCANIC:
			return GetColorVolcanic(p, height, norm);

		case COLOR_METHANE:
			return GetColorMethane(p, height, norm);

		case COLOR_TFGOOD:
			return GetColorTFGood(p, height, norm);

		case COLOR_TFPOOR:
			return GetColorTFPoor(p, height, norm);

		case COLOR_BANDED_ROCK:
			return GetColorBandedRock(p, height, norm);

		case COLOR_NONE:
		case COLOR_SOLID: 
			return GetColorSolid(p, height, norm);
	}

	assert(0 && "unknown terrain color type");
}
#endif
