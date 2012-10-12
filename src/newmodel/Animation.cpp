#include "Animation.h"
#include <iostream>

namespace Newmodel {

typedef std::vector<AnimationChannel>::iterator ChannelIterator;

Animation::Animation(const std::string &name, double duration, Behavior behavior, double tps)
: m_behavior(behavior)
, m_paused(true)
, m_dir(FORWARD)
, m_currentTime(0.0)
, m_duration(duration)
, m_ticksPerSecond(tps)
, m_name(name)
, m_prevMTime(0.0)
{

}

void Animation::Play(Direction dir)
{
	m_paused = false;
	m_prevMTime = 0.0;
	m_dir = dir;
}

void Animation::Pause()
{
	m_paused = true;
}

void Animation::Stop()
{
	m_paused = true;
	m_currentTime = 0.0;
	m_prevMTime = 0.0;
}

void Animation::Evaluate(const double time)
{
	if (!m_paused && m_duration > 0.0)
	{
		m_currentTime += time * m_ticksPerSecond;
	}

	//map into anim duration
	double mtime = fmod(m_currentTime, m_duration);

	if (m_behavior == ONCE && m_prevMTime > mtime) {
		Stop();
		mtime = m_duration;
	}
	m_prevMTime = mtime;
	if (m_dir == REVERSE) mtime = m_duration - mtime;

	m_time = mtime;
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
			while (frame < chan->rotationKeys.size() - 1) {
				if (mtime < chan->rotationKeys[frame+1].time)
					break;
				frame++;
			}
			const unsigned int nextFrame = (frame + 1) % chan->rotationKeys.size();

			const RotationKey &a = chan->rotationKeys[frame];
			const RotationKey &b = chan->rotationKeys[nextFrame];
			double diffTime = b.time - a.time;
			if (diffTime < 0.0)
				diffTime += m_duration;
			if (diffTime > 0.0) {
				const float factor = Clamp(float((mtime - a.time) / diffTime), 0.f, 1.f);
				trans.SetRotationOnly(Quaternionf::Nlerp(a.rotation, b.rotation, factor).ToMatrix4x4<float>());
			} else {
				trans.SetRotationOnly(a.rotation.ToMatrix4x4<float>());
			}

			//scaling will not work without rotation since it would
			//continously scale the transform (would have to add originalTransform or
			//something to MT)
			if (!chan->scaleKeys.empty()) {
				//find a frame. To optimize, should begin search from previous frame (when mTime > previous mTime)
				unsigned int frame = 0;
				while (frame < chan->scaleKeys.size() - 1) {
					if (mtime < chan->scaleKeys[frame+1].time)
						break;
					frame++;
				}
				const unsigned int nextFrame = (frame + 1) % chan->scaleKeys.size();

				const ScaleKey &a = chan->scaleKeys[frame];
				const ScaleKey &b = chan->scaleKeys[nextFrame];
				double diffTime = b.time - a.time;
				if (diffTime < 0.0)
					diffTime += m_duration;
				vector3f out;
				if (diffTime > 0.0) {
					const float factor = Clamp(float((mtime - a.time) / diffTime), 0.f, 1.f);
					out = a.scale + (b.scale - a.scale) * factor;
				} else {
					out = a.scale;
				}

				trans[0] *= out.x; trans[4] *= out.x; trans[8]  *= out.x;
				trans[1] *= out.y; trans[5] *= out.y; trans[9]  *= out.y;
				trans[2] *= out.z; trans[6] *= out.z; trans[10] *= out.z;
			}
		}

		if (!chan->positionKeys.empty()) {
			//find a frame. To optimize, should begin search from previous frame (when mTime > previous mTime)
			unsigned int frame = 0;
			while (frame < chan->positionKeys.size() - 1) {
				if (mtime < chan->positionKeys[frame+1].time)
					break;
				frame++;
			}
			const unsigned int nextFrame = (frame + 1) % chan->positionKeys.size();

			const PositionKey &a = chan->positionKeys[frame];
			const PositionKey &b = chan->positionKeys[nextFrame];
			double diffTime = b.time - a.time;
			if (diffTime < 0.0)
				diffTime += m_duration;
			if (diffTime > 0.0) {
				const float factor = Clamp(float((mtime - a.time) / diffTime), 0.f, 1.f);
				vector3f out(0.f);
				out = a.position + (b.position - a.position) * factor;
				trans.SetTranslate(out);
			} else {
				trans.SetTranslate(a.position);
			}
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
