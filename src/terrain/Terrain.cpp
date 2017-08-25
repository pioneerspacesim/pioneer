// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"
#include "perlin.h"
#include "Pi.h"
#include "FileSystem.h"
#include "FloatComparison.h"

// static instancer. selects the best height and color classes for the body
Terrain *Terrain::InstanceTerrain(const SystemBody *body)
{
	// special case for heightmaps
	// XXX this is terrible but will do for now until we get a unified
	// heightmap setup. if you add another height fractal, remember to change
	// the check in CustomSystem::l_height_map
	if (!body->GetHeightMapFilename().empty()) {
		const GeneratorInstancer choices[] = {
			InstanceGenerator<TerrainHeightMapped,TerrainColorEarthLikeHeightmapped>,
			InstanceGenerator<TerrainHeightMapped2,TerrainColorRock2>
		};
		assert(body->GetHeightMapFractal() < COUNTOF(choices));
		return choices[body->GetHeightMapFractal()](body);
	}

	Random rand(body->GetSeed());

	GeneratorInstancer gi = 0;

	switch (body->GetType()) {

		case SystemBody::TYPE_BROWN_DWARF:
			gi = InstanceGenerator<TerrainHeightEllipsoid,TerrainColorStarBrownDwarf>;
			break;

		case SystemBody::TYPE_WHITE_DWARF:
			gi = InstanceGenerator<TerrainHeightEllipsoid,TerrainColorStarWhiteDwarf>;
			break;

		case SystemBody::TYPE_STAR_M:
		case SystemBody::TYPE_STAR_M_GIANT:
		case SystemBody::TYPE_STAR_M_SUPER_GIANT:
		case SystemBody::TYPE_STAR_M_HYPER_GIANT: {
			const GeneratorInstancer choices[] = {
				InstanceGenerator<TerrainHeightEllipsoid,TerrainColorStarM>,
				InstanceGenerator<TerrainHeightEllipsoid,TerrainColorStarM>,
				InstanceGenerator<TerrainHeightEllipsoid,TerrainColorStarK>,
				InstanceGenerator<TerrainHeightEllipsoid,TerrainColorStarG>
			};
			gi = choices[rand.Int32(COUNTOF(choices))];
			break;
		}

		case SystemBody::TYPE_STAR_K:
		case SystemBody::TYPE_STAR_K_GIANT:
		case SystemBody::TYPE_STAR_K_SUPER_GIANT:
		case SystemBody::TYPE_STAR_K_HYPER_GIANT: {
			const GeneratorInstancer choices[] = {
				InstanceGenerator<TerrainHeightEllipsoid,TerrainColorStarM>,
				InstanceGenerator<TerrainHeightEllipsoid,TerrainColorStarK>,
				InstanceGenerator<TerrainHeightEllipsoid,TerrainColorStarK>,
				InstanceGenerator<TerrainHeightEllipsoid,TerrainColorStarG>
			};
			gi = choices[rand.Int32(COUNTOF(choices))];
			break;
		}

		case SystemBody::TYPE_STAR_G:
		case SystemBody::TYPE_STAR_G_GIANT:
		case SystemBody::TYPE_STAR_G_SUPER_GIANT:
		case SystemBody::TYPE_STAR_G_HYPER_GIANT: {
			const GeneratorInstancer choices[] = {
				InstanceGenerator<TerrainHeightEllipsoid,TerrainColorStarWhiteDwarf>,
				InstanceGenerator<TerrainHeightEllipsoid,TerrainColorStarG>
			};
			gi = choices[rand.Int32(COUNTOF(choices))];
			break;
		}

		case SystemBody::TYPE_STAR_F:
		case SystemBody::TYPE_STAR_F_GIANT:
		case SystemBody::TYPE_STAR_F_HYPER_GIANT:
		case SystemBody::TYPE_STAR_F_SUPER_GIANT:
		case SystemBody::TYPE_STAR_A:
		case SystemBody::TYPE_STAR_A_GIANT:
		case SystemBody::TYPE_STAR_A_HYPER_GIANT:
		case SystemBody::TYPE_STAR_A_SUPER_GIANT:
		case SystemBody::TYPE_STAR_B:
		case SystemBody::TYPE_STAR_B_GIANT:
		case SystemBody::TYPE_STAR_B_SUPER_GIANT:
		case SystemBody::TYPE_STAR_B_WF:
		case SystemBody::TYPE_STAR_O:
		case SystemBody::TYPE_STAR_O_GIANT:
		case SystemBody::TYPE_STAR_O_HYPER_GIANT:
		case SystemBody::TYPE_STAR_O_SUPER_GIANT:
		case SystemBody::TYPE_STAR_O_WF:
			gi = InstanceGenerator<TerrainHeightEllipsoid,TerrainColorSolid>;
		break;

		case SystemBody::TYPE_PLANET_GAS_GIANT: {
			const GeneratorInstancer choices[] = {
				InstanceGenerator<TerrainHeightFlat,TerrainColorGGJupiter>,
				InstanceGenerator<TerrainHeightFlat,TerrainColorGGSaturn>,
				InstanceGenerator<TerrainHeightFlat,TerrainColorGGSaturn2>,
				InstanceGenerator<TerrainHeightFlat,TerrainColorGGNeptune>,
				InstanceGenerator<TerrainHeightFlat,TerrainColorGGNeptune2>,
				InstanceGenerator<TerrainHeightFlat,TerrainColorGGUranus>,
				InstanceGenerator<TerrainHeightFlat,TerrainColorGGSaturn>
			};
			gi = choices[rand.Int32(COUNTOF(choices))];
			break;
		}

		case SystemBody::TYPE_PLANET_ASTEROID: {
			const GeneratorInstancer choices[] = {
				InstanceGenerator<TerrainHeightAsteroid,TerrainColorAsteroid>,
				InstanceGenerator<TerrainHeightAsteroid2,TerrainColorAsteroid>,
				InstanceGenerator<TerrainHeightAsteroid3,TerrainColorAsteroid>,
				InstanceGenerator<TerrainHeightAsteroid4,TerrainColorAsteroid>,
				InstanceGenerator<TerrainHeightAsteroid,TerrainColorRock>,
				InstanceGenerator<TerrainHeightAsteroid2,TerrainColorBandedRock>,
				InstanceGenerator<TerrainHeightAsteroid3,TerrainColorRock>,
				InstanceGenerator<TerrainHeightAsteroid4,TerrainColorBandedRock>
			};
			gi = choices[rand.Int32(COUNTOF(choices))];
			break;
		}

		case SystemBody::TYPE_PLANET_TERRESTRIAL: {

			//Over-ride:
			//gi = InstanceGenerator<TerrainHeightAsteroid3,TerrainColorRock>;
			//break;
			// Earth-like world

			if ((body->GetLifeAsFixed() > fixed(7,10)) && (body->GetVolatileGasAsFixed() > fixed(2,10))) {
				// There would be no life on the surface without atmosphere

				if (body->GetAverageTemp() > 240) {
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
					gi = choices[rand.Int32(COUNTOF(choices))];
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
					InstanceGenerator<TerrainHeightMountainsRiversVolcano,TerrainColorDesert>,
					InstanceGenerator<TerrainHeightBarrenRock,TerrainColorDesert>,
					InstanceGenerator<TerrainHeightBarrenRock2,TerrainColorDesert>//,
					//InstanceGenerator<TerrainHeightBarrenRock3,TerrainColorTFGood>
				};
				gi = choices[rand.Int32(COUNTOF(choices))];
				break;
			}

			// Harsh, habitable world
			if ((body->GetVolatileGasAsFixed() > fixed(2,10)) && (body->GetLifeAsFixed() > fixed(4,10)) ) {

				if (body->GetAverageTemp() > 240) {
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
						InstanceGenerator<TerrainHeightRuggedDesert,TerrainColorTFGood>,
						InstanceGenerator<TerrainHeightBarrenRock,TerrainColorTFGood>,
						InstanceGenerator<TerrainHeightBarrenRock2,TerrainColorTFGood>
						//InstanceGenerator<TerrainHeightBarrenRock3,TerrainColorTFGood>
					};
					gi = choices[rand.Int32(COUNTOF(choices))];
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
					InstanceGenerator<TerrainHeightRuggedDesert,TerrainColorIce>,
					InstanceGenerator<TerrainHeightBarrenRock,TerrainColorIce>,
					InstanceGenerator<TerrainHeightBarrenRock2,TerrainColorIce>,
					InstanceGenerator<TerrainHeightBarrenRock3,TerrainColorIce>
				};
				gi = choices[rand.Int32(COUNTOF(choices))];
				break;
			}

			// Marginally habitable world/ verging on mars like :)
			else if ((body->GetVolatileGasAsFixed() > fixed(1,10)) && (body->GetLifeAsFixed() > fixed(1,10)) ) {

				if (body->GetAverageTemp() > 240) {
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
						InstanceGenerator<TerrainHeightRuggedDesert,TerrainColorTFPoor>,
						InstanceGenerator<TerrainHeightBarrenRock,TerrainColorTFPoor>,
						InstanceGenerator<TerrainHeightBarrenRock2,TerrainColorTFPoor>,
						InstanceGenerator<TerrainHeightBarrenRock3,TerrainColorTFPoor>
					};
					gi = choices[rand.Int32(COUNTOF(choices))];
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
					InstanceGenerator<TerrainHeightRuggedDesert,TerrainColorIce>,
					InstanceGenerator<TerrainHeightBarrenRock,TerrainColorIce>,
					InstanceGenerator<TerrainHeightBarrenRock2,TerrainColorIce>,
					InstanceGenerator<TerrainHeightBarrenRock3,TerrainColorIce>
				};
				gi = choices[rand.Int32(COUNTOF(choices))];
				break;
			}

			// Desert-like world, Mars -like.
			if ((body->GetVolatileLiquidAsFixed() < fixed(1,10)) && (body->GetVolatileGasAsFixed() > fixed(1,5))) {
				const GeneratorInstancer choices[] = {
					InstanceGenerator<TerrainHeightHillsDunes,TerrainColorDesert>,
					InstanceGenerator<TerrainHeightWaterSolid,TerrainColorDesert>,
					InstanceGenerator<TerrainHeightRuggedDesert,TerrainColorDesert>,
					InstanceGenerator<TerrainHeightRuggedLava,TerrainColorDesert>,
					InstanceGenerator<TerrainHeightMountainsVolcano,TerrainColorDesert>,
					InstanceGenerator<TerrainHeightMountainsRiversVolcano,TerrainColorDesert>,
					InstanceGenerator<TerrainHeightBarrenRock,TerrainColorDesert>,
					InstanceGenerator<TerrainHeightBarrenRock2,TerrainColorDesert>
				};
				gi = choices[rand.Int32(COUNTOF(choices))];
				break;
			}

			// Frozen world
			if ((body->GetVolatileIcesAsFixed() > fixed(8,10)) &&  (body->GetAverageTemp() < 250)) {
				const GeneratorInstancer choices[] = {
					InstanceGenerator<TerrainHeightHillsDunes,TerrainColorIce>,
					InstanceGenerator<TerrainHeightHillsCraters,TerrainColorIce>,
					InstanceGenerator<TerrainHeightMountainsCraters,TerrainColorIce>,
					InstanceGenerator<TerrainHeightWaterSolid,TerrainColorIce>,
					InstanceGenerator<TerrainHeightWaterSolidCanyons,TerrainColorIce>,
					InstanceGenerator<TerrainHeightRuggedDesert,TerrainColorIce>,
					InstanceGenerator<TerrainHeightBarrenRock,TerrainColorIce>,
					InstanceGenerator<TerrainHeightBarrenRock2,TerrainColorIce>,
					InstanceGenerator<TerrainHeightBarrenRock3,TerrainColorIce>
				};
				gi = choices[rand.Int32(COUNTOF(choices))];
				break;
			}

			// Volcanic world
			if (body->GetVolcanicityAsFixed() > fixed(7,10)) {

				if (body->GetLifeAsFixed() > fixed(5,10))	// life on a volcanic world ;)
					gi = InstanceGenerator<TerrainHeightRuggedLava,TerrainColorTFGood>;
				else if (body->GetLifeAsFixed() > fixed(2,10))
					gi = InstanceGenerator<TerrainHeightRuggedLava,TerrainColorTFPoor>;
				else
					gi = InstanceGenerator<TerrainHeightRuggedLava,TerrainColorVolcanic>;
				break;
			}

			//Below might not be needed.
			//Alien life world:
			if (body->GetLifeAsFixed() > fixed(1,10))  {
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
					InstanceGenerator<TerrainHeightRuggedDesert,TerrainColorTFPoor>,
					InstanceGenerator<TerrainHeightBarrenRock,TerrainColorIce>,
					InstanceGenerator<TerrainHeightBarrenRock2,TerrainColorIce>,
					InstanceGenerator<TerrainHeightBarrenRock3,TerrainColorIce>
				};
				gi = choices[rand.Int32(COUNTOF(choices))];
				break;
			};

			if (body->GetVolatileGasAsFixed() > fixed(1,10)) {
				const GeneratorInstancer choices[] = {
					InstanceGenerator<TerrainHeightHillsNormal,TerrainColorRock>,
					InstanceGenerator<TerrainHeightMountainsNormal,TerrainColorRock>,
					InstanceGenerator<TerrainHeightRuggedDesert,TerrainColorRock>,
					InstanceGenerator<TerrainHeightBarrenRock,TerrainColorRock>,
					InstanceGenerator<TerrainHeightBarrenRock2,TerrainColorRock>,
					InstanceGenerator<TerrainHeightBarrenRock3,TerrainColorRock>
				};
				gi = choices[rand.Int32(COUNTOF(choices))];
				break;
			}

			const GeneratorInstancer choices[] = {
				InstanceGenerator<TerrainHeightHillsCraters2,TerrainColorRock>,
				InstanceGenerator<TerrainHeightMountainsCraters2,TerrainColorRock>,
				InstanceGenerator<TerrainHeightBarrenRock3,TerrainColorRock>
			};
			gi = choices[rand.Int32(COUNTOF(choices))];
			break;
		}

		default:
			gi = InstanceGenerator<TerrainHeightFlat,TerrainColorSolid>;
			break;
	}

	return gi(body);
}

