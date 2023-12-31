// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Animation.h"
#include "MathUtil.h"
#include "scenegraph/Model.h"
#include "profiler/Profiler.h"
#include <iostream>

namespace SceneGraph {

	typedef std::vector<AnimationChannel> ChannelList;
	typedef ChannelList::iterator ChannelIterator;

	Animation::Animation(const std::string &name, double duration) :
		m_duration(duration),
		m_time(0.0),
		m_name(name)
	{
	}

	Animation::Animation(const Animation &anim) :
		m_duration(anim.m_duration),
		m_time(0.0),
		m_name(anim.m_name)
	{
		for (ChannelList::const_iterator chan = anim.m_channels.begin(); chan != anim.m_channels.end(); ++chan) {
			m_channels.push_back(*chan);
		}
	}

	void Animation::UpdateChannelTargets(Node *root)
	{
		for (ChannelList::iterator chan = m_channels.begin(); chan != m_channels.end(); ++chan) {
			//update channels to point to new node structure
			MatrixTransform *trans = dynamic_cast<MatrixTransform *>(root->FindNode(chan->node->GetName()));
			assert(trans);
			chan->node = trans;
		}
	}

	void Animation::Interpolate()
	{
		PROFILE_SCOPED()
		const double mtime = m_time;

		//go through channels and calculate transforms
		for (ChannelIterator chan = m_channels.begin(); chan != m_channels.end(); ++chan) {
			matrix4x4f trans = chan->node->GetTransform();

			if (!chan->rotationKeys.empty()) {
				//find a frame. To optimize, should begin search from previous frame (when mTime > previous mTime)
				unsigned int frame = 0;
				while (frame + 1 < chan->rotationKeys.size()) {
					if (mtime < chan->rotationKeys[frame + 1].time)
						break;
					frame++;
				}

				const RotationKey &a = chan->rotationKeys[frame];
				vector3f saved_position = trans.GetTranslate();
				if (frame + 1 < chan->rotationKeys.size()) {
					const RotationKey &b = chan->rotationKeys[frame + 1];
					double diffTime = b.time - a.time;
					assert(diffTime > 0.0);
					const float factor = Clamp(float((mtime - a.time) / diffTime), 0.f, 1.f);
					trans = Quaternionf::Slerp(a.rotation, b.rotation, factor).ToMatrix3x3<float>();
				} else {
					trans = a.rotation.ToMatrix3x3<float>();
				}
				trans.SetTranslate(saved_position);
			}

			//scaling will not work without rotation since it would
			//continously scale the transform (would have to add originalTransform or
			//something to MT)
			if (!chan->scaleKeys.empty() && !chan->rotationKeys.empty()) {
				//find a frame. To optimize, should begin search from previous frame (when mTime > previous mTime)
				unsigned int frame = 0;
				while (frame + 1 < chan->scaleKeys.size()) {
					if (mtime < chan->scaleKeys[frame + 1].time)
						break;
					frame++;
				}

				const ScaleKey &a = chan->scaleKeys[frame];
				vector3f out;
				if (frame + 1 < chan->scaleKeys.size()) {
					const ScaleKey &b = chan->scaleKeys[frame + 1];
					double diffTime = b.time - a.time;
					assert(diffTime > 0.0);
					const float factor = Clamp(float((mtime - a.time) / diffTime), 0.f, 1.f);
					out = a.scale + (b.scale - a.scale) * factor;
				} else {
					out = a.scale;
				}
				trans.Scale(out.x, out.y, out.z);
			}

			if (!chan->positionKeys.empty()) {
				//find a frame. To optimize, should begin search from previous frame (when mTime > previous mTime)
				unsigned int frame = 0;
				while (frame + 1 < chan->positionKeys.size()) {
					if (mtime < chan->positionKeys[frame + 1].time)
						break;
					frame++;
				}

				const PositionKey &a = chan->positionKeys[frame];
				vector3f out;
				if (frame + 1 < chan->positionKeys.size()) {
					const PositionKey &b = chan->positionKeys[frame + 1];
					double diffTime = b.time - a.time;
					assert(diffTime > 0.0);
					const float factor = Clamp(float((mtime - a.time) / diffTime), 0.f, 1.f);
					out = a.position + (b.position - a.position) * factor;
				} else {
					out = a.position;
				}
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

} // namespace SceneGraph
