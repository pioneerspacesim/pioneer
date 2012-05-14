#include "Animation.h"

namespace Newmodel {

Animation::Animation(const std::string &name)
: m_name(name)
{

}

void Animation::Evaluate(const double time)
{
	//go through channels and calculate transforms
	for(unsigned int i = 0; i < channels.size(); i++) {
		AnimationChannel &chan = channels[i];
		if (!chan.rotationKeys.empty()) {
			//interpolation test
			const Quaternionf &a = chan.rotationKeys[0].rotation;
			const Quaternionf &b = chan.rotationKeys[1].rotation;
			matrix4x4f trans = chan.node->GetTransform();
			trans.SetRotationOnly(Quaternionf::Nlerp(a, b, fmod(time, 5.0) / 5.f).ToMatrix4x4<float>());
			chan.node->SetTransform(trans);
		}
	}
}

}
