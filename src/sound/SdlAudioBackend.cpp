#include "SdlAudioBackend.h"
#include "MathUtil.h"
#include "core/Log.h"

#include "SDL2/SDL.h"

#include <random>
#include <utility>

namespace {

	class AudioDeviceGuard {
	public:
		explicit AudioDeviceGuard(SDL_AudioDeviceID dev) :
			m_dev(dev), m_locked(true)
		{
			SDL_LockAudioDevice(m_dev);
		}

		AudioDeviceGuard(const AudioDeviceGuard &) = delete;

		AudioDeviceGuard(AudioDeviceGuard &&other) :
			m_dev(std::exchange(other.m_dev, 0)), m_locked(std::exchange(other.m_locked, false))
		{
		}

		~AudioDeviceGuard()
		{
			if (m_locked) {
				SDL_UnlockAudioDevice(m_dev);
				m_locked = false;
			}
		}

		AudioDeviceGuard &operator=(const AudioDeviceGuard &) = delete;

		AudioDeviceGuard &operator=(AudioDeviceGuard &&other)
		{
			m_dev = std::exchange(other.m_dev, 0);
			m_locked = std::exchange(other.m_locked, false);
			return *this;
		}

	private:
		SDL_AudioDeviceID m_dev;
		bool m_locked;
	};

} //namespace

Sound::SdlAudioBackend::SdlAudioBackend()
{
	if (SDL_Init(SDL_INIT_AUDIO) == -1) {
		Output("Could not initialize SDL Audio: %s.\n", SDL_GetError());
		throw std::exception();
	}

	SDL_AudioSpec wanted;
	wanted.freq = FREQ;
	wanted.channels = 2;
	wanted.format = AUDIO_S16;
	wanted.samples = BUF_SIZE;
	wanted.callback = fill_audio_callback;
	wanted.userdata = this;

	// Automatically pick the best device.
	m_audioDevice = SDL_OpenAudioDevice(nullptr, 0, &wanted, nullptr, 0);
	if (!m_audioDevice) {
		Output("Could not open audio device: %s\n", SDL_GetError());
		throw std::exception();
	}

	std::default_random_engine prng(std::random_device{}());
	std::uniform_int_distribution<decltype(identifier)> dist;
	identifier = dist(prng);

	Output("Initialized SDL audio backend");
}

Sound::SdlAudioBackend::~SdlAudioBackend()
{
	SDL_CloseAudioDevice(m_audioDevice);

	Output("Destroyed SDL audio backend");
}

void Sound::SdlAudioBackend::DestroyAllEvents()
{
	/* silence any sound events */
	AudioDeviceGuard guard(m_audioDevice);
	for (unsigned int idx = 0; idx < MAX_WAVSTREAMS; idx++) {
		this->DestroyEvent(&wavstream[idx]);
	}
}

void Sound::SdlAudioBackend::DestroyAllEventsExceptMusic()
{
	/* silence any sound events EXCEPT music
	which are on wavstream[0] and [1] */
	AudioDeviceGuard guard(m_audioDevice);
	for (unsigned int idx = 2; idx < MAX_WAVSTREAMS; idx++) {
		this->DestroyEvent(&wavstream[idx]);
	}
}

bool Sound::SdlAudioBackend::EventStop(eventid eid)
{
	if (eid) {
		AudioDeviceGuard guard(m_audioDevice);
		SoundEvent *s = GetEvent(eid);
		if (s) {
			DestroyEvent(s);
		}
		return s != nullptr;
	} else {
		return false;
	}
}

bool Sound::SdlAudioBackend::IsEventPlaying(eventid eid)
{
	if (eid == 0)
		return false;
	else
		return GetEvent(eid) != nullptr;
}

bool Sound::SdlAudioBackend::EventSetOp(eventid eid, Op op)
{
	if (eid == 0) return false;
	bool ret = false;
	AudioDeviceGuard guard(m_audioDevice);
	SoundEvent *se = GetEvent(eid);
	if (se) {
		se->op = op;
		ret = true;
	}
	return ret;
}

bool Sound::SdlAudioBackend::EventVolumeAnimate(eventid eid, const float targetVol1, const float targetVol2, const float dv_dt1, const float dv_dt2)
{
	AudioDeviceGuard guard(m_audioDevice);
	SoundEvent *ev = GetEvent(eid);
	if (ev) {
		ev->targetVolume[0] = targetVol1;
		ev->targetVolume[1] = targetVol2;
		ev->rateOfChange[0] = dv_dt1 / float(FREQ);
		ev->rateOfChange[1] = dv_dt2 / float(FREQ);
	}
	return (ev != nullptr);
}

