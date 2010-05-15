
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
	void Play(const char *fx, float volume_left, float volume_right, Op op);
	void Play(const char *fx) { Play(fx, 1.0f, 1.0f, 0); }
	bool Stop();
	bool IsPlaying() const;
	Uint32 EventId() { return eid; }
	bool SetOp(Op op);
	bool VolumeAnimate(float targetVols[2], float dv_dt[2]);
	bool SetVolume(float vol_left, float vol_right);
	bool SetVolume(float vol) {
		return SetVolume(vol, vol);
	}
private:
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
int PlayOgg (const char *filename);
eventid PlaySfx (const char *fx, float volume_left, float volume_right, Op op);
inline static eventid PlaySfx (const char *fx) { return PlaySfx(fx, 1.0f, 1.0f, 0); }
eventid BodyMakeNoise(const Body *b, const char *fx, float vol);

} /* namespace Sound */

#endif /* __OGGMIX_H */

