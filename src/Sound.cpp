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

/*static const char *sfx_wavs[SFX_MAX] = {
	"pulsecannon.wav",
	"collision.wav",
	"warning.wav",
	"gui_ping.wav",
	"engines.wav",
	"ecm.wav"
};*/

eventid BodyMakeNoise(const Body *b, const char *sfx, float vol)
{
	vector3d pos;
       
	if (b == Pi::player) {
		pos = vector3d(0.0);
	} else {
		pos = b->GetPositionRelTo(Pi::player->GetFrame()) - Pi::player->GetPosition();
		matrix4x4d m;
		Pi::player->GetRotMatrix(m);
		pos = m.InverseOf() * pos;
	}

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
	Uint32 op;

	float targetVolume[2];
	float rateOfChange[2]; // per sample
	bool ascend[2];
};	

static std::map<std::string, Sample> sfx_samples;
struct SoundEvent wavstream[MAX_WAVSTREAMS];

static Sample *GetSample(const char *filename)
{
	if (sfx_samples.find(filename) != sfx_samples.end()) {
		return &sfx_samples[filename];
	}
	char buf[1024];
	Sample *sam = &sfx_samples[filename];
	snprintf(buf, sizeof(buf), "data/sfx/%s", filename);
	SDL_AudioSpec spec;
	if (SDL_LoadWAV(buf, &spec, &sam->buf, &sam->buf_len) == 0) {
		fputs(SDL_GetError(), stderr);
		fputs("\n", stderr);
		sam->buf = 0;
	}
	assert(spec.freq == FREQ);
	assert(spec.format == AUDIO_S16);
	if (spec.channels == 1) {
		// mangle to stereo
		const unsigned int len = sam->buf_len;
		Sint16 *buf = (Sint16*)malloc(2*len);
		Sint16 *monobuf = (Sint16*)sam->buf;
		for (unsigned int s=0; s<len/sizeof(Sint16); s++) {
			buf[2*s] = monobuf[s];
			buf[2*s+1] = monobuf[s];
		}
		SDL_FreeWAV(sam->buf);
		sam->buf = (Uint8*)buf;
		sam->buf_len = 2*len;
	} else assert(spec.channels == 2);

	return sam;
}

static SoundEvent *GetEvent(eventid id)
{
	for (int i=0; i<MAX_WAVSTREAMS; i++) {
		if (wavstream[i].sample && (wavstream[i].identifier == id))
			return &wavstream[i];
	}
	return 0;
}

bool SetOp(eventid id, Op op)
{
	if (id == 0) return false;
	bool ret = false;
	SDL_LockAudio();
	SoundEvent *se = GetEvent(id);
	if (se) {
		se->op = op;
		ret = true;
	}
	SDL_UnlockAudio();
	return ret;
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
eventid PlaySfx (const char *fx, float volume_left, float volume_right, Op op)
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
	wavstream[idx].sample = GetSample(fx);
	wavstream[idx].volume[0] = volume_left;
	wavstream[idx].volume[1] = volume_right;
	wavstream[idx].op = op;
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
		if (wavstream[i].op & OP_STOP_AT_TARGET_VOLUME) {
			if ((wavstream[i].targetVolume[0] == wavstream[i].volume[0]) &&
			    (wavstream[i].targetVolume[1] == wavstream[i].volume[1])) {
				wavstream[i].sample = 0;
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
					if (!(wavstream[i].op & OP_REPEAT)) {
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

void DestroyAllEvents()
{
	/* silence any sound events */
	for (int idx=0; idx<MAX_WAVSTREAMS; idx++) {
		wavstream[idx].sample = 0;
	}
}

bool Init ()
{
	static bool isInitted = false;

	if (!isInitted) {
		isInitted = true;
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
	}

	/* silence any sound events */
	DestroyAllEvents();

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

void Event::Play(const char *fx, float volume_left, float volume_right, Op op)
{
	Stop();
	eid = PlaySfx(fx, volume_left, volume_right, op);
}

bool Event::Stop()
{
	if (eid) {
		SDL_LockAudio();
		SoundEvent *s = GetEvent(eid);
		if (s) {
			s->sample = 0;
		}
		SDL_UnlockAudio();
		return s != 0;
	} else {
		return false;
	}
}

bool Event::IsPlaying() const
{
	if (eid == 0) return false;
	else return GetEvent(eid) != 0;
}

bool Event::SetOp(Op op) {
	if (eid == 0) return false;
	bool ret = false;
	SDL_LockAudio();
	SoundEvent *se = GetEvent(eid);
	if (se) {
		se->op = op;
		ret = true;
	}
	SDL_UnlockAudio();
	return ret;
}

bool Event::VolumeAnimate(float targetVols[2], float dv_dt[2])
{
	SDL_LockAudio();
	SoundEvent *ev = GetEvent(eid);
	if (ev) {
		ev->targetVolume[0] = targetVols[0];
		ev->targetVolume[1] = targetVols[1];
		ev->rateOfChange[0] = dv_dt[0] / (float)FREQ;
		ev->rateOfChange[1] = dv_dt[1] / (float)FREQ;
	}
	SDL_UnlockAudio();
	return (ev != 0);
}

bool Event::SetVolume(float vol_left, float vol_right)
{
	SDL_LockAudio();
	bool status = false;
	for (int i=0; i<MAX_WAVSTREAMS; i++) {
		if (wavstream[i].sample && (wavstream[i].identifier == eid)) {
			wavstream[i].volume[0] = vol_left;
			wavstream[i].volume[1] = vol_right;
			status = true;
			break;
		}
	}
	SDL_UnlockAudio();
	return status;
}

} /* namespace Sound */
