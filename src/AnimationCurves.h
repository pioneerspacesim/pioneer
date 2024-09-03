// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef ANIMATIONCURVES_H
#define ANIMATIONCURVES_H

#include "FloatComparison.h"
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

	// easing from https://github.com/Michaelangel007/easing#tldr-shut-up-and-show-me-the-code
	// p should go from 0.0 to 1.0
	inline float OutSineEasing(float p)
	{
		return std::sin(p * M_PI * 0.5);
	}

	// Based on http://blog.moagrius.com/actionscript/jsas-understanding-easing/
	// and observations from Godot Engine and Star Citizen
	// This supports four different easing functions encoded in a single float:
	// e == 1.0|-1.0: linear easing, returns p
	// e  >  1.0: in-quadratic, in-cubic etc. for e = 2.0, e = 3.0 ...
	// e  <  1.0: out-quadratic, out-cubic etc. for e = 0.5, e = 0.33_ ...
	// e ==  0.0: returns zero
	// e  > -1.0: reverse inout-quadratic, inout-cubic for e = -0.5, e = -0.33_ ...
	// e  < -1.0: inout-quadratic, inout-cubic etc. for e = -2.0, e = -3.0 ...
	inline float SmoothEasing(double p, double e)
	{
		// e > 0.0 = ease in or out / e < 0.0 = ease in-out
		if (e > 0.0) {
			// e < 1.0 = ease out / e > 1.0 = ease in
			return e < 1.0 ? (1.0 - pow(1.0 - p, 1.0 / e)) : (pow(p, e));
		} else if (e < 0) {
			// Ease in-out at arbirary exponents (the e term is negated to get positive exponent)
			float m = (p - 0.5) * 2.0;
			float t = p * 2.0;
			if (t < 1)
				return pow(t, -e) * 0.5;
			else
				return (1.0 - pow(1.0 - m, -e)) * 0.5 + 0.5;
		} else // completely flat at 0.0
			return 0.0;
	}

} // namespace AnimationCurves

#endif
