// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "utils.h"
#include "DateTime.h"
#include "FileSystem.h"
#include "Lang.h"
#include "StringF.h"
#include "gameconsts.h"
#include "graphics/Graphics.h"
#include "libs.h"
#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>

std::string format_money(double cents, bool showCents)
{
	char *end; // for  error checking
	size_t groupDigits = strtol(Lang::NUMBER_GROUP_NUM, &end, 10);
	assert(*end == 0);

	double money = showCents ? 0.01 * cents : roundf(0.01 * cents);

	const char *format = (money < 0) ? "-$%.2f" : "$%.2f";
	char buf[64];
	snprintf(buf, sizeof(buf), format, fabs(money));
	std::string result(buf);

	size_t pos = result.find_first_of('.'); // pos to decimal point

	if (showCents) // replace decimal point
		result.replace(pos, 1, Lang::NUMBER_DECIMAL_POINT);
	else // or just remove frac. part
		result.erase(result.begin() + pos, result.end());

	size_t groupMin = strtol(Lang::NUMBER_GROUP_MIN, &end, 10);
	assert(*end == 0);

	if (groupDigits != 0 && fabs(money) >= groupMin) {

		std::string groupSep = std::string(Lang::NUMBER_GROUP_SEP) == " " ?
			"\u00a0" :
			Lang::NUMBER_GROUP_SEP; // space should be fixed space

		size_t skip = (money < 0) ? 2 : 1; // compensate for "$" or "-$"
		while (pos - skip > groupDigits) { // insert thousand seperator
			pos = pos - groupDigits;
			result.insert(pos, groupSep);
		}
	}
	return result;
}

static const char *const MONTH_NAMES[] = {
	Lang::MONTH_JAN,
	Lang::MONTH_FEB,
	Lang::MONTH_MAR,
	Lang::MONTH_APR,
	Lang::MONTH_MAY,
	Lang::MONTH_JUN,
	Lang::MONTH_JUL,
	Lang::MONTH_AUG,
	Lang::MONTH_SEP,
	Lang::MONTH_OCT,
	Lang::MONTH_NOV,
	Lang::MONTH_DEC
};

std::string format_date(double t)
{
	const Time::DateTime dt(t);
	int year, month, day, hour, minute, second;
	dt.GetDateParts(&year, &month, &day);
	dt.GetTimeParts(&hour, &minute, &second);

	char buf[32];
	snprintf(buf, sizeof(buf), "%02d:%02d:%02d %d %s %d",
		hour, minute, second, day, MONTH_NAMES[month - 1], year);
	return buf;
}

std::string format_date_only(double t)
{
	const Time::DateTime dt(t);
	int year, month, day;
	dt.GetDateParts(&year, &month, &day);

	char buf[16];
	snprintf(buf, sizeof(buf), "%d %s %d", day, MONTH_NAMES[month - 1], year);
	return buf;
}

std::string string_join(std::vector<std::string> &v, std::string sep)
{
	std::vector<std::string>::iterator i = v.begin();
	std::string out;

	while (i != v.end()) {
		out += *i;
		++i;
		if (i != v.end()) out += sep;
	}
	return out;
}

std::string format_duration(double seconds)
{
	std::ostringstream ss;
	int duration = seconds;
	int secs = duration % 60;
	int minutes = (duration / 60) % 60;
	int hours = (duration / 60 / 60) % 24;
	int days = (duration / 60 / 60 / 24) % 7;
	int weeks = (duration / 60 / 60 / 24 / 7);
	if (weeks != 0)
		ss << weeks << Lang::UNIT_WEEKS;
	if (days != 0)
		ss << days << Lang::UNIT_DAYS;
	if (hours != 0)
		ss << hours << Lang::UNIT_HOURS;
	if (minutes != 0)
		ss << minutes << Lang::UNIT_MINUTES;
	// do not show seconds unless the largest unit shown is minutes
	if (weeks == 0 && days == 0 && hours == 0)
		if (minutes == 0 || secs != 0)
			ss << secs << Lang::UNIT_SECONDS;
	return ss.str();
}

