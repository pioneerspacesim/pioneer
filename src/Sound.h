
#ifndef __OGGMIX_H
#define __OGGMIX_H

class Body;

namespace Sound {

typedef Uint32 eventid;

enum SFX { SFX_PULSECANNON, SFX_COLLISION, SFX_WARNING, SFX_GUI_PING, SFX_ENGINES, SFX_MAX };

int Init ();
void Close ();
void UpdateBufferFill ();
void Pause (int on);
int PlayOgg (const char *filename);
eventid PlaySfx (enum SFX fx, Uint16 volume_left, Uint16 volume_right, bool repeat);
inline static eventid PlaySfx (enum SFX fx) { return PlaySfx(fx, 65535U, 65535U, false); }
eventid BodyMakeNoise(const Body *b, enum SFX fx, float vol);
bool EventSetVolume(eventid id, Uint16 vol_left, Uint16 vol_right);
inline static bool EventSetVolume(eventid id, Uint16 vol) { return EventSetVolume(id, vol, vol); }
bool EventDestroy(eventid id);

} /* namespace Sound */

#endif /* __OGGMIX_H */

