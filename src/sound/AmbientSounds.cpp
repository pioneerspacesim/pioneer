// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "AmbientSounds.h"

#include "Frame.h"
#include "Game.h"
#include "Pi.h"
#include "Planet.h"
#include "Player.h"
#include "Sound.h"
#include "Space.h"
#include "SpaceStation.h"
#include "WorldView.h"

enum EAtmosphereNoiseChannels {
	eAtmoNoise1 = 0,
	eAtmoNoise2,
	eAtmoNoise3,
	eAtmoNoise4,
	eMaxNumAtmosphereSounds
};

static const char *s_airflowTable[eMaxNumAtmosphereSounds] = {
	"airflow01",
	"airflow02",
	"airflow03",
	"airflow04"
};

//	start, inverse range
static const float s_rangeTable[eMaxNumAtmosphereSounds][2] = {
	{ 0.0f, 1.0f / (1.0f - 0.0f) }, // 1
	{ 1.0f, 1.0f / (3.0f - 1.0f) }, // 2
	{ 2.0f, 1.0f / (7.0f - 3.0f) }, // 4
	{ 4.0f, 1.0f / (15.0f - 7.0f) } // 8
};

static const Uint32 NUM_SURFACE_LIFE_SOUNDS = 12;
static const char *s_surfaceLifeSounds[NUM_SURFACE_LIFE_SOUNDS] = {
	"Wind", "Thunder_1", "Thunder_2", "Thunder_3",
	"Thunder_4", "Storm", "Rain_Light", "River",
	"RainForestIntroducedNight", "RainForestIntroduced",
	"NormalForestIntroduced", "RainForestIndigeniusNight"
};

static const Uint32 NUM_SURFACE_DEAD_SOUNDS = 12;
static const char *s_surfaceSounds[NUM_SURFACE_DEAD_SOUNDS] = {
	"Wind", "Thunder_1", "Thunder_2", "Thunder_3",
	"Thunder_4", "Storm"
};

static const Uint32 NUM_STATION_SOUNDS = 3;
static const char *s_stationNoiseSounds[NUM_STATION_SOUNDS] = {
	"Large_Station_ambient",
	"Medium_Station_ambient",
	"Small_Station_ambient"
};

static int astroNoiseSeed;
static Sound::Event s_stationNoise;
static Sound::Event s_starNoise;
static Sound::Event s_atmosphereNoises[eMaxNumAtmosphereSounds];
static Sound::Event s_planetSurfaceNoise;
static sigc::connection onChangeCamTypeConnection;

void AmbientSounds::Init()
{
	onChangeCamTypeConnection = Pi::game->GetWorldView()->shipView->onChangeCamType.connect(sigc::ptr_fun(&AmbientSounds::UpdateForCamType));
}

void AmbientSounds::Uninit()
{
	onChangeCamTypeConnection.disconnect();
}