std::string format_distance(double dist, int precision)
{
	std::ostringstream ss;
	ss.setf(std::ios::fixed, std::ios::floatfield);
	if (dist < 1e3) {
		ss.precision(0);
		ss << dist << " m";
	} else {
		const float LY = 9.4607e15f;
		ss.precision(precision);

		if (dist < 1e6)
			ss << (dist * 1e-3) << " km";
		else if (dist < AU * 0.01)
			ss << (dist * 1e-6) << " Mm";
		else if (dist < LY * 0.1)
			ss << (dist / AU) << " " << Lang::UNIT_AU;
		else
			ss << (dist / LY) << " " << Lang::UNIT_LY;
	}
	return ss.str();
}

// strcasestr() adapted from gnulib
// (c) 2005 FSF. GPL2+

#define TOLOWER(c) (isupper(static_cast<unsigned char>(c)) ? tolower(static_cast<unsigned char>(c)) : (static_cast<unsigned char>(c)))

const char *pi_strcasestr(const char *haystack, const char *needle)
{
	if (!*needle)
		return haystack;

	// cache the first character for speed
	char b = TOLOWER(*needle);

	needle++;
	for (;; haystack++) {
		if (!*haystack)
			return 0;

		if (TOLOWER(*haystack) == b) {
			const char *rhaystack = haystack + 1;
			const char *rneedle = needle;

			for (;; rhaystack++, rneedle++) {
				if (!*rneedle)
					return haystack;

				if (!*rhaystack)
					return 0;

				if (TOLOWER(*rhaystack) != TOLOWER(*rneedle))
					break;
			}
		}
	}
}

std::vector<std::string> SplitString(const std::string &source, const std::string &delim)
{
	bool stringSplitted = false;
	std::vector<std::string> splitted;

	size_t startPos = 0;
	do {
		// try to find delim
		size_t delimPos = source.find(delim, startPos);

		// if delim found
		if (delimPos != std::string::npos) {
			std::string element = source.substr(startPos, delimPos);
			splitted.push_back(element);

			// prepare next loop
			startPos = delimPos + delim.length();
		} else {
			// push tail and exit
			splitted.push_back(source.substr(startPos));
			stringSplitted = true;
		}

	} while (!stringSplitted);

	return splitted;
}

//#define USE_HEX_FLOATS
#ifndef USE_HEX_FLOATS
union fu32 {
	fu32() {}
	fu32(float fIn) :
		f(fIn) {}
	fu32(uint32_t uIn) :
		u(uIn) {}
	float f;
	uint32_t u;
};
union fu64 {
	fu64() {}
	fu64(double dIn) :
		d(dIn) {}
	fu64(uint64_t uIn) :
		u(uIn) {}
	double d;
	uint64_t u;
};
#endif // USE_HEX_FLOATS

std::string FloatToStr(float val)
{
	PROFILE_SCOPED()
#ifdef USE_HEX_FLOATS
	char hex[32]; // Probably don't need such a large char array.
	std::sprintf(hex, "%a", val);
	return hex;
#else
	// Exact representation (but not human readable).
	static_assert(sizeof(float) == 4, "float isn't 4bytes");
	fu32 uval(val);
	char str[64];
	SDL_itoa(uval.u, str, 10);
	return str;
#endif
}

std::string DoubleToStr(double val)
{
	PROFILE_SCOPED()
#ifdef USE_HEX_FLOATS
	char hex[64]; // Probably don't need such a large char array.
	std::sprintf(hex, "%la", val);
	return hex;
#else
	// Exact representation (but not human readable).
	static_assert(sizeof(double) == 8, "double isn't 8 bytes");
	fu64 uval(val);
	char str[128];
	SDL_ulltoa(uval.u, str, 10);
	return str;
#endif
}

void Vector3fToStr(const vector3f &val, char *out, size_t size)
{
	PROFILE_SCOPED()
	static_assert(sizeof(vector3f) == 12, "vector3f isn't 12 bytes");
#ifdef USE_HEX_FLOATS
	const int amt = std::sprintf(out, "%a,%a,%a", val.x, val.y, val.z);
	assert(static_cast<size_t>(amt) <= size);
	(void)amt;
#else
	fu32 a(val.x);
	fu32 b(val.y);
	fu32 c(val.z);
	const int amt = sprintf(out, "(%" PRIu32 ",%" PRIu32 ",%" PRIu32 ")", a.u, b.u, c.u);
	assert(static_cast<size_t>(amt) <= size);
	(void)amt;
#endif
}

