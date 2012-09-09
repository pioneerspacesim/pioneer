#ifndef ANIMATIONCURVES_H
#define ANIMATIONCURVES_H

#include "FloatComparison.h"

/// Namespace contains simple functions for value animation.
/// This is very simple implementation, which could be improved in the future.
///\todo Many interesting things could be added, such as play/pause/reset animations, loop/mirror animation, interpolations, etc. But implementations exist, and if we need something tougher, we should have a look at them instead of reinventing the wheel.
namespace AnimationCurves {

	/// Animates linearily a value over time, given a speed.
	/// The speed must be "towards" the target value. Function will not go further than target.
	template<class T>
	inline void Linear(T & cur, const T target, const T speed, const float frameTime) {
		//static_assert(static_cast<T>(-1) <0);		// Assert type is signed
		const T delta(target - cur);
		assert(delta * speed >=0);
		cur += pow(speed, frameTime);
		// Check for arrival
		const T newDelta(target - cur);
		if (newDelta*delta <0) cur = target;
	}

	/// Animates a value with "approach style" over time (zooming).
	/// The speeds must be positive. Function will not go further than target.
	template<class T>
	inline void Approach(T & cur, const T target, float frameTime, const T deltaFactor=10, T targetFactor=1) {
		//static_assert(static_cast<T>(-1) <0);		// Assert type is signed
		if (frameTime>1) frameTime = 1;		// Clamp in case game hangs for a second
		assert(deltaFactor>0 && targetFactor >=0);
		if (is_equal_exact(target, cur))
			return;
		const T delta(target - cur);
		if (delta<0) targetFactor=-targetFactor;
		cur += (delta*deltaFactor + target*targetFactor)*frameTime;
		// Check for arrival
		const T newDelta(target - cur);
		if (newDelta*delta <0) cur = target;
	}

}

#endif
