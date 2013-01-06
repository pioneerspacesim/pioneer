// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SCENEGRAPH_ANIMATION_H
#define _SCENEGRAPH_ANIMATION_H
/*
 * A named animation, such as "GearDown".
 * An animation has a number of channels, each of which
 * animate the position/rotation of a single MatrixTransform node
 */
#include "AnimationChannel.h"

namespace SceneGraph {

class Loader;

class Animation {
public:
	enum Direction {
		FORWARD,
		REVERSE
	};
	enum Behavior {
		ONCE,
		LOOP
	};
	Animation(const std::string &name, double duration, Behavior b, double ticksPerSecond = 24.0);
	void Play(Direction = FORWARD);
	void Pause(); //pause or resume playback
	void Stop(); //abort playback and rewind
	void Evaluate(double time); //calculate m_time from time
	double GetDuration() const { return m_duration; }
	double GetTicksPerSecond() const { return m_ticksPerSecond; }
	void SetTicksPerSecond(double tps) { m_ticksPerSecond = tps; }
	const std::string &GetName() const { return m_name; }
	const Behavior GetBehavior() const { return m_behavior; }
	void SetBehavior(Behavior b) { m_behavior = b; }
	double GetProgress();
	void SetProgress(double); //0.0 -- 1.0, overrides m_time
	void Interpolate(); //update transforms according to m_time;

private:
	friend class Loader;
	Behavior m_behavior;
	bool m_paused;
	Direction m_dir;
	double m_currentTime;
	double m_duration;
	double m_prevMTime;
	double m_ticksPerSecond;
	double m_time; //final anim time used for Interpolate
	std::string m_name;
	std::vector<AnimationChannel> m_channels;
};

}

#endif
