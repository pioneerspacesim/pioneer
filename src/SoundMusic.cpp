#include "SoundMusic.h"
#include "libs.h" //for clamp
#include "Pi.h"
#include <map>

namespace Sound {

MusicEvent::MusicEvent() : Event() { }

MusicEvent::MusicEvent(Uint32 id) : Event(id) { }

MusicEvent::~MusicEvent() { }

void MusicEvent::Play(const char *fx, const float volume_left, const float volume_right, Op op)
{
	Stop();
	eid = PlayMusic(fx, volume_left, volume_right, op);
}

MusicPlayer::MusicPlayer() :
	m_volume(0.8),
	m_playing(false),
	m_eventOnePlaying(false),
	m_currentSongName("")
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
	m_volume = Clamp(vol, 0.f, 1.f);
	//the other song might be fading out so don't set its volume
	if (m_eventOnePlaying && m_eventOne.IsPlaying())
		m_eventOne.SetVolume(m_volume);
	else if (m_eventTwo.IsPlaying())
		m_eventTwo.SetVolume(m_volume);
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
		m_eventTwo.VolumeAnimate(m_volume, m_volume, fadeDelta, fadeDelta);
		m_eventOnePlaying = false;
	} else {
		m_eventTwo.VolumeAnimate(0.f, 0.f, fadeDelta, fadeDelta);
		m_eventTwo.SetOp(Sound::OP_STOP_AT_TARGET_VOLUME);
		m_eventOne.Play(name.c_str(), 0.f, 0.f, op);
		m_eventOne.VolumeAnimate(m_volume, m_volume, fadeDelta, fadeDelta);
		m_eventOnePlaying = true;
	}
	m_playing = true;
	m_currentSongName = name;
}

void MusicPlayer::Stop()
{
	m_eventOne.Stop();
	m_eventTwo.Stop();
	m_playing = false;
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
	if (m_playing) { //expecting report
		if ((m_eventOnePlaying && !m_eventOne.IsPlaying()) || (!m_eventOnePlaying && !m_eventTwo.IsPlaying())) {
			Pi::luaOnSongFinished.Signal();
			m_playing = false;
		}
	}
}

const std::string MusicPlayer::GetCurrentSongName()
{
	return m_currentSongName;
}

const std::vector<std::string> MusicPlayer::GetSongList()
{
	using std::string;
	using std::pair;
	std::vector<string> songs;
	const std::map<string, Sample> samples = Sound::GetSamples();
	for (std::map<string, Sample>::const_iterator it = samples.begin();
		it != samples.end(); ++it) {
			if (it->second.isMusic)
				songs.push_back(it->first.c_str());
	}

	return songs;
}

bool MusicPlayer::IsPlaying()
{
	return (m_eventOne.IsPlaying() || m_eventTwo.IsPlaying());
}

} /* namespace sound */
