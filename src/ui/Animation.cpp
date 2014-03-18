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
	SelectFunctions();
}

static float easeIn(::Easing::Function<float>::Type easingFunc, float t, float d)
{
	return easingFunc(t, 0.0f, 1.0f, d);
}

static float easeOut(::Easing::Function<float>::Type easingFunc, float t, float d)
{
	return 1.0f-easingFunc(t, 0.0f, 1.0f, d);
}

static float easeInOut(::Easing::Function<float>::Type easingFunc, float t, float d)
{
	return 1.0f-fabsf(easingFunc(t, -1.0, 2.0, d));
}

void Animation::TargetOpacity(const float &pos)
{
    m_widget->SetAnimatedOpacity(pos);
}

void Animation::TargetPositionX(const float &pos)
{
    m_widget->SetAnimatedPositionX(pos);
}

void Animation::TargetPositionY(const float &pos)
{
    m_widget->SetAnimatedPositionY(pos);
}

void Animation::TargetPositionXRev(const float &pos)
{
    m_widget->SetAnimatedPositionX(-pos);
}

void Animation::TargetPositionYRev(const float &pos)
{
    m_widget->SetAnimatedPositionY(-pos);
}

void Animation::SelectFunctions()
{
	switch (m_target) {
		case TARGET_OPACITY:        m_targetFunc = sigc::mem_fun(this, &Animation::TargetOpacity); break;
		case TARGET_POSITION_X:     m_targetFunc = sigc::mem_fun(this, &Animation::TargetPositionX); break;
		case TARGET_POSITION_Y:     m_targetFunc = sigc::mem_fun(this, &Animation::TargetPositionY); break;
		case TARGET_POSITION_X_REV: m_targetFunc = sigc::mem_fun(this, &Animation::TargetPositionXRev); break;
		case TARGET_POSITION_Y_REV: m_targetFunc = sigc::mem_fun(this, &Animation::TargetPositionYRev); break;
	}
	assert(m_targetFunc);

	switch (m_type) {
		case TYPE_IN: {
			m_wrapFunc = easeIn;
			switch (m_easing) {
				case EASING_LINEAR: m_easingFunc = ::Easing::Linear::EaseIn<float>; return;
				case EASING_QUAD:   m_easingFunc = ::Easing::Quad::EaseIn<float>; return;
				case EASING_CUBIC:  m_easingFunc = ::Easing::Cubic::EaseIn<float>; return;
				case EASING_QUART:  m_easingFunc = ::Easing::Quart::EaseIn<float>; return;
				case EASING_QUINT:  m_easingFunc = ::Easing::Quint::EaseIn<float>; return;
				case EASING_SINE:   m_easingFunc = ::Easing::Sine::EaseIn<float>; return;
				case EASING_EXPO:   m_easingFunc = ::Easing::Expo::EaseIn<float>; return;
				case EASING_CIRC:   m_easingFunc = ::Easing::Circ::EaseIn<float>; return;
			}
			assert(m_easingFunc);
			return;
		}

		case TYPE_OUT: {
			m_wrapFunc = easeOut;
			switch (m_easing) {
				case EASING_LINEAR: m_easingFunc = ::Easing::Linear::EaseOut<float>; return;
				case EASING_QUAD:   m_easingFunc = ::Easing::Quad::EaseOut<float>; return;
				case EASING_CUBIC:  m_easingFunc = ::Easing::Cubic::EaseOut<float>; return;
				case EASING_QUART:  m_easingFunc = ::Easing::Quart::EaseOut<float>; return;
				case EASING_QUINT:  m_easingFunc = ::Easing::Quint::EaseOut<float>; return;
				case EASING_SINE:   m_easingFunc = ::Easing::Sine::EaseOut<float>; return;
				case EASING_EXPO:   m_easingFunc = ::Easing::Expo::EaseOut<float>; return;
				case EASING_CIRC:   m_easingFunc = ::Easing::Circ::EaseOut<float>; return;
			}
			assert(m_easingFunc);
			return;
		}

		case TYPE_IN_OUT: {
			m_wrapFunc = easeInOut;
			switch (m_easing) {
				case EASING_LINEAR: m_easingFunc = ::Easing::Linear::EaseInOut<float>; return;
				case EASING_QUAD:   m_easingFunc = ::Easing::Quad::EaseInOut<float>; return;
				case EASING_CUBIC:  m_easingFunc = ::Easing::Cubic::EaseInOut<float>; return;
				case EASING_QUART:  m_easingFunc = ::Easing::Quart::EaseInOut<float>; return;
				case EASING_QUINT:  m_easingFunc = ::Easing::Quint::EaseInOut<float>; return;
				case EASING_SINE:   m_easingFunc = ::Easing::Sine::EaseInOut<float>; return;
				case EASING_EXPO:   m_easingFunc = ::Easing::Expo::EaseInOut<float>; return;
				case EASING_CIRC:   m_easingFunc = ::Easing::Circ::EaseInOut<float>; return;
			}
			assert(m_easingFunc);
			return;
		}
	}

	assert(m_easingFunc);
}

bool Animation::Update(float time)
{
	const bool completed = time >= m_duration && !m_continuous;
	m_targetFunc(m_wrapFunc(m_easingFunc, completed ? m_duration : fmodf(time, m_duration), m_duration));
	return completed;
}

Uint32 AnimationController::Add(Animation *animation)
{
	Uint32 id = m_nextId++;
	m_animations.insert(std::make_pair(id, std::make_pair(RefCountedPtr<Animation>(animation), SDL_GetTicks())));
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
		Animation *animation = a.first.Get();
		Uint32 start = a.second;

		if (animation->Update(float(now-start)/1000.f))
			m_animations.erase(i++);
		else
			++i;
	}
}

}
