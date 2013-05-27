// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"
#include "perlin.h"
#include "Pi.h"
#include "FileSystem.h"

//static 
const Terrain::GeneratorInstancer Terrain::sc_GeneratedTerrain[] = {
	Terrain::InstanceGenerator<TerrainHeightMapped,TerrainColorEarthLike>,
	Terrain::InstanceGenerator<TerrainHeightMapped2,TerrainColorRock2>,
	Terrain::InstanceGenerator<TerrainHeightEllipsoid,TerrainColorStarBrownDwarf>,
	Terrain::InstanceGenerator<TerrainHeightEllipsoid,TerrainColorStarWhiteDwarf>,
	Terrain::InstanceGenerator<TerrainHeightEllipsoid,TerrainColorStarM>,
	Terrain::InstanceGenerator<TerrainHeightEllipsoid,TerrainColorStarK>,
	Terrain::InstanceGenerator<TerrainHeightEllipsoid,TerrainColorStarG>,
	Terrain::InstanceGenerator<TerrainHeightEllipsoid,TerrainColorSolid>,
	Terrain::InstanceGenerator<TerrainHeightFlat,TerrainColorGGJupiter>,
	Terrain::InstanceGenerator<TerrainHeightFlat,TerrainColorGGSaturn>,
	Terrain::InstanceGenerator<TerrainHeightFlat,TerrainColorGGSaturn2>,
	Terrain::InstanceGenerator<TerrainHeightFlat,TerrainColorGGNeptune>,
	Terrain::InstanceGenerator<TerrainHeightFlat,TerrainColorGGNeptune2>,
	Terrain::InstanceGenerator<TerrainHeightFlat,TerrainColorGGUranus>,
	Terrain::InstanceGenerator<TerrainHeightAsteroid,TerrainColorAsteroid>,
	Terrain::InstanceGenerator<TerrainHeightAsteroid2,TerrainColorAsteroid>,
	Terrain::InstanceGenerator<TerrainHeightAsteroid3,TerrainColorAsteroid>,
	Terrain::InstanceGenerator<TerrainHeightAsteroid4,TerrainColorAsteroid>,
	Terrain::InstanceGenerator<TerrainHeightAsteroid,TerrainColorRock>,
	Terrain::InstanceGenerator<TerrainHeightAsteroid2,TerrainColorBandedRock>,
	Terrain::InstanceGenerator<TerrainHeightAsteroid3,TerrainColorRock>,
	Terrain::InstanceGenerator<TerrainHeightAsteroid4,TerrainColorBandedRock>,
	Terrain::InstanceGenerator<TerrainHeightHillsRidged,TerrainColorEarthLike>,
	Terrain::InstanceGenerator<TerrainHeightHillsRivers,TerrainColorEarthLike>,
	Terrain::InstanceGenerator<TerrainHeightHillsDunes,TerrainColorEarthLike>,
	Terrain::InstanceGenerator<TerrainHeightMountainsRidged,TerrainColorEarthLike>,
	Terrain::InstanceGenerator<TerrainHeightMountainsNormal,TerrainColorEarthLike>,
	Terrain::InstanceGenerator<TerrainHeightMountainsRivers,TerrainColorEarthLike>,
	Terrain::InstanceGenerator<TerrainHeightMountainsVolcano,TerrainColorEarthLike>,
	Terrain::InstanceGenerator<TerrainHeightMountainsRiversVolcano,TerrainColorEarthLike>,
	Terrain::InstanceGenerator<TerrainHeightHillsRidged,TerrainColorDesert>,
	Terrain::InstanceGenerator<TerrainHeightHillsRivers,TerrainColorDesert>,
	Terrain::InstanceGenerator<TerrainHeightHillsDunes,TerrainColorDesert>,
	Terrain::InstanceGenerator<TerrainHeightMountainsRidged,TerrainColorDesert>,
	Terrain::InstanceGenerator<TerrainHeightMountainsNormal,TerrainColorDesert>,
	Terrain::InstanceGenerator<TerrainHeightMountainsRivers,TerrainColorDesert>,
	Terrain::InstanceGenerator<TerrainHeightMountainsVolcano,TerrainColorDesert>,
	Terrain::InstanceGenerator<TerrainHeightMountainsRiversVolcano,TerrainColorDesert>,
	Terrain::InstanceGenerator<TerrainHeightBarrenRock,TerrainColorDesert>,
	Terrain::InstanceGenerator<TerrainHeightBarrenRock2,TerrainColorDesert>,
	Terrain::InstanceGenerator<TerrainHeightHillsRidged,TerrainColorTFGood>,
	Terrain::InstanceGenerator<TerrainHeightHillsRivers,TerrainColorTFGood>,
	Terrain::InstanceGenerator<TerrainHeightHillsDunes,TerrainColorTFGood>,
	Terrain::InstanceGenerator<TerrainHeightHillsNormal,TerrainColorTFGood>,
	Terrain::InstanceGenerator<TerrainHeightMountainsNormal,TerrainColorTFGood>,
	Terrain::InstanceGenerator<TerrainHeightMountainsRidged,TerrainColorTFGood>,
	Terrain::InstanceGenerator<TerrainHeightMountainsVolcano,TerrainColorTFGood>,
	Terrain::InstanceGenerator<TerrainHeightMountainsRiversVolcano,TerrainColorTFGood>,
	Terrain::InstanceGenerator<TerrainHeightMountainsRivers,TerrainColorTFGood>,
	Terrain::InstanceGenerator<TerrainHeightRuggedDesert,TerrainColorTFGood>,
	Terrain::InstanceGenerator<TerrainHeightBarrenRock,TerrainColorTFGood>,
	Terrain::InstanceGenerator<TerrainHeightBarrenRock2,TerrainColorTFGood>,
	Terrain::InstanceGenerator<TerrainHeightHillsRidged,TerrainColorIce>,
	Terrain::InstanceGenerator<TerrainHeightHillsRivers,TerrainColorIce>,
	Terrain::InstanceGenerator<TerrainHeightHillsDunes,TerrainColorIce>,
	Terrain::InstanceGenerator<TerrainHeightHillsNormal,TerrainColorIce>,
	Terrain::InstanceGenerator<TerrainHeightMountainsNormal,TerrainColorIce>,
	Terrain::InstanceGenerator<TerrainHeightMountainsRidged,TerrainColorIce>,
	Terrain::InstanceGenerator<TerrainHeightMountainsVolcano,TerrainColorIce>,
	Terrain::InstanceGenerator<TerrainHeightMountainsRiversVolcano,TerrainColorIce>,
	Terrain::InstanceGenerator<TerrainHeightMountainsRivers,TerrainColorIce>,
	Terrain::InstanceGenerator<TerrainHeightRuggedDesert,TerrainColorIce>,
	Terrain::InstanceGenerator<TerrainHeightBarrenRock,TerrainColorIce>,
	Terrain::InstanceGenerator<TerrainHeightBarrenRock2,TerrainColorIce>,
	Terrain::InstanceGenerator<TerrainHeightBarrenRock3,TerrainColorIce>,
	Terrain::InstanceGenerator<TerrainHeightHillsRidged,TerrainColorTFPoor>,
	Terrain::InstanceGenerator<TerrainHeightHillsRivers,TerrainColorTFPoor>,
	Terrain::InstanceGenerator<TerrainHeightHillsDunes,TerrainColorTFPoor>,
	Terrain::InstanceGenerator<TerrainHeightHillsNormal,TerrainColorTFPoor>,
	Terrain::InstanceGenerator<TerrainHeightMountainsNormal,TerrainColorTFPoor>,
	Terrain::InstanceGenerator<TerrainHeightMountainsRidged,TerrainColorTFPoor>,
	Terrain::InstanceGenerator<TerrainHeightMountainsVolcano,TerrainColorTFPoor>,
	Terrain::InstanceGenerator<TerrainHeightMountainsRiversVolcano,TerrainColorTFPoor>,
	Terrain::InstanceGenerator<TerrainHeightMountainsRivers,TerrainColorTFPoor>,
	Terrain::InstanceGenerator<TerrainHeightRuggedDesert,TerrainColorTFPoor>,
	Terrain::InstanceGenerator<TerrainHeightBarrenRock,TerrainColorTFPoor>,
	Terrain::InstanceGenerator<TerrainHeightBarrenRock2,TerrainColorTFPoor>,
	Terrain::InstanceGenerator<TerrainHeightBarrenRock3,TerrainColorTFPoor>,
	Terrain::InstanceGenerator<TerrainHeightWaterSolid,TerrainColorDesert>,
	Terrain::InstanceGenerator<TerrainHeightRuggedDesert,TerrainColorDesert>,
	Terrain::InstanceGenerator<TerrainHeightRuggedLava,TerrainColorDesert>,
	Terrain::InstanceGenerator<TerrainHeightHillsCraters,TerrainColorIce>,
	Terrain::InstanceGenerator<TerrainHeightMountainsCraters,TerrainColorIce>,
	Terrain::InstanceGenerator<TerrainHeightWaterSolid,TerrainColorIce>,
	Terrain::InstanceGenerator<TerrainHeightWaterSolidCanyons,TerrainColorIce>,
	Terrain::InstanceGenerator<TerrainHeightRuggedLava,TerrainColorTFGood>,
	Terrain::InstanceGenerator<TerrainHeightRuggedLava,TerrainColorTFPoor>,
	Terrain::InstanceGenerator<TerrainHeightRuggedLava,TerrainColorVolcanic>,
	Terrain::InstanceGenerator<TerrainHeightWaterSolid,TerrainColorTFPoor>,
	Terrain::InstanceGenerator<TerrainHeightHillsNormal,TerrainColorRock>,
	Terrain::InstanceGenerator<TerrainHeightMountainsNormal,TerrainColorRock>,
	Terrain::InstanceGenerator<TerrainHeightRuggedDesert,TerrainColorRock>,
	Terrain::InstanceGenerator<TerrainHeightBarrenRock,TerrainColorRock>,
	Terrain::InstanceGenerator<TerrainHeightBarrenRock2,TerrainColorRock>,
	Terrain::InstanceGenerator<TerrainHeightBarrenRock3,TerrainColorRock>,
	Terrain::InstanceGenerator<TerrainHeightHillsCraters2,TerrainColorRock>,
	Terrain::InstanceGenerator<TerrainHeightMountainsCraters2,TerrainColorRock>,
	Terrain::InstanceGenerator<TerrainHeightFlat,TerrainColorSolid>
};

