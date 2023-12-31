// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "StringUtils.h"

#include "Lang.h"
#include "DateTime.h"
#include "fmt/core.h"
#include "gameconsts.h"

#include <cinttypes>
#include <sstream>
#include <cassert>

std::string format_money(double cents, bool showCents)
{
	char *end; // for  error checking
	size_t groupDigits = strtol(Lang::NUMBER_GROUP_NUM, &end, 10);
	assert(*end == 0);

	double money = showCents ? 0.01 * cents : round(0.01 * cents);

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

size_t SplitString::step(std::string_view str)
{
	return m_reverse ? str.find_last_of(m_delim) : str.find_first_of(m_delim);
}

void SplitString::trim(std::string_view &str, size_t next)
{
	if (next == std::string_view::npos) {
		str = {};
		return;
	}

	if (m_reverse)
		str.remove_suffix((str.size() + 1) - str.find_last_not_of(m_delim, next));
	else
		str.remove_prefix(str.find_first_not_of(m_delim, next));
}

std::string FloatToStr(float val)
{
#ifdef USE_HEX_FLOATS
	char hex[32]; // Probably don't need such a large char array.
	std::sprintf(hex, "%a", val);
	return hex;
#else
	// Exact representation (but not human readable).
	static_assert(sizeof(float) == 4, "float isn't 4bytes");
	char str[64];
	SDL_itoa(*reinterpret_cast<uint32_t *>(&val), str, 10);
	return str;
#endif
}

std::string DoubleToStr(double val)
{
#ifdef USE_HEX_FLOATS
	char hex[64]; // Probably don't need such a large char array.
	std::sprintf(hex, "%la", val);
	return hex;
#else
	// Exact representation (but not human readable).
	static_assert(sizeof(double) == 8, "double isn't 8 bytes");
	char str[128];
	SDL_ulltoa(*reinterpret_cast<uint64_t *>(&val), str, 10);
	return str;
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
#ifdef USE_HEX_FLOATS
	float val;
	std::sscanf(str.c_str(), "%a", &val);
	return val;
#else
	// Exact representation (but not human readable).
	static_assert(sizeof(float) == 4, "float isn't 4 bytes");
	float out;
	const int amt = sscanf(str.c_str(), "%" SCNu32, reinterpret_cast<uint32_t *>(&out));
	assert(amt == 1);
	(void)amt;
	return out;
#endif
}

double StrToDouble(const std::string &str)
{
#ifdef USE_HEX_FLOATS
	double val;
	std::sscanf(str.c_str(), "%la", &val);
	return val;
#else
	// Exact representation (but not human readable).
	static_assert(sizeof(double) == 8, "double isn't 8 bytes");
	static_assert(sizeof(long long) == sizeof(uint64_t), "long long isn't equal in size to uint64_t");
	uint64_t out;
	const int amt = sscanf(str.c_str(), "%" SCNu64, reinterpret_cast<uint64_t *>(&out));
	(void)amt;
	assert(amt == 1);
	return out;
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

/**
    Converts geographic coordinates from decimal to degree/minutes/seconds format
    and returns a string.
*/
std::string DecimalToDegMinSec(float dec)
{
	int degrees = dec;
	int minutes = 60 * (dec - degrees);
	int seconds = 3600 * ((dec - degrees) - static_cast<float>(minutes) / 60);
	std::string str = fmt::format("{}° {}' {}\"", degrees, std::abs(minutes), std::abs(seconds));
	return str;
}
