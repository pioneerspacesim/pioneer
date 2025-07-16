// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef __SDL_AUDIO_BACKEND_H
#define __SDL_AUDIO_BACKEND_H

#include "AudioBackend.h"
#include "OggFileDataStream.h"

#include "SDL_audio.h"

namespace Sound {

	class SdlAudioBackend : public AudioBackend {
	public:
		SdlAudioBackend();
		~SdlAudioBackend();

		BackendId GetId() override { return AudioBackend_SDL; }
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

		void SetMasterVolume(float vol) override { m_masterVolume = vol; }
		float GetMasterVolume() override { return m_masterVolume; }
		void SetSfxVolume(float vol) override { m_sfxVolume = vol; }
		float GetSfxVolume() override { return m_sfxVolume; }

		void AddSample(std::string_view key, Sample &&sample) override;

	private:
		struct SoundEvent {
			const Sample *sample;
			OggVorbis_File *oggv; // if sample->buf = 0 then stream this
			OggFileDataStream ogg_data_stream;
			uint32_t buf_pos;
			float volume[2]; // left and right channels
			eventid identifier;
			uint32_t op;

			float targetVolume[2];
			float rateOfChange[2]; // per sample
			bool ascend[2];
		};

		constexpr inline static unsigned int FREQ = 44100;
		constexpr inline static unsigned int BUF_SIZE = 4096;
		constexpr inline static unsigned int MAX_WAVSTREAMS = 10; //first two are for music

		SoundEvent *GetEvent(eventid id);
		void DestroyEvent(SoundEvent *ev);
		SoundEvent &FindFreeEventForSample(const Sample &sample);

		template <int T_channels, int T_upsample>
		void fill_audio_1stream(float *buffer, int len, int stream_num);
		void fill_audio(Uint8 *dsp_buf, int len);
		static void fill_audio_callback(void *udata, Uint8 *dsp_buf, int len);

		float m_masterVolume = 1.F;
		float m_sfxVolume = 1.F;
		SDL_AudioDeviceID m_audioDevice;
		uint32_t identifier;
		int nextMusicStream = 0;
		SoundEvent wavstream[MAX_WAVSTREAMS]{};
		std::map<std::string, Sample> m_samples;
	};

} // namespace Sound

#endif // __SDL_AUDIO_BACKEND_H
