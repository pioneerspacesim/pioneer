
#ifndef __OGGMIX_H
#define __OGGMIX_H

class Body;

namespace Sound {

typedef Uint32 eventid;

enum SFX { SFX_PULSECANNON, SFX_COLLISION, SFX_WARNING, SFX_GUI_PING, SFX_ENGINES, SFX_ECM, SFX_MAX };

bool Init ();
void Close ();
void Pause (int on);
int PlayOgg (const char *filename);
eventid PlaySfx (enum SFX fx, float volume_left, float volume_right, bool repeat);
inline static eventid PlaySfx (enum SFX fx) { return PlaySfx(fx, 1.0f, 1.0f, false); }
eventid BodyMakeNoise(const Body *b, enum SFX fx, float vol);
bool EventSetVolume(eventid id, float vol_left, float vol_right);
bool EventVolumeAnimate(eventid id, float targetVols[2], float dv_dt[2]);
inline static bool EventSetVolume(eventid id, float vol) { return EventSetVolume(id, vol, vol); }
bool EventDestroy(eventid id);

} /* namespace Sound */

#endif /* __OGGMIX_H */

