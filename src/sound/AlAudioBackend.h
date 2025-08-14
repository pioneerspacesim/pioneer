// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef __AL_AUDIO_BACKEND_H
#define __AL_AUDIO_BACKEND_H
#ifdef PI_BUILD_WITH_OPENAL

#include "AudioBackend.h"
#include "OggFileDataStream.h"

#include "AL/al.h"
#include "AL/alc.h"
#include "vorbis/vorbisfile.h"

#include <memory>

namespace Sound {

	class AlAudioBackend : public AudioBackend {
	public:
		AlAudioBackend();
		~AlAudioBackend();

		void DestroyAllEvents() override;
		void DestroyAllEventsExceptMusic() override;

		bool EventStop(eventid eid) override;
		bool IsEventPlaying(eventid eid) override;
		bool EventSetOp(eventid eid, Op op) override;
		bool EventVolumeAnimate(eventid eid, const float targetVol1, const float targetVol2, const float dv_dt1, const float dv_dt2) override;
		bool EventSetVolume(eventid eid, const float vol_left, const float vol_right) override;

		void Pause(int on) override;

		eventid Play(std::string_view key, const float volume_left, const float volume_right, const Op op) override;
		void BodyMakeNoise(const Body *b, std::string_view key, float vol) override;

		void SetMasterVolume(float vol) override;
		float GetMasterVolume() override;
		void SetSfxVolume(float vol) override;
		float GetSfxVolume() override;

		void AddSample(std::string_view key, Sample &&sample) override;
		void Update(float delta_t) override;

		bool IsBinauralSupported() override;
		void EnableBinaural(bool enabled) override;

	private:
		class SoundEvent {
		public:
			SoundEvent(const Sample &sample);
			~SoundEvent();
			SoundEvent(const SoundEvent &) = delete;
			SoundEvent(SoundEvent &&);

			SoundEvent &operator=(const SoundEvent &) = delete;
			SoundEvent &operator=(SoundEvent &&);

			ALuint GetSource() const { return source; }
			bool IsDone() const;
			void Update(float delta_t);
			void SetGain(float gain);
			void SetGain(float gainL, float gainR);
			void SetOp(Op op);
			void SetTargetGain(float gain, float rate);
			void SetTargetGain(float gainL, float gainR, float rate);
			bool IsMusic() const { return is_music; }

		private:
			void FillBuffer(int idx);

			ALuint buffers[2]{};
			ALuint source{};
			int playing_buffer_idx{};
			int channels;
			int samplerate;
			float target_gain;
			float target_pan;
			float fade_rate;
			float pan_rate;
			bool streaming_finished;
			bool is_music;
			Op op;
			std::unique_ptr<OggVorbis_File> oggv;
			OggFileDataStream ogg_data_stream;
		};

		ALCdevice *m_device;
		ALCcontext *m_context;
		eventid m_next_event_id;
		std::map<std::string, Sample> m_samples;
		std::map<eventid, SoundEvent> m_events;
		float m_sfxVolume = 1.F;
	};

} // namespace Sound

#endif
#endif // __AL_AUDIO_BACKEND_H