void AmbientSounds::Update()
{
	const float v_env = (Pi::game->GetWorldView()->shipView->IsExteriorView() ? 1.0f : 0.5f) * Sound::GetSfxVolume();

	if (Pi::player->GetFlightState() == Ship::DOCKED) {
		if (s_starNoise.IsPlaying()) {
			const float target[2] = { 0.0f, 0.0f };
			const float dv_dt[2] = { 1.0f, 1.0f };
			s_starNoise.VolumeAnimate(target, dv_dt);
			s_starNoise.SetOp(Sound::OP_REPEAT | Sound::OP_STOP_AT_TARGET_VOLUME);
		}
		for (int i = 0; i < eMaxNumAtmosphereSounds; i++) {
			if (s_atmosphereNoises[i].IsPlaying()) {
				const float target[2] = { 0.0f, 0.0f };
				const float dv_dt[2] = { 1.0f, 1.0f };
				s_atmosphereNoises[i].VolumeAnimate(target, dv_dt);
				s_atmosphereNoises[i].SetOp(Sound::OP_REPEAT | Sound::OP_STOP_AT_TARGET_VOLUME);
			}
		}
		if (s_planetSurfaceNoise.IsPlaying()) {
			const float target[2] = { 0.0f, 0.0f };
			const float dv_dt[2] = { 1.0f, 1.0f };
			s_planetSurfaceNoise.VolumeAnimate(target, dv_dt);
			s_planetSurfaceNoise.SetOp(Sound::OP_REPEAT | Sound::OP_STOP_AT_TARGET_VOLUME);
		}

		if (!s_stationNoise.IsPlaying()) {
			// just use a random station noise until we have a
			// concept of 'station size'
			s_stationNoise.Play(s_stationNoiseSounds[Pi::player->GetDockedWith()->GetSystemBody()->GetSeed() % NUM_STATION_SOUNDS],
				0.3f * v_env, 0.3f * v_env, Sound::OP_REPEAT);
		}
	} else if (Pi::player->GetFlightState() == Ship::LANDED) {
		/* Planet surface noise on rough-landing */
		if (s_starNoise.IsPlaying()) {
			const float target[2] = { 0.0f, 0.0f };
			const float dv_dt[2] = { 1.0f, 1.0f };
			s_starNoise.VolumeAnimate(target, dv_dt);
			s_starNoise.SetOp(Sound::OP_REPEAT | Sound::OP_STOP_AT_TARGET_VOLUME);
		}
		for (int i = 0; i < eMaxNumAtmosphereSounds; i++) {
			if (s_atmosphereNoises[i].IsPlaying()) {
				const float target[2] = { 0.0f, 0.0f };
				const float dv_dt[2] = { 1.0f, 1.0f };
				s_atmosphereNoises[i].VolumeAnimate(target, dv_dt);
				s_atmosphereNoises[i].SetOp(Sound::OP_REPEAT | Sound::OP_STOP_AT_TARGET_VOLUME);
			}
		}
		if (s_stationNoise.IsPlaying()) {
			const float target[2] = { 0.0f, 0.0f };
			const float dv_dt[2] = { 1.0f, 1.0f };
			s_stationNoise.VolumeAnimate(target, dv_dt);
			s_stationNoise.SetOp(Sound::OP_REPEAT | Sound::OP_STOP_AT_TARGET_VOLUME);
		}

		// lets try something random for the time being
		if (!s_planetSurfaceNoise.IsPlaying()) {
			const SystemBody *sbody = Frame::GetFrame(Pi::player->GetFrame())->GetSystemBody();
			assert(sbody);
			const char *sample = 0;

			if (sbody->GetLifeAsFixed() > fixed(1, 5)) {
				sample = s_surfaceLifeSounds[sbody->GetSeed() % NUM_SURFACE_LIFE_SOUNDS];
			} else if (sbody->GetVolatileGasAsFixed() > fixed(1, 2)) {
				sample = s_surfaceSounds[sbody->GetSeed() % NUM_SURFACE_DEAD_SOUNDS];
			} else if (sbody->GetVolatileGasAsFixed() > fixed(1, 10)) {
				sample = "Wind";
			}

			if (sample) {
				s_planetSurfaceNoise.Play(sample, 0.3f * v_env, 0.3f * v_env, Sound::OP_REPEAT);
			}
		}
	} else if (s_planetSurfaceNoise.IsPlaying()) {
		// s_planetSurfaceNoise.IsPlaying() - if player is no longer on the ground then stop playing
		Frame *playerFrame = Frame::GetFrame(Pi::player->GetFrame());
		if (playerFrame->IsRotFrame()) {
			const Body *astro = playerFrame->GetBody();
			if (astro->IsType(ObjectType::PLANET)) {
				const double dist = Pi::player->GetPosition().Length();
				double pressure, density;
				static_cast<const Planet *>(astro)->GetAtmosphericState(dist, &pressure, &density);
				if (Pi::player->GetFlightState() != Ship::LANDED) {
					// Stop playing surface noise once the ship is off the ground
					s_planetSurfaceNoise.Stop();
				}
			}
		} else if (Pi::game->IsHyperspace()) {
			s_planetSurfaceNoise.Stop();
		}
	} else {
		if (s_stationNoise.IsPlaying()) {
			const float target[2] = { 0.0f, 0.0f };
			const float dv_dt[2] = { 1.0f, 1.0f };
			s_stationNoise.VolumeAnimate(target, dv_dt);
			s_stationNoise.SetOp(Sound::OP_REPEAT | Sound::OP_STOP_AT_TARGET_VOLUME);
		}
		if (Pi::game->IsNormalSpace()) {
			StarSystem *s = Pi::game->GetSpace()->GetStarSystem().Get();
			if (astroNoiseSeed != s->GetSeed()) {
				// change sound!
				astroNoiseSeed = s->GetSeed();
				const float target[2] = { 0.0f, 0.0f };
				const float dv_dt[2] = { 0.1f, 0.1f };
				s_starNoise.VolumeAnimate(target, dv_dt);
				s_starNoise.SetOp(Sound::OP_REPEAT | Sound::OP_STOP_AT_TARGET_VOLUME);
				// XXX the way Sound::Event works isn't totally obvious.
				// to destroy the object doesn't stop the sound. it is
				// really just a sound event reference
				s_starNoise = Sound::Event();
			}
		}
		// when all the sounds are in we can use the body we are in frame of reference to
		if (!s_starNoise.IsPlaying()) {
			Frame *f = Frame::GetFrame(Pi::player->GetFrame());
			if (!f) return; // When player has no frame (game abort) then get outta here!!
			const SystemBody *sbody = f->GetSystemBody();
			const char *sample = 0;
			for (; sbody && !sample; sbody = f->GetSystemBody()) {
				switch (sbody->GetType()) {
				case SystemBody::TYPE_BROWN_DWARF: sample = "Brown_Dwarf_Substellar_Object"; break;
				case SystemBody::TYPE_STAR_M: sample = "M_Red_Star"; break;
				case SystemBody::TYPE_STAR_K: sample = "K_Star"; break;
				case SystemBody::TYPE_WHITE_DWARF: sample = "White_Dwarf_Star"; break;
				case SystemBody::TYPE_STAR_G: sample = "G_Star"; break;
				case SystemBody::TYPE_STAR_F: sample = "F_Star"; break;
				case SystemBody::TYPE_STAR_A: sample = "A_Star"; break;
				case SystemBody::TYPE_STAR_B: sample = "B_Hot_Blue_STAR"; break;
				case SystemBody::TYPE_STAR_O: sample = "Blue_Super_Giant"; break;
				case SystemBody::TYPE_PLANET_GAS_GIANT: {
					if (sbody->GetMassAsFixed() > fixed(400, 1)) {
						sample = "Very_Large_Gas_Giant";
					} else if (sbody->GetMassAsFixed() > fixed(80, 1)) {
						sample = "Large_Gas_Giant";
					} else if (sbody->GetMassAsFixed() > fixed(20, 1)) {
						sample = "Medium_Gas_Giant";
					} else {
						sample = "Small_Gas_Giant";
					}
				} break;
				default: sample = 0; break;
				}
				if (sample) {
					s_starNoise.Play(sample, 0.0f, 0.0f, Sound::OP_REPEAT);
					s_starNoise.VolumeAnimate(.3f * v_env, .3f * v_env, .05f, .05f);
				} else {
					// go up orbital hierarchy tree to see if we can find a sound
					FrameId parent = f->GetParent();
					f = Frame::GetFrame(parent);
					if (f == nullptr) break;
				}
			}
		}

		Frame *playerFrame = Frame::GetFrame(Pi::player->GetFrame());
		const Body *astro = playerFrame->GetBody();
		if (astro && playerFrame->IsRotFrame() && (astro->IsType(ObjectType::PLANET))) {
			double dist = Pi::player->GetPosition().Length();
			double pressure, density;
			static_cast<const Planet *>(astro)->GetAtmosphericState(dist, &pressure, &density);
			// maximum volume at around 2km/sec at earth density, pressure
			const float pressureVolume = float(density * Pi::player->GetVelocity().Length() * 0.0005);
			//volume = Clamp(volume, 0.0f, 1.0f) * v_env;
			float volumes[eMaxNumAtmosphereSounds];
			for (int i = 0; i < eMaxNumAtmosphereSounds; i++) {
				const float beg = s_rangeTable[i][0];
				const float inv = s_rangeTable[i][1];
				volumes[i] = Clamp((pressureVolume - beg) * inv, 0.0f, 1.0f) * v_env;
			}

			for (int i = 0; i < eMaxNumAtmosphereSounds; i++) {
				const float volume = volumes[i];
				if (s_atmosphereNoises[i].IsPlaying()) {
					const float target[2] = { volume, volume };
					const float dv_dt[2] = { 1.0f, 1.0f };
					s_atmosphereNoises[i].VolumeAnimate(target, dv_dt);
				} else {
					s_atmosphereNoises[i].Play(s_airflowTable[i], volume, volume, Sound::OP_REPEAT);
				}
			}
		} else {
			const float target[2] = { 0.0f, 0.0f };
			const float dv_dt[2] = { 1.0f, 1.0f };
			for (int i = 0; i < eMaxNumAtmosphereSounds; i++) {
				s_atmosphereNoises[i].VolumeAnimate(target, dv_dt);
				s_atmosphereNoises[i].SetOp(Sound::OP_REPEAT | Sound::OP_STOP_AT_TARGET_VOLUME);
			}
		}
	}
}

void AmbientSounds::UpdateForCamType()
{
	const ShipViewController::CamType cam = Pi::game->GetWorldView()->shipView->GetCamType();
	float v_env = (cam == ShipViewController::CAM_EXTERNAL ? 1.0f : 0.5f) * Sound::GetSfxVolume();

	if (s_stationNoise.IsPlaying())
		s_stationNoise.SetVolume(0.3f * v_env, 0.3f * v_env);
	if (s_starNoise.IsPlaying())
		s_starNoise.SetVolume(0.3f * v_env, 0.3f * v_env);
	if (s_planetSurfaceNoise.IsPlaying())
		s_planetSurfaceNoise.SetVolume(0.3f * v_env, 0.3f * v_env);
}
