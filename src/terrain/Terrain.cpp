#include "Terrain.h"
#include "perlin.h"
#include "Pi.h"

// static instancer. selects the best height and color classes for the body
Terrain *Terrain::InstanceTerrain(const SBody *body)
{
	MTRand rand(body->seed);

	GeneratorInstancer gi = 0;

	switch (body->type) {

		case SBody::TYPE_BROWN_DWARF:
			gi = InstanceGenerator<TerrainHeightFlat,TerrainColorStarBrownDwarf>;
			break;

		case SBody::TYPE_WHITE_DWARF:
			gi = InstanceGenerator<TerrainHeightFlat,TerrainColorStarWhiteDwarf>;
			break;

		case SBody::TYPE_STAR_M:
		case SBody::TYPE_STAR_M_GIANT:
		case SBody::TYPE_STAR_M_SUPER_GIANT:
		case SBody::TYPE_STAR_M_HYPER_GIANT: {
			const GeneratorInstancer choices[] = {
				InstanceGenerator<TerrainHeightFlat,TerrainColorStarM>,
				InstanceGenerator<TerrainHeightFlat,TerrainColorStarM>,
				InstanceGenerator<TerrainHeightFlat,TerrainColorStarK>,
				InstanceGenerator<TerrainHeightFlat,TerrainColorStarG>
			};
			gi = choices[rand.Int32(4)];
			break;
		}

		case SBody::TYPE_STAR_K:
		case SBody::TYPE_STAR_K_GIANT:
		case SBody::TYPE_STAR_K_SUPER_GIANT:
		case SBody::TYPE_STAR_K_HYPER_GIANT: {
			const GeneratorInstancer choices[] = {
				InstanceGenerator<TerrainHeightFlat,TerrainColorStarM>,
				InstanceGenerator<TerrainHeightFlat,TerrainColorStarK>,
				InstanceGenerator<TerrainHeightFlat,TerrainColorStarK>,
				InstanceGenerator<TerrainHeightFlat,TerrainColorStarG>
			};
			gi = choices[rand.Int32(4)];
			break;
		}

		case SBody::TYPE_STAR_G:
		case SBody::TYPE_STAR_G_GIANT:
		case SBody::TYPE_STAR_G_SUPER_GIANT:
		case SBody::TYPE_STAR_G_HYPER_GIANT: {
			const GeneratorInstancer choices[] = {
				InstanceGenerator<TerrainHeightFlat,TerrainColorStarWhiteDwarf>,
				InstanceGenerator<TerrainHeightFlat,TerrainColorStarK>
			};
			gi = choices[rand.Int32(2)];
			break;
		}

		case SBody::TYPE_PLANET_GAS_GIANT: {
			const GeneratorInstancer choices[] = {
				InstanceGenerator<TerrainHeightFlat,TerrainColorGGSaturn>,
				InstanceGenerator<TerrainHeightFlat,TerrainColorGGSaturn>,
				InstanceGenerator<TerrainHeightFlat,TerrainColorGGSaturn>,
				InstanceGenerator<TerrainHeightFlat,TerrainColorGGSaturn>,
				InstanceGenerator<TerrainHeightFlat,TerrainColorGGSaturn>,
				InstanceGenerator<TerrainHeightFlat,TerrainColorGGSaturn>
			};
			gi = choices[rand.Int32(6)];
			break;
		}

		case SBody::TYPE_PLANET_ASTEROID:
			gi = InstanceGenerator<TerrainHeightAsteroid,TerrainColorAsteroid>;
			break;

		case SBody::TYPE_PLANET_TERRESTRIAL: {

			// Earth-like world
			if ((body->m_life > fixed(7,10)) && (body->m_volatileGas > fixed(2,10))) {
				// There would be no life on the surface without atmosphere

				if (body->averageTemp > 240) {
					const GeneratorInstancer choices[] = {
						InstanceGenerator<TerrainHeightHillsRidged,TerrainColorEarthLike>,
						InstanceGenerator<TerrainHeightHillsRivers,TerrainColorEarthLike>,
						InstanceGenerator<TerrainHeightHillsDunes,TerrainColorEarthLike>,
						InstanceGenerator<TerrainHeightMountainsRidged,TerrainColorEarthLike>,
						InstanceGenerator<TerrainHeightMountainsNormal,TerrainColorEarthLike>,
						InstanceGenerator<TerrainHeightMountainsRivers,TerrainColorEarthLike>,
						InstanceGenerator<TerrainHeightMountainsVolcano,TerrainColorEarthLike>,
						InstanceGenerator<TerrainHeightMountainsRiversVolcano,TerrainColorEarthLike>
					};
					gi = choices[rand.Int32(8)];
					break;
				}

				const GeneratorInstancer choices[] = {
					InstanceGenerator<TerrainHeightHillsRidged,TerrainColorDesert>,
					InstanceGenerator<TerrainHeightHillsRivers,TerrainColorDesert>,
					InstanceGenerator<TerrainHeightHillsDunes,TerrainColorDesert>,
					InstanceGenerator<TerrainHeightMountainsRidged,TerrainColorDesert>,
					InstanceGenerator<TerrainHeightMountainsNormal,TerrainColorDesert>,
					InstanceGenerator<TerrainHeightMountainsRivers,TerrainColorDesert>,
					InstanceGenerator<TerrainHeightMountainsVolcano,TerrainColorDesert>,
					InstanceGenerator<TerrainHeightMountainsRiversVolcano,TerrainColorDesert>
				};
				gi = choices[rand.Int32(8)];
				break;
			}

			// Harsh, habitable world 
			if ((body->m_volatileGas > fixed(2,10)) && (body->m_life > fixed(4,10)) ) {

				if (body->averageTemp > 240) {
					const GeneratorInstancer choices[] = {
						InstanceGenerator<TerrainHeightHillsRidged,TerrainColorTFGood>,
						InstanceGenerator<TerrainHeightHillsRivers,TerrainColorTFGood>,
						InstanceGenerator<TerrainHeightHillsDunes,TerrainColorTFGood>,
						InstanceGenerator<TerrainHeightHillsNormal,TerrainColorTFGood>,
						InstanceGenerator<TerrainHeightMountainsNormal,TerrainColorTFGood>,
						InstanceGenerator<TerrainHeightMountainsRidged,TerrainColorTFGood>,
						InstanceGenerator<TerrainHeightMountainsVolcano,TerrainColorTFGood>,
						InstanceGenerator<TerrainHeightMountainsRiversVolcano,TerrainColorTFGood>,
						InstanceGenerator<TerrainHeightMountainsRivers,TerrainColorTFGood>,
						InstanceGenerator<TerrainHeightRuggedDesert,TerrainColorTFGood>
					};
					gi = choices[rand.Int32(10)];
					break;
				}

				const GeneratorInstancer choices[] = {
					InstanceGenerator<TerrainHeightHillsRidged,TerrainColorIce>,
					InstanceGenerator<TerrainHeightHillsRivers,TerrainColorIce>,
					InstanceGenerator<TerrainHeightHillsDunes,TerrainColorIce>,
					InstanceGenerator<TerrainHeightHillsNormal,TerrainColorIce>,
					InstanceGenerator<TerrainHeightMountainsNormal,TerrainColorIce>,
					InstanceGenerator<TerrainHeightMountainsRidged,TerrainColorIce>,
					InstanceGenerator<TerrainHeightMountainsVolcano,TerrainColorIce>,
					InstanceGenerator<TerrainHeightMountainsRiversVolcano,TerrainColorIce>,
					InstanceGenerator<TerrainHeightMountainsRivers,TerrainColorIce>,
					InstanceGenerator<TerrainHeightRuggedDesert,TerrainColorIce>
				};
				gi = choices[rand.Int32(10)];
				break;
			}
			
			// Marginally habitable world/ verging on mars like :) 
			else if ((body->m_volatileGas > fixed(1,10)) && (body->m_life > fixed(1,10)) ) {

				if (body->averageTemp > 240) {
					const GeneratorInstancer choices[] = {
						InstanceGenerator<TerrainHeightHillsRidged,TerrainColorTFPoor>,
						InstanceGenerator<TerrainHeightHillsRivers,TerrainColorTFPoor>,
						InstanceGenerator<TerrainHeightHillsDunes,TerrainColorTFPoor>,
						InstanceGenerator<TerrainHeightHillsNormal,TerrainColorTFPoor>,
						InstanceGenerator<TerrainHeightMountainsNormal,TerrainColorTFPoor>,
						InstanceGenerator<TerrainHeightMountainsRidged,TerrainColorTFPoor>,
						InstanceGenerator<TerrainHeightMountainsVolcano,TerrainColorTFPoor>,
						InstanceGenerator<TerrainHeightMountainsRiversVolcano,TerrainColorTFPoor>,
						InstanceGenerator<TerrainHeightMountainsRivers,TerrainColorTFPoor>,
						InstanceGenerator<TerrainHeightRuggedDesert,TerrainColorTFPoor>
					};
					gi = choices[rand.Int32(10)];
					break;
				}

				const GeneratorInstancer choices[] = {
					InstanceGenerator<TerrainHeightHillsRidged,TerrainColorIce>,
					InstanceGenerator<TerrainHeightHillsRivers,TerrainColorIce>,
					InstanceGenerator<TerrainHeightHillsDunes,TerrainColorIce>,
					InstanceGenerator<TerrainHeightHillsNormal,TerrainColorIce>,
					InstanceGenerator<TerrainHeightMountainsNormal,TerrainColorIce>,
					InstanceGenerator<TerrainHeightMountainsRidged,TerrainColorIce>,
					InstanceGenerator<TerrainHeightMountainsVolcano,TerrainColorIce>,
					InstanceGenerator<TerrainHeightMountainsRiversVolcano,TerrainColorIce>,
					InstanceGenerator<TerrainHeightMountainsRivers,TerrainColorIce>,
					InstanceGenerator<TerrainHeightRuggedDesert,TerrainColorIce>
				};
				gi = choices[rand.Int32(10)];
				break;
			}

			// Desert-like world, Mars -like.
			if ((body->m_volatileLiquid < fixed(1,10)) && (body->m_volatileGas > fixed(1,5))) {
				const GeneratorInstancer choices[] = {
					InstanceGenerator<TerrainHeightHillsDunes,TerrainColorDesert>,
					InstanceGenerator<TerrainHeightWaterSolid,TerrainColorDesert>,
					InstanceGenerator<TerrainHeightRuggedDesert,TerrainColorDesert>,
					InstanceGenerator<TerrainHeightRuggedLava,TerrainColorDesert>,
					InstanceGenerator<TerrainHeightMountainsVolcano,TerrainColorDesert>,
					InstanceGenerator<TerrainHeightMountainsRiversVolcano,TerrainColorDesert>
				};
				gi = choices[rand.Int32(6)];
				break;
			}

			// Frozen world
			if ((body->m_volatileIces > fixed(8,10)) &&  (body->averageTemp < 250)) {
				const GeneratorInstancer choices[] = {
					InstanceGenerator<TerrainHeightHillsDunes,TerrainColorIce>,
					InstanceGenerator<TerrainHeightHillsCraters,TerrainColorIce>,
					InstanceGenerator<TerrainHeightMountainsCraters,TerrainColorIce>,
					InstanceGenerator<TerrainHeightWaterSolid,TerrainColorIce>,
					InstanceGenerator<TerrainHeightWaterSolidCanyons,TerrainColorIce>,
					InstanceGenerator<TerrainHeightRuggedDesert,TerrainColorIce>
				};
				gi = choices[rand.Int32(6)];
				break;
			}

			// Volcanic world
			if (body->m_volcanicity > fixed(7,10)) {

				if (body->m_life > fixed(5,10))	// life on a volcanic world ;)
					gi = InstanceGenerator<TerrainHeightRuggedLava,TerrainColorTFGood>;
				else if (body->m_life > fixed(2,10))
					gi = InstanceGenerator<TerrainHeightRuggedLava,TerrainColorTFPoor>;
				else
					gi = InstanceGenerator<TerrainHeightRuggedLava,TerrainColorVolcanic>;
				break;
			}

			//Below might not be needed.
			//Alien life world:
			if (body->m_life > fixed(1,10))  {
				const GeneratorInstancer choices[] = {
					InstanceGenerator<TerrainHeightHillsDunes,TerrainColorTFPoor>,
					InstanceGenerator<TerrainHeightHillsRidged,TerrainColorTFPoor>,
					InstanceGenerator<TerrainHeightHillsRivers,TerrainColorTFPoor>,
					InstanceGenerator<TerrainHeightMountainsNormal,TerrainColorTFPoor>,
					InstanceGenerator<TerrainHeightMountainsRidged,TerrainColorTFPoor>,
					InstanceGenerator<TerrainHeightMountainsVolcano,TerrainColorTFPoor>,
					InstanceGenerator<TerrainHeightMountainsRiversVolcano,TerrainColorTFPoor>,
					InstanceGenerator<TerrainHeightMountainsRivers,TerrainColorTFPoor>,
					InstanceGenerator<TerrainHeightWaterSolid,TerrainColorTFPoor>,
					InstanceGenerator<TerrainHeightRuggedLava,TerrainColorTFPoor>,
					InstanceGenerator<TerrainHeightRuggedDesert,TerrainColorTFPoor>
				};
				gi = choices[rand.Int32(11)];
				break;
			};

			if (body->m_volatileGas > fixed(1,10)) {
				const GeneratorInstancer choices[] = {
					InstanceGenerator<TerrainHeightHillsNormal,TerrainColorRock>,
					InstanceGenerator<TerrainHeightMountainsNormal,TerrainColorRock>,
					InstanceGenerator<TerrainHeightRuggedDesert,TerrainColorRock>
				};
				gi = choices[rand.Int32(3)];
				break;
			}

			const GeneratorInstancer choices[] = {
				InstanceGenerator<TerrainHeightHillsCraters2,TerrainColorRock>,
				InstanceGenerator<TerrainHeightMountainsCraters2,TerrainColorRock>,
			};
			gi = choices[rand.Int32(2)];
			break;
		}

		default:
			gi = InstanceGenerator<TerrainHeightFlat,TerrainColorSolid>;
			break;
	}

	return gi(body);
}

