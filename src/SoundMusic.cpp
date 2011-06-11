#include "SoundMusic.h"

namespace Sound {

MusicPlayer::MusicPlayer() :
	m_volume(1.f)
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

} /* namespace sound */