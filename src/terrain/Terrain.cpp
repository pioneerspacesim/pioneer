// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Terrain.h"

#include "FileSystem.h"
#include "FloatComparison.h"
#include "GameConfig.h"
#include "MathUtil.h"
#include "perlin.h"
#include "core/macros.h"
#include "core/Log.h"
#include "../galaxy/SystemBody.h"

namespace TerrainColours {

	const std::string TerrainColourNames[] = {
		"Asteroid",
		"BandedRock",
		"DeadWithWater",
		"Desert",
		"EarthLike",
		"EarthLikeHeightmapped",
		"Ice",
		"Methane",
		"Rock2",
		"Rock",
		"Volcanic",
		 // Gas Giants
		"GGJupiter",
		"GGNeptune2",
		"GGNeptune",
		"GGSaturn2",
		"GGSaturn",
		"GGUranus",
		// /Gas Giants
		// Stars
		"StarBrownDwarf",
		"StarG",
		"StarK",
		"StarM",
		"StarWhiteDwarf",
		// /Stars
		"Black",
		"White",
		"TFGood",
		"TFPoor"
	};

	const uint32_t TerrainSurfaceEffects[] = {
		Terrain::SurfaceEffectFlags::EFFECT_NONE,
		Terrain::SurfaceEffectFlags::EFFECT_NONE,
		Terrain::SurfaceEffectFlags::EFFECT_WATER,
		Terrain::SurfaceEffectFlags::EFFECT_NONE,
		Terrain::SurfaceEffectFlags::EFFECT_WATER,
		Terrain::SurfaceEffectFlags::EFFECT_WATER,
		Terrain::SurfaceEffectFlags::EFFECT_NONE,
		Terrain::SurfaceEffectFlags::EFFECT_NONE,
		Terrain::SurfaceEffectFlags::EFFECT_NONE,
		Terrain::SurfaceEffectFlags::EFFECT_NONE,
		Terrain::SurfaceEffectFlags::EFFECT_LAVA,
		// Gas Giants
		Terrain::SurfaceEffectFlags::EFFECT_NONE,
		Terrain::SurfaceEffectFlags::EFFECT_NONE,
		Terrain::SurfaceEffectFlags::EFFECT_NONE,
		Terrain::SurfaceEffectFlags::EFFECT_NONE,
		Terrain::SurfaceEffectFlags::EFFECT_NONE,
		Terrain::SurfaceEffectFlags::EFFECT_NONE,
		// /Gas Giants
		// Stars
		Terrain::SurfaceEffectFlags::EFFECT_NONE,
		Terrain::SurfaceEffectFlags::EFFECT_NONE,
		Terrain::SurfaceEffectFlags::EFFECT_NONE,
		Terrain::SurfaceEffectFlags::EFFECT_NONE,
		Terrain::SurfaceEffectFlags::EFFECT_NONE,
		// /Stars
		Terrain::SurfaceEffectFlags::EFFECT_NONE,
		Terrain::SurfaceEffectFlags::EFFECT_NONE,
		Terrain::SurfaceEffectFlags::EFFECT_WATER,
		Terrain::SurfaceEffectFlags::EFFECT_WATER
	};

	const std::string &GetTerrainColourNameFromEnum(const Terrain::ETerrainColours etc)
	{
		return TerrainColourNames[etc];
	}
} //namespace TerrainColours

