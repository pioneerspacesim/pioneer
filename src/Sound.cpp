/*
 * From an old project of mine, way back in 2001 or so.
 */

/*
 * This is tom's dodgy SDL sound code v1.0
 */

#include <SDL.h>
#include <stdio.h>
#include <string.h>
//#include <vorbis/vorbisfile.h>
//#include <unistd.h>
#include <assert.h>
#include "Sound.h"
#include "Body.h"
#include "Pi.h"
#include "Player.h"

namespace Sound {

#define FREQ            22050
#define BUF_SIZE	2048
#define MAX_OGGSTREAMS	2
#define MAX_WAVSTREAMS	16

static const char *sfx_wavs[SFX_MAX] = {
	"pulsecannon.wav",
	"collision.wav",
	"warning.wav",
	"gui_ping.wav",
	"engines.wav"
};

eventid BodyMakeNoise(const Body *b, enum SFX sfx, float vol)
{
	vector3d pos = b->GetPositionRelTo(Pi::player->GetFrame()) - Pi::player->GetPosition();
	matrix4x4d m;
	Pi::player->GetRotMatrix(m);
	pos = m.InverseOf() * pos;

	float len = pos.Length();
	float v[2];
	if (len != 0) {
		vol = vol / (0.002*len);
		double dot = vector3d::Dot(pos.Normalized(), vector3d(vol, 0, 0));

		v[0] = vol * (2.0f - (1.0+dot));
		v[1] = vol * (1.0 + dot);
	} else {
		v[0] = v[1] = vol;
	}
	v[0] = CLAMP(v[0], 0.0f, 1.0f);
	v[1] = CLAMP(v[1], 0.0f, 1.0f);

	return Sound::PlaySfx(sfx, v[0], v[1], false);
}

#if 0
static int bs = 0;
static int sign = 1;
static int bits = 16;
static int endian = 0;

struct ogg_stream {
	int pipe_fd[2];
	OggVorbis_File vf;
	char buf[BUF_SIZE];
	int buf_pos;
	int buf_end;
};
#endif

struct Sample {
	Uint8 *buf;
	Uint32 buf_len;
};

struct SoundEvent {
	const Sample *sample;
	Uint32 buf_pos;
	float volume[2]; // left and right channels
	eventid identifier;
	bool repeat;