static size_t bufread_or_die(void *ptr, size_t size, size_t nmemb, ByteRange &buf)
{
	size_t read_count = buf.read(static_cast<char*>(ptr), size, nmemb);
	if (read_count < nmemb) {
		Output("Error: failed to read file (truncated)\n");
		abort();
	}
	return read_count;
}

// XXX this sucks, but there isn't a reliable cross-platform way to get them
#ifndef INT16_MIN
# define INT16_MIN   (-32767-1)
#endif
#ifndef INT16_MAX
# define INT16_MAX   (32767)
#endif
#ifndef UINT16_MAX
# define UINT16_MAX  (65535)
#endif

Terrain::Terrain(const SystemBody *body) : m_seed(body->GetSeed()), m_rand(body->GetSeed()), m_heightScaling(0), m_minh(0), m_minBody(body) {

	// load the heightmap
	if (!body->GetHeightMapFilename().empty()) {
		RefCountedPtr<FileSystem::FileData> fdata = FileSystem::gameDataFiles.ReadFile(body->GetHeightMapFilename());
		if (!fdata) {
			Output("Error: could not open file '%s'\n", body->GetHeightMapFilename().c_str());
			abort();
		}

		ByteRange databuf = fdata->AsByteRange();

		Sint16 minHMap = INT16_MAX, maxHMap = INT16_MIN;
		Uint16 minHMapScld = UINT16_MAX, maxHMapScld = 0;

		// XXX unify heightmap types
		switch (body->GetHeightMapFractal()) {
			case 0: {
				Uint16 v;
				bufread_or_die(&v, 2, 1, databuf); m_heightMapSizeX = v;
				bufread_or_die(&v, 2, 1, databuf); m_heightMapSizeY = v;
				const Uint32 heightmapPixelArea = (m_heightMapSizeX * m_heightMapSizeY);

				std::unique_ptr<Sint16[]> heightMap(new Sint16[heightmapPixelArea]);
				bufread_or_die(heightMap.get(), sizeof(Sint16), heightmapPixelArea, databuf);
				m_heightMap.reset(new double[heightmapPixelArea]);
				double *pHeightMap = m_heightMap.get();
				for(Uint32 i=0; i<heightmapPixelArea; i++) {
					const Sint16 val = heightMap.get()[i];
					minHMap = std::min(minHMap, val);
					maxHMap = std::max(maxHMap, val);
					// store then increment pointer
					(*pHeightMap) = val;
					++pHeightMap;
				}
				assert(pHeightMap == &m_heightMap[heightmapPixelArea]);
				//Output("minHMap = (%hd), maxHMap = (%hd)\n", minHMap, maxHMap);
				break;
			}

			case 1: {
				Uint16 v;
				// XXX x and y reversed from above *sigh*
				bufread_or_die(&v, 2, 1, databuf); m_heightMapSizeY = v;
				bufread_or_die(&v, 2, 1, databuf); m_heightMapSizeX = v;
				const Uint32 heightmapPixelArea = (m_heightMapSizeX * m_heightMapSizeY);

				// read height scaling and min height which are doubles
				double te;
				bufread_or_die(&te, 8, 1, databuf);
				m_heightScaling = te;
				bufread_or_die(&te, 8, 1, databuf);
				m_minh = te;

				std::unique_ptr<Uint16[]> heightMapScaled(new Uint16[heightmapPixelArea]);
				bufread_or_die(heightMapScaled.get(), sizeof(Uint16), heightmapPixelArea, databuf);
				m_heightMap.reset(new double[heightmapPixelArea]);
				double *pHeightMap = m_heightMap.get();
				for(Uint32 i=0; i<heightmapPixelArea; i++) {
					const Uint16 val = heightMapScaled[i];
					minHMapScld = std::min(minHMapScld, val);
					maxHMapScld = std::max(maxHMapScld, val);
					// store then increment pointer
					(*pHeightMap) = val;
					++pHeightMap;
				}
				assert(pHeightMap == &m_heightMap[heightmapPixelArea]);
				//Output("minHMapScld = (%hu), maxHMapScld = (%hu)\n", minHMapScld, maxHMapScld);
				break;
			}

			default:
				assert(0);
		}

	}

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

	m_sealevel = Clamp(body->GetVolatileLiquid(), 0.0, 1.0);
	m_icyness  = Clamp(body->GetVolatileIces(), 0.0, 1.0);
	m_volcanic = Clamp(body->GetVolcanicity(), 0.0, 1.0); // height scales with volcanicity as well
	m_surfaceEffects = 0;

	const double rad = m_minBody.m_radius;

	// calculate max height
	if (!body->GetHeightMapFilename().empty() && body->GetHeightMapFractal() > 1){ // if scaled heightmap
		m_maxHeightInMeters = 1.1*pow(2.0, 16.0)*m_heightScaling; // no min height required as it's added to radius in lua
	}else {
		// max mountain height for earth-like planet (same mass, radius)
		m_maxHeightInMeters = std::max(100.0, (9000.0*rad*rad*(m_volcanic+0.5)) / (body->GetMass() * 6.64e-12));
		m_maxHeightInMeters = std::min(rad, m_maxHeightInMeters);		// small asteroid case
	}
	// and then in sphere normalized jizz
	m_maxHeight = m_maxHeightInMeters / rad;
	//Output("%s: max terrain height: %fm [%f]\n", m_minBody.name.c_str(), m_maxHeightInMeters, m_maxHeight);
	m_invMaxHeight = 1.0 / m_maxHeight;
	m_planetRadius = rad;
	m_planetEarthRadii = rad / EARTH_RADIUS;

	// Pick some colors, mainly reds and greens
	for (int i=0; i<int(COUNTOF(m_entropy)); i++) m_entropy[i] = m_rand.Double();
	for (int i=0; i<int(COUNTOF(m_rockColor)); i++) {
		double r,g,b;
		r = m_rand.Double(0.3, 1.0);
		g = m_rand.Double(0.3, r);
		b = m_rand.Double(0.3, g);
		r = std::max(b, r * body->GetMetallicity());
		g = std::max(b, g * body->GetMetallicity());
		m_rockColor[i] = vector3d(r, g, b);
	}

	// Pick some darker colours mainly reds and greens
	for (int i=0; i<int(COUNTOF(m_entropy)); i++) m_entropy[i] = m_rand.Double();
	for (int i=0; i<int(COUNTOF(m_darkrockColor)); i++) {
		double r,g,b;
		r = m_rand.Double(0.05, 0.3);
		g = m_rand.Double(0.05, r);
		b = m_rand.Double(0.05, g);
		r = std::max(b, r * body->GetMetallicity());
		g = std::max(b, g * body->GetMetallicity());
		m_darkrockColor[i] = vector3d(r, g, b);
	}

	// grey colours, in case you simply must have a grey colour on a world with high metallicity
	for (int i=0; i<int(COUNTOF(m_entropy)); i++) m_entropy[i] = m_rand.Double();
	for (int i=0; i<int(COUNTOF(m_greyrockColor)); i++) {
		double g;
		g = m_rand.Double(0.3, 0.9);
		m_greyrockColor[i] = vector3d(g, g, g);
	}

	// Pick some plant colours, mainly greens
	// TODO take star class into account
	for (int i=0; i<int(COUNTOF(m_entropy)); i++) m_entropy[i] = m_rand.Double();
	for (int i=0; i<int(COUNTOF(m_plantColor)); i++) {
		double r,g,b;
		g = m_rand.Double(0.3, 1.0);
		r = m_rand.Double(0.3, g);
		b = m_rand.Double(0.2, r);
		g = std::max(r, g * body->GetLife());
		b *= (1.0-body->GetLife());
		m_plantColor[i] = vector3d(r, g, b);
	}

	// Pick some darker plant colours mainly greens
	// TODO take star class into account
	for (int i=0; i<int(COUNTOF(m_entropy)); i++) m_entropy[i] = m_rand.Double();
	for (int i=0; i<int(COUNTOF(m_darkplantColor)); i++) {
		double r,g,b;
		g = m_rand.Double(0.05, 0.3);
		r = m_rand.Double(0.00, g);
		b = m_rand.Double(0.00, r);
		g = std::max(r, g * body->GetLife());
		b *= (1.0-body->GetLife());
		m_darkplantColor[i] = vector3d(r, g, b);
	}

	// Pick some sand colours, mainly yellow
	// TODO let some planetary value scale this colour
	for (int i=0; i<int(COUNTOF(m_entropy)); i++) m_entropy[i] = m_rand.Double();
	for (int i=0; i<int(COUNTOF(m_sandColor)); i++) {
		double r,g,b;
		r = m_rand.Double(0.6, 1.0);
		g = m_rand.Double(0.6, r);
		//b = m_rand.Double(0.0, g/2.0);
		b = 0;
		m_sandColor[i] = vector3d(r, g, b);
	}

	// Pick some darker sand colours mainly yellow
	// TODO let some planetary value scale this colour
	for (int i=0; i<int(COUNTOF(m_entropy)); i++) m_entropy[i] = m_rand.Double();
	for (int i=0; i<int(COUNTOF(m_darksandColor)); i++) {
		double r,g,b;
		r = m_rand.Double(0.05, 0.6);
		g = m_rand.Double(0.00, r);
		//b = m_rand.Double(0.00, g/2.0);
		b = 0;
		m_darksandColor[i] = vector3d(r, g, b);
	}

	// Pick some dirt colours, mainly red/brown
	// TODO let some planetary value scale this colour
	for (int i=0; i<int(COUNTOF(m_entropy)); i++) m_entropy[i] = m_rand.Double();
	for (int i=0; i<int(COUNTOF(m_dirtColor)); i++) {
		double r,g,b;
		r = m_rand.Double(0.3, 0.7);
		g = m_rand.Double(r-0.1, 0.75);
		b = m_rand.Double(0.0, r/2.0);
		m_dirtColor[i] = vector3d(r, g, b);
	}

	// Pick some darker dirt colours mainly red/brown
	// TODO let some planetary value scale this colour
	for (int i=0; i<int(COUNTOF(m_entropy)); i++) m_entropy[i] = m_rand.Double();
	for (int i=0; i<int(COUNTOF(m_darkdirtColor)); i++) {
		double r,g,b;
		r = m_rand.Double(0.05, 0.3);
		g = m_rand.Double(r-0.05, 0.35);
		b = m_rand.Double(0.0, r/2.0);
		m_darkdirtColor[i] = vector3d(r, g, b);
	}

	// These are used for gas giant colours, they are more m_random and *should* really use volatileGasses - TODO
	for (int i=0; i<int(COUNTOF(m_entropy)); i++) m_entropy[i] = m_rand.Double();
	for (int i=0; i<int(COUNTOF(m_gglightColor)); i++) {
		double r,g,b;
		r = m_rand.Double(0.0, 0.5);
		g = m_rand.Double(0.0, 0.5);
		b = m_rand.Double(0.0, 0.5);
		m_gglightColor[i] = vector3d(r, g, b);
	}
	//darker gas giant colours, more reds and greens
	for (int i=0; i<int(COUNTOF(m_entropy)); i++) m_entropy[i] = m_rand.Double();
	for (int i=0; i<int(COUNTOF(m_ggdarkColor)); i++) {
		double r,g,b;
		r = m_rand.Double(0.0, 0.3);
		g = m_rand.Double(0.0, r);
		b = m_rand.Double(0.0, std::min(r, g));
		m_ggdarkColor[i] = vector3d(r, g, b);
	}
}

