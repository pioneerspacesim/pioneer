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
		//interpolation test
		const float factor = fmod(time, 5.0) / 5.f;
		matrix4x4f trans = chan.node->GetTransform();

		if (!chan.rotationKeys.empty()) {
			const unsigned int frame = 0;
			const unsigned int nextFrame = (frame + 1) % chan.rotationKeys.size();
			const Quaternionf &a = chan.rotationKeys[frame].rotation;
			const Quaternionf &b = chan.rotationKeys[nextFrame].rotation;
			trans.SetRotationOnly(Quaternionf::Nlerp(a, b, factor).ToMatrix4x4<float>());
		}

		if (!chan.positionKeys.empty()) {
			vector3f out(0.f);
			const vector3f &a = chan.positionKeys[0].position;
			const vector3f &b = chan.positionKeys[1].position;
			out = a + (b - a) * factor;
			trans.SetTranslate(out);
		}

		chan.node->SetTransform(trans);
	}
}

}
