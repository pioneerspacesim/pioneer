// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef ANIMATIONCURVES_H
#define ANIMATIONCURVES_H

#include "FloatComparison.h"
#include "Pi.h"
#include <cmath>

namespace AnimationCurves {

	// Animates a value with "approach style" over time (zooming).
	// The speeds must be positive. Function will not go further than target.
	template <class T>
	inline void Approach(T &cur, const T target, float frameTime, const T exponentialFactor = 10, T linearFactor = 1)
	{
		//static_assert(static_cast<T>(-1) <0); // Assert type is signed
		if (frameTime > 1) frameTime = 1; // Clamp in case game hangs for a second
		assert(exponentialFactor >= 0 && linearFactor >= 0);
		if (is_equal_exact(target, cur))
			return;
		const T delta(target - cur);
		if (delta < 0) linearFactor = -linearFactor;
		cur += (delta * exponentialFactor + linearFactor) * frameTime;
		// clamp to target (independent of the direction of motion)
		const T newDelta(target - cur);
		if (newDelta * delta < 0) cur = target;
	}

	// easing from https://github.com/Michaelangel007/easing#tldr-shut-up-and-show-me-the-code
	// p should go from 0.0 to 1.0
	inline float InOutQuadraticEasing(float p)
	{
		float m = p - 1.0;
		float t = p * 2.0;
		if (t < 1)
			return p * t;
		else
			return 1.0 - m * m * 2.0;
	}

	// easing from https://github.com/Michaelangel007/easing#tldr-shut-up-and-show-me-the-code
	// p should go from 0.0 to 1.0
	inline float InOutCubicEasing(float p)
	{
		float m = p - 1.0;
		float t = p * 2.0;
		if (t < 1)
			return p * t * t;
		else
			return 1.0 + m * m * m * 4.0;
	}

	// easing from https://github.com/Michaelangel007/easing#tldr-shut-up-and-show-me-the-code
	// p should go from 0.0 to 1.0
	inline float InOutSineEasing(float p)
	{
		return 0.5 * (1.0 - std::cos(p * M_PI));
	}

} // namespace AnimationCurves

#endif
