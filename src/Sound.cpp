/*
 * Sound, dude
 */

#include <SDL.h>
#include <stdio.h>
#include <assert.h>
#include <vorbis/vorbisfile.h>
#include <vector>
#include <string>
#include <cerrno>
#include "Sound.h"
#include "Body.h"
#include "Pi.h"
#include "Player.h"
#include "FileSystem.h"

namespace Sound {

class OggFileDataStream {
public:
	static const ov_callbacks CALLBACKS;

	OggFileDataStream(): m_cursor(0) {}
	explicit OggFileDataStream(const RefCountedPtr<FileSystem::FileData> &data):
		m_data(data), m_cursor(data->GetData()) { assert(data); }

	void Reset()
	{
		m_data.Reset();
		m_cursor = 0;
	}

	void Reset(const RefCountedPtr<FileSystem::FileData> &data)
	{
		assert(data);
		m_data = data;
		m_cursor = m_data->GetData();
	}

	size_t read(char *buf, size_t sz, int n)
	{
		assert(n >= 0);
		ptrdiff_t offset = tell();
		// clamp to available data
		n = std::min(n, int((m_data->GetSize() - offset) / sz));
		size_t fullsize = sz * n;
		assert(offset + fullsize <= m_data->GetSize());
		memcpy(buf, m_cursor, fullsize);
		m_cursor += fullsize;
		return n;
	}

	int seek(ogg_int64_t offset, int whence)
	{
		switch (whence) {
			case SEEK_SET: m_cursor = m_data->GetData() + offset; break;
			case SEEK_END: m_cursor = m_data->GetData() + (m_data->GetSize() + offset); break;
			case SEEK_CUR: m_cursor += offset; break;
			default: return -1;
		}
		return 0;
	}

	long tell()
	{
		assert(m_data && m_cursor);
		return long(m_cursor - m_data->GetData());
	}

	int close()
	{
		if (m_data) {
			m_data.Reset();
			m_cursor = 0;
			return 0;
		} else {
			return -1;
		}
	}

private:
	static size_t ov_callback_read(void *buf, size_t sz, size_t n, void *stream)
	{ return reinterpret_cast<OggFileDataStream*>(stream)->read(reinterpret_cast<char*>(buf), sz, n); }
	static int ov_callback_seek(void *stream, ogg_int64_t offset, int whence)
	{ return reinterpret_cast<OggFileDataStream*>(stream)->seek(offset, whence); }
	static long ov_callback_tell(void *stream)
	{ return reinterpret_cast<OggFileDataStream*>(stream)->tell(); }
	static int ov_callback_close(void *stream)
	{ return reinterpret_cast<OggFileDataStream*>(stream)->close(); }

	RefCountedPtr<FileSystem::FileData> m_data;
	const char *m_cursor;
};

const ov_callbacks OggFileDataStream::CALLBACKS = {
	&OggFileDataStream::ov_callback_read,
	&OggFileDataStream::ov_callback_seek,
	&OggFileDataStream::ov_callback_close,
	&OggFileDataStream::ov_callback_tell
};

static float m_masterVol = 1.0f;
static float m_sfxVol = 1.0f;

#define FREQ            44100
#define BUF_SIZE	4096
#define MAX_WAVSTREAMS	10 //first two are for music
#define STREAM_IF_LONGER_THAN 10.0

void SetMasterVolume(const float vol)
{
	m_masterVol = vol;
}

float GetMasterVolume()
{
	return m_masterVol;
}

void SetSfxVolume(const float vol)
{
	m_sfxVol = vol;
}

float GetSfxVolume()
{
	return m_sfxVol;
}

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
	if (! is_zero_general(len)) {
		vol = vol / (0.002*len);
		double dot = pos.Normalized().Dot(vector3d(vol, 0, 0));

		v[0] = vol * (2.0f - (1.0+dot));
		v[1] = vol * (1.0 + dot);
	} else {
		v[0] = v[1] = vol;
	}
	v[0] = Clamp(v[0], 0.0f, 1.0f);
	v[1] = Clamp(v[1], 0.0f, 1.0f);

	return Sound::PlaySfx(sfx, v[0], v[1], false);
}

struct SoundEvent {
	const Sample *sample;
	OggVorbis_File *oggv; // if sample->buf = 0 then stream this
	OggFileDataStream ogg_data_stream;
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
	} else {
		//SilentWarning("Unknown sound sample: %s", filename);
		return 0;
	}
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

static void DestroyEvent(SoundEvent *ev)
{
	if (ev->oggv) {
		// streaming ogg
		ov_clear(ev->oggv);
		delete ev->oggv;
		ev->oggv = 0;
		ev->ogg_data_stream.Reset();
	}
	ev->sample = 0;
}


