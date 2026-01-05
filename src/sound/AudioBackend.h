// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef __AUDIO_BACKEND_H
#define __AUDIO_BACKEND_H

#include "Sound.h"

#include <map>
#include <string_view>

namespace Sound {

	struct Sample {
		std::vector<uint16_t> buf;
		uint32_t buf_len;
		uint32_t channels;
		int samplerate;
		/* if buf is null, this will be path to an ogg we must stream */
		std::string path;
		bool isMusic;
	};

	class AudioBackend {
	public:
		using eventid = uint32_t;

		virtual ~AudioBackend() = default;

		virtual void DestroyAllEvents() = 0;
		virtual void DestroyAllEventsExceptMusic() = 0;

		virtual bool EventStop(eventid eid) = 0;
		virtual bool IsEventPlaying(eventid eid) = 0;
		virtual bool EventSetOp(eventid eid, Op op) = 0;
		virtual bool EventVolumeAnimate(eventid eid, const float targetVol1, const float targetVol2, const float dv_dt1, const float dv_dt2) = 0;
		virtual bool EventSetVolume(eventid eid, const float vol_left, const float vol_right) = 0;

		virtual void Pause(int on) = 0;

		virtual eventid Play(std::string_view key, const float volume_left, const float volume_right, const Op op) = 0;
		virtual void BodyMakeNoise(const Body *b, std::string_view key, float vol) = 0;

		virtual void SetMasterVolume(float vol) = 0;
		virtual float GetMasterVolume() = 0;
		virtual void SetSfxVolume(float vol) = 0;
		virtual float GetSfxVolume() = 0;

		virtual void AddSample(std::string_view key, Sample &&sample) = 0;
		virtual void Update(float delta_t) {}

		virtual bool IsBinauralSupported() { return false; }
		virtual void EnableBinaural(bool enabled) {}
	};

} // namespace Sound

#endif // __AUDIO_BACKEND_H