enum EGeneratedTerrains {
	eTerrainHeightMappedTerrainColorEarthLike=0,
	eTerrainHeightMapped2TerrainColorRock2,
	eTerrainHeightEllipsoidTerrainColorStarBrownDwarf,
	eTerrainHeightEllipsoidTerrainColorStarWhiteDwarf,
	eTerrainHeightEllipsoidTerrainColorStarM,
	eTerrainHeightEllipsoidTerrainColorStarK,
	eTerrainHeightEllipsoidTerrainColorStarG,
	eTerrainHeightEllipsoidTerrainColorSolid,
	eTerrainHeightFlatTerrainColorGGJupiter,
	eTerrainHeightFlatTerrainColorGGSaturn,
	eTerrainHeightFlatTerrainColorGGSaturn2,
	eTerrainHeightFlatTerrainColorGGNeptune,
	eTerrainHeightFlatTerrainColorGGNeptune2,
	eTerrainHeightFlatTerrainColorGGUranus,
	eTerrainHeightAsteroidTerrainColorAsteroid,
	eTerrainHeightAsteroid2TerrainColorAsteroid,
	eTerrainHeightAsteroid3TerrainColorAsteroid,
	eTerrainHeightAsteroid4TerrainColorAsteroid,
	eTerrainHeightAsteroidTerrainColorRock,
	eTerrainHeightAsteroid2TerrainColorBandedRock,
	eTerrainHeightAsteroid3TerrainColorRock,
	eTerrainHeightAsteroid4TerrainColorBandedRock,
	eTerrainHeightHillsRidgedTerrainColorEarthLike,
	eTerrainHeightHillsRiversTerrainColorEarthLike,
	eTerrainHeightHillsDunesTerrainColorEarthLike,
	eTerrainHeightMountainsRidgedTerrainColorEarthLike,
	eTerrainHeightMountainsNormalTerrainColorEarthLike,
	eTerrainHeightMountainsRiversTerrainColorEarthLike,
	eTerrainHeightMountainsVolcanoTerrainColorEarthLike,
	eTerrainHeightMountainsRiversVolcanoTerrainColorEarthLike,
	eTerrainHeightHillsRidgedTerrainColorDesert,
	eTerrainHeightHillsRiversTerrainColorDesert,
	eTerrainHeightHillsDunesTerrainColorDesert,
	eTerrainHeightMountainsRidgedTerrainColorDesert,
	eTerrainHeightMountainsNormalTerrainColorDesert,
	eTerrainHeightMountainsRiversTerrainColorDesert,
	eTerrainHeightMountainsVolcanoTerrainColorDesert,
	eTerrainHeightMountainsRiversVolcanoTerrainColorDesert,
	eTerrainHeightBarrenRockTerrainColorDesert,
	eTerrainHeightBarrenRock2TerrainColorDesert,
	eTerrainHeightHillsRidgedTerrainColorTFGood,
	eTerrainHeightHillsRiversTerrainColorTFGood,
	eTerrainHeightHillsDunesTerrainColorTFGood,
	eTerrainHeightHillsNormalTerrainColorTFGood,
	eTerrainHeightMountainsNormalTerrainColorTFGood,
	eTerrainHeightMountainsRidgedTerrainColorTFGood,
	eTerrainHeightMountainsVolcanoTerrainColorTFGood,
	eTerrainHeightMountainsRiversVolcanoTerrainColorTFGood,
	eTerrainHeightMountainsRiversTerrainColorTFGood,
	eTerrainHeightRuggedDesertTerrainColorTFGood,
	eTerrainHeightBarrenRockTerrainColorTFGood,
	eTerrainHeightBarrenRock2TerrainColorTFGood,
	eTerrainHeightHillsRidgedTerrainColorIce,
	eTerrainHeightHillsRiversTerrainColorIce,
	eTerrainHeightHillsDunesTerrainColorIce,
	eTerrainHeightHillsNormalTerrainColorIce,
	eTerrainHeightMountainsNormalTerrainColorIce,
	eTerrainHeightMountainsRidgedTerrainColorIce,
	eTerrainHeightMountainsVolcanoTerrainColorIce,
	eTerrainHeightMountainsRiversVolcanoTerrainColorIce,
	eTerrainHeightMountainsRiversTerrainColorIce,
	eTerrainHeightRuggedDesertTerrainColorIce,
	eTerrainHeightBarrenRockTerrainColorIce,
	eTerrainHeightBarrenRock2TerrainColorIce,
	eTerrainHeightBarrenRock3TerrainColorIce,
	eTerrainHeightHillsRidgedTerrainColorTFPoor,
	eTerrainHeightHillsRiversTerrainColorTFPoor,
	eTerrainHeightHillsDunesTerrainColorTFPoor,
	eTerrainHeightHillsNormalTerrainColorTFPoor,
	eTerrainHeightMountainsNormalTerrainColorTFPoor,
	eTerrainHeightMountainsRidgedTerrainColorTFPoor,
	eTerrainHeightMountainsVolcanoTerrainColorTFPoor,
	eTerrainHeightMountainsRiversVolcanoTerrainColorTFPoor,
	eTerrainHeightMountainsRiversTerrainColorTFPoor,
	eTerrainHeightRuggedDesertTerrainColorTFPoor,
	eTerrainHeightBarrenRockTerrainColorTFPoor,
	eTerrainHeightBarrenRock2TerrainColorTFPoor,
	eTerrainHeightBarrenRock3TerrainColorTFPoor,
	eTerrainHeightWaterSolidTerrainColorDesert,
	eTerrainHeightRuggedDesertTerrainColorDesert,
	eTerrainHeightRuggedLavaTerrainColorDesert,
	eTerrainHeightHillsCratersTerrainColorIce,
	eTerrainHeightMountainsCratersTerrainColorIce,
	eTerrainHeightWaterSolidTerrainColorIce,
	eTerrainHeightWaterSolidCanyonsTerrainColorIce,
	eTerrainHeightRuggedLavaTerrainColorTFGood,
	eTerrainHeightRuggedLavaTerrainColorTFPoor,
	eTerrainHeightRuggedLavaTerrainColorVolcanic,
	eTerrainHeightWaterSolidTerrainColorTFPoor,
	eTerrainHeightHillsNormalTerrainColorRock,
	eTerrainHeightMountainsNormalTerrainColorRock,
	eTerrainHeightRuggedDesertTerrainColorRock,
	eTerrainHeightBarrenRockTerrainColorRock,
	eTerrainHeightBarrenRock2TerrainColorRock,
	eTerrainHeightBarrenRock3TerrainColorRock,
	eTerrainHeightHillsCraters2TerrainColorRock,
	eTerrainHeightMountainsCraters2TerrainColorRock,
	eTerrainHeightFlatTerrainColorSolid
};

