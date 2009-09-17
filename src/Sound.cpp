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
#define BUF_SIZE	4096
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

	return Sound::PlaySfx(sfx, (Uint16)floor(65535.0*v[0]), (Uint16)floor(65535.0*v[1]), false);
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
	Uint16 volume[2]; // left and right channels
	eventid identifier;
	bool repeat;
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

bool EventSetVolume(eventid id, Uint16 vol_left, Uint16 vol_right)
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
eventid PlaySfx (enum SFX fx, Uint16 volume_left, Uint16 volume_right, bool repeat)
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
	SDL_UnlockAudio();
	return identifier++;
}

#if 0
int PlayOgg (const char *filename)
{
	FILE *f;
	struct ogg_stream *s;

	if ((s = get_free_stream ()) == NULL) return -2;
	
	if ((f = fopen (filename, "rb"))==NULL) return -1;

	if ((ov_open (f, &s->vf, NULL, 0)) < 0) {
		fclose (f);
		return -3;
	}
	pipe (s->pipe_fd);
	return 1;
}
#endif
	
#if 0
static void stream_close (struct ogg_stream *s)
{
	ov_clear (&s->vf);
	close (s->pipe_fd[0]);
	close (s->pipe_fd[1]);
	s->pipe_fd[0] = s->pipe_fd[1] = 0;
}
#endif

/*
 * Call this regularly. It stuffs the pipes to the buffer
 * filling callback thingy with decoded ogg poo.
 */
void UpdateBufferFill ()
{
#if 0
	int i;
	static char buf[BUF_SIZE];
	struct timeval _delay;
	fd_set wr;
	int n=0;

	FD_ZERO (&wr);
	for (i=0; i<MAX_OGGSTREAMS; i++) {
		if (oggstream[i].pipe_fd[0] == 0) continue;
		FD_SET (oggstream[i].pipe_fd[1], &wr);
		/* select wants the highest numbered fd... */
		if (oggstream[i].pipe_fd[1] >= n) {
			n = oggstream[i].pipe_fd[1];
		}
	}

	memset (&_delay, 0, sizeof (_delay));
	_delay.tv_usec = 0;
	
	if (select (n+1, NULL, &wr, NULL, &_delay) <= 0) return;

	/* Some poo to write */
	for (i=0; i<MAX_OGGSTREAMS; i++) {
		if (oggstream[i].pipe_fd[0] == 0) continue;
		if (FD_ISSET (oggstream[i].pipe_fd[1], &wr)) {
			n = ov_read (&oggstream[i].vf, buf, sizeof (buf), endian, bits/8, sign, &bs);
			if (n <= 0) {
				stream_close (&oggstream[i]);
			} else {
				write (oggstream[i].pipe_fd[1], buf, n);
			}
		}

	}
#endif
}

static void fill_audio (void *udata, Uint8 *dsp_buf, int len)
{
	int written = 0;
	int i;
	int val[2];
	int buf_end;
	
	while (written < len) {
#if 0
		for (i=0; i<MAX_OGGSTREAMS; i++) {
			if (oggstream[i].pipe_fd[0]==0) continue;
			if (oggstream[i].buf_end) continue;
			val = read (oggstream[i].pipe_fd[0], oggstream[i].buf, BUF_SIZE);
			oggstream[i].buf_end = val;
			oggstream[i].buf_pos = 0;
			/* End of file */
			if (val == 0) {
				stream_close (&oggstream[i]);
			}
		}
#endif
		/* Mix them */
		buf_end = 0;
		while ((written < len) && (!buf_end)) {
			val[0] = val[1] = 0;
#if 0
			for (i=0; i<MAX_OGGSTREAMS; i++) {
				if (oggstream[i].pipe_fd[0]==0) continue;
				if (oggstream[i].buf_pos < oggstream[i].buf_end) {
					val += ((Sint16*)oggstream[i].buf) [ oggstream[i].buf_pos/2 ];
					oggstream[i].buf_pos += 2;
					if (oggstream[i].buf_pos >= oggstream[i].buf_end) {
						oggstream[i].buf_end = 0;
						buf_end = 1;
					}
				}
			}
#endif
			for (i=0; i<MAX_WAVSTREAMS; i++) {
				if (wavstream[i].sample != NULL) {
					const Sample *s = wavstream[i].sample;
					val[0] += ((int)(((Sint16*)s->buf) [ wavstream[i].buf_pos/2 ]) *
						(int)(wavstream[i].volume[0]))>>16;
					wavstream[i].buf_pos += 2;
					val[1] += ((int)(((Sint16*)s->buf) [ wavstream[i].buf_pos/2 ]) *
						(int)(wavstream[i].volume[1]))>>16;
					wavstream[i].buf_pos += 2;
					if (wavstream[i].buf_pos >= s->buf_len) {
						wavstream[i].buf_pos = 0;
						if (!wavstream[i].repeat) {
							wavstream[i].sample = 0;
						}
					}
				}
			}
			val[0] = CLAMP(val[0], -32768, 32767);
			val[1] = CLAMP(val[1], -32768, 32767);
			((Sint16*)dsp_buf)[written/2] = val[0];
			written+=2;
			((Sint16*)dsp_buf)[written/2] = val[1];
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
