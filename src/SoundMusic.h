#ifndef _MUSIC_H
#define _MUSIC_H

namespace Sound
{
	class MusicPlayer
	{
	public:
		MusicPlayer();
		~MusicPlayer();
		float GetVolume() const;
		void SetVolume(const float);
		void Play(const std::string&);
		void Stop();
	private:
		float m_volume;
	};
}

#endif