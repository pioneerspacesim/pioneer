// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "SoundMusic.h"
#include "Pi.h"
#include "utils.h"
#include <map>

namespace Sound {

	MusicPlayer::MusicPlayer() :
		m_volume(0.8f),
		m_playing(false),
		m_eventOnePlaying(false),
		m_currentSongName(""),
		m_enabled(true)
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

	void MusicPlayer::Play(const std::string &name, const bool repeat /* = false */, const float fadeDelta /* = 1.f */)
	{
		if (!m_enabled) return;

		Event *current, *next;
		if (m_eventOnePlaying) {
			m_eventOnePlaying = false;
			current = &m_eventOne;
			next = &m_eventTwo;
		} else {
			m_eventOnePlaying = true;
			current = &m_eventTwo;
			next = &m_eventOne;
		}
		next->PlayMusic(name.c_str(), m_volume, fadeDelta, repeat, current);
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
		if (m_eventOnePlaying) { //2 might be already fading out
			m_eventOne.FadeOut(fadeDelta);
		} else { // 1 might be already fading out
			m_eventTwo.FadeOut(fadeDelta);
		}
	}

	void MusicPlayer::Update()
	{
		PROFILE_SCOPED()
		if (m_playing) { //expecting report
			if ((m_eventOnePlaying && !m_eventOne.IsPlaying()) || (!m_eventOnePlaying && !m_eventTwo.IsPlaying())) {
				m_playing = false;
				onSongFinished.emit();
			}
		}
	}

	const std::string& MusicPlayer::GetCurrentSongName() const
	{
		return m_currentSongName;
	}

	const std::vector<std::string> MusicPlayer::GetSongList() const
	{
		return GetMusicFiles();
	}

	bool MusicPlayer::IsPlaying() const
	{
		return (m_eventOne.IsPlaying() || m_eventTwo.IsPlaying());
	}

	void MusicPlayer::SetEnabled(const bool en)
	{
		m_enabled = en;
		if (!en && IsPlaying()) Stop();
	}

} // namespace Sound
