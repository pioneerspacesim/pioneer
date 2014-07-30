// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SCENEGRAPH_ANIMATIONCHANNEL_H
#define _SCENEGRAPH_ANIMATIONCHANNEL_H
/*
 * Animation channel affecting a single transform node
 */
#include "MatrixTransform.h"
#include "AnimationKey.h"
namespace SceneGraph {

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
