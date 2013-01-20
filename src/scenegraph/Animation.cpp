// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Animation.h"
#include <iostream>

namespace SceneGraph {

typedef std::vector<AnimationChannel>::iterator ChannelIterator;

Animation::Animation(const std::string &name, double duration)
: m_duration(duration)
, m_name(name)
{
}

void Animation::Interpolate()
{
	const double mtime = m_time;

	//go through channels and calculate transforms
	for(ChannelIterator chan = m_channels.begin(); chan != m_channels.end(); ++chan) {
		matrix4x4f trans = chan->node->GetTransform();

		if (!chan->rotationKeys.empty()) {
			//find a frame. To optimize, should begin search from previous frame (when mTime > previous mTime)
			unsigned int frame = 0;
			while (frame < chan->rotationKeys.size() - 2) {
				if (mtime < chan->rotationKeys[frame+1].time)
					break;
				frame++;
			}
			const unsigned int nextFrame = (frame + 1);

			const RotationKey &a = chan->rotationKeys[frame];
			const RotationKey &b = chan->rotationKeys[nextFrame];
			double diffTime = b.time - a.time;
			assert(diffTime > 0.0);
			const float factor = Clamp(float((mtime - a.time) / diffTime), 0.f, 1.f);
			vector3f temp = trans.GetTranslate();
			trans = Quaternionf::Nlerp(a.rotation, b.rotation, factor).ToMatrix3x3<float>();
			trans.SetTranslate(temp);
		}

		//scaling will not work without rotation since it would
		//continously scale the transform (would have to add originalTransform or
		//something to MT)
		if (!chan->scaleKeys.empty() && !chan->rotationKeys.empty()) {
			//find a frame. To optimize, should begin search from previous frame (when mTime > previous mTime)
			unsigned int frame = 0;
			while (frame < chan->scaleKeys.size() - 2) {
				if (mtime < chan->scaleKeys[frame+1].time)
					break;
				frame++;
			}
			const unsigned int nextFrame = (frame + 1);

			const ScaleKey &a = chan->scaleKeys[frame];
			const ScaleKey &b = chan->scaleKeys[nextFrame];
			double diffTime = b.time - a.time;
			assert(diffTime > 0.0);
			const float factor = Clamp(float((mtime - a.time) / diffTime), 0.f, 1.f);
			vector3f out = a.scale + (b.scale - a.scale) * factor;
			trans.Scale(out.x, out.y, out.z);
		}

		if (!chan->positionKeys.empty()) {
			//find a frame. To optimize, should begin search from previous frame (when mTime > previous mTime)
			unsigned int frame = 0;
			while (frame < chan->positionKeys.size() - 2) {
				if (mtime < chan->positionKeys[frame+1].time)
					break;
				frame++;
			}
			const unsigned int nextFrame = (frame + 1);

			const PositionKey &a = chan->positionKeys[frame];
			const PositionKey &b = chan->positionKeys[nextFrame];
			double diffTime = b.time - a.time;
			assert(diffTime > 0.0);
			const float factor = Clamp(float((mtime - a.time) / diffTime), 0.f, 1.f);
			vector3f out = a.position + (b.position - a.position) * factor;
			trans.SetTranslate(out);
		}

		chan->node->SetTransform(trans);
	}
}

double Animation::GetProgress()
{
	return m_time / m_duration;
}

void Animation::SetProgress(double prog)
{
	m_time = Clamp(prog, 0.0, 1.0) * m_duration;
}

}
