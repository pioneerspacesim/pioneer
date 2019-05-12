// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _UTILS_H
#define _UTILS_H

#if defined(_MSC_VER) && !defined(NOMINMAX)
#define NOMINMAX
#endif

#include "core/Log.h"
#include "libs.h"
#include <fmt/printf.h>
#include <stdarg.h>
#include <stdio.h>
#include <string>
#include <vector>

#ifndef __GNUC__
#define __attribute(x)
#endif /* __GNUC__ */

// GCC warns when a function marked __attribute((noreturn)) actually returns a value
// but other compilers which don't see the noreturn attribute of course require that
// a function with a non-void return type should return something.
#ifndef __GNUC__
#define RETURN_ZERO_NONGNU_ONLY return 0;
#else
#define RETURN_ZERO_NONGNU_ONLY
#endif

// align x to a. taken from the Linux kernel
#define ALIGN(x, a) __ALIGN_MASK(x, (a - 1))
#define __ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))

// void Error(const char *format, ...) __attribute((format(printf, 1, 2))) __attribute((noreturn));
// void Warning(const char *format, ...) __attribute((format(printf, 1, 2)));
// void Output(const char *format, ...) __attribute((format(printf, 1, 2)));

template <typename... Args>
inline void Output(const char *message, Args... args)
{
	Log::LogOld(Log::Severity::Info, fmt::sprintf(message, args...));
}

template <typename... Args>
inline void Warning(const char *message, Args... args)
{
	Log::LogOld(Log::Severity::Warning, fmt::sprintf(message, args...));
}

template <typename... Args>
[[noreturn]] inline void Error(const char *message, Args... args)
{
	Log::LogFatalOld(fmt::sprintf(message, args...));
}

template <typename... Args>
inline void DebugMsg(const char *message, Args... args)
{
	Log::LogOld(Log::Severity::Debug, fmt::sprintf(message, args...));
}

std::string string_join(std::vector<std::string> &v, std::string sep);
std::string format_date(double time);
std::string format_date_only(double time);
std::string format_distance(double dist, int precision = 2);
std::string format_money(double cents, bool showCents = true);
std::string format_duration(double seconds);

static inline Sint64 isqrt(Sint64 a)
{
	// replace with cast from sqrt below which is between x7.3 (win32, Debug) & x15 (x64, Release) times faster
	return static_cast<int64_t>(sqrt(static_cast<double>(a)));
}

static inline Sint64 isqrt(fixed v)
{
	Sint64 ret = 0;
	Sint64 s;
	Sint64 ret_sq = -v.v - 1;
	for (s = 62; s >= 0; s -= 2) {
		Sint64 b;
		ret += ret;
		b = ret_sq + ((2 * ret + 1) << s);
		if (b < 0) {
			ret_sq = b;
			ret++;
		}
	}
	return ret;
}

// find string in bigger string, ignoring case
const char *pi_strcasestr(const char *haystack, const char *needle);

inline bool starts_with(const char *s, const char *t)
{
	assert(s && t);
	while ((*s == *t) && *t) {
		++s;
		++t;
	}
	return (*t == '\0');
}

inline bool starts_with(const std::string &s, const char *t)
{
	assert(t);
	return starts_with(s.c_str(), t);
}

inline bool starts_with(const std::string &s, const std::string &t)
{
	return starts_with(s.c_str(), t.c_str());
}

inline bool ends_with(const char *s, size_t ns, const char *t, size_t nt)
{
	return (ns >= nt) && (memcmp(s + (ns - nt), t, nt) == 0);
}

inline bool ends_with(const char *s, const char *t)
{
	return ends_with(s, strlen(s), t, strlen(t));
}

inline bool ends_with(const std::string &s, const char *t)
{
	return ends_with(s.c_str(), s.size(), t, strlen(t));
}

inline bool ends_with(const std::string &s, const std::string &t)
{
	return ends_with(s.c_str(), s.size(), t.c_str(), t.size());
}

inline bool ends_with_ci(const char *s, size_t ns, const char *t, size_t nt)
{
	if (ns < nt) return false;
	s += (ns - nt);
	for (size_t i = 0; i < nt; i++)
		if (tolower(*s++) != tolower(*t++)) return false;
	return true;
}

inline bool ends_with_ci(const char *s, const char *t)
{
	return ends_with_ci(s, strlen(s), t, strlen(t));
}

