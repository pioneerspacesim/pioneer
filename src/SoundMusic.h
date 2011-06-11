#ifndef _MUSIC_H
#define _MUSIC_H

#include "Sound.h"

namespace Sound
{
	class MusicPlayer
	{
	public:
		MusicPlayer();
		~MusicPlayer();
		float GetVolume() const;
		void SetVolume(const float);
		void Play(const std::string&, bool repeat = false);
		void Stop();
	private:
		float m_volume;
		//two streams for crossfade
		Event m_eventOne;
		Event m_eventTwo;
	};
}

#endif