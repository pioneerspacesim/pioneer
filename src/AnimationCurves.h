#ifndef ANIMATIONCURVES_H
#define ANIMATIONCURVES_H

#include "FloatComparison.h"

namespace AnimationCurves {

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
