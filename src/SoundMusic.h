#ifndef _MUSIC_H
#define _MUSIC_H

#include <SDL.h>
#include "Sound.h"
#include <string>

namespace Sound
{
	class MusicEvent : public Event
	{
	public:
		MusicEvent();
		MusicEvent(Uint32 id);
		~MusicEvent();
		virtual void Play(const char *fx, float volume_left, float volume_right, Op op);
	};

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
		MusicEvent m_eventOne;
		MusicEvent m_eventTwo;
	};
}

#endif
