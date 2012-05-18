#include "Animation.h"

namespace Newmodel {

Animation::Animation(const std::string &name)
: m_name(name)
, m_ticksPerSecond(25.0)
, m_duration(100.0)
{

}

void Animation::Evaluate(const double time)
{
	const double ptime = time * m_ticksPerSecond;

	//map into anim duration
	double mtime = 0.0;
	if (m_duration > 0.0)
		mtime = fmod(ptime, m_duration);


	//go through channels and calculate transforms
	for(unsigned int i = 0; i < channels.size(); i++) {
		AnimationChannel &chan = channels[i];
		//interpolation test
		matrix4x4f trans = chan.node->GetTransform();

		if (!chan.rotationKeys.empty()) {
			//find a frame. To optimize, should begin search from previous frame (when mTime > previous mTime)
			unsigned int frame = 0;
			while (frame < chan.rotationKeys.size() - 1) {
				if (mtime < chan.rotationKeys[frame+1].time)
					break;
				frame++;
			}
			const unsigned int nextFrame = (frame + 1) % chan.rotationKeys.size();

			const RotationKey &a = chan.rotationKeys[frame];
			const RotationKey &b = chan.rotationKeys[nextFrame];
			double diffTime = b.time - a.time;
			if (diffTime < 0.0)
				diffTime += m_duration;
			if (diffTime > 0.0) {
				const float factor = float((mtime - a.time) / diffTime);
				trans.SetRotationOnly(Quaternionf::Nlerp(a.rotation, b.rotation, factor).ToMatrix4x4<float>());
			} else {
				trans.SetRotationOnly(a.rotation.ToMatrix4x4<float>());
			}
		}

		if (!chan.positionKeys.empty()) {
			//find a frame. To optimize, should begin search from previous frame (when mTime > previous mTime)
			unsigned int frame = 0;
			while (frame < chan.positionKeys.size() - 1) {
				if (mtime < chan.positionKeys[frame+1].time)
					break;
				frame++;
			}
			const unsigned int nextFrame = (frame + 1) % chan.positionKeys.size();

			const PositionKey &a = chan.positionKeys[frame];
			const PositionKey &b = chan.positionKeys[nextFrame];
			double diffTime = b.time - a.time;
			if (diffTime < 0.0)
				diffTime += m_duration;
			if (diffTime > 0.0) {
				const float factor = float((mtime - a.time) / diffTime);
				vector3f out(0.f);
				out = a.position + (b.position - a.position) * factor;
				trans.SetTranslate(out);
			} else {
				trans.SetTranslate(a.position);
			}
		}

		chan.node->SetTransform(trans);
	}
}

}
