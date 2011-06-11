#include "SoundMusic.h"

namespace Sound {

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
	m_eventOne.Play("scj", m_volume, m_volume, Sound::OP_REPEAT);
}

void MusicPlayer::Stop()
{

}

} /* namespace sound */