/*
 * Volume should be 0-65535
 */
static Uint32 identifier = 1;
eventid PlaySfx (const char *fx, const float volume_left, const float volume_right, const Op op)
{
	SDL_LockAudio();
	int idx;
	Uint32 age;
	/* find free wavstream (first two reserved for music) */
	for (idx=2; idx<MAX_WAVSTREAMS; idx++) {
		if (wavstream[idx].sample == NULL) break;
	}
	if (idx == MAX_WAVSTREAMS) {
		/* otherwise overwrite oldest one */
		age = 0; idx = 0;
		for (int i=2; i<MAX_WAVSTREAMS; i++) {
			if ((i==0) || (wavstream[i].buf_pos > age)) {
				idx = i;
				age = wavstream[i].buf_pos;
			}
		}
		DestroyEvent(&wavstream[idx]);
	}
	wavstream[idx].sample = GetSample(fx);
	wavstream[idx].oggv = 0;
	wavstream[idx].buf_pos = 0;
	wavstream[idx].volume[0] = volume_left * GetSfxVolume();
	wavstream[idx].volume[1] = volume_right * GetSfxVolume();
	wavstream[idx].op = op;
	wavstream[idx].identifier = identifier;
	wavstream[idx].targetVolume[0] = volume_left * GetSfxVolume();
	wavstream[idx].targetVolume[1] = volume_right * GetSfxVolume();
	wavstream[idx].rateOfChange[0] = wavstream[idx].rateOfChange[1] = 0.0f;
	SDL_UnlockAudio();
	return identifier++;
}

//unlike PlaySfx, we want uninterrupted play and do not care about age
//alternate between two streams for crossfade
static int nextMusicStream = 0;
eventid PlayMusic(const char *fx, const float volume_left, const float volume_right, const Op op)
{
	const int idx = nextMusicStream;
	nextMusicStream ^= 1;
	SDL_LockAudio();
	if (wavstream[idx].sample != NULL)
		DestroyEvent(&wavstream[idx]);
	wavstream[idx].sample = GetSample(fx);
	wavstream[idx].oggv = 0;
	wavstream[idx].buf_pos = 0;
	wavstream[idx].volume[0] = volume_left;
	wavstream[idx].volume[1] = volume_right;
	wavstream[idx].op = op;
	wavstream[idx].identifier = identifier;
	wavstream[idx].targetVolume[0] = volume_left; //already scaled in MusicPlayer
	wavstream[idx].targetVolume[1] = volume_right;
	wavstream[idx].rateOfChange[0] = wavstream[idx].rateOfChange[1] = 0.0f;
	SDL_UnlockAudio();
	return identifier++;
}

/*
 * len is the number of floats to put in buffer, NOT full samples (a sample would be 2 floats since stereo)
 */
