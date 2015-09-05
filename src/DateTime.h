#ifndef DATETIME_H
#define DATETIME_H

#include <SDL_stdinc.h>
#include <string>

namespace Time {

	// We separate lengths of time (TimeDelta) from points in time (DateTime),
	// because they have different operations defined on them
	//
	// The TimeDelta constructor takes a TimeUnit parameter, so you specify
	// a length of time with, e.g., Time::TimeDelta(42, Time::Day)
	//
	// TimeUnit is an enum (rather than a set of const TimeDelta values) because
	// that ensures the compiler will treat the values as compile time constants
	// instead of objects that must be defined in a translation unit and initialised.
	//
	// The base units are (currently) microseconds, which is chosen
	// because it's plenty high enough resolution for most uses, and it's
	// easily understandable, and it gives good range when stored in
	// signed 64-bit integers (approx. +/- 292277 years)
	//
	// The base units can be trivially changed by modifying the TimeUnit enum.
	//
	// DateTime is mostly about dealing with the Gregorian calendar.
	//
	// These types have a fairly unsophisticated understanding of the calendar
	// and the passage of time.  For example, leap seconds are not supported at all.
	// But... I'm pretty sure we don't need leap seconds for Pioneer.

enum TimeUnit : Sint64 {
	Microsecond = 1ll,
	Millisecond = 1000ll * Microsecond,
	Second = 1000ll * Millisecond,
	Minute = 60ll * Second,
	Hour = 60ll * Minute,
	Day = 24ll * Hour,
	Week = 7ll * Day
};

class TimeDelta;
class DateTime;

class TimeDelta {
	public:
		TimeDelta(): m_delta(0) {}
		explicit TimeDelta(Sint64 t, TimeUnit unit=Second): m_delta(t*unit) {}

		Sint64 GetTotalWeeks() const { return (m_delta / Week); }
		Sint64 GetTotalDays() const { return (m_delta / Day); }
		Sint64 GetTotalHours() const { return (m_delta / Hour); }
		Sint64 GetTotalMinutes() const { return (m_delta / Minute); }
		Sint64 GetTotalSeconds() const { return (m_delta / Second); }
		Sint64 GetTotalMilliseconds() const { return (m_delta / Millisecond); }
		Sint64 GetTotalMicroseconds() const { return (m_delta / Microsecond); }

		TimeDelta &operator+=(const TimeDelta &x) { m_delta += x.m_delta; return *this; }
		TimeDelta &operator-=(const TimeDelta &x) { m_delta -= x.m_delta; return *this; }

		friend TimeDelta operator+(const TimeDelta &a, const TimeDelta &b) { return TimeDelta(a.m_delta + b.m_delta, TimeUnit(1)); }
		friend TimeDelta operator-(const TimeDelta &a, const TimeDelta &b) { return TimeDelta(a.m_delta - b.m_delta, TimeUnit(1)); }
		friend TimeDelta operator*(Sint64 x, const TimeDelta &t) { return TimeDelta(x * t.m_delta, TimeUnit(1)); }
		friend TimeDelta operator/(const TimeDelta &t, Sint64 x) { return TimeDelta(t.m_delta / x, TimeUnit(1)); }
		friend Sint64 operator/(const TimeDelta &a, const TimeDelta &b) { return (a.m_delta / b.m_delta); }

		friend DateTime operator+(const DateTime &a, const TimeDelta &b);
		friend DateTime operator-(const DateTime &a, const TimeDelta &b);
	private:
		friend class DateTime;
		Sint64 m_delta;
};

class DateTime {
	public:
		DateTime(): m_timestamp(-Sint64(24*60*60) * Sint64(400*365 + 97) * Sint64(Second)) {}
		// month = 1 to 12
		// day = 1 to N where N is the number of days in the specified month and year
		DateTime(int year, int month, int day, int hour=0, int minute=0, int second=0, int microsecond=0);
		DateTime(double gameTime);

		void GetDateParts(int *year, int *month, int *day) const;
		void GetTimeParts(int *hour, int *minute, int *second, int *microsecond = nullptr) const;

		double ToGameTime() const;
		std::string ToDateString() const;
		std::string ToTimeString() const;
		std::string ToDateTimeString() const { return ToStringISO8601(); }
		std::string ToStringISO8601() const;

		friend TimeDelta operator-(const DateTime &a, const DateTime &b)
		{ return TimeDelta(a.m_timestamp - b.m_timestamp, TimeUnit(1)); }
		friend DateTime operator+(const DateTime &a, const TimeDelta &b)
		{ return DateTime(a.m_timestamp + b.m_delta); }
		friend DateTime operator-(const DateTime &a, const TimeDelta &b)
		{ return DateTime(a.m_timestamp - b.m_delta); }
		friend DateTime operator+(const TimeDelta &a, const DateTime &b) { return (b + a); }

		DateTime &operator+=(const TimeDelta &x) { m_timestamp += x.m_delta; return *this; }
		DateTime &operator-=(const TimeDelta &x) { m_timestamp -= x.m_delta; return *this; }

		friend bool operator<(const DateTime &a, const DateTime &b) { return (a.m_timestamp < b.m_timestamp); }
		friend bool operator<=(const DateTime &a, const DateTime &b) { return (a.m_timestamp <= b.m_timestamp); }
		friend bool operator==(const DateTime &a, const DateTime &b) { return (a.m_timestamp == b.m_timestamp); }
		friend bool operator!=(const DateTime &a, const DateTime &b) { return (a.m_timestamp != b.m_timestamp); }
		friend bool operator>(const DateTime &a, const DateTime &b) { return (a.m_timestamp > b.m_timestamp); }
		friend bool operator>=(const DateTime &a, const DateTime &b) { return (a.m_timestamp >= b.m_timestamp); }

		Sint64 GetTimestamp() const { return m_timestamp; }

	private:
		explicit DateTime(Sint64 tstamp): m_timestamp(tstamp) {}

		// The timestamp is the number of microseconds since the epoch (2001-01-01T00:00:00Z)
		//
		// This epoch (the start of the year 2001) is chosen because it makes counting
		// leap years easier (see DateTime constructor from date components)
		//
		// However, these are *not* SI microseconds, they are mean solar microseconds.
		// SI microseconds are defined in terms of the behaviour of the caesium atom.
		// Mean solar microseconds are defined in terms of the rotation of the Earth.
		// This means that there are always precisely 86400 * 10^6 mean solar microseconds in each day.
		// Since every day contains precisely the same number of mean solar microseconds, it is practical
		// to compute the number of mean solar microseconds between any two dates (specified
		// according to the Gregorian calendar) or to compute a date from a timestamp.
		//
		// If we used SI microseconds, we would need to know all past and future leap seconds to be able to correctly
		// convert between the timestamp value and a year-month-day-hour-minute-second-microsecond tuple.
		// Apart from making the calculations a pain in the ass, that is actually impossible for dates in the future,
		// because leap seconds are not predictable (they're introduced as necessary based on astronomical observations)
		//
		// (Incidentally, this is the way all integer timestamps work, at least all the ones I've ever seen)
		Sint64 m_timestamp; // (units: microseconds; 0 means 2001-01-01T00:00:00Z)
};

} // namespace Time

#endif