inline bool ends_with_ci(const std::string &s, const char *t)
{
	return ends_with_ci(s.c_str(), s.size(), t, strlen(t));
}

inline bool ends_with_ci(const std::string &s, const std::string &t)
{
	return ends_with_ci(s.c_str(), s.size(), t.c_str(), t.size());
}

static inline size_t SplitSpec(const std::string &spec, std::vector<int> &output)
{
	static const std::string delim(",");

	size_t i = 0, start = 0, end = 0;
	while (end != std::string::npos) {
		// get to the first non-delim char
		start = spec.find_first_not_of(delim, end);

		// read the end, no more to do
		if (start == std::string::npos)
			break;

		// find the end - next delim or end of string
		end = spec.find_first_of(delim, start);

		// extract the fragment and remember it
		output[i++] = atoi(spec.substr(start, (end == std::string::npos) ? std::string::npos : end - start).c_str());
	}

	return i;
}

static inline size_t SplitSpec(const std::string &spec, std::vector<float> &output)
{
	static const std::string delim(",");

	size_t i = 0, start = 0, end = 0;
	while (end != std::string::npos) {
		// get to the first non-delim char
		start = spec.find_first_not_of(delim, end);

		// read the end, no more to do
		if (start == std::string::npos)
			break;

		// find the end - next delim or end of string
		end = spec.find_first_of(delim, start);

		// extract the fragment and remember it
		output[i++] = atof(spec.substr(start, (end == std::string::npos) ? std::string::npos : end - start).c_str());
	}

	return i;
}

std::vector<std::string> SplitString(const std::string &source, const std::string &delim);

// 'Numeric type' to string conversions.
std::string FloatToStr(float val);
std::string DoubleToStr(double val);
std::string AutoToStr(Sint32 val);
std::string AutoToStr(Sint64 val);
std::string AutoToStr(float val);
std::string AutoToStr(double val);

void Vector3fToStr(const vector3f &val, char *out, size_t size);
void Vector3dToStr(const vector3d &val, char *out, size_t size);
void Matrix3x3fToStr(const matrix3x3f &val, char *out, size_t size);
void Matrix3x3dToStr(const matrix3x3d &val, char *out, size_t size);
void Matrix4x4fToStr(const matrix4x4f &val, char *out, size_t size);
void Matrix4x4dToStr(const matrix4x4d &val, char *out, size_t size);

// String to 'Numeric type' conversions.
Sint64 StrToSInt64(const std::string &str);
Uint64 StrToUInt64(const std::string &str);
float StrToFloat(const std::string &str);
double StrToDouble(const std::string &str);
void StrToAuto(Sint32 *pVal, const std::string &str);
void StrToAuto(Sint64 *pVal, const std::string &str);
void StrToAuto(float *pVal, const std::string &str);
void StrToAuto(double *pVal, const std::string &str);

void StrToVector3f(const char *str, vector3f &val);
void StrToVector3d(const char *str, vector3d &val);
void StrToMatrix3x3f(const char *str, matrix3x3f &val);
void StrToMatrix3x3d(const char *str, matrix3x3d &val);
void StrToMatrix4x4f(const char *str, matrix4x4f &val);
void StrToMatrix4x4d(const char *str, matrix4x4d &val);

// Convert decimal coordinates to degree/minute/second format and return as string
std::string DecimalToDegMinSec(float dec);

// add a few things that MSVC is missing
#if defined(_MSC_VER) && (_MSC_VER < 1800)

// round & roundf. taken from http://cgit.freedesktop.org/mesa/mesa/tree/src/gallium/auxiliary/util/u_math.h
static inline double round(double x)
{
	return x >= 0.0 ? floor(x + 0.5) : ceil(x - 0.5);
}

static inline float roundf(float x)
{
	return x >= 0.0f ? floorf(x + 0.5f) : ceilf(x - 0.5f);
}
#endif /* _MSC_VER < 1800 */

static inline Uint32 ceil_pow2(Uint32 v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

void hexdump(const unsigned char *buf, int bufsz);

inline Color4f ColourFromNormal(const vector3d &n)
{
	return Color4f(
		(n.x + 1.0f) * 0.5f,
		(n.y + 1.0f) * 0.5f,
		(n.z + 1.0f) * 0.5f);
}

#endif /* _UTILS_H */
