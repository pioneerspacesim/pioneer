// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifdef PI_BUILD_WITH_OPENAL

#include "AlAudioBackend.h"
#include "Pi.h"
#include "Player.h"
#include "core/Log.h"

#include "AL/alext.h"

#include <algorithm>
#include <utility>

#define CHECK_OPENAL_ERROR_IMPL(file, line, fn, ...)                                                                                    \
	do {                                                                                                                                \
		fn(__VA_ARGS__);                                                                                                                \
		if (auto error = alGetError(); error) {                                                                                         \
			throw std::runtime_error("OpenAL error invoking " #fn "(" file ":" + std::to_string(line) + "): " + std::to_string(error)); \
		}                                                                                                                               \
	} while (0)

#define CHECK_OPENAL_ERROR(fn, ...) CHECK_OPENAL_ERROR_IMPL(__FILE__, __LINE__, fn, __VA_ARGS__)

namespace {
	float calculate_pan(float left, float right)
	{
		if (left == 0.f && right == 0.f) {
			return 0.f;
		}
		return (right - left) / (right + left);
	}
} //namespace

Sound::AlAudioBackend::AlAudioBackend()
{
	m_device = alcOpenDevice(nullptr);
	if (!m_device) {
		Error("Could not open OpenAL device");
		throw std::exception();
	}

	m_context = alcCreateContext(m_device, nullptr);
	if (!m_context) {
		Error("Could not create OpenAL context");
		throw std::exception();
	}

	const auto success = alcMakeContextCurrent(m_context);
	if (!success) {
		Error("Could not make OpenAL context current");
		throw std::exception();
	}

	CHECK_OPENAL_ERROR(alDistanceModel, AL_INVERSE_DISTANCE_CLAMPED);
	CHECK_OPENAL_ERROR(alSpeedOfSound, 1000.f);

	m_next_event_id = Pi::rng.Int32();

	Output("Initialized OpenAL audio backend");
}

Sound::AlAudioBackend::~AlAudioBackend()
{
	m_events.clear();
	alcDestroyContext(m_context);
	m_context = nullptr;
	alcCloseDevice(m_device);
	m_device = nullptr;
	Output("Destroyed OpenAL audio backend");
}

void Sound::AlAudioBackend::DestroyAllEvents()
{
	m_events.clear();
}

void Sound::AlAudioBackend::DestroyAllEventsExceptMusic()
{
	std::vector<eventid> events_to_remove;
	for (auto &[id, ev] : m_events) {
		if (!ev.IsMusic()) {
			events_to_remove.push_back(id);
		}
	}
	for (auto id : events_to_remove) {
		m_events.erase(id);
	}
}

bool Sound::AlAudioBackend::EventStop(eventid eid)
{
	return m_events.erase(eid) > 0;
}

bool Sound::AlAudioBackend::IsEventPlaying(eventid eid)
{
	auto it = m_events.find(eid);
	if (it == m_events.end()) {
		return false;
	}
	return !it->second.IsDone();
}

bool Sound::AlAudioBackend::EventSetOp(eventid eid, Op op)
{
	auto it = m_events.find(eid);
	if (it == m_events.end()) {
		return false;
	}
	it->second.SetOp(op);
	return true;
}

bool Sound::AlAudioBackend::EventVolumeAnimate(eventid eid, const float targetVol1, const float targetVol2, const float dv_dt1, const float /*dv_dt2*/)
{
	auto it = m_events.find(eid);
	if (it == m_events.end()) {
		return false;
	}
	it->second.SetTargetGain(targetVol1, targetVol2, dv_dt1);
	return true;
}

bool Sound::AlAudioBackend::EventSetVolume(eventid eid, const float vol_left, const float vol_right)
{
	auto it = m_events.find(eid);
	if (it == m_events.end()) {
		return false;
	}
	it->second.SetGain(vol_left, vol_right);
	return true;
}

void Sound::AlAudioBackend::Pause(int on)
{
	for (auto &[id, ev] : m_events) {
		ALint state;
		CHECK_OPENAL_ERROR(alGetSourcei, ev.GetSource(), AL_SOURCE_STATE, &state);

		if (on && state == AL_PLAYING) {
			CHECK_OPENAL_ERROR(alSourcePause, ev.GetSource());
		} else if (!on && state == AL_PAUSED) {
			CHECK_OPENAL_ERROR(alSourcePlay, ev.GetSource());
		}
	}
}

Sound::AudioBackend::eventid Sound::AlAudioBackend::Play(std::string_view key, const float volume_left, const float volume_right, const Op op)
{
	const std::string key_str(key);
	auto sample_it = m_samples.find(key_str);
	if (sample_it == m_samples.end()) {
		Output("Could not find %s in samples", key_str.c_str());
		return 0;
	}
	auto it = m_events.emplace(++m_next_event_id, sample_it->second).first;
	it->second.SetOp(op);
	it->second.SetGain(volume_left * m_sfxVolume, volume_right * m_sfxVolume);
	CHECK_OPENAL_ERROR(alSourcePlay, it->second.GetSource());
	return it->first;
}

void Sound::AlAudioBackend::BodyMakeNoise(const Body *b, std::string_view key, float vol)
{
	constexpr double distance_threshold = 3000;
	auto pos = b->GetPositionRelTo(Pi::player);
	pos = pos * Pi::player->GetOrient();

	if (pos.Length() > distance_threshold) {
		return;
	}

	const std::string key_str(key);
	auto sample_it = m_samples.find(key_str);
	if (sample_it == m_samples.end()) {
		Output("Could not find %s in samples", key_str.c_str());
		return;
	}

	auto it = m_events.emplace(++m_next_event_id, sample_it->second).first;
	it->second.SetGain(m_sfxVolume);

	CHECK_OPENAL_ERROR(alSource3f, it->second.GetSource(), AL_POSITION, pos.x, pos.y, pos.z);

	auto vel = b->GetVelocityRelTo(Pi::player);
	vel = vel * Pi::player->GetOrient();
	CHECK_OPENAL_ERROR(alSource3f, it->second.GetSource(), AL_VELOCITY, vel.x, vel.y, vel.z);

	CHECK_OPENAL_ERROR(alSourcei, it->second.GetSource(), AL_REFERENCE_DISTANCE, 10);
	CHECK_OPENAL_ERROR(alSourcef, it->second.GetSource(), AL_ROLLOFF_FACTOR, 0.1f);
	CHECK_OPENAL_ERROR(alSourcePlay, it->second.GetSource());
}

void Sound::AlAudioBackend::SetMasterVolume(float vol)
{
	m_masterVolume = vol;
	CHECK_OPENAL_ERROR(alListenerf, AL_GAIN, m_masterVolume);
}

float Sound::AlAudioBackend::GetMasterVolume()
{
	return m_masterVolume;
}

void Sound::AlAudioBackend::SetSfxVolume(float vol)
{
	m_sfxVolume = vol;
}

float Sound::AlAudioBackend::GetSfxVolume()
{
	return m_sfxVolume;
}

void Sound::AlAudioBackend::AddSample(std::string_view key, Sample &&sample)
{
	m_samples[std::string(key)] = std::move(sample);
}

void Sound::AlAudioBackend::Update(float delta_t)
{
	std::vector<eventid> events_to_remove;
	for (auto &[id, ev] : m_events) {
		ev.Update(delta_t);
		if (ev.IsDone()) {
			events_to_remove.push_back(id);
		}
	}
	for (auto id : events_to_remove) {
		m_events.erase(id);
	}
}

bool Sound::AlAudioBackend::IsBinauralSupported()
{
	return alcIsExtensionPresent(m_device, "ALC_SOFT_HRTF");
}

void Sound::AlAudioBackend::EnableBinaural(bool enabled)
{
	if (!IsBinauralSupported()) {
		Output("Cannot enable/disable binaural audio rendering, because it is not supported on the current OpenAL implementation");
		return;
	}
	auto alcResetDeviceSOFT_fnptr = reinterpret_cast<LPALCRESETDEVICESOFT>(alGetProcAddress("alcResetDeviceSOFT"));
	if (alcResetDeviceSOFT_fnptr == nullptr) {
		Output("Could not get address of function alcResetDeviceSOFT");
		return;
	}
	ALCint attributes[] = {
		ALC_HRTF_SOFT, enabled ? ALC_TRUE : ALC_FALSE, 0
	};
	const auto result = alcResetDeviceSOFT_fnptr(m_device, &attributes[0]);
	if (result != AL_TRUE) {
		Output("Could not reset device properties with alcResetDeviceSOFT");
	}
}

Sound::AlAudioBackend::SoundEvent::SoundEvent(const Sample &sample) :
	channels(sample.channels), samplerate(sample.samplerate), target_gain(1.F), target_pan(0.F), fade_rate(0.F), pan_rate(0.F), streaming_finished(false), is_music(sample.isMusic), op(0)
{
	CHECK_OPENAL_ERROR(alGenSources, 1, &source);
	CHECK_OPENAL_ERROR(alSourcei, source, AL_REFERENCE_DISTANCE, 1);
	if (!sample.buf.empty()) {
		CHECK_OPENAL_ERROR(alGenBuffers, 1, &buffers[0]);
		CHECK_OPENAL_ERROR(alBufferData,
			buffers[0],
			sample.channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
			sample.buf.data(),
			sample.buf.size() * sizeof(sample.buf[0]),
			sample.samplerate);
		CHECK_OPENAL_ERROR(alSourcei, source, AL_BUFFER, buffers[0]);
	} else {
		CHECK_OPENAL_ERROR(alGenBuffers, 2, &buffers[0]);
		oggv = std::make_unique<OggVorbis_File>();
		ogg_data_stream.Reset(FileSystem::gameDataFiles.ReadFile(sample.path));
		if (ov_open_callbacks(&ogg_data_stream, oggv.get(), nullptr, 0, OggFileDataStream::CALLBACKS) < 0) {
			Output("Vorbis could not understand '%s'", sample.path.c_str());
			throw std::exception();
		}
		FillBuffer(0);
		FillBuffer(1);
		CHECK_OPENAL_ERROR(alSourceQueueBuffers, source, 2, &buffers[0]);
	}
}

Sound::AlAudioBackend::SoundEvent::~SoundEvent()
{
	if (source != 0) {
		alDeleteSources(1, &source);
	}
	source = 0;
	if (buffers[0] != 0) {
		alDeleteBuffers(1, &buffers[0]);
		buffers[0] = 0;
	}
	if (buffers[1] != 0) {
		alDeleteBuffers(1, &buffers[1]);
		buffers[1] = 0;
	}
	if (oggv) {
		ov_clear(oggv.get());
		oggv = nullptr;
	}
}

Sound::AlAudioBackend::SoundEvent::SoundEvent(SoundEvent &&other)
{
	channels = other.channels;
	samplerate = other.samplerate;
	target_gain = other.target_gain;
	fade_rate = other.fade_rate;
	streaming_finished = other.streaming_finished;
	is_music = other.is_music;
	op = other.op;
	buffers[0] = std::exchange(other.buffers[0], 0);
	buffers[1] = std::exchange(other.buffers[1], 0);
	source = std::exchange(other.source, 0);
	playing_buffer_idx = other.playing_buffer_idx;
	oggv = std::exchange(other.oggv, nullptr);
	ogg_data_stream = std::move(other.ogg_data_stream);
}

Sound::AlAudioBackend::SoundEvent &Sound::AlAudioBackend::SoundEvent::operator=(SoundEvent &&other)
{
	channels = other.channels;
	samplerate = other.samplerate;
	target_gain = other.target_gain;
	fade_rate = other.fade_rate;
	streaming_finished = other.streaming_finished;
	is_music = other.is_music;
	op = other.op;
	buffers[0] = std::exchange(other.buffers[0], 0);
	buffers[1] = std::exchange(other.buffers[1], 0);
	source = std::exchange(other.source, 0);
	playing_buffer_idx = other.playing_buffer_idx;
	oggv = std::exchange(other.oggv, nullptr);
	ogg_data_stream = std::move(other.ogg_data_stream);
	return *this;
}

bool Sound::AlAudioBackend::SoundEvent::IsDone() const
{
	ALint state;
	CHECK_OPENAL_ERROR(alGetSourcei, source, AL_SOURCE_STATE, &state);
	return state == AL_STOPPED;
}

void Sound::AlAudioBackend::SoundEvent::FillBuffer(int idx)
{
	constexpr int buffer_bytes = 44100 * 2;
	char data[buffer_bytes] {};
	int bitstream {};
	int remaining_bytes = buffer_bytes;
	while (remaining_bytes > 0) {
		const int bytes_read = ov_read(oggv.get(), &data[0] + buffer_bytes - remaining_bytes,
			remaining_bytes, 0, 2, 1, &bitstream);
		if (bytes_read == 0) { // EOF
			if (op & OP_REPEAT) {
				const auto result = ov_pcm_seek(oggv.get(), 0);
				if (result != 0) {
					throw std::exception();
				}
			} else {
				streaming_finished = true;
				break;
			}
		}
		remaining_bytes -= bytes_read;
	}
	CHECK_OPENAL_ERROR(alBufferData,
		buffers[idx],
		channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
		data,
		buffer_bytes - remaining_bytes,
		samplerate);
}

void Sound::AlAudioBackend::SoundEvent::Update(float delta_t)
{
	if (!streaming_finished && oggv) // streaming
	{
		ALint num_processed_buffers;
		CHECK_OPENAL_ERROR(alGetSourcei, source, AL_BUFFERS_PROCESSED, &num_processed_buffers);
		if (num_processed_buffers > 0) {
			CHECK_OPENAL_ERROR(alSourceUnqueueBuffers, source, 1, &buffers[playing_buffer_idx]);
			FillBuffer(playing_buffer_idx);
			CHECK_OPENAL_ERROR(alSourceQueueBuffers, source, 1, &buffers[playing_buffer_idx]);
			playing_buffer_idx = 1 - playing_buffer_idx;
		}
	}
	if (fade_rate != 0.f) {
		float current_gain, current_pan, y, z;
		CHECK_OPENAL_ERROR(alGetSourcef, source, AL_GAIN, &current_gain);
		CHECK_OPENAL_ERROR(alGetSource3f, source, AL_POSITION, &current_pan, &y, &z);
		float new_gain = current_gain + fade_rate * delta_t;
		float new_pan = current_pan + pan_rate * delta_t;
		if ((fade_rate > 0.F && new_gain >= target_gain) || (fade_rate < 0.F && new_gain < target_gain)) {
			if (op & OP_STOP_AT_TARGET_VOLUME) {
				CHECK_OPENAL_ERROR(alSourceStop, source);
				return;
			}
			new_gain = target_gain;
			new_pan = target_pan;
			fade_rate = 0.f;
			pan_rate = 0.f;
		}
		CHECK_OPENAL_ERROR(alSourcef, source, AL_GAIN, new_gain);
		CHECK_OPENAL_ERROR(alSource3f, source, AL_POSITION, new_pan, y, z);
	}
}

void Sound::AlAudioBackend::SoundEvent::SetGain(float gain)
{
	SetGain(gain, gain);
}

void Sound::AlAudioBackend::SoundEvent::SetGain(float gainL, float gainR)
{
	target_gain = (gainL + gainR) * 0.5f;
	target_pan = calculate_pan(gainL, gainR);
	fade_rate = 0.f;
	pan_rate = 0.f;
	CHECK_OPENAL_ERROR(alSourcef, source, AL_GAIN, target_gain);
	if (target_pan != 0.f) {
		CHECK_OPENAL_ERROR(alSource3f, source, AL_POSITION, target_pan, 0.f, 0.f);
	}
}

void Sound::AlAudioBackend::SoundEvent::SetOp(Op new_op)
{
	op = new_op;
	if (oggv) { // streamed shouldn't use AL_LOOPING
		return;
	}
	if (op & OP_REPEAT) {
		CHECK_OPENAL_ERROR(alSourcei, source, AL_LOOPING, AL_TRUE);
	} else {
		CHECK_OPENAL_ERROR(alSourcei, source, AL_LOOPING, AL_FALSE);
	}
}

void Sound::AlAudioBackend::SoundEvent::SetTargetGain(float gain, float rate)
{
	SetTargetGain(gain, gain, rate);
}

void Sound::AlAudioBackend::SoundEvent::SetTargetGain(float gainL, float gainR, float rate)
{
	target_gain = (gainL + gainR) * 0.5f;
	fade_rate = rate;
	target_pan = calculate_pan(gainL, gainR);
	float current_pan, y, z;
	CHECK_OPENAL_ERROR(alGetSource3f, source, AL_POSITION, &current_pan, &y, &z);
	pan_rate = rate * (target_pan - current_pan);
}

#endif