Terrain::Terrain(const SBody *body) : m_body(body), m_rand(body->seed) {
	printf("in terrain constructor for %s\n", body->name.c_str());

	// XXX hardcoded until we get the config/change detail stuff back
	textures = false;
	m_fracnum = 2;
	m_fracmult = 1;

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
	for (int i=0; i<12; i++) m_entropy[i] = m_rand.Double();
	for (int i=0; i<8; i++) {
		double r,g,b;
		r = m_rand.Double(0.3, 1.0);
		g = m_rand.Double(0.3, r);
		b = m_rand.Double(0.3, g);
		r = std::max(b, r * m_body->m_metallicity.ToFloat());
		g = std::max(b, g * m_body->m_metallicity.ToFloat());
		m_rockColor[i] = vector3d(r, g, b);
	}

	// Pick some darker colours mainly reds and greens
	for (int i=0; i<12; i++) m_entropy[i] = m_rand.Double();
	for (int i=0; i<8; i++) {
		double r,g,b;
		r = m_rand.Double(0.05, 0.3);
		g = m_rand.Double(0.05, r);
		b = m_rand.Double(0.05, g);
		r = std::max(b, r * m_body->m_metallicity.ToFloat());
		g = std::max(b, g * m_body->m_metallicity.ToFloat());
		m_darkrockColor[i] = vector3d(r, g, b);
	}

	// grey colours, in case you simply must have a grey colour on a world with high metallicity
	for (int i=0; i<12; i++) m_entropy[i] = m_rand.Double();
	for (int i=0; i<8; i++) {
		double g;
		g = m_rand.Double(0.3, 0.9);
		m_greyrockColor[i] = vector3d(g, g, g);
	}

	// Pick some plant colours, mainly greens
	// TODO take star class into account
	for (int i=0; i<12; i++) m_entropy[i] = m_rand.Double();
	for (int i=0; i<8; i++) {
		double r,g,b;
		g = m_rand.Double(0.3, 1.0);
		r = m_rand.Double(0.3, g);
		b = m_rand.Double(0.2, r);
		g = std::max(r, g * m_body->m_life.ToFloat());
		b *= (1.0-m_body->m_life.ToFloat());
		m_plantColor[i] = vector3d(r, g, b);
	}

	// Pick some darker plant colours mainly greens
	// TODO take star class into account
	for (int i=0; i<12; i++) m_entropy[i] = m_rand.Double();
	for (int i=0; i<8; i++) {
		double r,g,b;
		g = m_rand.Double(0.05, 0.3);
		r = m_rand.Double(0.00, g);
		b = m_rand.Double(0.00, r);
		g = std::max(r, g * m_body->m_life.ToFloat());
		b *= (1.0-m_body->m_life.ToFloat());
		m_darkplantColor[i] = vector3d(r, g, b);
	}

	// Pick some sand colours, mainly yellow
	// TODO let some planetary value scale this colour
	for (int i=0; i<12; i++) m_entropy[i] = m_rand.Double();
	for (int i=0; i<8; i++) {
		double r,g,b;
		r = m_rand.Double(0.6, 1.0);
		g = m_rand.Double(0.6, r);
		//b = m_rand.Double(0.0, g/2.0);
		b = 0;
		m_sandColor[i] = vector3d(r, g, b);
	}

	// Pick some darker sand colours mainly yellow
	// TODO let some planetary value scale this colour
	for (int i=0; i<12; i++) m_entropy[i] = m_rand.Double();
	for (int i=0; i<8; i++) {
		double r,g,b;
		r = m_rand.Double(0.05, 0.6);
		g = m_rand.Double(0.00, r);
		//b = m_rand.Double(0.00, g/2.0);
		b = 0;
		m_darksandColor[i] = vector3d(r, g, b);
	}

	// Pick some dirt colours, mainly red/brown
	// TODO let some planetary value scale this colour
	for (int i=0; i<12; i++) m_entropy[i] = m_rand.Double();
	for (int i=0; i<8; i++) {
		double r,g,b;
		r = m_rand.Double(0.3, 0.7);
		g = m_rand.Double(r-0.1, 0.75);
		b = m_rand.Double(0.0, r/2.0);
		m_dirtColor[i] = vector3d(r, g, b);
	}

	// Pick some darker dirt colours mainly red/brown
	// TODO let some planetary value scale this colour
	for (int i=0; i<12; i++) m_entropy[i] = m_rand.Double();
	for (int i=0; i<8; i++) {
		double r,g,b;
		r = m_rand.Double(0.05, 0.3);
		g = m_rand.Double(r-0.05, 0.35);
		b = m_rand.Double(0.0, r/2.0);
		m_darkdirtColor[i] = vector3d(r, g, b);
	}

	// These are used for gas giant colours, they are more m_random and *should* really use volatileGasses - TODO
	for (int i=0; i<12; i++) m_entropy[i] = m_rand.Double();
	for (int i=0; i<8; i++) {
		double r,g,b;
		r = m_rand.Double(0.0, 0.5);
		g = m_rand.Double(0.0, 0.5);
		b = m_rand.Double(0.0, 0.5);
		m_gglightColor[i] = vector3d(r, g, b);
	}
	//darker gas giant colours, more reds and greens
	for (int i=0; i<12; i++) m_entropy[i] = m_rand.Double();
	for (int i=0; i<8; i++) {
		double r,g,b;
		r = m_rand.Double(0.0, 0.3);
		g = m_rand.Double(0.0, r);
		b = m_rand.Double(0.0, std::min(r, g));
		m_ggdarkColor[i] = vector3d(r, g, b);
	}
}

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

	//m_terrainType = TERRAIN_HILLS_DUNES;
	//m_colorType = COLOR_DESERT;
	printf("%s: \n", m_body->name.c_str());
	printf("|   Terrain: [%d]\n", m_terrainType);
	printf("|    Colour: [%d]\n", m_colorType);


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
void Terrain::SetFracDef(unsigned int index, double featureHeightMeters, double featureWidthMeters, double smallestOctaveMeters)
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