bool Sound::SdlAudioBackend::EventSetVolume(eventid eid, const float vol_left, const float vol_right)
{
	AudioDeviceGuard guard(m_audioDevice);
	bool status = false;
	for (unsigned int i = 0; i < MAX_WAVSTREAMS; i++) {
		if (wavstream[i].sample && (wavstream[i].identifier == eid)) {
			wavstream[i].volume[0] = vol_left;
			wavstream[i].volume[1] = vol_right;
			wavstream[i].targetVolume[0] = vol_left;
			wavstream[i].targetVolume[1] = vol_right;
			status = true;
			break;
		}
	}
	return status;
}

void Sound::SdlAudioBackend::Pause(int on)
{
	if (bool(on) == (SDL_AUDIO_PAUSED == SDL_GetAudioDeviceStatus(m_audioDevice))) {
		return;
	}
	SDL_PauseAudioDevice(m_audioDevice, on);
}

/*
 * Volume should be 0-65535
 */
Sound::AudioBackend::eventid Sound::SdlAudioBackend::Play(std::string_view key, const float volume_left, const float volume_right, const Op op)
{
	std::string key_str(key);
	auto sample_it = m_samples.find(key_str);
	if (sample_it == m_samples.end()) {
		Warning("Could not find sample with key %s", key_str.c_str());
		return 0;
	}
	const float mix_volume = sample_it->second.isMusic ? 1.0F : GetSfxVolume();
	AudioDeviceGuard guard(m_audioDevice);
	SoundEvent &empty_event = FindFreeEventForSample(sample_it->second);
	empty_event.sample = &sample_it->second;
	empty_event.oggv = 0;
	empty_event.buf_pos = 0;
	empty_event.volume[0] = volume_left * mix_volume;
	empty_event.volume[1] = volume_right * mix_volume;
	empty_event.op = op;
	empty_event.identifier = identifier;
	empty_event.targetVolume[0] = mix_volume;
	empty_event.targetVolume[1] = mix_volume;
	empty_event.rateOfChange[0] = empty_event.rateOfChange[1] = 0.0f;
	return identifier++;
}

void Sound::SdlAudioBackend::BodyMakeNoise(const Body *b, std::string_view key, float vol)
{
	float vl, vr;
	CalculateStereo(b, vol, &vl, &vr);
	this->Play(key, vl, vr, 0);
}

void Sound::SdlAudioBackend::AddSample(std::string_view key, Sample &&sample)
{
	m_samples.emplace(std::string(key), std::move(sample));
}

Sound::SdlAudioBackend::SoundEvent *Sound::SdlAudioBackend::GetEvent(eventid id)
{
	for (unsigned int i = 0; i < MAX_WAVSTREAMS; i++) {
		if (wavstream[i].sample && (wavstream[i].identifier == id))
			return &wavstream[i];
	}
	return nullptr;
}

void Sound::SdlAudioBackend::DestroyEvent(SoundEvent *ev)
{
	if (ev->oggv) {
		// streaming ogg
		ov_clear(ev->oggv);
		delete ev->oggv;
		ev->oggv = 0;
		ev->ogg_data_stream.Reset();
	}
	ev->sample = nullptr;
}

Sound::SdlAudioBackend::SoundEvent &Sound::SdlAudioBackend::FindFreeEventForSample(const Sample &sample)
{
	int idx = -1;
	if (sample.isMusic) {
		idx = nextMusicStream;
		nextMusicStream ^= 1;
		if (wavstream[idx].sample)
			DestroyEvent(&wavstream[idx]);
	} else {
		uint32_t age;
		/* find free wavstream (first two reserved for music) */
		for (idx = 2; idx < MAX_WAVSTREAMS; idx++) {
			if (!wavstream[idx].sample) {
				return wavstream[idx];
			}
		}
		/* otherwise overwrite oldest one */
		age = 0;
		idx = 0;
		for (unsigned int i = 2; i < MAX_WAVSTREAMS; i++) {
			if ((i == 0) || (wavstream[i].buf_pos > age)) {
				idx = i;
				age = wavstream[i].buf_pos;
			}
		}
		DestroyEvent(&wavstream[idx]);
	}

	return wavstream[idx];
}

/*
 * len is the number of floats to put in buffer, NOT full samples (a sample would be 2 floats since stereo)
 */