	float targetVolume[2];
	float rateOfChange[2]; // per sample
	bool ascend[2];
};	

struct Sample sfx_samples[SFX_MAX];
struct SoundEvent wavstream[MAX_WAVSTREAMS];

static SoundEvent *GetEvent(eventid id)
{
	for (int i=0; i<MAX_WAVSTREAMS; i++) {
		if (wavstream[i].sample && (wavstream[i].identifier == id))
			return &wavstream[i];
	}
	return 0;
}

bool IsEventActive(eventid id)
{
	return GetEvent(id) != 0;
}

bool EventDestroy(eventid id)
{
	SDL_LockAudio();
	SoundEvent *s = GetEvent(id);
	if (s) {
		s->sample = 0;
	}
	SDL_UnlockAudio();
	return s != 0;
}

bool EventSetVolume(eventid id, float vol_left, float vol_right)
{
	SDL_LockAudio();
	bool status = false;
	for (int i=0; i<MAX_WAVSTREAMS; i++) {
		if (wavstream[i].sample && (wavstream[i].identifier == id)) {
			wavstream[i].volume[0] = vol_left;
			wavstream[i].volume[1] = vol_right;
			status = true;
			break;
		}
	}
	SDL_UnlockAudio();
	return status;
}

bool EventVolumeAnimate(eventid id, float targetVols[2], float dv_dt[2])
{
	SDL_LockAudio();
	SoundEvent *ev = GetEvent(id);
	if (ev) {
		ev->targetVolume[0] = targetVols[0];
		ev->targetVolume[1] = targetVols[1];
		ev->rateOfChange[0] = dv_dt[0] / (float)FREQ;
		ev->rateOfChange[1] = dv_dt[1] / (float)FREQ;
	}
	SDL_UnlockAudio();
	return (ev != 0);
}

#if 0
struct ogg_stream oggstream[MAX_OGGSTREAMS];

static struct ogg_stream *get_free_stream ()
{
	int i;
	for (i=0; i<MAX_OGGSTREAMS; i++) {
		if (oggstream[i].pipe_fd[0] == 0) {
			return &oggstream[i];
		}
	}
	return NULL;
}
#endif
/*
 * Volume should be 0-65535
 */
eventid PlaySfx (enum SFX fx, float volume_left, float volume_right, bool repeat)
{
	SDL_LockAudio();
	static Uint32 identifier = 1;
	int idx;
	Uint32 age;
	/* find free wavstream */
	for (idx=0; idx<MAX_WAVSTREAMS; idx++) {
		if (wavstream[idx].sample == NULL) break;
	}
	if (idx == MAX_WAVSTREAMS) {
		/* otherwise overwrite oldest one */
		age = 0; idx = 0;
		for (int i=0; i<MAX_WAVSTREAMS; i++) {
			if ((i==0) || (wavstream[i].buf_pos > age)) {
				idx = i;
				age = wavstream[i].buf_pos;
			}
		}
	}
	wavstream[idx].sample = &sfx_samples[fx];
	wavstream[idx].volume[0] = volume_left;
	wavstream[idx].volume[1] = volume_right;
	wavstream[idx].repeat = repeat;
	wavstream[idx].identifier = identifier;
	wavstream[idx].targetVolume[0] = volume_left;
	wavstream[idx].targetVolume[1] = volume_right;
	wavstream[idx].rateOfChange[0] = wavstream[idx].rateOfChange[1] = 0.0f;
	SDL_UnlockAudio();
	return identifier++;
}

static void fill_audio (void *udata, Uint8 *dsp_buf, int len)
{
	int written = 0;
	int i;
	float val[2];
	int buf_end;
	
	for (i=0; i<MAX_WAVSTREAMS; i++) {
		if (wavstream[i].sample == NULL) continue;
		for (int chan=0; chan<2; chan++) {
			if (wavstream[i].targetVolume[chan] > wavstream[i].volume[chan]) {
				wavstream[i].ascend[chan] = true;
			} else {
				wavstream[i].ascend[chan] = false;
			}
		}
	}
	
	while (written < len) {
		/* Mix them */
		buf_end = 0;
		while ((written < len) && (!buf_end)) {
			val[0] = val[1] = 0;

			for (i=0; i<MAX_WAVSTREAMS; i++) {
				if (wavstream[i].sample == NULL) continue;
				
				for (int chan=0; chan<2; chan++) {
					if (wavstream[i].ascend[chan]) {
						wavstream[i].volume[chan] = MIN(wavstream[i].volume[chan] + wavstream[i].rateOfChange[chan], wavstream[i].targetVolume[chan]);
					} else {
						wavstream[i].volume[chan] = MAX(wavstream[i].volume[chan] - wavstream[i].rateOfChange[chan], wavstream[i].targetVolume[chan]);
					}
				}

				const Sample *s = wavstream[i].sample;
				val[0] += wavstream[i].volume[0] *
					(float) ((Sint16*)s->buf)[wavstream[i].buf_pos/2];
				wavstream[i].buf_pos += 2;
				val[1] += wavstream[i].volume[1] *
					(float) ((Sint16*)s->buf)[wavstream[i].buf_pos/2];
				wavstream[i].buf_pos += 2;
				if (wavstream[i].buf_pos >= s->buf_len) {
					wavstream[i].buf_pos = 0;
					if (!wavstream[i].repeat) {
						wavstream[i].sample = 0;
					}
				}
			}
			val[0] = CLAMP(val[0], -32768.0, 32767.0);
			val[1] = CLAMP(val[1], -32768.0, 32767.0);
			((Sint16*)dsp_buf)[written/2] = (Sint16)val[0];
			written+=2;
			((Sint16*)dsp_buf)[written/2] = (Sint16)val[1];
			written+=2;
		}
	}
}

bool Init ()
{
	SDL_AudioSpec wanted;
	
	if (SDL_Init (SDL_INIT_AUDIO) == -1) {
		fprintf (stderr, "Count not initialise SDL: %s.\n", SDL_GetError ());
		return false;
	}

	wanted.freq = FREQ;
	wanted.channels = 2;
	wanted.format = AUDIO_S16;
	wanted.samples = BUF_SIZE;
	wanted.callback = fill_audio;
	wanted.userdata = NULL;

	if (SDL_OpenAudio (&wanted, NULL) < 0) {
		fprintf (stderr, "Could not open audio: %s\n", SDL_GetError ());
		return false;
	}

	for (int i=0; i<SFX_MAX; i++) {
		char buf[1024];
		snprintf(buf, sizeof(buf), "data/sfx/%s", sfx_wavs[i]);
		SDL_AudioSpec spec;
		if (SDL_LoadWAV(buf, &spec, &sfx_samples[i].buf, &sfx_samples[i].buf_len) == 0) {
			fputs(SDL_GetError(), stderr);
			fputs("\n", stderr);
			sfx_samples[i].buf = 0;
		}
		assert(spec.freq == FREQ);
		assert(spec.format == AUDIO_S16);
		if (spec.channels == 1) {
			// mangle to stereo
			const unsigned int len = sfx_samples[i].buf_len;
			Sint16 *buf = (Sint16*)malloc(2*len);
			Sint16 *monobuf = (Sint16*)sfx_samples[i].buf;
			for (unsigned int s=0; s<len/sizeof(Sint16); s++) {
				buf[2*s] = monobuf[s];
				buf[2*s+1] = monobuf[s];
			}
			SDL_FreeWAV(sfx_samples[i].buf);
			sfx_samples[i].buf = (Uint8*)buf;
			sfx_samples[i].buf_len = 2*len;
		} else assert(spec.channels == 2);
	}
	return true;
}

void Close ()
{
	SDL_CloseAudio ();
}

void Pause (int on)
{
	SDL_PauseAudio (on);
}

} /* namespace Sound */
