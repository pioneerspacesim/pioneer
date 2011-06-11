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
	m_volume(0.8)
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
	m_eventOne.Play(name.c_str(), m_volume, m_volume, Sound::OP_REPEAT);
}

void MusicPlayer::Stop()
{

}

} /* namespace sound */
