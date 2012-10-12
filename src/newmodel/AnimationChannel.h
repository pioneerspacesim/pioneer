#ifndef _NEWMODEL_ANIMATIONCHANNEL_H
#define _NEWMODEL_ANIMATIONCHANNEL_H
/*
 * Animation channel affecting a single transform node
 */
#include "MatrixTransform.h"
#include "AnimationKey.h"
namespace Newmodel {

class AnimationChannel {
public:
	AnimationChannel(MatrixTransform *t) : node(t) { }
	std::vector<PositionKey> positionKeys;
	std::vector<RotationKey> rotationKeys;
	std::vector<ScaleKey> scaleKeys;
	MatrixTransform *node;
};

}

#endif