void Vector3dToStr(const vector3d &val, char *out, size_t size)
{
	PROFILE_SCOPED()
	static_assert(sizeof(vector3d) == 24, "vector3d isn't 24 bytes");
#ifdef USE_HEX_FLOATS
	const int amt = std::sprintf(out, "%la,%la,%la", val.x, val.y, val.z);
	assert(static_cast<size_t>(amt) <= size);
	(void)amt;
#else
	fu64 a(val.x);
	fu64 b(val.y);
	fu64 c(val.z);
	const int amt = sprintf(out, "(%" PRIu64 ",%" PRIu64 ",%" PRIu64 ")", a.u, b.u, c.u);
	assert(static_cast<size_t>(amt) <= size);
	(void)amt;
#endif
}

void Matrix3x3fToStr(const matrix3x3f &val, char *out, size_t size)
{
	PROFILE_SCOPED()
	static_assert(sizeof(matrix3x3f) == 36, "matrix3x3f isn't 36 bytes");
#ifdef USE_HEX_FLOATS
	const int amt = std::sprintf(out, "%a,%a,%a,%a,%a,%a,%a,%a,%a", val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7], val[8]);
	assert(static_cast<size_t>(amt) <= size);
	(void)amt;
#else
	fu32 fuvals[9];
	for (int i = 0; i < 9; i++)
		fuvals[i].f = val[i];
	const int amt = sprintf(out,
		"(%" PRIu32 ",%" PRIu32 ",%" PRIu32
		",%" PRIu32 ",%" PRIu32 ",%" PRIu32
		",%" PRIu32 ",%" PRIu32 ",%" PRIu32 ")",
		fuvals[0].u, fuvals[1].u, fuvals[2].u,
		fuvals[3].u, fuvals[4].u, fuvals[5].u,
		fuvals[6].u, fuvals[7].u, fuvals[8].u);
	assert(static_cast<size_t>(amt) <= size);
	(void)amt;
#endif
}

void Matrix3x3dToStr(const matrix3x3d &val, char *out, size_t size)
{
	PROFILE_SCOPED()
	static_assert(sizeof(matrix3x3d) == 72, "matrix3x3d isn't 72 bytes");
#ifdef USE_HEX_FLOATS
	const int amt = std::sprintf(out, "%a,%a,%a,%a,%a,%a,%a,%a,%a", val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7], val[8]);
	assert(static_cast<size_t>(amt) <= size);
	(void)amt;
#else
	fu64 fuvals[9];
	for (int i = 0; i < 9; i++)
		fuvals[i].d = val[i];
	const int amt = sprintf(out,
		"(%" PRIu64 ",%" PRIu64 ",%" PRIu64
		",%" PRIu64 ",%" PRIu64 ",%" PRIu64
		",%" PRIu64 ",%" PRIu64 ",%" PRIu64 ")",
		fuvals[0].u, fuvals[1].u, fuvals[2].u,
		fuvals[3].u, fuvals[4].u, fuvals[5].u,
		fuvals[6].u, fuvals[7].u, fuvals[8].u);
	assert(static_cast<size_t>(amt) <= size);
	(void)amt;
#endif
}

void Matrix4x4fToStr(const matrix4x4f &val, char *out, size_t size)
{
	PROFILE_SCOPED()
	static_assert(sizeof(matrix4x4f) == 64, "matrix4x4f isn't 64 bytes");
#ifdef USE_HEX_FLOATS
	const int amt = std::sprintf(out, "%a,%a,%a,%a,%a,%a,%a,%a,%a,%a,%a,%a,%a,%a,%a,%a", val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7], val[8], val[9], val[10], val[11], val[12], val[13], val[14], val[15]);
	assert(static_cast<size_t>(amt) <= size);
	(void)amt;
