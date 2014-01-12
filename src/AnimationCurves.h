// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef ANIMATIONCURVES_H
#define ANIMATIONCURVES_H

#include "FloatComparison.h"

namespace AnimationCurves {

	// Animates a value with "approach style" over time (zooming).
	// The speeds must be positive. Function will not go further than target.
	template<class T>
	inline void Approach(T & cur, const T target, float frameTime, const T exponentialFactor=10, T linearFactor=1) {
		//static_assert(static_cast<T>(-1) <0); // Assert type is signed
		if (frameTime>1) frameTime = 1; // Clamp in case game hangs for a second
		assert(exponentialFactor >= 0 && linearFactor >= 0);
		if (is_equal_exact(target, cur))
			return;
		const T delta(target - cur);
		if (delta < 0) linearFactor = -linearFactor;
		cur += (delta*exponentialFactor + linearFactor) * frameTime;
		// clamp to target (independent of the direction of motion)
		const T newDelta(target - cur);
		if (newDelta*delta < 0) cur = target;
	}

}

#endif