// static instancer. selects the best height and color classes for the body
Terrain *Terrain::InstanceTerrain(const SystemBody *body)
{

	// special case for heightmaps
	// XXX this is terrible but will do for now until we get a unified
	// heightmap setup. if you add another height fractal, remember to change
	// the check in CustomSystem::l_height_map / SystemBodyData::LoadFromJson
	if (!body->GetHeightMapFilename().empty()) {
		const std::pair<GeneratorInstancer, ETerrainColours> choices[] = {
			std::make_pair(InstanceGenerator<TerrainHeightMapped>, ETerrainColours::eTerrainColorEarthLikeHeightmapped),
			std::make_pair(InstanceGenerator<TerrainHeightMapped2>, ETerrainColours::eTerrainColorRock2)
		};
		assert(body->GetHeightMapFractal() < COUNTOF(choices));
		const int32_t idx = body->GetHeightMapFractal();
		return choices[idx].first(body, TerrainColours::TerrainSurfaceEffects[choices[idx].second], choices[idx].second);
	}

	Random rand(body->GetSeed());

	GeneratorInstancer gi = 0;
	Uint32 surfaceEffects = 0;
	ETerrainColours terrainColour = ETerrainColours::eTerrainColorAsteroid;

	switch (body->GetType()) {

	case SystemBody::TYPE_BROWN_DWARF:
		gi = InstanceGenerator<TerrainHeightEllipsoid>;
		terrainColour = ETerrainColours::eTerrainColorStarBrownDwarf;
		break;

	case SystemBody::TYPE_WHITE_DWARF:
		gi = InstanceGenerator<TerrainHeightEllipsoid>;
		terrainColour = ETerrainColours::eTerrainColorStarWhiteDwarf;
		break;

	case SystemBody::TYPE_STAR_M:
	case SystemBody::TYPE_STAR_M_GIANT:
	case SystemBody::TYPE_STAR_M_SUPER_GIANT:
	case SystemBody::TYPE_STAR_M_HYPER_GIANT: {
		const std::pair<GeneratorInstancer, ETerrainColours> choices[] = {
			std::make_pair(InstanceGenerator<TerrainHeightEllipsoid>, ETerrainColours::eTerrainColorStarM),
			std::make_pair(InstanceGenerator<TerrainHeightEllipsoid>, ETerrainColours::eTerrainColorStarM),
			std::make_pair(InstanceGenerator<TerrainHeightEllipsoid>, ETerrainColours::eTerrainColorStarK),
			std::make_pair(InstanceGenerator<TerrainHeightEllipsoid>, ETerrainColours::eTerrainColorStarG)
		};
		const int32_t idx = rand.Int32(COUNTOF(choices));
		gi = choices[idx].first;
		terrainColour = choices[idx].second;
		break;
	}

	case SystemBody::TYPE_STAR_K:
	case SystemBody::TYPE_STAR_K_GIANT:
	case SystemBody::TYPE_STAR_K_SUPER_GIANT:
	case SystemBody::TYPE_STAR_K_HYPER_GIANT: {
		const std::pair<GeneratorInstancer, ETerrainColours> choices[] = {
			std::make_pair(InstanceGenerator<TerrainHeightEllipsoid>, ETerrainColours::eTerrainColorStarM),
			std::make_pair(InstanceGenerator<TerrainHeightEllipsoid>, ETerrainColours::eTerrainColorStarK),
			std::make_pair(InstanceGenerator<TerrainHeightEllipsoid>, ETerrainColours::eTerrainColorStarK),
			std::make_pair(InstanceGenerator<TerrainHeightEllipsoid>, ETerrainColours::eTerrainColorStarG)
		};
		const int32_t idx = rand.Int32(COUNTOF(choices));
		gi = choices[idx].first;
		terrainColour = choices[idx].second;
		break;
	}

	case SystemBody::TYPE_STAR_G:
	case SystemBody::TYPE_STAR_G_GIANT:
	case SystemBody::TYPE_STAR_G_SUPER_GIANT:
	case SystemBody::TYPE_STAR_G_HYPER_GIANT: {
		const std::pair<GeneratorInstancer, ETerrainColours> choices[] = {
			std::make_pair(InstanceGenerator<TerrainHeightEllipsoid>, ETerrainColours::eTerrainColorStarWhiteDwarf),
			std::make_pair(InstanceGenerator<TerrainHeightEllipsoid>, ETerrainColours::eTerrainColorStarG)
		};
		const int32_t idx = rand.Int32(COUNTOF(choices));
		gi = choices[idx].first;
		terrainColour = choices[idx].second;
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
		gi = InstanceGenerator<TerrainHeightEllipsoid>;
		terrainColour = ETerrainColours::eTerrainColorWhite;
		break;

	case SystemBody::TYPE_STAR_S_BH:
	case SystemBody::TYPE_STAR_IM_BH:
	case SystemBody::TYPE_STAR_SM_BH:
		gi = InstanceGenerator<TerrainHeightEllipsoid>;
		terrainColour = ETerrainColours::eTerrainColorBlack;
		break;

	case SystemBody::TYPE_PLANET_GAS_GIANT: {
		const std::pair<GeneratorInstancer, ETerrainColours> choices[] = {
			std::make_pair(InstanceGenerator<TerrainHeightFlat>, ETerrainColours::eTerrainColorGGJupiter),
			std::make_pair(InstanceGenerator<TerrainHeightFlat>, ETerrainColours::eTerrainColorGGSaturn),
			std::make_pair(InstanceGenerator<TerrainHeightFlat>, ETerrainColours::eTerrainColorGGSaturn2),
			std::make_pair(InstanceGenerator<TerrainHeightFlat>, ETerrainColours::eTerrainColorGGNeptune),
			std::make_pair(InstanceGenerator<TerrainHeightFlat>, ETerrainColours::eTerrainColorGGNeptune2),
			std::make_pair(InstanceGenerator<TerrainHeightFlat>, ETerrainColours::eTerrainColorGGUranus),
			std::make_pair(InstanceGenerator<TerrainHeightFlat>, ETerrainColours::eTerrainColorGGSaturn)
		};
		const int32_t idx = rand.Int32(COUNTOF(choices));
		gi = choices[idx].first;
		terrainColour = choices[idx].second;
		break;
	}

	case SystemBody::TYPE_PLANET_ASTEROID: {
		const std::pair<GeneratorInstancer, ETerrainColours> choices[] = {
			std::make_pair(InstanceGenerator<TerrainHeightAsteroid>, ETerrainColours::eTerrainColorAsteroid),
			std::make_pair(InstanceGenerator<TerrainHeightAsteroid2>, ETerrainColours::eTerrainColorAsteroid),
			std::make_pair(InstanceGenerator<TerrainHeightAsteroid3>, ETerrainColours::eTerrainColorAsteroid),
			std::make_pair(InstanceGenerator<TerrainHeightAsteroid4>, ETerrainColours::eTerrainColorAsteroid),
			std::make_pair(InstanceGenerator<TerrainHeightAsteroid>, ETerrainColours::eTerrainColorRock),
			std::make_pair(InstanceGenerator<TerrainHeightAsteroid2>, ETerrainColours::eTerrainColorBandedRock),
			std::make_pair(InstanceGenerator<TerrainHeightAsteroid3>, ETerrainColours::eTerrainColorRock),
			std::make_pair(InstanceGenerator<TerrainHeightAsteroid4>, ETerrainColours::eTerrainColorBandedRock)
		};
		const int32_t idx = rand.Int32(COUNTOF(choices));
		gi = choices[idx].first;
		terrainColour = choices[idx].second;
		break;
	}

	case SystemBody::TYPE_PLANET_TERRESTRIAL: {

		//Over-ride:
		//gi = InstanceGenerator<TerrainHeightAsteroid3,TerrainColorRock>;
		//break;
		// Earth-like world

		if ((body->GetLifeAsFixed() > fixed(7, 10)) && (body->GetVolatileGasAsFixed() > fixed(2, 10))) {
			// There would be no life on the surface without atmosphere

			if (body->GetAverageTemp() > 240) {
				const std::pair<GeneratorInstancer, ETerrainColours> choices[] = {
					std::make_pair(InstanceGenerator<TerrainHeightHillsRidged>, ETerrainColours::eTerrainColorEarthLike),
					std::make_pair(InstanceGenerator<TerrainHeightHillsRivers>, ETerrainColours::eTerrainColorEarthLike),
					std::make_pair(InstanceGenerator<TerrainHeightHillsDunes>, ETerrainColours::eTerrainColorEarthLike),
					std::make_pair(InstanceGenerator<TerrainHeightMountainsRidged>, ETerrainColours::eTerrainColorEarthLike),
					std::make_pair(InstanceGenerator<TerrainHeightMountainsNormal>, ETerrainColours::eTerrainColorEarthLike),
					std::make_pair(InstanceGenerator<TerrainHeightMountainsRivers>, ETerrainColours::eTerrainColorEarthLike),
					std::make_pair(InstanceGenerator<TerrainHeightMountainsVolcano>, ETerrainColours::eTerrainColorEarthLike),
					std::make_pair(InstanceGenerator<TerrainHeightMountainsRiversVolcano>, ETerrainColours::eTerrainColorEarthLike)
				};
				const int32_t idx = rand.Int32(COUNTOF(choices));
				gi = choices[idx].first;
				terrainColour = choices[idx].second;
				break;
			}

			const std::pair<GeneratorInstancer, ETerrainColours> choices[] = {
				std::make_pair(InstanceGenerator<TerrainHeightHillsRidged>, ETerrainColours::eTerrainColorDesert),
				std::make_pair(InstanceGenerator<TerrainHeightHillsRivers>, ETerrainColours::eTerrainColorDesert),
				std::make_pair(InstanceGenerator<TerrainHeightHillsDunes>, ETerrainColours::eTerrainColorDesert),
				std::make_pair(InstanceGenerator<TerrainHeightMountainsRidged>, ETerrainColours::eTerrainColorDesert),
				std::make_pair(InstanceGenerator<TerrainHeightMountainsNormal>, ETerrainColours::eTerrainColorDesert),
				std::make_pair(InstanceGenerator<TerrainHeightMountainsRivers>, ETerrainColours::eTerrainColorDesert),
				std::make_pair(InstanceGenerator<TerrainHeightMountainsVolcano>, ETerrainColours::eTerrainColorDesert),
				std::make_pair(InstanceGenerator<TerrainHeightMountainsRiversVolcano>, ETerrainColours::eTerrainColorDesert),
				std::make_pair(InstanceGenerator<TerrainHeightBarrenRock>, ETerrainColours::eTerrainColorDesert),
				std::make_pair(InstanceGenerator<TerrainHeightBarrenRock2>, ETerrainColours::eTerrainColorDesert)
				//std::make_pair(InstanceGenerator<TerrainHeightBarrenRock3,TerrainColorTFGood>
			};
			const int32_t idx = rand.Int32(COUNTOF(choices));
			gi = choices[idx].first;
			terrainColour = choices[idx].second;
			break;
		}

		// Harsh, habitable world
		if ((body->GetVolatileGasAsFixed() > fixed(2, 10)) && (body->GetLifeAsFixed() > fixed(4, 10))) {

			if (body->GetAverageTemp() > 240) {
				const std::pair<GeneratorInstancer, ETerrainColours> choices[] = {
					std::make_pair(InstanceGenerator<TerrainHeightHillsRidged>, ETerrainColours::eTerrainColorTFGood),
					std::make_pair(InstanceGenerator<TerrainHeightHillsRivers>, ETerrainColours::eTerrainColorTFGood),
					std::make_pair(InstanceGenerator<TerrainHeightHillsDunes>, ETerrainColours::eTerrainColorTFGood),
					std::make_pair(InstanceGenerator<TerrainHeightHillsNormal>, ETerrainColours::eTerrainColorTFGood),
					std::make_pair(InstanceGenerator<TerrainHeightMountainsNormal>, ETerrainColours::eTerrainColorTFGood),
					std::make_pair(InstanceGenerator<TerrainHeightMountainsRidged>, ETerrainColours::eTerrainColorTFGood),
					std::make_pair(InstanceGenerator<TerrainHeightMountainsVolcano>, ETerrainColours::eTerrainColorTFGood),
					std::make_pair(InstanceGenerator<TerrainHeightMountainsRiversVolcano>, ETerrainColours::eTerrainColorTFGood),
					std::make_pair(InstanceGenerator<TerrainHeightMountainsRivers>, ETerrainColours::eTerrainColorTFGood),
					std::make_pair(InstanceGenerator<TerrainHeightRuggedDesert>, ETerrainColours::eTerrainColorTFGood),
					std::make_pair(InstanceGenerator<TerrainHeightBarrenRock>, ETerrainColours::eTerrainColorTFGood),
					std::make_pair(InstanceGenerator<TerrainHeightBarrenRock2>, ETerrainColours::eTerrainColorTFGood)
					//InstanceGenerator<TerrainHeightBarrenRock3,TerrainColorTFGood>
				};
				const int32_t idx = rand.Int32(COUNTOF(choices));
				gi = choices[idx].first;
				terrainColour = choices[idx].second;
				break;
			}

			const std::pair<GeneratorInstancer, ETerrainColours> choices[] = {
				std::make_pair(InstanceGenerator<TerrainHeightHillsRidged>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightHillsRivers>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightHillsDunes>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightHillsNormal>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightMountainsNormal>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightMountainsRidged>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightMountainsVolcano>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightMountainsRiversVolcano>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightMountainsRivers>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightRuggedDesert>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightBarrenRock>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightBarrenRock2>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightBarrenRock3>, ETerrainColours::eTerrainColorIce)
			};
			const int32_t idx = rand.Int32(COUNTOF(choices));
			gi = choices[idx].first;
			terrainColour = choices[idx].second;
			break;
		}

		// Marginally habitable world/ verging on mars like :)
		else if ((body->GetVolatileGasAsFixed() > fixed(1, 10)) && (body->GetLifeAsFixed() > fixed(1, 10))) {

			if (body->GetAverageTemp() > 240) {
				const std::pair<GeneratorInstancer, ETerrainColours> choices[] = {
					std::make_pair(InstanceGenerator<TerrainHeightHillsRidged>, ETerrainColours::eTerrainColorTFPoor),
					std::make_pair(InstanceGenerator<TerrainHeightHillsRivers>, ETerrainColours::eTerrainColorTFPoor),
					std::make_pair(InstanceGenerator<TerrainHeightHillsDunes>, ETerrainColours::eTerrainColorTFPoor),
					std::make_pair(InstanceGenerator<TerrainHeightHillsNormal>, ETerrainColours::eTerrainColorTFPoor),
					std::make_pair(InstanceGenerator<TerrainHeightMountainsNormal>, ETerrainColours::eTerrainColorTFPoor),
					std::make_pair(InstanceGenerator<TerrainHeightMountainsRidged>, ETerrainColours::eTerrainColorTFPoor),
					std::make_pair(InstanceGenerator<TerrainHeightMountainsVolcano>, ETerrainColours::eTerrainColorTFPoor),
					std::make_pair(InstanceGenerator<TerrainHeightMountainsRiversVolcano>, ETerrainColours::eTerrainColorTFPoor),
					std::make_pair(InstanceGenerator<TerrainHeightMountainsRivers>, ETerrainColours::eTerrainColorTFPoor),
					std::make_pair(InstanceGenerator<TerrainHeightRuggedDesert>, ETerrainColours::eTerrainColorTFPoor),
					std::make_pair(InstanceGenerator<TerrainHeightBarrenRock>, ETerrainColours::eTerrainColorTFPoor),
					std::make_pair(InstanceGenerator<TerrainHeightBarrenRock2>, ETerrainColours::eTerrainColorTFPoor),
					std::make_pair(InstanceGenerator<TerrainHeightBarrenRock3>, ETerrainColours::eTerrainColorTFPoor)
				};
				const int32_t idx = rand.Int32(COUNTOF(choices));
				gi = choices[idx].first;
				terrainColour = choices[idx].second;
				break;
			}

			const std::pair<GeneratorInstancer, ETerrainColours> choices[] = {
				std::make_pair(InstanceGenerator<TerrainHeightHillsRidged>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightHillsRivers>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightHillsDunes>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightHillsNormal>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightMountainsNormal>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightMountainsRidged>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightMountainsVolcano>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightMountainsRiversVolcano>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightMountainsRivers>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightRuggedDesert>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightBarrenRock>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightBarrenRock2>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightBarrenRock3>, ETerrainColours::eTerrainColorIce)
			};
			const int32_t idx = rand.Int32(COUNTOF(choices));
			gi = choices[idx].first;
			terrainColour = choices[idx].second;
			break;
		}

		// Desert-like world, Mars -like.
		if ((body->GetVolatileLiquidAsFixed() < fixed(1, 10)) && (body->GetVolatileGasAsFixed() > fixed(1, 5))) {
			const std::pair<GeneratorInstancer, ETerrainColours> choices[] = {
				std::make_pair(InstanceGenerator<TerrainHeightHillsDunes>, ETerrainColours::eTerrainColorDesert),
				std::make_pair(InstanceGenerator<TerrainHeightWaterSolid>, ETerrainColours::eTerrainColorDesert),
				std::make_pair(InstanceGenerator<TerrainHeightRuggedDesert>, ETerrainColours::eTerrainColorDesert),
				std::make_pair(InstanceGenerator<TerrainHeightRuggedLava>, ETerrainColours::eTerrainColorDesert),
				std::make_pair(InstanceGenerator<TerrainHeightMountainsVolcano>, ETerrainColours::eTerrainColorDesert),
				std::make_pair(InstanceGenerator<TerrainHeightMountainsRiversVolcano>, ETerrainColours::eTerrainColorDesert),
				std::make_pair(InstanceGenerator<TerrainHeightBarrenRock>, ETerrainColours::eTerrainColorDesert),
				std::make_pair(InstanceGenerator<TerrainHeightBarrenRock2>, ETerrainColours::eTerrainColorDesert)
			};
			const int32_t idx = rand.Int32(COUNTOF(choices));
			gi = choices[idx].first;
			terrainColour = choices[idx].second;
			break;
		}

		// Frozen world
		if ((body->GetVolatileIcesAsFixed() > fixed(8, 10)) && (body->GetAverageTemp() < 250)) {
			const std::pair<GeneratorInstancer, ETerrainColours> choices[] = {
				std::make_pair(InstanceGenerator<TerrainHeightHillsDunes>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightHillsCraters>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightMountainsCraters>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightWaterSolid>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightWaterSolidCanyons>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightRuggedDesert>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightBarrenRock>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightBarrenRock2>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightBarrenRock3>, ETerrainColours::eTerrainColorIce)
			};
			const int32_t idx = rand.Int32(COUNTOF(choices));
			gi = choices[idx].first;
			terrainColour = choices[idx].second;
			break;
		}

		// Volcanic world
		if (body->GetVolcanicityAsFixed() > fixed(7, 10)) {

			if (body->GetLifeAsFixed() > fixed(5, 10)) { // life on a volcanic world ;)
				gi = InstanceGenerator<TerrainHeightRuggedLava>;
				terrainColour = ETerrainColours::eTerrainColorTFGood;
			} else if (body->GetLifeAsFixed() > fixed(2, 10)) {
				gi = InstanceGenerator<TerrainHeightRuggedLava>;
				terrainColour = ETerrainColours::eTerrainColorTFPoor;
			} else {
				gi = InstanceGenerator<TerrainHeightRuggedLava>;
				terrainColour = ETerrainColours::eTerrainColorVolcanic;
			}
			break;
		}

		//Below might not be needed.
		//Alien life world:
		if (body->GetLifeAsFixed() > fixed(1, 10)) {
			const std::pair<GeneratorInstancer, ETerrainColours> choices[] = {
				std::make_pair(InstanceGenerator<TerrainHeightHillsDunes>, ETerrainColours::eTerrainColorTFPoor),
				std::make_pair(InstanceGenerator<TerrainHeightHillsRidged>, ETerrainColours::eTerrainColorTFPoor),
				std::make_pair(InstanceGenerator<TerrainHeightHillsRivers>, ETerrainColours::eTerrainColorTFPoor),
				std::make_pair(InstanceGenerator<TerrainHeightMountainsNormal>, ETerrainColours::eTerrainColorTFPoor),
				std::make_pair(InstanceGenerator<TerrainHeightMountainsRidged>, ETerrainColours::eTerrainColorTFPoor),
				std::make_pair(InstanceGenerator<TerrainHeightMountainsVolcano>, ETerrainColours::eTerrainColorTFPoor),
				std::make_pair(InstanceGenerator<TerrainHeightMountainsRiversVolcano>, ETerrainColours::eTerrainColorTFPoor),
				std::make_pair(InstanceGenerator<TerrainHeightMountainsRivers>, ETerrainColours::eTerrainColorTFPoor),
				std::make_pair(InstanceGenerator<TerrainHeightWaterSolid>, ETerrainColours::eTerrainColorTFPoor),
				std::make_pair(InstanceGenerator<TerrainHeightRuggedLava>, ETerrainColours::eTerrainColorTFPoor),
				std::make_pair(InstanceGenerator<TerrainHeightRuggedDesert>, ETerrainColours::eTerrainColorTFPoor),
				std::make_pair(InstanceGenerator<TerrainHeightBarrenRock>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightBarrenRock2>, ETerrainColours::eTerrainColorIce),
				std::make_pair(InstanceGenerator<TerrainHeightBarrenRock3>, ETerrainColours::eTerrainColorIce)
			};
			const int32_t idx = rand.Int32(COUNTOF(choices));
			gi = choices[idx].first;
			terrainColour = choices[idx].second;
			break;
		};

		if (body->GetVolatileGasAsFixed() > fixed(1, 10)) {
			const std::pair<GeneratorInstancer, ETerrainColours> choices[] = {
				std::make_pair(InstanceGenerator<TerrainHeightHillsNormal>, ETerrainColours::eTerrainColorRock),
				std::make_pair(InstanceGenerator<TerrainHeightMountainsNormal>, ETerrainColours::eTerrainColorRock),
				std::make_pair(InstanceGenerator<TerrainHeightRuggedDesert>, ETerrainColours::eTerrainColorRock),
				std::make_pair(InstanceGenerator<TerrainHeightBarrenRock>, ETerrainColours::eTerrainColorRock),
				std::make_pair(InstanceGenerator<TerrainHeightBarrenRock2>, ETerrainColours::eTerrainColorRock),
				std::make_pair(InstanceGenerator<TerrainHeightBarrenRock3>, ETerrainColours::eTerrainColorRock)
			};
			const int32_t idx = rand.Int32(COUNTOF(choices));
			gi = choices[idx].first;
			terrainColour = choices[idx].second;
			break;
		}

		const std::pair<GeneratorInstancer, ETerrainColours> choices[] = {
			std::make_pair(InstanceGenerator<TerrainHeightHillsCraters2>, ETerrainColours::eTerrainColorRock),
			std::make_pair(InstanceGenerator<TerrainHeightMountainsCraters2>, ETerrainColours::eTerrainColorRock),
			std::make_pair(InstanceGenerator<TerrainHeightBarrenRock3>, ETerrainColours::eTerrainColorRock)
		};
		const int32_t idx = rand.Int32(COUNTOF(choices));
		gi = choices[idx].first;
		terrainColour = choices[idx].second;
		break;
	}

	default:
		gi = InstanceGenerator<TerrainHeightFlat>;
		terrainColour = ETerrainColours::eTerrainColorWhite;
		break;
	}

	// only have to do this once because we're just looking up the surface effects from the table based on the terrain colour
	surfaceEffects |= TerrainColours::TerrainSurfaceEffects[terrainColour];

	return gi(body, surfaceEffects, terrainColour);
}

static size_t bufread_or_die(void *ptr, size_t size, size_t nmemb, ByteRange &buf)
{
	size_t read_count = buf.read(static_cast<char *>(ptr), size, nmemb);
	if (read_count < nmemb) {
		Output("Error: failed to read file (truncated)\n");
		abort();
	}
	return read_count;
}

// XXX this sucks, but there isn't a reliable cross-platform way to get them
#ifndef INT16_MIN
#define INT16_MIN (-32767 - 1)
#endif
#ifndef INT16_MAX
#define INT16_MAX (32767)
#endif
#ifndef UINT16_MAX
#define UINT16_MAX (65535)
#endif

Terrain::Terrain(const SystemBody *body, const Uint32 surfaceEffects, const ETerrainColours terrainColour) :
	m_body(const_cast<SystemBody*>(body)),
	m_seed(body->GetSeed()),
	m_rand(body->GetSeed()),
	m_surfaceEffects(surfaceEffects),
	m_terrainColour(terrainColour),
	m_heightScaling(0),
	m_minh(0),
	m_heightMapSizeX(0),
	m_heightMapSizeY(0)
{

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
			bufread_or_die(&v, 2, 1, databuf);
			m_heightMapSizeX = v;
			bufread_or_die(&v, 2, 1, databuf);
			m_heightMapSizeY = v;
			const Uint32 heightmapPixelArea = (m_heightMapSizeX * m_heightMapSizeY);

			std::unique_ptr<Sint16[]> heightMap(new Sint16[heightmapPixelArea]);
			bufread_or_die(heightMap.get(), sizeof(Sint16), heightmapPixelArea, databuf);
			m_heightMap.reset(new double[heightmapPixelArea]);
			double *pHeightMap = m_heightMap.get();
			for (Uint32 i = 0; i < heightmapPixelArea; i++) {
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
			bufread_or_die(&v, 2, 1, databuf);
			m_heightMapSizeY = v;
			bufread_or_die(&v, 2, 1, databuf);
			m_heightMapSizeX = v;
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
			for (Uint32 i = 0; i < heightmapPixelArea; i++) {
				const Uint16 val = heightMapScaled[i];
				minHMapScld = std::min(minHMapScld, val);
				maxHMapScld = std::max(maxHMapScld, val);
				// store then increment pointer
				// pre-multiply and offset the height value
				(*pHeightMap) = double(val) * m_heightScaling + m_minh;
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

	m_sealevel = Clamp(body->GetVolatileLiquid(), 0.0, 1.0);
	m_icyness = Clamp(body->GetVolatileIces(), 0.0, 1.0);
	m_volcanic = Clamp(body->GetVolcanicity(), 0.0, 1.0); // height scales with volcanicity as well
	m_surfaceEffects = 0;

	const double rad = m_body->GetRadius();

	// calculate max height
	if (!body->GetHeightMapFilename().empty() && body->GetHeightMapFractal() > 1) { // if scaled heightmap
		m_maxHeightInMeters = 1.1 * pow(2.0, 16.0) * m_heightScaling; // no min height required as it's added to radius in lua
	} else {
		// max mountain height for earth-like planet (same mass, radius)
		m_maxHeightInMeters = std::max(100.0, (9000.0 * rad * rad * (m_volcanic + 0.5)) / (body->GetMass() * 6.64e-12));
		m_maxHeightInMeters = std::min(rad, m_maxHeightInMeters); // small asteroid case
	}
	// and then in sphere normalized jizz
	m_maxHeight = m_maxHeightInMeters / rad;
	//Output("%s: max terrain height: %fm [%f]\n", m_body->GetName().c_str(), m_maxHeightInMeters, m_maxHeight);
	m_invMaxHeight = 1.0 / m_maxHeight;
	m_planetRadius = rad;
	m_invPlanetRadius = 1.0 / rad;
	m_planetEarthRadii = rad / EARTH_RADIUS;
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
	assert(index < MAX_FRACDEFS);
	// feature
	m_fracdef[index].amplitude = featureHeightMeters / (m_maxHeight * m_planetRadius);
	m_fracdef[index].frequency = m_planetRadius / featureWidthMeters;
	m_fracdef[index].octaves = std::max(1, int(ceil(log(featureWidthMeters / smallestOctaveMeters) / log(2.0))));
	m_fracdef[index].lacunarity = 2.0;
	//Output("%d octaves\n", m_fracdef[index].octaves); //print
}

const char* Terrain::GetColorFractalName() const
{
	assert(m_terrainColour >= eTerrainColorAsteroid && m_terrainColour <= eTerrainColorTFPoor);
	return TerrainColours::TerrainColourNames[m_terrainColour].c_str();
}

double Terrain::BiCubicInterpolation(const vector3d &p) const
{
	double latitude = -asin(p.y);
	if (p.y < -1.0) latitude = -0.5 * M_PI;
	if (p.y > 1.0) latitude = 0.5 * M_PI;
	//	if (!isfinite(latitude)) {
	//		// p.y is just n of asin domain [-1,1]
	//		latitude = (p.y < 0 ? -0.5*M_PI : M_PI*0.5);
	//	}
	const double longitude = atan2(p.x, p.z);
	const double px = (((m_heightMapSizeX - 1) * (longitude + M_PI)) / (2 * M_PI));
	const double py = ((m_heightMapSizeY - 1) * (latitude + 0.5 * M_PI)) / M_PI;
	int ix = int(floor(px));
	int iy = int(floor(py));
	ix = Clamp(ix, 0, m_heightMapSizeX - 1);
	iy = Clamp(iy, 0, m_heightMapSizeY - 1);
	const double dx = px - ix;
	const double dy = py - iy;

	// p0,3 p1,3 p2,3 p3,3
	// p0,2 p1,2 p2,2 p3,2
	// p0,1 p1,1 p2,1 p3,1
	// p0,0 p1,0 p2,0 p3,0
	double map[4][4];
	const double *pHMap = m_heightMap.get();
	for (int x = -1; x < 3; x++) {
		for (int y = -1; y < 3; y++) {
			map[x + 1][y + 1] = pHMap[Clamp(iy + y, 0, m_heightMapSizeY - 1) * m_heightMapSizeX + Clamp(ix + x, 0, m_heightMapSizeX - 1)];
		}
	}

	double c[4];
	for (int j = 0; j < 4; j++) {
		const double d0 = map[0][j] - map[1][j];
		const double d2 = map[2][j] - map[1][j];
		const double d3 = map[3][j] - map[1][j];
		const double a0 = map[1][j];
		const double a1 = -(1 / 3.0) * d0 + d2 - (1 / 6.0) * d3;
		const double a2 = 0.5 * d0 + 0.5 * d2;
		const double a3 = -(1 / 6.0) * d0 - 0.5 * d2 + (1 / 6.0) * d3;
		c[j] = a0 + a1 * dx + a2 * dx * dx + a3 * dx * dx * dx;
	}

	const double d0 = c[0] - c[1];
	const double d2 = c[2] - c[1];
	const double d3 = c[3] - c[1];
	const double a0 = c[1];
	const double a1 = -(1 / 3.0) * d0 + d2 - (1 / 6.0) * d3;
	const double a2 = 0.5 * d0 + 0.5 * d2;
	const double a3 = -(1 / 6.0) * d0 - 0.5 * d2 + (1 / 6.0) * d3;
	return (0.1 + a0 + a1 * dy + a2 * dy * dy + a3 * dy * dy * dy);
}
