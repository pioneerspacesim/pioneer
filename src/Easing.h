// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

// Adapted from Robert Penner's easing equations
// and Jesus Gollonet's implementation for C++
//
// http://www.robertpenner.com/easing/
// https://github.com/jesusgollonet/ofpennereasing

#ifndef EASING_H
#define EASING_H

#include "FloatComparison.h"
#include <cmath>

namespace Easing {

	// args are:
	// t: time point to calculate
	// b: value at beginning of range
	// c: change over range (ie end-begin)
	// d: duration of range

	template <typename T>
	struct Function {
		typedef T (*Type)(T t, T b, T c, T d);
	};

	// p(t) = t
	namespace Linear {
		template <typename T>
		T EaseIn(T t, T b, T c, T d)
		{
			return c * t / d + b;
		}
		template <typename T>
		T EaseOut(T t, T b, T c, T d)
		{
			return c * t / d + b;
		}
		template <typename T>
		T EaseInOut(T t, T b, T c, T d)
		{
			return c * t / d + b;
		}
	} // namespace Linear

	// p(t) = t^2
	namespace Quad {
		template <typename T>
		T EaseIn(T t, T b, T c, T d)
		{
			t /= d;
			return c * t * t + b;
		}
		template <typename T>
		T EaseOut(T t, T b, T c, T d)
		{
			t /= d;
			return -c * t * (t - 2) + b;
		}
		template <typename T>
		T EaseInOut(T t, T b, T c, T d)
		{
			t /= d / 2;
			if (t < 1) return ((c / 2) * (t * t)) + b;
			return -c / 2 * ((t - 1) * (t - 3) - 1) + b;
		}
	} // namespace Quad

	// p(t) = t^3
	namespace Cubic {
		template <typename T>
		T EaseIn(T t, T b, T c, T d)
		{
			t /= d;
			return c * t * t * t + b;
		}
		template <typename T>
		T EaseOut(T t, T b, T c, T d)
		{
			t = t / d - 1;
			return c * (t * t * t + 1) + b;
		}
		template <typename T>
		T EaseInOut(T t, T b, T c, T d)
		{
			t /= d / 2;
			if (t < 1) return c / 2 * t * t * t + b;
			t -= 2;
			return c / 2 * (t * t * t + 2) + b;
		}
	} // namespace Cubic

	// p(t) = t^4
	namespace Quart {
		template <typename T>
		T EaseIn(T t, T b, T c, T d)
		{
			t /= d;
			return c * t * t * t * t + b;
		}
		template <typename T>
		T EaseOut(T t, T b, T c, T d)
		{
			t = t / d - 1;
			return -c * (t * t * t * t - 1) + b;
		}
		template <typename T>
		T EaseInOut(T t, T b, T c, T d)
		{
			t /= d / 2;
			if (t < 1) return c / 2 * t * t * t * t + b;
			t -= 2;
			return -c / 2 * (t * t * t * t - 2) + b;
		}
	} // namespace Quart

	// p(t) = t^5
	namespace Quint {
		template <typename T>
		T EaseIn(T t, T b, T c, T d)
		{
			t /= d;
			return c * t * t * t * t * t + b;
		}
		template <typename T>
		T EaseOut(T t, T b, T c, T d)
		{
			t = t / d - 1;
			return c * (t * t * t * t * t + 1) + b;
		}
		template <typename T>
		T EaseInOut(T t, T b, T c, T d)
		{
			t /= d / 2;
			if (t < 1) return c / 2 * t * t * t * t * t + b;
			t -= 2;
			return c / 2 * (t * t * t * t * t + 2) + b;
		}
	} // namespace Quint

	// p(t) = sin(t*pi/2)
	namespace Sine {
		template <typename T>
		T EaseIn(T t, T b, T c, T d)
		{
			return -c * cos(t / d * (M_PI / 2)) + c + b;
		}
		template <typename T>
		T EaseOut(T t, T b, T c, T d)
		{
			return c * sin(t / d * (M_PI / 2)) + b;
		}
		template <typename T>
		T EaseInOut(T t, T b, T c, T d)
		{
			return -c / 2 * (cos(M_PI * t / d) - 1) + b;
		}
	} // namespace Sine

	// p(t) = 2^(10*(t-1))
	namespace Expo {
		template <typename T>
		T EaseIn(T t, T b, T c, T d)
		{
			return (is_zero_general(t)) ? b : c * pow(2, 10 * (t / d - 1)) + b;
		}
		template <typename T>
		T EaseOut(T t, T b, T c, T d)
		{
			return (is_equal_general(t, d)) ? b + c : c * (-pow(2, -10 * t / d) + 1) + b;
		}
		template <typename T>
		T EaseInOut(T t, T b, T c, T d)
		{
			if (is_zero_general(t)) return b;
			if (is_equal_general(t, d)) return b + c;
			t /= d / 2;
			if (t < 1) return c / 2 * pow(2, 10 * (t - 1)) + b;
			return c / 2 * (-pow(2, -10 * --t) + 2) + b;
		}
	} // namespace Expo

	// p(t) = 1-sqrt(1-t^2)
	namespace Circ {
		template <typename T>
		T EaseIn(T t, T b, T c, T d)
		{
			t /= d;
			return -c * (sqrt(1 - t * t) - 1) + b;
		}
		template <typename T>
		T EaseOut(T t, T b, T c, T d)
		{
			t /= d;
			return c * (sqrt(1 - (t - 1) * (t - 1))) + b;
		}
		template <typename T>
		T EaseInOut(T t, T b, T c, T d)
		{
			t /= d / 2;
			if (t < 1) return -c / 2 * (sqrt(1 - t * t) - 1) + b;
			return c / 2 * (sqrt(1 - (t - 2) * (t - 2)) + 1) + b;
		}
	} // namespace Circ

} // namespace Easing

#endif
