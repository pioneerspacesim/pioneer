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
	void Evaluate(double time);
	std::vector<AnimationChannel> channels;

private:
	std::string m_name;
	double fps;
};

}

#endif
