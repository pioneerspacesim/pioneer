// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "TerrainLocalLights.h"

#include "Camera.h"
#include "Color.h"
#include "Frame.h"
#include "Game.h"
#include "Pi.h"
#include "Planet.h"
#include "Player.h"
#include "Ship.h"
#include "Space.h"
#include "SpaceStation.h"
#include "Random.h"
#include "core/Log.h"
#include "core/StringHash.h"

#include "graphics/Material.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace {
	static const int MAX_TERRAIN_LOCAL_LIGHTS = 16;
	static const double MAX_SCAN_DISTANCE = 20000.0;

	// Ship lights and starports only illuminate the terrain at night, this is the threshold below which they start showing
	static const float NIGHT_LIGHT_THRESHOLD = 0.4f;

	static const float SHIP_NAV_LIGHT_RANGE = 600.f;
	static const float SHIP_NAV_LIGHT_STRENGTH = 0.5f;

	static const float SHIP_THRUSTER_LIGHT_RANGE = 1200.f;
	static const float SHIP_THRUSTER_LIGHT_STRENGTH = 0.8f;
	static const float SHIP_THRUSTER_ADDITIVE_FACTOR = 0.5f;
	static const float SHIP_THRUSTER_FLICKER = 0.15f;

	static const float STARPORT_LIGHT_STRENGTH = 0.6f;
	static const Color4f STARPORT_LIGHT_COLOR(1.f, 0.9f, 0.7f, 1.f);

	struct TerrainLocalLightGPU {
		Color4f position;
		Color4f colour;
		Color4f falloff; // x = range (meters), y = strength, z = additiveFactor
	};

	struct TerrainLocalLightsBlock {
		int32_t numLights;
		float _pad[3];
		TerrainLocalLightGPU lights[MAX_TERRAIN_LOCAL_LIGHTS];
	};
	static_assert(sizeof(TerrainLocalLightsBlock) == 16 + MAX_TERRAIN_LOCAL_LIGHTS * 48, "");

	static size_t s_terrainLocalLightsData = "TerrainLocalLightsData"_hash;
	static bool s_loggedUploadFailure = false;

	struct LightCandidate {
		double distSq;
		vector3d posPlanet;
		Color4f colour;
		float range;
		float strength;
		float additiveFactor;
	};

	static bool BodyOrbitsPlanet(const Body *body, const Planet *planet)
	{
		Frame *bodyFrame = Frame::GetFrame(body->GetFrame());
		Frame *planetFrame = Frame::GetFrame(planet->GetFrame());
		if (!bodyFrame || !planetFrame)
			return false;
		return bodyFrame->GetNonRotFrame() == planetFrame->GetNonRotFrame();
	}

	static float CalcNightTime(const Camera *camera, const Planet *planet, const vector3d &posRelToPlanet)
	{
		const double sunVisibility = camera->CalcSunVisibilityForLocation(planet, posRelToPlanet);
		if (!std::isfinite(sunVisibility) || sunVisibility >= NIGHT_LIGHT_THRESHOLD)
			return 0.f;

		return float(1.0 - (sunVisibility / NIGHT_LIGHT_THRESHOLD));
	}

	static void UploadBlock(Graphics::Material *material, TerrainLocalLightsBlock &block)
	{
		if (!material->SetBufferDynamic(s_terrainLocalLightsData, &block)) {
			if (!s_loggedUploadFailure) {
				Log::Warning("TerrainLocalLights: failed to upload TerrainLocalLightsData (restart after shader changes)\n");
				s_loggedUploadFailure = true;
			}
		}
	}

	static void TryAddLight(
		const vector3d &posPlanet,
		const vector3d &playerPosPlanet,
		const Color4f &colour,
		float range,
		float strength,
		float additiveFactor,
		std::vector<LightCandidate> &candidates)
	{
		if (colour.r + colour.g + colour.b <= 0.f)
			return;

		if (strength <= 0.f || !std::isfinite(strength))
			return;

		if ((posPlanet - playerPosPlanet).LengthSqr() > MAX_SCAN_DISTANCE * MAX_SCAN_DISTANCE)
			return;

		LightCandidate c{};
		c.distSq = (posPlanet - playerPosPlanet).LengthSqr();
		c.posPlanet = posPlanet;
		c.colour = colour;
		c.range = range;
		c.strength = strength;
		c.additiveFactor = additiveFactor;
		candidates.push_back(c);
	}

	static float CalcShipStationLightAttenuation(
		const Ship *ship,
		const Planet *planet,
		const FrameId planetFrame)
	{
		// When a ship is docked, or close to a station, the blinking nav lights are kind of annoying and also don't
		// look realistic because the station is covered in lights so should be illuminated pretty well already.
		// So we dim the ship's lights as it gets closer to the centre of the station's illumination range.

		const Ship::FlightState flightState = ship->GetFlightState();
		if (flightState == Ship::DOCKED || flightState == Ship::DOCKING || flightState == Ship::UNDOCKING)
			return 0.f;

		const vector3d shipPos = ship->GetInterpPositionRelTo(planetFrame);
		float factor = 1.f;

		for (Body *body : Pi::game->GetSpace()->GetBodies()) {
			if (!body->IsType(ObjectType::SPACESTATION))
				continue;

			const SpaceStation *station = static_cast<const SpaceStation *>(body);
			if (!station->IsGroundStation() || !BodyOrbitsPlanet(station, planet))
				continue;

			const vector3d stationPos = station->GetInterpPositionRelTo(planetFrame);
			const float dist = float((shipPos - stationPos).Length());
			const float suppressMax = station->GetTerrainLocalLightRange();
			const float suppressMin = suppressMax * 0.5f;
			const float span = suppressMax - suppressMin;
			const float t = span > 0.f
				? std::max(0.f, std::min(1.f, (dist - suppressMin) / span))
				: 0.f;
			factor = std::min(factor, t);
		}

		return factor;
	}

	static float CalcThrusterLightFlicker(const Ship *ship)
	{
		const uint32_t tick = uint32_t(Pi::game->GetTime() / std::max(Pi::game->GetTimeStep(), 1e-6f));
		Random rng({ tick, uint32_t(uintptr_t(ship)) });
		return 1.f - float(rng.Double()) * SHIP_THRUSTER_FLICKER;
	}

	static void TryAddStarportLight(
		const SpaceStation *station,
		const Planet *planet,
		const FrameId planetFrame,
		const vector3d &playerPosPlanet,
		float nightTime,
		std::vector<LightCandidate> &candidates)
	{
		if (!station->IsGroundStation() || !BodyOrbitsPlanet(station, planet))
			return;

		const vector3d posPlanet = station->GetInterpPositionRelTo(planetFrame);
		const float range = station->GetTerrainLocalLightRange();
		const float strength = STARPORT_LIGHT_STRENGTH * nightTime;
		TryAddLight(posPlanet, playerPosPlanet, STARPORT_LIGHT_COLOR, range, strength, 0.f, candidates);
	}

	static void TryAddShipLights(
		const Ship *ship,
		const Planet *planet,
		const FrameId planetFrame,
		const vector3d &playerPosPlanet,
		float nightTime,
		std::vector<LightCandidate> &candidates)
	{
		if (!BodyOrbitsPlanet(ship, planet))
			return;

		const vector3d posPlanet = ship->GetInterpPositionRelTo(planetFrame);
		const float stationAtten = CalcShipStationLightAttenuation(ship, planet, planetFrame);

		const Color4f navColor = ship->GetNavLightTerrainColor();
		if (navColor.r + navColor.g + navColor.b > 0.f) {
			TryAddLight(
				posPlanet, playerPosPlanet, navColor,
				SHIP_NAV_LIGHT_RANGE, SHIP_NAV_LIGHT_STRENGTH * nightTime * stationAtten, 0.f, candidates);
		}

		Color4f thrusterColor;
		float thrustLevel = 0.f;
		if (!ship->GetThrusterTerrainLight(thrusterColor, thrustLevel))
			return;

		TryAddLight(
			posPlanet, playerPosPlanet, thrusterColor,
			SHIP_THRUSTER_LIGHT_RANGE,
			SHIP_THRUSTER_LIGHT_STRENGTH * thrustLevel * CalcThrusterLightFlicker(ship) * nightTime * stationAtten,
			SHIP_THRUSTER_ADDITIVE_FACTOR, candidates);
	}

	static void FillLightGPU(
		TerrainLocalLightGPU &gpu,
		const LightCandidate &c,
		double planetRadius)
	{
		gpu.position = Color4f(
			float(c.posPlanet.x / planetRadius),
			float(c.posPlanet.y / planetRadius),
			float(c.posPlanet.z / planetRadius),
			0.f);
		gpu.colour = Color4f(c.colour.r, c.colour.g, c.colour.b, 0.f);
		gpu.falloff = Color4f(c.range, c.strength, c.additiveFactor, 0.f);
	}

} // namespace

