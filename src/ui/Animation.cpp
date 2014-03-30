// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Animation.h"

namespace UI {

Animation::Animation(Widget *widget, Type type, Easing easing, Target target, float duration, bool continuous, Animation *next, sigc::slot<void> callback) :
	m_widget(widget),
	m_type(type),
	m_easing(easing),
	m_target(target),
	m_duration(duration),
	m_continuous(continuous),
	m_next(next),
	m_callback(callback),
	m_running(false),
	m_completed(false)
{
	SelectFunctions();
}

static float easingZero(float t, float b, float c, float d)
{
	return 0.0f;
}

static float easingOne(float t, float b, float c, float d)
{
	return 1.0f;
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

void Animation::TargetPause(const float &pos)
{
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
		case TARGET_PAUSE:          m_targetFunc = sigc::mem_fun(this, &Animation::TargetPause); break;
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
				case EASING_ZERO:   m_easingFunc = easingZero; return;
				case EASING_ONE:    m_easingFunc = easingOne; return;
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
				case EASING_ZERO:   m_easingFunc = easingZero; return;
				case EASING_ONE:    m_easingFunc = easingOne; return;
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
				case EASING_ZERO:   m_easingFunc = easingZero; return;
				case EASING_ONE:    m_easingFunc = easingOne; return;
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

float Animation::Update(float time)
{
	if (m_completed)
		return 0.0f;

	const float remaining = m_continuous ? m_duration : (m_duration - time);
	m_completed = remaining < 0.0f;

	float pos;
	if (m_continuous)
		pos = m_completed ? m_duration : fmodf(time, m_duration);
	else
		pos = m_completed ? m_duration : Clamp(time, 0.0f, m_duration);

	m_targetFunc(m_wrapFunc(m_easingFunc, pos, m_duration));

	m_running = !m_completed;

	if (m_completed && m_callback)
		m_callback();

	return remaining;
}

void Animation::Finish()
{
	if (m_completed) return;
	m_targetFunc(1.0f);
	m_running = false;
	m_completed = true;
}

void AnimationController::Add(Animation *animation)
{
	assert(!animation->IsRunning());
	m_animations.push_back(std::make_pair(RefCountedPtr<Animation>(animation), SDL_GetTicks()));
	animation->Running();
}

void AnimationController::Update()
{
	const Uint32 now = SDL_GetTicks();

	for (auto i = m_animations.begin(); i != m_animations.end();) {
		Animation *animation = (*i).first.Get();
		Uint32 start = (*i).second;

		float remaining = 0.0f;
		if (!animation->IsCompleted())
			remaining = animation->Update(float(now-start)/1000.f);

		if (animation->IsCompleted()) {
			Animation *nextAnimation = animation->GetNext();
			if (nextAnimation) {
				assert(!animation->IsRunning());
				m_animations.push_back(std::make_pair(RefCountedPtr<Animation>(nextAnimation), now-Uint32(-remaining*1000.0f)));
				nextAnimation->Running();
			}
			m_animations.erase(i++);
		}
		else
			++i;
	}
}

}
