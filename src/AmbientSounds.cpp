#include "libs.h"
#include "Pi.h"
#include "WorldView.h"
#include "Player.h"
#include "AmbientSounds.h"
#include "Frame.h"
#include "Planet.h"
#include "Sound.h"
#include "SpaceStation.h"

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
	WorldView::CamType cam = Pi::worldView->GetCamType();
	float v_env = (cam == WorldView::CAM_EXTERNAL ? 1.0f : 0.5f);

	if (Pi::player->GetDockedWith()) {
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
			stationNoise.Play(sounds[Pi::player->GetDockedWith()->GetSBody()->seed % 3],
					0.3f*v_env, 0.3f*v_env, true);
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
			SBody *sbody = Pi::player->GetFrame()->GetSBodyFor();
			assert(sbody);
			const char *sample;
			switch (sbody->type) {
				case SBody::TYPE_PLANET_SMALL:
				case SBody::TYPE_PLANET_CO2:
				case SBody::TYPE_PLANET_METHANE:
					{
						const char *s[] = { "Wind", "Storm" };
						sample = s[sbody->seed % 2];
					}
					break;
				case SBody::TYPE_PLANET_CO2_THICK_ATMOS:
				case SBody::TYPE_PLANET_METHANE_THICK_ATMOS:
				case SBody::TYPE_PLANET_HIGHLY_VOLCANIC:
				case SBody::TYPE_PLANET_WATER:
				case SBody::TYPE_PLANET_DESERT:
					{
						const char *s[] = {
							"Wind", "Thunder_1", "Thunder_2", "Thunder_3",
							"Thunder_4", "Storm"
						};
						sample = s[sbody->seed % 6];
					}
					break;
				case SBody::TYPE_PLANET_WATER_THICK_ATMOS:
					{
						const char *s[] = {
							"Wind", "Thunder_1", "Thunder_2", "Thunder_3",
							"Thunder_4", "Storm", "Rain_Light", "River"
						};
						sample = s[sbody->seed % 8];
					}
					break;
				case SBody::TYPE_PLANET_INDIGENOUS_LIFE:
				case SBody::TYPE_PLANET_TERRAFORMED_POOR:
				case SBody::TYPE_PLANET_TERRAFORMED_GOOD:
					{
						const char *s[] = {
							"Wind", "Thunder_1", "Thunder_2", "Thunder_3",
							"Thunder_4", "Storm", "Rain_Light", "River",
							"RainForestIntroducedNight", "RainForestIntroduced",
							"NormalForestIntroduced"
						};
						sample = s[sbody->seed % 11];
					}
					break;
				default: sample = 0;
			}
			if (sample) {
				planetSurfaceNoise.Play(sample, 0.3f*v_env, 0.3f*v_env, Sound::OP_REPEAT);
			}
		}
	} else {
		if (stationNoise.IsPlaying()) {
			float target[2] = {0.0f,0.0f};
			float dv_dt[2] = {1.0f,1.0f};
			stationNoise.VolumeAnimate(target, dv_dt);
			stationNoise.SetOp(Sound::OP_REPEAT | Sound::OP_STOP_AT_TARGET_VOLUME);
		}
		if (astroNoiseSeed != Pi::currentSystem->m_seed) {
			// change sound!
			astroNoiseSeed = Pi::currentSystem->m_seed;
			float target[2] = {0.0f,0.0f};
			float dv_dt[2] = {0.1f,0.1f};
			starNoise.VolumeAnimate(target, dv_dt);
			starNoise.SetOp(Sound::OP_REPEAT | Sound::OP_STOP_AT_TARGET_VOLUME);
			// XXX the way Sound::Event works isn't totally obvious.
			// to destroy the object doesn't stop the sound. it is
			// really just a sound event reference
			starNoise = Sound::Event();
		} 
		// when all the sounds are in we can use the body we are in frame of reference to
		if (!starNoise.IsPlaying()) {
			Frame *f = Pi::player->GetFrame();
			const SBody *sbody = f->GetSBodyFor();
			const char *sample = 0;
			for (; sbody && !sample; sbody = f->GetSBodyFor()) {
				switch (sbody->type) {
					case SBody::TYPE_BROWN_DWARF: sample = "Brown_Dwarf_Substellar_Object"; break;
					case SBody::TYPE_STAR_M: sample = "M_Red_Star"; break;
					case SBody::TYPE_STAR_K: sample = "K_Star"; break;
					case SBody::TYPE_WHITE_DWARF: sample = "White_Dwarf_Star"; break;
					case SBody::TYPE_STAR_G: sample = "G_Star"; break;
					case SBody::TYPE_STAR_F: sample = "F_Star"; break;
					case SBody::TYPE_STAR_A: sample = "A_Star"; break;
					case SBody::TYPE_STAR_B: sample = "B_Hot_Blue_STAR"; break;
					case SBody::TYPE_STAR_O: sample = "Blue_Super_Giant"; break;
					case SBody::TYPE_PLANET_SMALL_GAS_GIANT: sample = "Small_Gas_Giant"; break;
					case SBody::TYPE_PLANET_MEDIUM_GAS_GIANT: sample = "Medium_Gas_Giant"; break;
					case SBody::TYPE_PLANET_LARGE_GAS_GIANT: sample = "Large_Gas_Giant"; break;
					case SBody::TYPE_PLANET_VERY_LARGE_GAS_GIANT: sample = "Very_Large_Gas_Giant"; break;
					default: sample = 0; break;
				}
				if (sample) {
					starNoise.Play(sample, 0.0f, 0.0f, Sound::OP_REPEAT);
					starNoise.VolumeAnimate(.3f*v_env, .3f*v_env, .05f, .05f);
				} else {
					// go up orbital hierarchy tree to see if we can find a sound
					f = f->m_parent;
					if (f == 0) break;
				}
			}
		}

		Body *astro;
		if ((astro = Pi::player->GetFrame()->m_astroBody) && (astro->IsType(Object::PLANET))) {
			double dist = Pi::player->GetPosition().Length();
			float pressure, density;
			((Planet*)astro)->GetAtmosphericState(dist, pressure, density);
			// maximum volume at around 2km/sec at earth density, pressure
			float volume = density * Pi::player->GetVelocity().Length() * 0.0005;
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
	float v_env = (cam == WorldView::CAM_EXTERNAL ? 1.0f : 0.5f);

	if (stationNoise.IsPlaying())
		stationNoise.SetVolume(0.3f*v_env, 0.3f*v_env);
	if (starNoise.IsPlaying())
		starNoise.SetVolume(0.3f*v_env, 0.3f*v_env);
	if (planetSurfaceNoise.IsPlaying())
		planetSurfaceNoise.SetVolume(0.3f*v_env, 0.3f*v_env);
}