void TerrainLocalLights::UploadToMaterial(
	Graphics::Material *material,
	const Camera *camera,
	const Planet *planet,
	double planetRadius)
{
	TerrainLocalLightsBlock block{};
	if (!material)
		return;

	if (planetRadius <= 0.0)
		planetRadius = 1.0;

	if (!camera || !planet || !Pi::game) {
		UploadBlock(material, block);
		return;
	}

	Player *player = Pi::game->GetPlayer();
	if (!player || !BodyOrbitsPlanet(player, planet)) {
		UploadBlock(material, block);
		return;
	}

	const FrameId planetFrame = planet->GetFrame();
	const vector3d playerPosPlanet = player->GetInterpPositionRelTo(planetFrame);

	const float nightTime = CalcNightTime(camera, planet, playerPosPlanet);

	std::vector<LightCandidate> candidates;
	candidates.reserve(32);

	for (Body *body : Pi::game->GetSpace()->GetBodies()) {
		if (body->IsType(ObjectType::SPACESTATION)) {
			TryAddStarportLight(static_cast<const SpaceStation *>(body), planet, planetFrame, playerPosPlanet, nightTime, candidates);
		} else if (body->IsType(ObjectType::SHIP)) {
			TryAddShipLights(static_cast<const Ship *>(body), planet, planetFrame, playerPosPlanet, nightTime, candidates);
		}
	}

	std::sort(candidates.begin(), candidates.end(), [](const LightCandidate &a, const LightCandidate &b) {
		return a.distSq < b.distSq;
	});

	block.numLights = std::min<int>(int(candidates.size()), MAX_TERRAIN_LOCAL_LIGHTS);

	for (int i = 0; i < block.numLights; ++i)
		FillLightGPU(block.lights[i], candidates[i], planetRadius);

	UploadBlock(material, block);
}