template <int T_channels, int T_upsample>
static void fill_audio_1stream(float *buffer, int len, int stream_num)
{
	// inbuf will be smaller for mono and for 22050hz samples
	Sint16 *inbuf = static_cast<Sint16*>(alloca(len*T_channels / T_upsample));
	// hm pity to put this here ^^ since not used by ev.sample->buf case
	SoundEvent &ev = wavstream[stream_num];
	int inbuf_pos = 0;
	int pos = 0;
	while ((pos < len) && ev.sample) {
		if (ev.sample->buf) {
			// already decoded
			inbuf = reinterpret_cast<Sint16 *>(ev.sample->buf);
			inbuf_pos = ev.buf_pos;
		} else {
			// stream ogg vorbis
			// ogg vorbis streaming
			if (!ev.oggv) {
				// open file to start streaming
				ev.oggv = new OggVorbis_File;
				RefCountedPtr<FileSystem::FileData> oggdata = FileSystem::gameDataFiles.ReadFile(ev.sample->path);
				if (!oggdata) {
					fprintf(stderr, "Could not open '%s'", ev.sample->path.c_str());
					ev.sample = 0;
					return;
				}
				ev.ogg_data_stream.Reset(oggdata);
				oggdata.Reset();
				if (ov_open_callbacks(&ev.ogg_data_stream, ev.oggv, 0, 0, OggFileDataStream::CALLBACKS) < 0) {
					fprintf(stderr, "Vorbis could not understand '%s'", ev.sample->path.c_str());
					ev.sample = 0;
					return;
				}
			}
			int i=0;
			// (len-pos) = num floats the destination buffer wants.
			// if we are stereo then to fill this we need (len-pos)*2 bytes
			// if we are mono we want (len-pos) bytes
			int wanted_bytes = (len-pos)*T_channels / T_upsample;
			for (;;) {
				int music_section;
				if (wanted_bytes == 0) break;
				int amt = ov_read(ev.oggv, reinterpret_cast<char*>(inbuf) + i,
						wanted_bytes, 0, 2, 1, &music_section);
				i += amt;
				wanted_bytes -= amt;
				if (amt == 0) break;
			}
		}

		while (pos < len) {
			/* Volume animations */
			for (int chan=0; chan<2; chan++) {
				if (ev.ascend[chan]) {
					ev.volume[chan] = std::min(ev.volume[chan] + ev.rateOfChange[chan], ev.targetVolume[chan]);
				} else {
					ev.volume[chan] = std::max(ev.volume[chan] - ev.rateOfChange[chan], ev.targetVolume[chan]);
				}
			}

			float s0, s1;

			if (T_channels == 1) {
				s0 = float(inbuf[inbuf_pos++]);
				s1 = ev.volume[1] * s0;
				s0 = ev.volume[0] * s0;
				ev.buf_pos += 1;
			} else /* stereo */ {
				s0 = ev.volume[0] * float(inbuf[inbuf_pos++]);
				s1 = ev.volume[1] * float(inbuf[inbuf_pos++]);
				ev.buf_pos += 2;
			}

			if (T_upsample == 1) {
				buffer[pos] += s0;
				buffer[pos+1] += s1;
				pos += 2;
			} else {
				buffer[pos] += s0;
				buffer[pos+1] += s1;
				buffer[pos+2] += s0;
				buffer[pos+3] += s1;
				pos += 4;
			}

			/* Repeat or end? */
			if (ev.buf_pos >= ev.sample->buf_len) {
				ev.buf_pos = 0;
				inbuf_pos = 0;
				if (!(ev.op & OP_REPEAT)) {
					DestroyEvent(&ev);
					break;
				}
				if (ev.oggv) {
					// streaming ogg
					ov_pcm_seek(ev.oggv, 0);
					// repeat outer loop to decode some
					// more vorbis from the start of the stream
					break;
				}
			}
		}
	}
}

static void fill_audio(void *udata, Uint8 *dsp_buf, int len)
{
	const int len_in_floats = len>>1;
	float *tmpbuf = static_cast<float*>(alloca(sizeof(float)*len_in_floats)); // len is in chars not samples
	memset(static_cast<void*>(tmpbuf), 0, sizeof(float)*len_in_floats);

	for (int i=0; i<MAX_WAVSTREAMS; i++) {
		if (wavstream[i].sample == NULL) continue;

		wavstream[i].ascend[0] = (wavstream[i].targetVolume[0] > wavstream[i].volume[0]);
		wavstream[i].ascend[1] = (wavstream[i].targetVolume[1] > wavstream[i].volume[1]);

		if (wavstream[i].op & OP_STOP_AT_TARGET_VOLUME) {
			if ((wavstream[i].targetVolume[0] <= wavstream[i].volume[0]) &&
			    (wavstream[i].targetVolume[1] <= wavstream[i].volume[1])) {
				DestroyEvent(&wavstream[i]);
				continue;
			}
		}

		if (wavstream[i].sample->channels == 1) {
			if (wavstream[i].sample->upsample == 1) {
				fill_audio_1stream<1,1>(tmpbuf, len_in_floats, i);
			} else {
				fill_audio_1stream<1,2>(tmpbuf, len_in_floats, i);
			}
		} else {
			if (wavstream[i].sample->upsample == 1) {
				fill_audio_1stream<2,1>(tmpbuf, len_in_floats, i);
			} else {
				fill_audio_1stream<2,2>(tmpbuf, len_in_floats, i);
			}
		}
	}
	
	/* Convert float sample buffer to Sint16 samples the hardware likes */
	for (int pos=0; pos<len_in_floats; pos++) {
		const float val = m_masterVol * tmpbuf[pos];
		(reinterpret_cast<Sint16*>(dsp_buf))[pos] = Sint16(Clamp(val, -32768.0f, 32767.0f));
	}
}

void DestroyAllEvents()
{
	/* silence any sound events */
	SDL_LockAudio();
	for (int idx=0; idx<MAX_WAVSTREAMS; idx++) {
		DestroyEvent(&wavstream[idx]);
	}
	SDL_UnlockAudio();
}

