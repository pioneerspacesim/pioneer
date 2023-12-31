// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
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
	class BinaryConverter;
	class Node;

	class Animation {
	public:
		Animation(const std::string &name, double duration);
		Animation(const Animation &);
		void UpdateChannelTargets(Node *root);
		double GetDuration() const { return m_duration; }
		const std::string &GetName() const { return m_name; }
		double GetProgress();
		void SetProgress(double); //0.0 -- 1.0, overrides m_time
		void Interpolate(); //update transforms according to m_time;
		const std::vector<AnimationChannel> &GetChannels() const { return m_channels; }

	private:
		friend class Loader;
		friend class BinaryConverter;
		double m_duration;
		double m_time;
		std::string m_name;
		std::vector<AnimationChannel> m_channels;
	};

} // namespace SceneGraph

#endif
