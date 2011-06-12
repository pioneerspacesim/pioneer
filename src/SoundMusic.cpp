#include "SoundMusic.h"

namespace Sound {

MusicEvent::MusicEvent() : Event() { }

MusicEvent::MusicEvent(Uint32 id) : Event(id) { }

MusicEvent::~MusicEvent() { }

void MusicEvent::Play(const char *fx, float volume_left, float volume_right, Op op)
{
	Stop();
	eid = PlayMusic(fx, volume_left, volume_right, op);
}

MusicPlayer::MusicPlayer() :
	m_volume(0.8),
	m_playing(false),
	m_eventOnePlaying(false)
{

}

MusicPlayer::~MusicPlayer()
{

}

float MusicPlayer::GetVolume() const
{
	return m_volume;
}

void MusicPlayer::SetVolume(const float vol)
{
	m_volume = vol;
}

void MusicPlayer::Play(const std::string& name, bool repeat /* = false */ )
{
	Sound::Op op;
	if (repeat)
		op |= Sound::OP_REPEAT;
	if (m_eventOnePlaying) {
		float target[2] = {0.0f,0.0f};
		float dv_dt[2] = {1.0f,1.0f};
		m_eventOne.VolumeAnimate(target, dv_dt);
		m_eventOne.SetOp(Sound::OP_STOP_AT_TARGET_VOLUME);
		m_eventTwo.Play(name.c_str(), m_volume, m_volume, op);
		m_eventOnePlaying = false;
	} else {
		float target[2] = {0.0f,0.0f};
		float dv_dt[2] = {1.0f,1.0f};
		m_eventTwo.VolumeAnimate(target, dv_dt);
		m_eventTwo.SetOp(Sound::OP_STOP_AT_TARGET_VOLUME);
		m_eventOne.Play(name.c_str(), m_volume, m_volume, op);
		m_eventOnePlaying = true;
	}
	m_playing = true;
}

void MusicPlayer::Stop()
{
	m_eventOne.Stop();
}

void MusicPlayer::Update()
{
	//finish should trigger if:
	// - song plays all the way to the end
	// - song is not repeating
	if(m_playing && !m_eventOne.IsPlaying()) {
		Pi::luaOnSongFinished.Signal();
		m_playing = false;
	}

}

} /* namespace sound */
