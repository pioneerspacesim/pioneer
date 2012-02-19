#ifndef _FLOATCOMPARISON_H
#define _FLOATCOMPARISON_H

#include <SDL_stdinc.h>
#include <limits>

// Fuzzy floating point comparisons based on:
//   http://realtimecollisiondetection.net/blog/?p=89
//   (absolute & relative error tolerance)
//
//   http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
//   (ULP based tolerance)
//
// ULP-based tolerance implementation takes some architectural ideas from the
// implementation in the Google test framework, and
// http://stackoverflow.com/questions/17333/most-effective-way-for-float-and-double-comparison

//   provides (for float & double):

// bool is_equal_exact(float a, float b);
// bool is_equal_ulps(float a, float b, int ulps = DefaultUlpTolerance);
// int32_t float_ulp_difference(float a, float b);
// bool is_equal_relative(float a, float b, float tolerance = DefaultRelTolerance());
// bool is_equal_absolute(float a, float b, float tolerance = DefaultAbsTolerance());
// bool is_equal_general(float a, float b, float tolerance = DefaultTolerance());
// bool is_equal_general(float a, float b, float relative_tolerance, float absolute_tolerance);

// bool is_zero_exact(float x);
// bool is_zero_or_denorm(float x);
// bool is_zero_general(float x, float tolerance = IEEEFloatTraits<float>::DefaultRelTolerance());

// bool is_nan(float x);
// bool is_finite(float x);
// bool is_denorm(float x);


// ====================================================================

// in the following code, IEEEFloatTraits<T>::bool_type is used to limit
// the application of the functions by SFINAE

template <typename T> struct IEEEFloatTraits;

// --- float function helpers

template <typename T>
inline typename IEEEFloatTraits<T>::float_type float_abs(T x)
{ return (x < T(0)) ? (-x) : x; }

template <typename T>
inline typename IEEEFloatTraits<T>::float_type float_max(T x, T y)
{ return (y > x) ? y : x; }

template <typename T>
inline typename IEEEFloatTraits<T>::float_type float_max(T x, T y, T z)
{ return float_max(x, float_max(y, z)); }

// --- float property helpers

template <typename T>
inline typename IEEEFloatTraits<T>::bool_type is_nan_bits
	(const typename IEEEFloatTraits<T>::uint_type& bits)
{
	typedef typename IEEEFloatTraits<T>::uint_type uint_type;
	const uint_type top = IEEEFloatTraits<T>::TopBit;
	const uint_type ebits = IEEEFloatTraits<T>::ExponentBits;

	// NaN has the exponent bits set, and at least one mantissa bit set
	// (therefore, if you mask off the top bit, the result must be strictly greater than
	//  just the exponent bits set; if it's equal then it's just an infinity; if it's
	//  less, then it's a valid finite number)
	return ((bits & ~top) > ebits);
}

template <typename T>
inline typename IEEEFloatTraits<T>::bool_type is_finite_bits
	(const typename IEEEFloatTraits<T>::uint_type& bits)
{
	typedef typename IEEEFloatTraits<T>::uint_type uint_type;
	const uint_type ebits = IEEEFloatTraits<T>::ExponentBits;
	return ((bits & ebits) != ebits);
}

template <typename T>
inline typename IEEEFloatTraits<T>::bool_type is_denorm_bits
	(const typename IEEEFloatTraits<T>::uint_type& bits)
{
	typedef typename IEEEFloatTraits<T>::uint_type uint_type;
	const uint_type top = IEEEFloatTraits<T>::TopBit;
	const uint_type ebits = IEEEFloatTraits<T>::ExponentBits;
	// denormal numbers have a zero exponent and a non-zero mantissa
	return (bits & ~top) && !(bits & ebits);
}

// ---- float properties (nan, finite, denormal)

template <typename T>
inline typename IEEEFloatTraits<T>::bool_type is_nan(T x) {
	typedef typename IEEEFloatTraits<T>::FloatOrInt union_type;
	union_type fi;
	fi.f = x;
	return is_nan_bits(fi.ui);
}

template <typename T>
inline typename IEEEFloatTraits<T>::bool_type is_finite(T x) {
	typedef typename IEEEFloatTraits<T>::FloatOrInt union_type;
	union_type fi;
	fi.f = x;
	return is_finite_bits(fi.ui);
}

template <typename T>
inline typename IEEEFloatTraits<T>::bool_type is_denorm(T x) {
	typedef typename IEEEFloatTraits<T>::FloatOrInt union_type;
	union_type fi;
	fi.f = x;
	return is_denorm_bits(fi.ui);
}

template <typename T>
inline typename IEEEFloatTraits<T>::bool_type is_zero_or_denorm(T x) {
	return (float_abs(x) < IEEEFloatTraits<T>::SmallestNormalisedValue());
}

// --- exact comparisons

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wfloat-equal"
#endif
inline bool is_equal_exact(float a, float b) { return (a == b); }
inline bool is_equal_exact(double a, double b) { return (a == b); }

inline bool is_zero_exact(float x) { return (x == 0.0f); }
inline bool is_zero_exact(double x) { return (x == 0.0); }
#ifdef __GNUC__
#pragma GCC diagnostic warning "-Wfloat-equal"
#endif

// --- relative & absolute error comparisons

