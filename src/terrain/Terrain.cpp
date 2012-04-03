#include "Terrain.h"
#include "perlin.h"
#include "Pi.h"
#include "FileSystem.h"

// static instancer. selects the best height and color classes for the body
Terrain *Terrain::InstanceTerrain(const SBody *body)
{
	// special case for heightmaps
	// XXX this is terrible but will do for now until we get a unified
	// heightmap setup. if you add another height fractal, remember to change
	// the check in CustomSystem::l_height_map
	if (body->heightMapFilename) {
		const GeneratorInstancer choices[] = {
			InstanceGenerator<TerrainHeightMapped,TerrainColorEarthLike>,
			InstanceGenerator<TerrainHeightMapped2,TerrainColorRock>
		};
		assert(body->heightMapFractal < 2);
		return choices[body->heightMapFractal](body);
	}

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
				InstanceGenerator<TerrainHeightFlat,TerrainColorStarG>
			};
			gi = choices[rand.Int32(2)];
			break;
		}

		case SBody::TYPE_PLANET_GAS_GIANT: {
			const GeneratorInstancer choices[] = {
				InstanceGenerator<TerrainHeightFlat,TerrainColorGGJupiter>,
				InstanceGenerator<TerrainHeightFlat,TerrainColorGGSaturn>,
				InstanceGenerator<TerrainHeightFlat,TerrainColorGGSaturn2>,
				InstanceGenerator<TerrainHeightFlat,TerrainColorGGNeptune>,
				InstanceGenerator<TerrainHeightFlat,TerrainColorGGNeptune2>,
				InstanceGenerator<TerrainHeightFlat,TerrainColorGGUranus>,
				InstanceGenerator<TerrainHeightFlat,TerrainColorGGSaturn>
			};
			gi = choices[rand.Int32(7)];
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

static size_t bufread_or_die(void *ptr, size_t size, size_t nmemb, ByteRange &buf)
{
	size_t read_count = buf.read(reinterpret_cast<char*>(ptr), size, nmemb);
	if (read_count < nmemb) {
		fprintf(stderr, "Error: failed to read file (truncated)\n");
		abort();
	}
	return read_count;
}

Terrain::Terrain(const SBody *body) : m_body(body), m_rand(body->seed), m_heightMap(0), m_heightMapScaled(0), m_heightScaling(0), m_minh(0) {

	// load the heightmap
	if (m_body->heightMapFilename) {
		RefCountedPtr<FileSystem::FileData> fdata = FileSystem::gameDataFiles.ReadFile(m_body->heightMapFilename);
		if (!fdata) {
			fprintf(stderr, "Error: could not open file '%s'\n", m_body->heightMapFilename);
			abort();
		}

		ByteRange databuf = fdata->AsByteRange();

		// read size!
		Uint16 v;

		// XXX unify heightmap types
		switch (m_body->heightMapFractal) {
			case 0: {
				bufread_or_die(&v, 2, 1, databuf); m_heightMapSizeX = v;
				bufread_or_die(&v, 2, 1, databuf); m_heightMapSizeY = v;

				m_heightMap = new Sint16[m_heightMapSizeX * m_heightMapSizeY];
				bufread_or_die(m_heightMap, sizeof(Sint16), m_heightMapSizeX * m_heightMapSizeY, databuf);
				break;
			}

			case 1: {
				// XXX x and y reversed from above *sigh*
				bufread_or_die(&v, 2, 1, databuf); m_heightMapSizeY = v;
				bufread_or_die(&v, 2, 1, databuf); m_heightMapSizeX = v;

				// read height scaling and min height which are doubles
				double te;
				bufread_or_die(&te, 8, 1, databuf);
				m_heightScaling = te;
				bufread_or_die(&te, 8, 1, databuf);
				m_minh = te;

				m_heightMapScaled = new Uint16[m_heightMapSizeX * m_heightMapSizeY];
				bufread_or_die(m_heightMapScaled, sizeof(Uint16), m_heightMapSizeX * m_heightMapSizeY, databuf);

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

Terrain::~Terrain()
{
	if (m_heightMap)
		delete [] m_heightMap;
	if (m_heightMapScaled)
		delete [] m_heightMapScaled;
}


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