#else
	fu32 fuvals[16];
	for (int i = 0; i < 16; i++)
		fuvals[i].f = val[i];
	const int amt = sprintf(out,
		"(%" PRIu32 ",%" PRIu32 ",%" PRIu32 ",%" PRIu32
		",%" PRIu32 ",%" PRIu32 ",%" PRIu32 ",%" PRIu32
		",%" PRIu32 ",%" PRIu32 ",%" PRIu32 ",%" PRIu32
		",%" PRIu32 ",%" PRIu32 ",%" PRIu32 ",%" PRIu32 ")",
		fuvals[0].u, fuvals[1].u, fuvals[2].u, fuvals[3].u,
		fuvals[4].u, fuvals[5].u, fuvals[6].u, fuvals[7].u,
		fuvals[8].u, fuvals[9].u, fuvals[10].u, fuvals[11].u,
		fuvals[12].u, fuvals[13].u, fuvals[14].u, fuvals[15].u);
	assert(static_cast<size_t>(amt) <= size);
	(void)amt;
#endif
}

void Matrix4x4dToStr(const matrix4x4d &val, char *out, size_t size)
{
	PROFILE_SCOPED()
	static_assert(sizeof(matrix4x4d) == 128, "matrix4x4d isn't 128 bytes");
#ifdef USE_HEX_FLOATS
	const int amt = std::sprintf(out, "%a,%a,%a,%a,%a,%a,%a,%a,%a,%a,%a,%a,%a,%a,%a,%a", val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7], val[8], val[9], val[10], val[11], val[12], val[13], val[14], val[15]);
	assert(static_cast<size_t>(amt) <= size);
	(void)amt;
#else
	fu64 fuvals[16];
	for (int i = 0; i < 16; i++)
		fuvals[i].d = val[i];
	const int amt = sprintf(out,
		"(%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64
		",%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64
		",%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64
		",%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64 ")",
		fuvals[0].u, fuvals[1].u, fuvals[2].u, fuvals[3].u,
		fuvals[4].u, fuvals[5].u, fuvals[6].u, fuvals[7].u,
		fuvals[8].u, fuvals[9].u, fuvals[10].u, fuvals[11].u,
		fuvals[12].u, fuvals[13].u, fuvals[14].u, fuvals[15].u);
	assert(static_cast<size_t>(amt) <= size);
	(void)amt;
#endif
}

std::string AutoToStr(Sint32 val)
{
	char str[64];
	sprintf(str, "%" PRId32, val);
	return str;
}

std::string AutoToStr(Sint64 val)
{
	char str[128];
	sprintf(str, "%" PRId64, val);
	return str;
}

std::string AutoToStr(float val)
{
	return FloatToStr(val);
}

std::string AutoToStr(double val)
{
	return DoubleToStr(val);
}

Sint64 StrToSInt64(const std::string &str)
{
	Sint64 val;
	sscanf(str.c_str(), "%" SCNd64, &val);
	return val;
}

Uint64 StrToUInt64(const std::string &str)
{
	Uint64 val;
	sscanf(str.c_str(), "%" SCNu64, &val);
	return val;
}

float StrToFloat(const std::string &str)
{
	PROFILE_SCOPED()
#ifdef USE_HEX_FLOATS
	float val;
	std::sscanf(str.c_str(), "%a", &val);
	return val;
#else
	// Exact representation (but not human readable).
	static_assert(sizeof(float) == 4, "float isn't 4 bytes");
	fu32 uval;
	const int amt = sscanf(str.c_str(), "%" SCNu32, &uval.u);
	assert(amt == 1);
	(void)amt;
	return uval.f;
#endif
}

double StrToDouble(const std::string &str)
{
	PROFILE_SCOPED()
#ifdef USE_HEX_FLOATS
	double val;
	std::sscanf(str.c_str(), "%la", &val);
	return val;
#else
	// Exact representation (but not human readable).
	static_assert(sizeof(double) == 8, "double isn't 8 bytes");
	static_assert(sizeof(long long) == sizeof(uint64_t), "long long isn't equal in size to uint64_t");
	fu64 uval;
	const int amt = sscanf(str.c_str(), "%" SCNu64, &uval.u);
	(void)amt;
	assert(amt == 1);
	return uval.d;
#endif
}

