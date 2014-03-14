// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Animation.h"

namespace UI {

Animation::Animation(Widget *widget, Type type, Easing easing, Target target, float duration, bool continuous) :
	m_widget(widget),
	m_type(type),
	m_easing(easing),
	m_target(target),
	m_duration(duration),
	m_continuous(continuous)
{
	m_easingFunc = SelectEasingFunction(type, easing);
}

::Easing::Function<float>::Type Animation::SelectEasingFunction(Type type, Easing easing)
{
	switch (m_type) {
		case TYPE_IN: {
			switch (m_easing) {
				case EASING_LINEAR: return ::Easing::Linear::EaseIn<float>;
				case EASING_QUAD:   return ::Easing::Quad::EaseIn<float>;
				case EASING_CUBIC:  return ::Easing::Cubic::EaseIn<float>;
				case EASING_QUART:  return ::Easing::Quart::EaseIn<float>;
				case EASING_QUINT:  return ::Easing::Quint::EaseIn<float>;
				case EASING_SINE:   return ::Easing::Sine::EaseIn<float>;
				case EASING_EXPO:   return ::Easing::Expo::EaseIn<float>;
				case EASING_CIRC:   return ::Easing::Circ::EaseIn<float>;
			}
		}

		case TYPE_OUT: {
			switch (m_easing) {
				case EASING_LINEAR: return ::Easing::Linear::EaseOut<float>;
				case EASING_QUAD:   return ::Easing::Quad::EaseOut<float>;
				case EASING_CUBIC:  return ::Easing::Cubic::EaseOut<float>;
				case EASING_QUART:  return ::Easing::Quart::EaseOut<float>;
				case EASING_QUINT:  return ::Easing::Quint::EaseOut<float>;
				case EASING_SINE:   return ::Easing::Sine::EaseOut<float>;
				case EASING_EXPO:   return ::Easing::Expo::EaseOut<float>;
				case EASING_CIRC:   return ::Easing::Circ::EaseOut<float>;
			}
		}

		case TYPE_IN_OUT: {
			switch (m_easing) {
				case EASING_LINEAR: return ::Easing::Linear::EaseInOut<float>;
				case EASING_QUAD:   return ::Easing::Quad::EaseInOut<float>;
				case EASING_CUBIC:  return ::Easing::Cubic::EaseInOut<float>;
				case EASING_QUART:  return ::Easing::Quart::EaseInOut<float>;
				case EASING_QUINT:  return ::Easing::Quint::EaseInOut<float>;
				case EASING_SINE:   return ::Easing::Sine::EaseInOut<float>;
				case EASING_EXPO:   return ::Easing::Expo::EaseInOut<float>;
				case EASING_CIRC:   return ::Easing::Circ::EaseInOut<float>;
			}
		}
	}

	assert(0);
	return nullptr;
}

bool Animation::Update(float time)
{
	bool completed = time >= m_duration && !m_continuous;

	float pos = m_easingFunc(completed ? m_duration : fmod(time, m_duration), 0.0f, 1.0f, m_duration);

	switch (m_target) {
		case TARGET_POSITION:
			assert(0);
			break;

		case TARGET_OPACITY:
			m_widget->SetAnimatedOpacity(pos);
			break;
	}

	return completed;
}

Uint32 AnimationController::Add(const Animation &animation)
{
	Uint32 id = m_nextId++;
	m_animations.insert(std::make_pair(id, std::make_pair(animation, SDL_GetTicks())));
	return id;
}

void AnimationController::Remove(Uint32 id)
{
	auto i = m_animations.find(id);
	m_animations.erase(i);
}

void AnimationController::Update()
{
	const Uint32 now = SDL_GetTicks();

	for (auto i = m_animations.begin(); i != m_animations.end();) {
		auto &a = (*i).second;
		Animation &animation = a.first;
		Uint32 start = a.second;

		if (animation.Update(float(now-start)/1000.f))
			m_animations.erase(i++);
		else
			++i;
	}
}

}
