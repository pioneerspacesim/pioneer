// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef __SOUND_H
#define __SOUND_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

class Body;

namespace Sound {

	enum {
		OP_REPEAT = (1 << 0),
		OP_STOP_AT_TARGET_VOLUME = (1 << 1)
	};
	typedef uint32_t Op;

	class Event {
	public:
		Event() :
			eid(0) {}
		void Play(const char *fx, const float volume_left, const float volume_right, Op op);
		void Play(const char *fx) { Play(fx, 1.0f, 1.0f, 0); }
		void PlayMusic(const char *fx, float volume, float fadeDelta, bool repeat, Event* fadeOut = nullptr);
		bool Stop();
		bool IsPlaying() const;
		bool SetOp(Op op);
		bool VolumeAnimate(const float targetVol1, const float targetVol2, const float dv_dt1, const float dv_dt2);
		bool VolumeAnimate(const float targetVols[2], const float dv_dt[2])
		{
			return VolumeAnimate(targetVols[0], targetVols[1],
				dv_dt[0], dv_dt[1]);
		}
		bool SetVolume(const float vol_left, const float vol_right);
		bool SetVolume(const float vol)
		{
			return SetVolume(vol, vol);
		}
		bool FadeOut(float dv_dt, Op op = 0);

	private:
		uint32_t eid;
	};

	bool Init(bool automaticallyOpenDevice = true);
	bool InitDevice(std::string &name);
	void Uninit();
	std::vector<std::string> &GetAudioDevices();
	void UpdateAudioDevices();
	/**
	 * Silence all active sound events.
	 */
	void DestroyAllEvents();
	void DestroyAllEventsExceptMusic();
	void Pause(int on);
	void PlaySfx(const char *fx, const float volume_left, const float volume_right, const Op op);
	inline static void PlaySfx(const char *fx) { PlaySfx(fx, 1.0f, 1.0f, 0); }
	void CalculateStereo(const Body *b, float vol, float *volLeftOut, float *volRightOut);
	void BodyMakeNoise(const Body *b, const char *fx, float vol);
	void SetMasterVolume(const float vol);
	float GetMasterVolume();
	void SetSfxVolume(const float vol);
	float GetSfxVolume();
	const std::vector<std::string> GetMusicFiles();

} /* namespace Sound */

#endif /* __SOUND_H */