template <int T_channels, int T_upsample>
void Sound::SdlAudioBackend::fill_audio_1stream(float *buffer, int len, int stream_num)
{
	// inbuf will be smaller for mono and for 22050hz samples
	Sint16 *inbuf = static_cast<Sint16 *>(alloca(len * T_channels / T_upsample));
	// hm pity to put this here ^^ since not used by ev.sample->buf case
	SoundEvent &ev = wavstream[stream_num];
	int inbuf_pos = 0;
	int pos = 0;
	while ((pos < len) && ev.sample) {
		if (!ev.sample->buf.empty()) {
			// already decoded
			inbuf = reinterpret_cast<const Sint16 *>(ev.sample->buf.data());
			inbuf_pos = ev.buf_pos;
		} else {
			// stream ogg vorbis
			// ogg vorbis streaming
			if (!ev.oggv) {
				// open file to start streaming
				ev.oggv = new OggVorbis_File;
				RefCountedPtr<FileSystem::FileData> oggdata = FileSystem::gameDataFiles.ReadFile(ev.sample->path);
				if (!oggdata) {
					Output("Could not open '%s'", ev.sample->path.c_str());
					ev.sample = nullptr;
					return;
				}
				ev.ogg_data_stream.Reset(oggdata);
				oggdata.Reset();
				if (ov_open_callbacks(&ev.ogg_data_stream, ev.oggv, 0, 0, OggFileDataStream::CALLBACKS) < 0) {
					Output("Vorbis could not understand '%s'", ev.sample->path.c_str());
					ev.sample = nullptr;
					return;
				}
			}
			int i = 0;
			// (len-pos) = num floats the destination buffer wants.
			// if we are stereo then to fill this we need (len-pos)*2 bytes
			// if we are mono we want (len-pos) bytes
			int wanted_bytes = (len - pos) * T_channels / T_upsample;
			for (;;) {
				int music_section;
				if (wanted_bytes == 0) break;
				int amt = ov_read(ev.oggv, const_cast<char *>(reinterpret_cast<const char *>(inbuf)) + i,
					wanted_bytes, 0, 2, 1, &music_section);
				i += amt;
				wanted_bytes -= amt;
				if (amt == 0) break;
			}
		}

		while (pos < len) {
			/* Volume animations */
			for (int chan = 0; chan < 2; chan++) {
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
				buffer[pos + 1] += s1;
				pos += 2;
			} else {
				buffer[pos] += s0;
				buffer[pos + 1] += s1;
				buffer[pos + 2] += s0;
				buffer[pos + 3] += s1;
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

void Sound::SdlAudioBackend::fill_audio(Uint8 *dsp_buf, int len)
{
	const int len_in_floats = len >> 1;
	float *tmpbuf = static_cast<float *>(alloca(sizeof(float) * len_in_floats)); // len is in chars not samples
	memset(static_cast<void *>(tmpbuf), 0, sizeof(float) * len_in_floats);

	for (unsigned int i = 0; i < MAX_WAVSTREAMS; i++) {
		if (!wavstream[i].sample) continue;

		wavstream[i].ascend[0] = (wavstream[i].targetVolume[0] > wavstream[i].volume[0]);
		wavstream[i].ascend[1] = (wavstream[i].targetVolume[1] > wavstream[i].volume[1]);

		if (wavstream[i].op & OP_STOP_AT_TARGET_VOLUME) {
			if (wavstream[i].ascend[0] && wavstream[i].ascend[1]) {
				if ((wavstream[i].targetVolume[0] <= wavstream[i].volume[0]) &&
					(wavstream[i].targetVolume[1] <= wavstream[i].volume[1])) {
					DestroyEvent(&wavstream[i]);
					continue;
				}
			} else {
				if ((wavstream[i].targetVolume[0] >= wavstream[i].volume[0]) &&
					(wavstream[i].targetVolume[1] >= wavstream[i].volume[1])) {
					DestroyEvent(&wavstream[i]);
					continue;
				}
			}
		}

		if (wavstream[i].sample->channels == 1) {
			if (wavstream[i].sample->samplerate == FREQ) {
				fill_audio_1stream<1, 1>(tmpbuf, len_in_floats, i);
			} else {
				fill_audio_1stream<1, 2>(tmpbuf, len_in_floats, i);
			}
		} else {
			if (wavstream[i].sample->samplerate == FREQ) {
				fill_audio_1stream<2, 1>(tmpbuf, len_in_floats, i);
			} else {
				fill_audio_1stream<2, 2>(tmpbuf, len_in_floats, i);
			}
		}
	}

	/* Convert float sample buffer to Sint16 samples the hardware likes */
	for (int pos = 0; pos < len_in_floats; pos++) {
		const float val = m_masterVolume * tmpbuf[pos];
		(reinterpret_cast<Sint16 *>(dsp_buf))[pos] = Sint16(Clamp(val, -32768.0f, 32767.0f));
	}
}

void Sound::SdlAudioBackend::fill_audio_callback(void *udata, Uint8 *dsp_buf, int len)
{
	reinterpret_cast<SdlAudioBackend *>(udata)->fill_audio(dsp_buf, len);
}
