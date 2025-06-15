// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef __AUDIO_BACKEND_H
#define __AUDIO_BACKEND_H

#include "Sound.h"

namespace Sound
{

struct Sample {
    uint16_t *buf;
    uint32_t buf_len;
    uint32_t channels;
    int upsample; // 1 = 44100, 2=22050
    /* if buf is null, this will be path to an ogg we must stream */
    std::string path;
    bool isMusic;
};

class AudioBackend
{
public:
	using eventid = uint32_t;

    virtual ~AudioBackend() = default;

    virtual bool IsAvailable() { return true; }
	virtual void DestroyAllEvents() = 0;
	virtual void DestroyAllEventsExceptMusic() = 0;
    
    virtual bool EventStop(eventid eid) = 0;
    virtual bool IsEventPlaying(eventid eid) = 0;
    virtual bool EventSetOp(eventid eid, Op op) = 0;
    virtual bool EventVolumeAnimate(eventid eid, const float targetVol1, const float targetVol2, const float dv_dt1, const float dv_dt2) = 0;
    virtual bool EventSetVolume(eventid eid, const float vol_left, const float vol_right) = 0;
    virtual bool EventFadeOut(eventid eid, float dv_dt, Op op) = 0;

	virtual void Pause(int on) = 0;
	
    virtual eventid PlaySfxSample(Sample *sample, const float volume_left, const float volume_right, const Op op) = 0;
    virtual eventid PlayMusicSample(Sample *sample, const float volume_left, const float volume_right, const Op op) = 0;
	virtual void BodyMakeNoise(const Body *b, Sample* sample, float vol) = 0;
	
    virtual void SetMasterVolume(const float vol) { m_masterVolume = vol; }
	float GetMasterVolume() { return m_masterVolume; }
	virtual void SetSfxVolume(const float vol) { m_sfxVolume = vol; }
	float GetSfxVolume() { return m_sfxVolume; }

protected:
    float m_masterVolume{};
    float m_sfxVolume{};
};

} // namespace Sound

#endif // __AUDIO_BACKEND_H
