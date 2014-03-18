// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_ANIMATION_H
#define UI_ANIMATION_H

#include "libs.h"
#include "RefCounted.h"
#include "Widget.h"
#include "Easing.h"

namespace UI {

class Animation : public RefCounted {
public:
	enum Type {
		TYPE_IN,
		TYPE_OUT,
		TYPE_IN_OUT,
	};

	enum Easing {
		EASING_LINEAR,
		EASING_QUAD,
		EASING_CUBIC,
		EASING_QUART,
		EASING_QUINT,
		EASING_SINE,
		EASING_EXPO,
		EASING_CIRC,
	};

	enum Target {
		TARGET_OPACITY,
		TARGET_POSITION_X,
		TARGET_POSITION_Y,
		TARGET_POSITION_X_REV,
		TARGET_POSITION_Y_REV
	};

	Animation(Widget *widget, Type type, Easing easing, Target target, float duration = 1.0f, bool continuous = false);

	bool Update(float time);

	void Completed() { m_running = false; m_completed = true; Update(1.0f); }

	bool IsRunning() const { return m_running; }
	bool IsCompleted() const { return m_completed; }

private:
	RefCountedPtr<Widget> m_widget;
	Type m_type;
	Easing m_easing;
	Target m_target;
	float m_duration;
	bool m_continuous;

	void TargetOpacity(const float &pos);
	void TargetPositionX(const float &pos);
	void TargetPositionY(const float &pos);
	void TargetPositionXRev(const float &pos);
	void TargetPositionYRev(const float &pos);

	void SelectFunctions();
	::Easing::Function<float>::Type m_easingFunc;
	float (*m_wrapFunc)(::Easing::Function<float>::Type easingFunc, float t, float d);
	sigc::slot<void,const float &> m_targetFunc;

	// AnimationController needs to set running/completed flags
	friend class AnimationController;
	void Running() { m_running = true; m_completed = false; Update(0.0f); }

	bool m_running;
	bool m_completed;

};

class AnimationController {
public:
	void Add(Animation *animation);

	void Update();

private:
	std::list<std::pair<RefCountedPtr<Animation>,Uint32>> m_animations;
};

}

#endif