// static instancer. selects the best height and color classes for the body
Terrain *Terrain::InstanceTerrain(const SystemBody *body)
{
	uint32_t gs = 0;

	// special case for heightmaps
	// XXX this is terrible but will do for now until we get a unified
	// heightmap setup. if you add another height fractal, remember to change
	// the check in CustomSystem::l_height_map
	if (body->heightMapFilename) {
		static const uint32_t choices[] = {
			eTerrainHeightMappedTerrainColorEarthLike,
			eTerrainHeightMapped2TerrainColorRock2
		};
		assert(body->heightMapFractal < COUNTOF(choices));
		gs = choices[body->heightMapFractal];
		return sc_GeneratedTerrain[gs](body,gs);
	}

	Random rand(body->seed);
	switch (body->type) {

		case SystemBody::TYPE_BROWN_DWARF:
			gs = eTerrainHeightEllipsoidTerrainColorStarBrownDwarf;
			break;

		case SystemBody::TYPE_WHITE_DWARF:
			gs = eTerrainHeightEllipsoidTerrainColorStarWhiteDwarf;
			break;

		case SystemBody::TYPE_STAR_M:
		case SystemBody::TYPE_STAR_M_GIANT:
		case SystemBody::TYPE_STAR_M_SUPER_GIANT:
		case SystemBody::TYPE_STAR_M_HYPER_GIANT: {
			static const uint32_t choices[] = {
				eTerrainHeightEllipsoidTerrainColorStarM,
				eTerrainHeightEllipsoidTerrainColorStarM,
				eTerrainHeightEllipsoidTerrainColorStarK,
				eTerrainHeightEllipsoidTerrainColorStarG
			};
			gs = choices[rand.Int32(COUNTOF(choices))];
			break;
		}

		case SystemBody::TYPE_STAR_K:
		case SystemBody::TYPE_STAR_K_GIANT:
		case SystemBody::TYPE_STAR_K_SUPER_GIANT:
		case SystemBody::TYPE_STAR_K_HYPER_GIANT: {
			static const uint32_t choices[] = {
				eTerrainHeightEllipsoidTerrainColorStarM,
				eTerrainHeightEllipsoidTerrainColorStarK,
				eTerrainHeightEllipsoidTerrainColorStarK,
				eTerrainHeightEllipsoidTerrainColorStarG
			};
			gs = choices[rand.Int32(COUNTOF(choices))];
			break;
		}

		case SystemBody::TYPE_STAR_G:
		case SystemBody::TYPE_STAR_G_GIANT:
		case SystemBody::TYPE_STAR_G_SUPER_GIANT:
		case SystemBody::TYPE_STAR_G_HYPER_GIANT: {
			static const uint32_t choices[] = {
				eTerrainHeightEllipsoidTerrainColorStarWhiteDwarf,
				eTerrainHeightEllipsoidTerrainColorStarG
			};
			gs = choices[rand.Int32(COUNTOF(choices))];
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
			gs = eTerrainHeightEllipsoidTerrainColorSolid;
		break;

		case SystemBody::TYPE_PLANET_GAS_GIANT: {
			static const uint32_t choices[] = {
				eTerrainHeightFlatTerrainColorGGJupiter,
				eTerrainHeightFlatTerrainColorGGSaturn,
				eTerrainHeightFlatTerrainColorGGSaturn2,
				eTerrainHeightFlatTerrainColorGGNeptune,
				eTerrainHeightFlatTerrainColorGGNeptune2,
				eTerrainHeightFlatTerrainColorGGUranus,
				eTerrainHeightFlatTerrainColorGGSaturn
			};
			gs = choices[rand.Int32(COUNTOF(choices))];
			break;
		}

		case SystemBody::TYPE_PLANET_ASTEROID: {
			static const uint32_t choices[] = {
				eTerrainHeightAsteroidTerrainColorAsteroid,
				eTerrainHeightAsteroid2TerrainColorAsteroid,
				eTerrainHeightAsteroid3TerrainColorAsteroid,
				eTerrainHeightAsteroid4TerrainColorAsteroid,
				eTerrainHeightAsteroidTerrainColorRock,
				eTerrainHeightAsteroid2TerrainColorBandedRock,
				eTerrainHeightAsteroid3TerrainColorRock,
				eTerrainHeightAsteroid4TerrainColorBandedRock
			};
			gs = choices[rand.Int32(COUNTOF(choices))];
			break;
		}

		case SystemBody::TYPE_PLANET_TERRESTRIAL: {

			//Over-ride:
			//gs = eTerrainHeightAsteroid3TerrainColorRock;
			//break;
			// Earth-like world

			if ((body->m_life > fixed(7,10)) && (body->m_volatileGas > fixed(2,10))) {
				// There would be no life on the surface without atmosphere

				if (body->averageTemp > 240) {
					static const uint32_t choices[] = {
						eTerrainHeightHillsRidgedTerrainColorEarthLike,
						eTerrainHeightHillsRiversTerrainColorEarthLike,
						eTerrainHeightHillsDunesTerrainColorEarthLike,
						eTerrainHeightMountainsRidgedTerrainColorEarthLike,
						eTerrainHeightMountainsNormalTerrainColorEarthLike,
						eTerrainHeightMountainsRiversTerrainColorEarthLike,
						eTerrainHeightMountainsVolcanoTerrainColorEarthLike,
						eTerrainHeightMountainsRiversVolcanoTerrainColorEarthLike
					};
					gs = choices[rand.Int32(COUNTOF(choices))];
					break;
				}

				static const uint32_t choices[] = {
					eTerrainHeightHillsRidgedTerrainColorDesert,
					eTerrainHeightHillsRiversTerrainColorDesert,
					eTerrainHeightHillsDunesTerrainColorDesert,
					eTerrainHeightMountainsRidgedTerrainColorDesert,
					eTerrainHeightMountainsNormalTerrainColorDesert,
					eTerrainHeightMountainsRiversTerrainColorDesert,
					eTerrainHeightMountainsVolcanoTerrainColorDesert,
					eTerrainHeightMountainsRiversVolcanoTerrainColorDesert,
					eTerrainHeightBarrenRockTerrainColorDesert,
					eTerrainHeightBarrenRock2TerrainColorDesert//,
					//eTerrainHeightBarrenRock3TerrainColorTFGood
				};
				gs = choices[rand.Int32(COUNTOF(choices))];
				break;
			}

			// Harsh, habitable world
			if ((body->m_volatileGas > fixed(2,10)) && (body->m_life > fixed(4,10)) ) {

				if (body->averageTemp > 240) {
					static const uint32_t choices[] = {
						eTerrainHeightHillsRidgedTerrainColorTFGood,
						eTerrainHeightHillsRiversTerrainColorTFGood,
						eTerrainHeightHillsDunesTerrainColorTFGood,
						eTerrainHeightHillsNormalTerrainColorTFGood,
						eTerrainHeightMountainsNormalTerrainColorTFGood,
						eTerrainHeightMountainsRidgedTerrainColorTFGood,
						eTerrainHeightMountainsVolcanoTerrainColorTFGood,
						eTerrainHeightMountainsRiversVolcanoTerrainColorTFGood,
						eTerrainHeightMountainsRiversTerrainColorTFGood,
						eTerrainHeightRuggedDesertTerrainColorTFGood,
						eTerrainHeightBarrenRockTerrainColorTFGood,
						eTerrainHeightBarrenRock2TerrainColorTFGood
						//eTerrainHeightBarrenRock3TerrainColorTFGood
					};
					gs = choices[rand.Int32(COUNTOF(choices))];
					break;
				}

				static const uint32_t choices[] = {
					eTerrainHeightHillsRidgedTerrainColorIce,
					eTerrainHeightHillsRiversTerrainColorIce,
					eTerrainHeightHillsDunesTerrainColorIce,
					eTerrainHeightHillsNormalTerrainColorIce,
					eTerrainHeightMountainsNormalTerrainColorIce,
					eTerrainHeightMountainsRidgedTerrainColorIce,
					eTerrainHeightMountainsVolcanoTerrainColorIce,
					eTerrainHeightMountainsRiversVolcanoTerrainColorIce,
					eTerrainHeightMountainsRiversTerrainColorIce,
					eTerrainHeightRuggedDesertTerrainColorIce,
					eTerrainHeightBarrenRockTerrainColorIce,
					eTerrainHeightBarrenRock2TerrainColorIce,
					eTerrainHeightBarrenRock3TerrainColorIce
				};
				gs = choices[rand.Int32(COUNTOF(choices))];
				break;
			}

			// Marginally habitable world/ verging on mars like :)
			else if ((body->m_volatileGas > fixed(1,10)) && (body->m_life > fixed(1,10)) ) {

				if (body->averageTemp > 240) {
					static const uint32_t choices[] = {
						eTerrainHeightHillsRidgedTerrainColorTFPoor,
						eTerrainHeightHillsRiversTerrainColorTFPoor,
						eTerrainHeightHillsDunesTerrainColorTFPoor,
						eTerrainHeightHillsNormalTerrainColorTFPoor,
						eTerrainHeightMountainsNormalTerrainColorTFPoor,
						eTerrainHeightMountainsRidgedTerrainColorTFPoor,
						eTerrainHeightMountainsVolcanoTerrainColorTFPoor,
						eTerrainHeightMountainsRiversVolcanoTerrainColorTFPoor,
						eTerrainHeightMountainsRiversTerrainColorTFPoor,
						eTerrainHeightRuggedDesertTerrainColorTFPoor,
						eTerrainHeightBarrenRockTerrainColorTFPoor,
						eTerrainHeightBarrenRock2TerrainColorTFPoor,
						eTerrainHeightBarrenRock3TerrainColorTFPoor
					};
					gs = choices[rand.Int32(COUNTOF(choices))];
					break;
				}

				static const uint32_t choices[] = {
					eTerrainHeightHillsRidgedTerrainColorIce,
					eTerrainHeightHillsRiversTerrainColorIce,
					eTerrainHeightHillsDunesTerrainColorIce,
					eTerrainHeightHillsNormalTerrainColorIce,
					eTerrainHeightMountainsNormalTerrainColorIce,
					eTerrainHeightMountainsRidgedTerrainColorIce,
					eTerrainHeightMountainsVolcanoTerrainColorIce,
					eTerrainHeightMountainsRiversVolcanoTerrainColorIce,
					eTerrainHeightMountainsRiversTerrainColorIce,
					eTerrainHeightRuggedDesertTerrainColorIce,
					eTerrainHeightBarrenRockTerrainColorIce,
					eTerrainHeightBarrenRock2TerrainColorIce,
					eTerrainHeightBarrenRock3TerrainColorIce
				};
				gs = choices[rand.Int32(COUNTOF(choices))];
				break;
			}

			// Desert-like world, Mars -like.
			if ((body->m_volatileLiquid < fixed(1,10)) && (body->m_volatileGas > fixed(1,5))) {
				static const uint32_t choices[] = {
					eTerrainHeightHillsDunesTerrainColorDesert,
					eTerrainHeightWaterSolidTerrainColorDesert,
					eTerrainHeightRuggedDesertTerrainColorDesert,
					eTerrainHeightRuggedLavaTerrainColorDesert,
					eTerrainHeightMountainsVolcanoTerrainColorDesert,
					eTerrainHeightMountainsRiversVolcanoTerrainColorDesert,
					eTerrainHeightBarrenRockTerrainColorDesert,
					eTerrainHeightBarrenRock2TerrainColorDesert
				};
				gs = choices[rand.Int32(COUNTOF(choices))];
				break;
			}

			// Frozen world
			if ((body->m_volatileIces > fixed(8,10)) &&  (body->averageTemp < 250)) {
				static const uint32_t choices[] = {
					eTerrainHeightHillsDunesTerrainColorIce,
					eTerrainHeightHillsCratersTerrainColorIce,
					eTerrainHeightMountainsCratersTerrainColorIce,
					eTerrainHeightWaterSolidTerrainColorIce,
					eTerrainHeightWaterSolidCanyonsTerrainColorIce,
					eTerrainHeightRuggedDesertTerrainColorIce,
					eTerrainHeightBarrenRockTerrainColorIce,
					eTerrainHeightBarrenRock2TerrainColorIce,
					eTerrainHeightBarrenRock3TerrainColorIce
				};
				gs = choices[rand.Int32(COUNTOF(choices))];
				break;
			}

			// Volcanic world
			if (body->m_volcanicity > fixed(7,10)) {

				if (body->m_life > fixed(5,10))	// life on a volcanic world ;)
					gs = eTerrainHeightRuggedLavaTerrainColorTFGood;
				else if (body->m_life > fixed(2,10))
					gs = eTerrainHeightRuggedLavaTerrainColorTFPoor;
				else
					gs = eTerrainHeightRuggedLavaTerrainColorVolcanic;
				break;
			}

			//Below might not be needed.
			//Alien life world:
			if (body->m_life > fixed(1,10))  {
				static const uint32_t choices[] = {
					eTerrainHeightHillsDunesTerrainColorTFPoor,
					eTerrainHeightHillsRidgedTerrainColorTFPoor,
					eTerrainHeightHillsRiversTerrainColorTFPoor,
					eTerrainHeightMountainsNormalTerrainColorTFPoor,
					eTerrainHeightMountainsRidgedTerrainColorTFPoor,
					eTerrainHeightMountainsVolcanoTerrainColorTFPoor,
					eTerrainHeightMountainsRiversVolcanoTerrainColorTFPoor,
					eTerrainHeightMountainsRiversTerrainColorTFPoor,
					eTerrainHeightWaterSolidTerrainColorTFPoor,
					eTerrainHeightRuggedLavaTerrainColorTFPoor,
					eTerrainHeightRuggedDesertTerrainColorTFPoor,
					eTerrainHeightBarrenRockTerrainColorIce,
					eTerrainHeightBarrenRock2TerrainColorIce,
					eTerrainHeightBarrenRock3TerrainColorIce
				};
				gs = choices[rand.Int32(COUNTOF(choices))];
				break;
			};

			if (body->m_volatileGas > fixed(1,10)) {
				static const uint32_t choices[] = {
					eTerrainHeightHillsNormalTerrainColorRock,
					eTerrainHeightMountainsNormalTerrainColorRock,
					eTerrainHeightRuggedDesertTerrainColorRock,
					eTerrainHeightBarrenRockTerrainColorRock,
					eTerrainHeightBarrenRock2TerrainColorRock,
					eTerrainHeightBarrenRock3TerrainColorRock
				};
				gs = choices[rand.Int32(COUNTOF(choices))];
				break;
			}

			static const uint32_t choices[] = {
				eTerrainHeightHillsCraters2TerrainColorRock,
				eTerrainHeightMountainsCraters2TerrainColorRock,
				eTerrainHeightBarrenRock3TerrainColorRock
			};
			gs = choices[rand.Int32(COUNTOF(choices))];
			break;
		}

		default:
			gs = eTerrainHeightFlatTerrainColorSolid;
			break;
	}

	return sc_GeneratedTerrain[gs](body,gs);
}

//static 
Terrain *Terrain::InstanceTerrain(const Terrain& terrainIn)
{
	return sc_GeneratedTerrain[terrainIn.m_terrainEnumType](terrainIn.m_body, terrainIn.m_terrainEnumType);
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

Terrain::Terrain(const SystemBody *body, const uint32_t tET) : m_terrainEnumType(tET), m_body(body), m_seed(body->seed), m_rand(body->seed), m_heightMap(0), m_heightMapScaled(0), m_heightScaling(0), m_minh(0) {

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
	m_surfaceEffects = 0;

	const double rad = m_body->GetRadius();

	// calculate max height
	if ((m_body->heightMapFilename) && m_body->heightMapFractal > 1){ // if scaled heightmap
		m_maxHeightInMeters = 1.1*pow(2.0, 16.0)*m_heightScaling; // no min height required as it's added to radius in lua
	}else {
		m_maxHeightInMeters = std::max(100.0, (9000.0*rad*rad*(m_volcanic+0.5)) / (m_body->GetMass() * 6.64e-12));
		if (!isfinite(m_maxHeightInMeters)) m_maxHeightInMeters = rad * 0.5;
		//             ^^^^ max mountain height for earth-like planet (same mass, radius)
		// and then in sphere normalized jizz
	}
	m_maxHeight = std::min(1.0, m_maxHeightInMeters / rad);
	//printf("%s: max terrain height: %fm [%f]\n", m_body->name.c_str(), m_maxHeightInMeters, m_maxHeight);
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
		r = std::max(b, r * m_body->m_metallicity.ToFloat());
		g = std::max(b, g * m_body->m_metallicity.ToFloat());
		m_rockColor[i] = vector3d(r, g, b);
	}

	// Pick some darker colours mainly reds and greens
	for (int i=0; i<int(COUNTOF(m_entropy)); i++) m_entropy[i] = m_rand.Double();
	for (int i=0; i<int(COUNTOF(m_darkrockColor)); i++) {
		double r,g,b;
		r = m_rand.Double(0.05, 0.3);
		g = m_rand.Double(0.05, r);
		b = m_rand.Double(0.05, g);
		r = std::max(b, r * m_body->m_metallicity.ToFloat());
		g = std::max(b, g * m_body->m_metallicity.ToFloat());
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
		g = std::max(r, g * m_body->m_life.ToFloat());
		b *= (1.0-m_body->m_life.ToFloat());
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
		g = std::max(r, g * m_body->m_life.ToFloat());
		b *= (1.0-m_body->m_life.ToFloat());
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

void Terrain::DebugDump() const
{
	fprintf(stderr, "Terrain state dump:\n");
	fprintf(stderr, "  Height fractal: %s\n", GetHeightFractalName());
	fprintf(stderr, "  Color fractal: %s\n", GetColorFractalName());
	fprintf(stderr, "  Detail: fracnum %d  fracmult %f  textures %s\n", m_fracnum, m_fracmult, textures ? "true" : "false");
	fprintf(stderr, "  Config: DetailPlanets %d   FractalMultiple %d  Textures  %d\n", Pi::config->Int("DetailPlanets"), Pi::config->Int("FractalMultiple"), Pi::config->Int("Textures"));
	fprintf(stderr, "  Seed: %d\n", m_seed);
	fprintf(stderr, "  Body: %s [%d,%d,%d,%u,%u]\n", m_body->name.c_str(), m_body->path.sectorX, m_body->path.sectorY, m_body->path.sectorZ, m_body->path.systemIndex, m_body->path.bodyIndex);
	fprintf(stderr, "  Aspect Ratio: %g\n", m_body->aspectRatio.ToDouble());
	fprintf(stderr, "  Fracdefs:\n");
	for (int i = 0; i < 10; i++) {
		fprintf(stderr, "    %d: amp %f  freq %f  lac %f  oct %d\n", i, m_fracdef[i].amplitude, m_fracdef[i].frequency, m_fracdef[i].lacunarity, m_fracdef[i].octaves);
	}
}