void StrToAuto(Sint32 *pVal, const std::string &str)
{
	sscanf(str.c_str(), "%" SCNd32, pVal);
}

void StrToAuto(Sint64 *pVal, const std::string &str)
{
	sscanf(str.c_str(), "%" SCNd64, pVal);
}

void StrToAuto(float *pVal, const std::string &str)
{
	*pVal = StrToFloat(str);
}

void StrToAuto(double *pVal, const std::string &str)
{
	*pVal = StrToDouble(str);
}

void StrToVector3f(const char *str, vector3f &val)
{
	PROFILE_SCOPED()
#ifdef USE_HEX_FLOATS
	const int amt = std::sscanf(str, "%a,%a,%a", &val.x, &val.y, &val.z);
	assert(amt == 3);
	(void)amt;
#else
	fu32 a, b, c;
	const int amt = std::sscanf(str, "(%" SCNu32 ",%" SCNu32 ",%" SCNu32 ")", &a.u, &b.u, &c.u);
	assert(amt == 3);
	(void)amt;
	val.x = a.f;
	val.y = b.f;
	val.z = c.f;
#endif
}

void StrToVector3d(const char *str, vector3d &val)
{
	PROFILE_SCOPED()
#ifdef USE_HEX_FLOATS
	const int amt = std::sscanf(str, "%la,%la,%la", &val.x, &val.y, &val.z);
	assert(amt == 3);
	(void)amt;
#else
	fu64 a, b, c;
	const int amt = std::sscanf(str, "(%" SCNu64 ",%" SCNu64 ",%" SCNu64 ")", &a.u, &b.u, &c.u);
	assert(amt == 3);
	(void)amt;
	val.x = a.d;
	val.y = b.d;
	val.z = c.d;
#endif
}

void StrToMatrix3x3f(const char *str, matrix3x3f &val)
{
	PROFILE_SCOPED()
#ifdef USE_HEX_FLOATS
	const int amt = std::sscanf(str, "%a,%a,%a,%a,%a,%a,%a,%a,%a", &val[0], &val[1], &val[2], &val[3], &val[4], &val[5], &val[6], &val[7], &val[8]);
	assert(amt == 9);
	(void)amt;
#else
	fu32 fu[9];
	const int amt = std::sscanf(str, "(%" SCNu32 ",%" SCNu32 ",%" SCNu32 ",%" SCNu32 ",%" SCNu32 ",%" SCNu32 ",%" SCNu32 ",%" SCNu32 ",%" SCNu32 ")",
		&fu[0].u, &fu[1].u, &fu[2].u,
		&fu[3].u, &fu[4].u, &fu[5].u,
		&fu[6].u, &fu[7].u, &fu[8].u);
	assert(amt == 9);
	(void)amt;
	for (int i = 0; i < 9; i++)
		val[i] = fu[i].f;
#endif
}

void StrToMatrix3x3d(const char *str, matrix3x3d &val)
{
	PROFILE_SCOPED()
#ifdef USE_HEX_FLOATS
	const int amt = std::sscanf(str, "%la,%la,%la,%la,%la,%la,%la,%la,%la", &val[0], &val[1], &val[2], &val[3], &val[4], &val[5], &val[6], &val[7], &val[8]);
	assert(amt == 9);
	(void)amt;
#else
	fu64 fu[9];
	const int amt = std::sscanf(str, "(%" SCNu64 ",%" SCNu64 ",%" SCNu64 ",%" SCNu64 ",%" SCNu64 ",%" SCNu64 ",%" SCNu64 ",%" SCNu64 ",%" SCNu64 ")",
		&fu[0].u, &fu[1].u, &fu[2].u,
		&fu[3].u, &fu[4].u, &fu[5].u,
		&fu[6].u, &fu[7].u, &fu[8].u);
	assert(amt == 9);
	(void)amt;
	for (int i = 0; i < 9; i++)
		val[i] = fu[i].d;
#endif
}

