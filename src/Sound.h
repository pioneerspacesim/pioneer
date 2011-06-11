
#ifndef __OGGMIX_H
#define __OGGMIX_H

class Body;

namespace Sound {

enum {
	OP_REPEAT = (1<<0),
	OP_STOP_AT_TARGET_VOLUME = (1<<1)
};
typedef Uint32 Op;

class Event {
public:
	Event(): eid(0) {}
	Event(Uint32 id): eid(id) {}
	virtual void Play(const char *fx, float volume_left, float volume_right, Op op);
	void Play(const char *fx) { Play(fx, 1.0f, 1.0f, 0); }
	bool Stop();
	bool IsPlaying() const;
	Uint32 EventId() { return eid; }
	bool SetOp(Op op);
	bool VolumeAnimate(float targetVol1, float targetVol2, float dv_dt1, float dv_dt2);
	bool VolumeAnimate(float targetVols[2], float dv_dt[2]) {
		return VolumeAnimate(targetVols[0], targetVols[1],
				dv_dt[0], dv_dt[1]);
	}
	bool SetVolume(float vol_left, float vol_right);
	bool SetVolume(float vol) {
		return SetVolume(vol, vol);
	}
protected:
	Uint32 eid;
};
typedef Uint32 eventid;

bool Init ();
/**
 * Silence all active sound events.
 */
void DestroyAllEvents();
void Close ();
void Pause (int on);
eventid PlaySfx (const char *fx, float volume_left, float volume_right, Op op);
eventid PlayMusic (const char *fx, const float volume_left, const float volume_right, Op op);
inline static eventid PlaySfx (const char *fx) { return PlaySfx(fx, 1.0f, 1.0f, 0); }
eventid BodyMakeNoise(const Body *b, const char *fx, float vol);
void SetGlobalVolume(float vol);
float GetGlobalVolume();

} /* namespace Sound */

#endif /* __OGGMIX_H */