template <typename T>
inline typename IEEEFloatTraits<T>::bool_type is_equal_relative
	(T a, T b, T tol = IEEEFloatTraits<T>::DefaultRelTolerance())
{
	return (float_abs(a - b) <= tol * float_max(float_abs(a), float_abs(b)));
}

template <typename T>
inline typename IEEEFloatTraits<T>::bool_type is_equal_absolute
	(T a, T b, T tol = IEEEFloatTraits<T>::DefaultAbsTolerance())
{
	return (float_abs(a - b) <= tol);
}

template <typename T>
inline typename IEEEFloatTraits<T>::bool_type is_equal_general
	(T a, T b, T rel_tol, T abs_tol)
{
	return (float_abs(a - b) <= float_max(abs_tol, rel_tol * float_max(float_abs(a), float_abs(b))));
}

template <typename T>
inline typename IEEEFloatTraits<T>::bool_type is_equal_general
	(T a, T b, T tol = IEEEFloatTraits<T>::DefaultTolerance())
{
	return (float_abs(a - b) <= tol * float_max(T(1), float_abs(a), float_abs(b)));
}

template <typename T>
inline typename IEEEFloatTraits<T>::bool_type is_zero_general
	(T x, T tol = IEEEFloatTraits<T>::DefaultRelTolerance())
{
	return (float_abs(x) <= tol);
}

// --- ulp-based comparisons

template <typename T>
inline typename IEEEFloatTraits<T>::int_type float_ulp_difference(T a, T b) {
	typedef typename IEEEFloatTraits<T>::FloatOrInt union_type;
	union_type afi, bfi;
	afi.f = a;
	bfi.f = b;

	// transform from sign-magnitude to two's-complement
	if (afi.i < 0) afi.ui = (IEEEFloatTraits<T>::TopBit - afi.ui);
	if (bfi.i < 0) bfi.ui = (IEEEFloatTraits<T>::TopBit - bfi.ui);

	return (bfi.i - afi.i);
}

// IEEEFloatTraits<T>::bool_type used for SFINAE
template <typename T>
inline typename IEEEFloatTraits<T>::bool_type is_equal_ulps
	(T a, T b, typename IEEEFloatTraits<T>::int_type max_ulps
		= IEEEFloatTraits<T>::DefaultUlpTolerance)
{
	typedef typename IEEEFloatTraits<T>::FloatOrInt union_type;
	typedef typename IEEEFloatTraits<T>::int_type int_type;
	union_type afi, bfi;
	afi.f = a;
	bfi.f = b;

	// Infinities aren't close to anything except themselves
	if (  (!is_finite_bits<T>(afi.ui) && is_finite_bits<T>(bfi.ui))
	  || (is_finite_bits<T>(afi.ui) && !is_finite_bits<T>(bfi.ui)))
	  return false;

	// IEEE says NaNs are unequal to everything (even themselves)
	if (is_nan_bits<T>(afi.ui) || is_nan_bits<T>(bfi.ui))
	  return false;

	// transform from sign-magnitude to two's-complement
	if (afi.i < 0) afi.ui = (IEEEFloatTraits<T>::TopBit - afi.ui);
	if (bfi.i < 0) bfi.ui = (IEEEFloatTraits<T>::TopBit - bfi.ui);

	int_type difference = (bfi.i - afi.i);
	difference = (difference < int_type(0)) ? -difference : difference;
	return (difference <= max_ulps);
}

// ====================================================================

template <typename T>
struct IEEEFloatTraits {};

template <>
struct IEEEFloatTraits<double>
{
	typedef double float_type;
	typedef bool bool_type;

	typedef int64_t int_type;
	typedef uint64_t uint_type;

	union FloatOrInt {
		double f;
		uint_type ui;
		int_type i;
	};

	static const uint_type TopBit
		= static_cast<uint_type>(1) << (sizeof(double)*8 - 1);
	static const uint_type ExponentBits
		= (~static_cast<uint_type>(0) << std::numeric_limits<double>::digits) & ~TopBit;
	static const uint_type MantissaBits
		= ~TopBit & ~ExponentBits;

	static const int_type DefaultUlpTolerance
		= 16;

	static double DefaultAbsTolerance() { return 1e-12; }
	static double DefaultRelTolerance() { return 1e-6; }
	static double DefaultTolerance() { return 1e-8; }
	static double SmallestNormalisedValue() { return std::numeric_limits<double>::min(); }
};

template <>
struct IEEEFloatTraits<float>
{
	typedef float float_type;
	typedef bool bool_type;

	typedef int32_t int_type;
	typedef uint32_t uint_type;

	union FloatOrInt {
		float f;
		uint_type ui;
		int_type i;
	};

	static const uint_type TopBit
		= uint_type(1) << (sizeof(float)*8 - 1);
	static const uint_type ExponentBits
		= (~uint_type(0) << std::numeric_limits<float>::digits) & ~TopBit;
	static const uint_type MantissaBits
		= ~TopBit & ~ExponentBits;

	static const int_type DefaultUlpTolerance
		= 4;

	static float DefaultAbsTolerance() { return 1e-6f; }
	static float DefaultRelTolerance() { return 1e-5f; }
	static float DefaultTolerance() { return 1e-5f; }
	static float SmallestNormalisedValue() { return std::numeric_limits<float>::min(); }
};

#endif
