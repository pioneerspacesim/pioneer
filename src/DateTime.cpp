// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "DateTime.h"

#include <cassert>
#include <cstdint>
#include <tuple>
#include <utility>

static char month_days[2][12] = {
	{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
	{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

static bool is_leap_year(int year)
{
	return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

enum {
	// we're using signed 64-bit integers for timestamps
	// divide by 366 (let's just pretend every year is a leap year!)
	// subtract 5000 years for a safety margin, and because timestamp 0 is not at year 0
	APPROX_MAX_YEAR = (INT64_MAX / (366 * Time::Day)) - 5000
};

// C99 and C++11 specify that division truncates toward zero,
// this apparently came from fortran, and is *really unhelpful*,
//
// For a very useful paper on this, see:
//   Division and Modulus for Computer Scientists --- Daan Leijen
//
// Also see Guido van Rossum's explanation of why he chose different
// behaviour for Python 3's integer division operator (//):
//   http://python-history.blogspot.co.uk/2010/08/why-pythons-integer-division-floors.html
//
// divmod_euclid provides a division operation that always gives a non-negative remainder
template <typename T>
static std::pair<T, T> divmod_euclid(const T dividend, const T divisor)
{
	T quot = dividend / divisor, rem = dividend % divisor;
	if (rem < 0) {
		if (divisor > 0) {
			quot -= 1;
			rem += divisor;
		} else {
			quot += 1;
			rem -= divisor;
		}
	}
	return std::make_pair(quot, rem);
}

template <typename T>
static std::pair<T, T> divmod_trunc(const T a, const T b)
{
	return std::make_pair(a / b, a % b);
}

Time::DateTime::DateTime(int year, int month, int day, int hour, int minute, int second, int microsecond)
{
	// minimum year is just a sanity check; the code should work for earlier years
	assert(year >= 1600 && year <= APPROX_MAX_YEAR);
	assert(month >= 1 && month <= 12);
	assert(day >= 1 && day <= month_days[is_leap_year(year)][month - 1]);
	assert(hour >= 0 && hour < 24);
	assert(minute >= 0 && minute < 60);
	assert(second >= 0 && second < 60);
	assert(microsecond >= 0 && microsecond < 1000000);

	const int yoffset = (year - 2001);
	// C99 and C++11 specify that integer division truncates toward zero
	const int nleap = (yoffset >= 0) ? (yoffset / 400 - yoffset / 100 + yoffset / 4) : ((yoffset - 399) / 400 - (yoffset - 99) / 100 + (yoffset - 3) / 4);

	int days = 365 * yoffset + nleap;

	// month offset
	const bool leap = is_leap_year(year);
	for (int i = 1; i < month; ++i) {
		days += month_days[leap][i - 1];
	}

	// day offset
	days += (day - 1);

	// final timestamp
	m_timestamp = days * Time::Day + hour * Time::Hour + minute * Time::Minute + second * Time::Second + microsecond * Time::Microsecond;
}

Time::DateTime::DateTime(double gameTime) :
	DateTime(3200, 1, 1, 0, 0, 0)
{
	*this += Time::TimeDelta(gameTime, Time::Second);
}

void Time::DateTime::GetDateParts(int *out_year, int *out_month, int *out_day) const
{
	if (out_year || out_month || out_day) {
		static_assert(Time::Day > (int64_t(1) << 32),
			"code below assumes that the 'date' part of a 64-bit timestamp fits in 32 bits");

		// number of days from the epoch to the beginning of the stored date
		int days = divmod_euclid(m_timestamp, int64_t(Time::Day)).first;

		// work out how many completed cycles, centuries, 'quads' and years we've measured since the epoch
		// computed such that n400 may be negative, but all other values must be positive

		int n400, n100, n4, n1;
		std::tie(n400, days) = divmod_euclid(days, 365 * 400 + 97);
		// days must be non-negative after this, so we can use truncating division from here
		std::tie(n100, days) = divmod_trunc(days, 365 * 100 + 24);
		std::tie(n4, days) = divmod_trunc(days, 365 * 4 + 1);
		std::tie(n1, days) = divmod_trunc(days, 365);

		int year = 2001 + 400 * n400 + 100 * n100 + 4 * n4 + n1;
		int day = days;

		// the last day in a 400-year or a 4-year cycle are handled incorrectly,
		// because n100 is calculated assuming each century has 24 (not 25) leaps,
		// and n1 is calculated assuming each year has 365 (not 366) days
		// adjust for those mistakes here
		if (n100 == 4 || n1 == 4) {
			assert(!((n100 == 4) && (n1 == 4)));
			--year;
			day += 365;
			assert(is_leap_year(year));
		}

		bool leap = is_leap_year(year);

		int month = 0;
		while (day >= month_days[leap][month]) {
			day -= month_days[leap][month++];
			assert(month < 12);
		}

		if (out_year) {
			*out_year = year;
		}
		if (out_month) {
			*out_month = month + 1;
		}
		if (out_day) {
			*out_day = day + 1;
		}
	}
}

void Time::DateTime::GetTimeParts(int *out_hour, int *out_minute, int *out_second, int *out_microsecond) const
{
	if (out_hour || out_minute || out_second || out_microsecond) {
		const int64_t tstamp = divmod_euclid(m_timestamp, int64_t(Time::Day)).second;
		assert(tstamp >= 0);

		if (out_microsecond) {
			*out_microsecond = (tstamp / Time::Microsecond) % 1000000;
		}

		const int seconds = (tstamp / Time::Second);
		assert(seconds >= 0 && seconds < 24 * 60 * 60);

		if (out_hour) {
			*out_hour = (seconds / 3600);
		}
		if (out_minute) {
			*out_minute = (seconds / 60) % 60;
		}
		if (out_second) {
			*out_second = (seconds / 1) % 60;
		}
	}
}

double Time::DateTime::ToGameTime() const
{
	const Time::DateTime base(3200, 1, 1, 0, 0, 0);
	Time::TimeDelta tstamp = (*this - base);
	if (*this < base) {
		// adjustment to give correct rounding for GetTotalSeconds()
		tstamp -= Time::TimeDelta(Time::Second - 1, Time::TimeUnit(1));
	}
	return double(tstamp.GetTotalSeconds());
}

std::string Time::DateTime::ToDateString() const
{
	char buf[32];
	int year, month, day;
	GetDateParts(&year, &month, &day);
	snprintf(buf, sizeof(buf), "%04d-%02d-%02d", year, month, day);
	return std::string(buf);
}

std::string Time::DateTime::ToTimeString() const
{
	char buf[16];
	int hour, minute, second, microsecond;
	GetTimeParts(&hour, &minute, &second, &microsecond);
	snprintf(buf, sizeof(buf), "%02d:%02d:%02d.%06d", hour, minute, second, microsecond);
	return std::string(buf);
}

std::string Time::DateTime::ToStringISO8601() const
{
	char buf[64];
	int year, month, day, hour, minute, second;
	GetDateParts(&year, &month, &day);
	GetTimeParts(&hour, &minute, &second);
	snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02dZ", year, month, day, hour, minute, second);
	return std::string(buf);
}
