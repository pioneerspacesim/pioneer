#ifndef _NEWMODEL_ANIMATION_H
#define _NEWMODEL_ANIMATION_H
/*
 * A named animation, such as "GearDown".
 * An animation has a number of channels, each of which
 * animate the position/rotation of a single MatrixTransform node
 */
#include "AnimationChannel.h"

namespace Newmodel {

class Animation {
public:
	Animation(const std::string &name);
	void Play();
	void Pause(); //pause or resume playback
	void Stop(); //abort playback and rewind
	void Evaluate(double time);
	std::vector<AnimationChannel> channels;
	double GetDuration() const { return m_duration; }
	double GetTicksPerSecond() const { return m_ticksPerSecond; }
	const std::string &GetName() const { return m_name; }

private:
	bool m_paused;
	double m_currentTime;
	double m_duration;
	double m_lastTime;
	double m_ticksPerSecond;
	std::string m_name;
};

}

#endif
