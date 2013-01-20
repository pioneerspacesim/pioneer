// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "Pi.h"
#include "WorldView.h"
#include "Player.h"
#include "AmbientSounds.h"
#include "Frame.h"
#include "Planet.h"
#include "Sound.h"
#include "SpaceStation.h"
#include "Game.h"

static int astroNoiseSeed;
static Sound::Event stationNoise;
static Sound::Event starNoise;
static Sound::Event atmosphereNoise;
static Sound::Event planetSurfaceNoise;
static sigc::connection onChangeCamTypeConnection;

void AmbientSounds::Init()
{
	onChangeCamTypeConnection = Pi::worldView->onChangeCamType.connect(sigc::ptr_fun(&AmbientSounds::UpdateForCamType));
}

void AmbientSounds::Uninit()
{
	onChangeCamTypeConnection.disconnect();
}

void AmbientSounds::Update()
{
	float v_env = (Pi::worldView->GetCameraController()->IsExternal() ? 1.0f : 0.5f) * Sound::GetSfxVolume();

	if (Pi::player->GetFlightState() == Ship::DOCKED) {
		if (starNoise.IsPlaying()) {
			float target[2] = {0.0f,0.0f};
			float dv_dt[2] = {1.0f,1.0f};
			starNoise.VolumeAnimate(target, dv_dt);
			starNoise.SetOp(Sound::OP_REPEAT | Sound::OP_STOP_AT_TARGET_VOLUME);
		}
		if (atmosphereNoise.IsPlaying()) {
			float target[2] = {0.0f,0.0f};
			float dv_dt[2] = {1.0f,1.0f};
			atmosphereNoise.VolumeAnimate(target, dv_dt);
			atmosphereNoise.SetOp(Sound::OP_REPEAT | Sound::OP_STOP_AT_TARGET_VOLUME);
		}
		if (planetSurfaceNoise.IsPlaying()) {
			float target[2] = {0.0f,0.0f};
			float dv_dt[2] = {1.0f,1.0f};
			planetSurfaceNoise.VolumeAnimate(target, dv_dt);
			planetSurfaceNoise.SetOp(Sound::OP_REPEAT | Sound::OP_STOP_AT_TARGET_VOLUME);
		}

		if (!stationNoise.IsPlaying()) {
			const char *sounds[] = {
				"Large_Station_ambient",
				"Medium_Station_ambient",
				"Small_Station_ambient"
			};
			// just use a random station noise until we have a
			// concept of 'station size'
			stationNoise.Play(sounds[Pi::player->GetDockedWith()->GetSystemBody()->seed % 3],
					0.3f*v_env, 0.3f*v_env, Sound::OP_REPEAT);
		}
	} else if (Pi::player->GetFlightState() == Ship::LANDED) {
		/* Planet surface noise on rough-landing */
		if (starNoise.IsPlaying()) {
			float target[2] = {0.0f,0.0f};
			float dv_dt[2] = {1.0f,1.0f};
			starNoise.VolumeAnimate(target, dv_dt);
			starNoise.SetOp(Sound::OP_REPEAT | Sound::OP_STOP_AT_TARGET_VOLUME);
		}
		if (atmosphereNoise.IsPlaying()) {
			float target[2] = {0.0f,0.0f};
			float dv_dt[2] = {1.0f,1.0f};
			atmosphereNoise.VolumeAnimate(target, dv_dt);
			atmosphereNoise.SetOp(Sound::OP_REPEAT | Sound::OP_STOP_AT_TARGET_VOLUME);
		}
		if (stationNoise.IsPlaying()) {
			float target[2] = {0.0f,0.0f};
			float dv_dt[2] = {1.0f,1.0f};
			stationNoise.VolumeAnimate(target, dv_dt);
			stationNoise.SetOp(Sound::OP_REPEAT | Sound::OP_STOP_AT_TARGET_VOLUME);
		}

		// lets try something random for the time being
		if (!planetSurfaceNoise.IsPlaying()) {
			SystemBody *sbody = Pi::player->GetFrame()->GetSystemBody();
			assert(sbody);
			const char *sample = NULL;

			if (sbody->m_life > fixed(1,5)) {
				const char *s[] = {
					"Wind", "Thunder_1", "Thunder_2", "Thunder_3",
					"Thunder_4", "Storm", "Rain_Light", "River",
					"RainForestIntroducedNight", "RainForestIntroduced",
					"NormalForestIntroduced"
				};
				sample = s[sbody->seed % 11];
			}
			else if (sbody->m_volatileGas > fixed(1,2)) {
				const char *s[] = {
					"Wind", "Thunder_1", "Thunder_2", "Thunder_3",
					"Thunder_4", "Storm"
				};
				sample = s[sbody->seed % 6];
			}
			else if (sbody->m_volatileGas > fixed(1,10)) {
				sample = "Wind";
			}

			if (sample) {
				planetSurfaceNoise.Play(sample, 0.3f*v_env, 0.3f*v_env, Sound::OP_REPEAT);
			}
		}
    } else if (planetSurfaceNoise.IsPlaying()) {
        // planetSurfaceNoise.IsPlaying() - if we are out of the atmosphere then stop playing
        if (Pi::player->GetFrame()->IsRotFrame()) {
            Body *astro = Pi::player->GetFrame()->GetBody();
            if (astro->IsType(Object::PLANET)) {
                double dist = Pi::player->GetPosition().Length();
                double pressure, density;
                static_cast<Planet*>(astro)->GetAtmosphericState(dist, &pressure, &density);
                if (pressure < 0.001) {
                    // Stop playing surface noise once out of the atmosphere
                    planetSurfaceNoise.Stop();
                }
            }
        }
    } else {
		if (stationNoise.IsPlaying()) {
			float target[2] = {0.0f,0.0f};
			float dv_dt[2] = {1.0f,1.0f};
			stationNoise.VolumeAnimate(target, dv_dt);
			stationNoise.SetOp(Sound::OP_REPEAT | Sound::OP_STOP_AT_TARGET_VOLUME);
		}
		{
			if (Pi::game->IsNormalSpace()) {
				StarSystem *s = Pi::game->GetSpace()->GetStarSystem().Get();
				if (astroNoiseSeed != s->GetSeed()) {
					// change sound!
					astroNoiseSeed = s->GetSeed();
					float target[2] = {0.0f,0.0f};
					float dv_dt[2] = {0.1f,0.1f};
					starNoise.VolumeAnimate(target, dv_dt);
					starNoise.SetOp(Sound::OP_REPEAT | Sound::OP_STOP_AT_TARGET_VOLUME);
					// XXX the way Sound::Event works isn't totally obvious.
					// to destroy the object doesn't stop the sound. it is
					// really just a sound event reference
					starNoise = Sound::Event();
				}
			}
		}
		// when all the sounds are in we can use the body we are in frame of reference to
		if (!starNoise.IsPlaying()) {
			Frame *f = Pi::player->GetFrame();
			if (!f) return; // When player has no frame (game abort) then get outta here!!
			const SystemBody *sbody = f->GetSystemBody();
			const char *sample = 0;
			for (; sbody && !sample; sbody = f->GetSystemBody()) {
				switch (sbody->type) {
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
							if (sbody->mass > fixed(400,1)) {
								sample = "Very_Large_Gas_Giant";
							} else if (sbody->mass > fixed(80,1)) {
								sample = "Large_Gas_Giant";
							} else if (sbody->mass > fixed(20,1)) {
								sample = "Medium_Gas_Giant";
							} else {
								sample = "Small_Gas_Giant";
							}
						}
						break;
					default: sample = 0; break;
				}
				if (sample) {
					starNoise.Play(sample, 0.0f, 0.0f, Sound::OP_REPEAT);
					starNoise.VolumeAnimate(.3f*v_env, .3f*v_env, .05f, .05f);
				} else {
					// go up orbital hierarchy tree to see if we can find a sound
					f = f->GetParent();
					if (f == 0) break;
				}
			}
		}

		Body *astro;
		if ((astro = Pi::player->GetFrame()->GetBody())
			&& Pi::player->GetFrame()->IsRotFrame() && (astro->IsType(Object::PLANET))) {
			double dist = Pi::player->GetPosition().Length();
			double pressure, density;
			static_cast<Planet*>(astro)->GetAtmosphericState(dist, &pressure, &density);
			// maximum volume at around 2km/sec at earth density, pressure
			float volume = float(density * Pi::player->GetVelocity().Length() * 0.0005);
			volume = Clamp(volume, 0.0f, 1.0f) * v_env;
			if (atmosphereNoise.IsPlaying()) {
				float target[2] = {volume, volume};
				float dv_dt[2] = {1.0f,1.0f};
				atmosphereNoise.VolumeAnimate(target, dv_dt);
			} else {
				atmosphereNoise.Play("Atmosphere_Flying", volume, volume, Sound::OP_REPEAT);
			}
		} else {
			float target[2] = {0.0f,0.0f};
			float dv_dt[2] = {1.0f,1.0f};
			atmosphereNoise.VolumeAnimate(target, dv_dt);
			atmosphereNoise.SetOp(Sound::OP_REPEAT | Sound::OP_STOP_AT_TARGET_VOLUME);
		}
	}
}

void AmbientSounds::UpdateForCamType()
{
	const WorldView::CamType cam = Pi::worldView->GetCamType();
	float v_env = (cam == WorldView::CAM_EXTERNAL ? 1.0f : 0.5f) * Sound::GetSfxVolume();

	if (stationNoise.IsPlaying())
		stationNoise.SetVolume(0.3f*v_env, 0.3f*v_env);
	if (starNoise.IsPlaying())
		starNoise.SetVolume(0.3f*v_env, 0.3f*v_env);
	if (planetSurfaceNoise.IsPlaying())
		planetSurfaceNoise.SetVolume(0.3f*v_env, 0.3f*v_env);
}