static void load_sound(const std::string &basename, const std::string &path, bool is_music)
{
	if (basename.size() < 4) return;
	if (basename.substr(basename.size()-4) != ".ogg") return;

	Sample sample;
	OggVorbis_File oggv;

	RefCountedPtr<FileSystem::FileData> oggdata = FileSystem::gameDataFiles.ReadFile(path);
	if (!oggdata) {
		Error("Could not read '%s'", path.c_str());
	}
	OggFileDataStream datastream(oggdata);
	oggdata.Reset();
	if (ov_open_callbacks(&datastream, &oggv, 0, 0, OggFileDataStream::CALLBACKS) < 0) {
		Error("Vorbis could not understand '%s'", path.c_str());
	}
	struct vorbis_info *info;
	info = ov_info(&oggv, -1);

	if ((info->rate != FREQ) && (info->rate != (FREQ>>1))) {
		Error("Vorbis file %s is not %dHz or %dHz. Bad!", path.c_str(), FREQ, FREQ>>1);
	}
	if ((info->channels < 1) || (info->channels > 2)) {
		Error("Vorbis file %s is not mono or stereo. Bad!", path.c_str());
	}

	int resample_multiplier = ((info->rate == (FREQ>>1)) ? 2 : 1);
	const Sint64 num_samples = ov_pcm_total(&oggv, -1);
	// since samples are 16 bits we have:

	sample.buf = 0;
	sample.buf_len = num_samples * info->channels;
	sample.channels = info->channels;
	sample.upsample = resample_multiplier;
	sample.path = path;

	const float seconds = num_samples/float(info->rate);
	//printf("%f seconds\n", seconds);

	// immediately decode and store as raw sample if short enough
	if (seconds < STREAM_IF_LONGER_THAN) {
		sample.buf = new Uint16[sample.buf_len];

		int i=0;
		for (;;) {
			int music_section;
			int amt = ov_read(&oggv, reinterpret_cast<char*>(sample.buf) + i,
					2*sample.buf_len - i, 0, 2, 1, &music_section);
			i += amt;
			if (amt == 0) break;
		}
	}

	if (is_music) {
		sample.isMusic = true;
		// music keyed by pathname minus (datapath)/music/ and extension
		sfx_samples[path.substr(0, path.size() - 4)] = sample;
	} else {
		sample.isMusic = false;
		// sfx keyed by basename minus the .ogg
		sfx_samples[basename.substr(0, basename.size()-4)] = sample;
	}

	ov_clear(&oggv);
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

		// load all the wretched effects
		for (FileSystem::FileEnumerator files(FileSystem::gameDataFiles, "sounds", FileSystem::FileEnumerator::Recurse); !files.Finished(); files.Next()) {
			const FileSystem::FileInfo &info = files.Current();
			if (info.IsFile()) {
				load_sound(info.GetName(), info.GetPath(), false);
			}
		}

		//I'd rather do this in MusicPlayer and store in a different map too, this will do for now
		for (FileSystem::FileEnumerator files(FileSystem::gameDataFiles, "music", FileSystem::FileEnumerator::Recurse); !files.Finished(); files.Next()) {
			const FileSystem::FileInfo &info = files.Current();
			if (info.IsFile()) {
				load_sound(info.GetName(), info.GetPath(), true);
			}
		}
	}

	/* silence any sound events */
	DestroyAllEvents();

	return true;
}

void Uninit ()
{
	DestroyAllEvents();
	std::map<std::string, Sample>::iterator i;
	for (i=sfx_samples.begin(); i!=sfx_samples.end(); ++i) delete[] (*i).second.buf;
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
			DestroyEvent(s);
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

bool Event::VolumeAnimate(const float targetVol1, const float targetVol2, const float dv_dt1, const float dv_dt2)
{
	SDL_LockAudio();
	SoundEvent *ev = GetEvent(eid);
	if (ev) {
		ev->targetVolume[0] = targetVol1;
		ev->targetVolume[1] = targetVol2;
		ev->rateOfChange[0] = dv_dt1 / float(FREQ);
		ev->rateOfChange[1] = dv_dt2 / float(FREQ);
	}
	SDL_UnlockAudio();
	return (ev != 0);
}

bool Event::SetVolume(const float vol_left, const float vol_right)
{
	SDL_LockAudio();
	bool status = false;
	for (int i=0; i<MAX_WAVSTREAMS; i++) {
		if (wavstream[i].sample && (wavstream[i].identifier == eid)) {
			wavstream[i].volume[0] = vol_left;
			wavstream[i].volume[1] = vol_right;
			wavstream[i].targetVolume[0] = vol_left;
			wavstream[i].targetVolume[1] = vol_right;
			status = true;
			break;
		}
	}
	SDL_UnlockAudio();
	return status;
}

const std::map<std::string, Sample> & GetSamples()
{
	return sfx_samples;
}

} /* namespace Sound */
