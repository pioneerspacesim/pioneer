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

void MusicPlayer::Play(const std::string& name, const bool repeat /* = false */ , const float fadeDelta /* = 1.f */ )
{
	Sound::Op op = 0;
	if (repeat)
		op |= Sound::OP_REPEAT;
	if (m_eventOnePlaying) {
		m_eventOne.VolumeAnimate(0.f, 0.f, fadeDelta, fadeDelta);
		m_eventOne.SetOp(Sound::OP_STOP_AT_TARGET_VOLUME);
		m_eventTwo.Play(name.c_str(), 0.f, 0.f, op);
		m_eventTwo.VolumeAnimate(1.f, 1.f, fadeDelta, fadeDelta);
		m_eventOnePlaying = false;
	} else {
		m_eventTwo.VolumeAnimate(0.f, 0.f, fadeDelta, fadeDelta);
		m_eventTwo.SetOp(Sound::OP_STOP_AT_TARGET_VOLUME);
		m_eventOne.Play(name.c_str(), 0.f, 0.f, op);
		m_eventOne.VolumeAnimate(1.f, 1.f, fadeDelta, fadeDelta);
		m_eventOnePlaying = true;
	}
	m_playing = true;
}

void MusicPlayer::Stop()
{
	m_eventOne.Stop();
	m_eventTwo.Stop();
}

void MusicPlayer::FadeOut(const float fadeDelta)
{
	if (m_eventOnePlaying) {//2 might be already fading out
		m_eventOne.SetOp(Sound::OP_STOP_AT_TARGET_VOLUME);
		m_eventOne.VolumeAnimate(0.f, 0.f, fadeDelta, fadeDelta);
	} else { // 1 might be already fading out
		m_eventTwo.SetOp(Sound::OP_STOP_AT_TARGET_VOLUME);
		m_eventTwo.VolumeAnimate(0.f, 0.f, fadeDelta, fadeDelta);
	}
}

void MusicPlayer::Update()
{
	//finish should trigger if:
	// - song plays all the way to the end
	// - song is not repeating
	/*if(m_playing && !m_eventOne.IsPlaying()) {
		Pi::luaOnSongFinished.Signal();
		m_playing = false;
	}*/
}

} /* namespace sound */
