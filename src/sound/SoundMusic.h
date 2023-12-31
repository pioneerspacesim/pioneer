// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _MUSIC_H
#define _MUSIC_H

#include "Sound.h"

#include <sigc++/sigc++.h>
#include <string>
#include <vector>

namespace Sound {
	class MusicEvent : public Event {
	public:
		MusicEvent();
		MusicEvent(Uint32 id);
		~MusicEvent();
		virtual void Play(const char *fx, const float volume_left, const float volume_right, Op op);
	};

	class MusicPlayer {
	public:
		MusicPlayer();
		~MusicPlayer();
		float GetVolume() const;
		void SetVolume(const float);
		void Play(const std::string &, const bool repeat = false, const float fadeDelta = 1.f);
		void Stop();
		void FadeOut(const float fadeDelta);
		void Update();
		const std::string GetCurrentSongName() const;
		const std::vector<std::string> GetSongList() const;
		bool IsPlaying() const;
		void SetEnabled(bool);

		sigc::signal<void> onSongFinished;

	private:
		float m_volume;
		//two streams for crossfade
		MusicEvent m_eventOne;
		MusicEvent m_eventTwo;
		bool m_playing;
		bool m_eventOnePlaying;
		std::string m_currentSongName;
		bool m_enabled;
	};
} // namespace Sound

#endif