void StrToMatrix4x4f(const char *str, matrix4x4f &val)
{
	PROFILE_SCOPED()
#ifdef USE_HEX_FLOATS
	const int amt = std::sscanf(str, "%a,%a,%a,%a,%a,%a,%a,%a,%a,%a,%a,%a,%a,%a,%a,%a", &val[0], &val[1], &val[2], &val[3], &val[4], &val[5], &val[6], &val[7], &val[8], &val[9], &val[10], &val[11], &val[12], &val[13], &val[14], &val[15]);
	assert(amt == 16);
#else
	fu32 fu[16];
	const int amt = std::sscanf(str, "(%" SCNu32 ",%" SCNu32 ",%" SCNu32 ",%" SCNu32 ",%" SCNu32 ",%" SCNu32 ",%" SCNu32 ",%" SCNu32 ",%" SCNu32 ",%" SCNu32 ",%" SCNu32 ",%" SCNu32 ",%" SCNu32 ",%" SCNu32 ",%" SCNu32 ",%" SCNu32 ")",
		&fu[0].u, &fu[1].u, &fu[2].u, &fu[3].u,
		&fu[4].u, &fu[5].u, &fu[6].u, &fu[7].u,
		&fu[8].u, &fu[9].u, &fu[10].u, &fu[11].u,
		&fu[12].u, &fu[13].u, &fu[14].u, &fu[15].u);
	assert(amt == 16);
	(void)amt;
	for (int i = 0; i < 16; i++)
		val[i] = fu[i].f;
#endif
}

void StrToMatrix4x4d(const char *str, matrix4x4d &val)
{
	PROFILE_SCOPED()
#ifdef USE_HEX_FLOATS
	const int amt = std::sscanf(str, "%la,%la,%la,%la,%la,%la,%la,%la,%la,%la,%la,%la,%la,%la,%la,%la", &val[0], &val[1], &val[2], &val[3], &val[4], &val[5], &val[6], &val[7], &val[8], &val[9], &val[10], &val[11], &val[12], &val[13], &val[14], &val[15]);
	assert(amt == 16);
#else
	fu64 fu[16];
	const int amt = std::sscanf(str, "(%" SCNu64 ",%" SCNu64 ",%" SCNu64 ",%" SCNu64 ",%" SCNu64 ",%" SCNu64 ",%" SCNu64 ",%" SCNu64 ",%" SCNu64 ",%" SCNu64 ",%" SCNu64 ",%" SCNu64 ",%" SCNu64 ",%" SCNu64 ",%" SCNu64 ",%" SCNu64 ")",
		&fu[0].u, &fu[1].u, &fu[2].u, &fu[3].u,
		&fu[4].u, &fu[5].u, &fu[6].u, &fu[7].u,
		&fu[8].u, &fu[9].u, &fu[10].u, &fu[11].u,
		&fu[12].u, &fu[13].u, &fu[14].u, &fu[15].u);
	assert(amt == 16);
	(void)amt;
	for (int i = 0; i < 16; i++)
		val[i] = fu[i].d;
#endif
}

/**
    Converts geographic coordinates from decimal to degree/minutes/seconds format
    and returns a string.
*/
std::string DecimalToDegMinSec(float dec)
{
	int degrees = dec;
	int minutes = 60 * (dec - degrees);
	int seconds = 3600 * ((dec - degrees) - static_cast<float>(minutes) / 60);
	std::string str = stringf("%0° %1' %2\"", degrees, std::abs(minutes), std::abs(seconds));
	return str;
}

static const int HEXDUMP_CHUNK = 16;
void hexdump(const unsigned char *buf, int len)
{
	int count;

	for (int i = 0; i < len; i += HEXDUMP_CHUNK) {
		Output("0x%06x  ", i);

		count = ((len - i) > HEXDUMP_CHUNK ? HEXDUMP_CHUNK : len - i);

		for (int j = 0; j < count; j++) {
			if (j == HEXDUMP_CHUNK / 2) Output(" ");
			Output("%02x ", buf[i + j]);
		}

		for (int j = count; j < HEXDUMP_CHUNK; j++) {
			if (j == HEXDUMP_CHUNK / 2) Output(" ");
			Output("   ");
		}

		Output(" ");

		for (int j = 0; j < count; j++)
			Output("%c", isprint(buf[i + j]) ? buf[i + j] : '.');

		Output("\n");
	}
}