Terrain::~Terrain()
{
}


/**
 * Feature width means roughly one perlin noise blob or grain.
 * This will end up being one hill, mountain or continent, roughly.
 */
void Terrain::SetFracDef(const unsigned int index, const double featureHeightMeters, const double featureWidthMeters, const double smallestOctaveMeters)
{
	assert(index>=0 && index<MAX_FRACDEFS);
	// feature
	m_fracdef[index].amplitude = featureHeightMeters / (m_maxHeight * m_planetRadius);
	m_fracdef[index].frequency = m_planetRadius / featureWidthMeters;
	m_fracdef[index].octaves = std::max(1, int(ceil(log(featureWidthMeters / smallestOctaveMeters) / log(2.0))));
	m_fracdef[index].lacunarity = 2.0;
	//Output("%d octaves\n", m_fracdef[index].octaves); //print
}

void Terrain::DebugDump() const
{
	Output("Terrain state dump:\n");
	Output("  Height fractal: %s\n", GetHeightFractalName());
	Output("  Color fractal: %s\n", GetColorFractalName());
	Output("  Detail: fracnum %d  fracmult %f  textures %s\n", m_fracnum, m_fracmult, textures ? "true" : "false");
	Output("  Config: DetailPlanets %d   FractalMultiple %d  Textures  %d\n", Pi::config->Int("DetailPlanets"), Pi::config->Int("FractalMultiple"), Pi::config->Int("Textures"));
	Output("  Seed: %d\n", m_seed);
	Output("  Body: %s [%d,%d,%d,%u,%u]\n", m_minBody.m_name.c_str(), m_minBody.m_path.sectorX, m_minBody.m_path.sectorY, m_minBody.m_path.sectorZ, m_minBody.m_path.systemIndex, m_minBody.m_path.bodyIndex);
	Output("  Aspect Ratio: %g\n", m_minBody.m_aspectRatio);
	Output("  Fracdefs:\n");
	for (int i = 0; i < 10; i++) {
		Output("    %d: amp %f  freq %f  lac %f  oct %d\n", i, m_fracdef[i].amplitude, m_fracdef[i].frequency, m_fracdef[i].lacunarity, m_fracdef[i].octaves);
	}
}
