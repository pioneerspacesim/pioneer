// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef __NULL_AUDIO_BACKEND_H
#define __NULL_AUDIO_BACKEND_H

#include "AudioBackend.h"

namespace Sound {

	class NullAudioBackend : public AudioBackend {
	public:
		void DestroyAllEvents() override {}
		void DestroyAllEventsExceptMusic() override {}

		bool EventStop(eventid) override { return false; }
		bool IsEventPlaying(eventid) override { return false; }
		bool EventSetOp(eventid, Op) override { return false; }
		bool EventVolumeAnimate(eventid, const float, const float, const float, const float) override
		{
			return false;
		}
		bool EventSetVolume(eventid, const float, const float) override { return false; }

		void Pause(int) override {}

		eventid Play(std::string_view, const float, const float, const Op) override
		{
			return 0;
		}
		void BodyMakeNoise(const Body *, std::string_view, float) override {}

		void SetMasterVolume(float vol) override { m_masterVolume = vol; }
		float GetMasterVolume() override { return m_masterVolume; }
		void SetSfxVolume(float vol) override { m_sfxVolume = vol; }
		float GetSfxVolume() override { return m_sfxVolume; }

		void AddSample(std::string_view, Sample &&) override {}

	private:
		float m_masterVolume = 1.0f;
		float m_sfxVolume = 1.0f;
	};

} // namespace Sound

#endif // __NULL_AUDIO_BACKEND_H
