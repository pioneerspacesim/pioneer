// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "DateTime.h"
#include <iomanip>
#include <iostream>
#include <cassert>

using namespace Time;

static void check_datetime_round_trip(int year, int month, int day, int hour, int minute, int second, bool emit_message) {
	const DateTime t(year, month, day, hour, minute, second);
	int year2, month2, day2, hour2, minute2, second2;
	t.GetDateParts(&year2, &month2, &day2);
	t.GetTimeParts(&hour2, &minute2, &second2);

	assert(year == year2);
	assert(month == month2);
	assert(day == day2);
	assert(hour == hour2);
	assert(minute == minute2);
	assert(second == second2);

	if (emit_message) {
		std::cout << "roundtrip: " << t.ToDateString() << "  " << t.ToTimeString() << "  (" << t.GetTimestamp() << ")\n";
	}
}

static const int CHECK_AROUND_TIMES[][6] = {
	{1600,1,1, 6,0,0},
	{1601,1,1, 6,0,0},

	{2000,2,28, 23,59,59},
	{2000,2,29, 0,0,0},
	{2000,2,29, 0,0,1},

	{2000,2,29, 12,0,0},
	{2004,2,29, 12,0,0},
	{2008,2,29, 12,0,0},

	{2100,2,28, 23,59,59},
	{2100,3,1, 0,0,0},
	{2100,3,1, 0,0,1},

	{1999,12,31, 23,59,59},
	{2000,1,1, 0,0,0},
	{2000,1,1, 0,0,1},

	{2000,12,31, 23,59,59},
	{2001,1,1, 0,0,0},
	{2001,1,1, 0,0,1},

	{3200,1,1, 0,0,0},
	{3200,2,28, 0,0,0},
	{3200,2,29, 0,0,0},

	{-1,-1,-1,-1,-1,-1}
};

static void check_datetime_round_trip_2(Sint64 timestamp) {
	const DateTime t = DateTime() + TimeDelta(timestamp, TimeUnit(1));

	int year, month, day, hour, minute, second;
	t.GetDateParts(&year, &month, &day);
	t.GetTimeParts(&hour, &minute, &second);

	check_datetime_round_trip(year, month, day, hour, minute, second, false);
}

void test_datetime() {
	const Time::DateTime EPOCH;

	std::cout << "Microsecond: " << std::setw(12) << Sint64(Time::Microsecond) << "\n";
	std::cout << "Millisecond: " << std::setw(12) << Sint64(Time::Millisecond) << "\n";
	std::cout << "     Second: " << std::setw(12) << Sint64(Time::Second) << "\n";
	std::cout << "     Minute: " << std::setw(12) << Sint64(Time::Minute) << "\n";
	std::cout << "       Hour: " << std::setw(12) << Sint64(Time::Hour) << "\n";
	std::cout << "        Day: " << std::setw(12) << Sint64(Time::Day) << "\n";
	std::cout << "       Week: " << std::setw(12) << Sint64(Time::Week) << "\n";

	std::cout << "epoch: " << EPOCH.ToStringISO8601() << "\n";

	for (int i = -10; i <= 10; ++i) {
		const Time::DateTime t = EPOCH + TimeDelta(i, Time::Hour);
		std::cout << "epoch + " << i << " hours = " << t.ToStringISO8601() << "\n";
	}

	for (int i = -10; i <= 10; ++i) {
		const Time::DateTime t = EPOCH + TimeDelta(i, Time::Day);
		std::cout << "epoch + " << i << " days = " << t.ToStringISO8601() << "\n";
	}

	for (int i = 0; CHECK_AROUND_TIMES[i][3] != -1; ++i) {
		const int *x = CHECK_AROUND_TIMES[i];
		check_datetime_round_trip(x[0],x[1],x[2], x[3],x[4],x[5], true);
	}

	Time::DateTime t = DateTime(1600,1,1, 0,0,0);
	std::cout << "Checking round trips for *many* timestamps...\n";
	std::cout << "  beginning at " << t.ToDateString() << "  " << t.ToTimeString() << "\n";
	std::cout << "  (checking round-trip every 6 hours)\n";
	for (int i = 0; i <= 4000000; ++i) {
		int year, month, day, hour, minute, second;
		t.GetDateParts(&year, &month, &day);
		t.GetTimeParts(&hour, &minute, &second);
		check_datetime_round_trip(year, month, day, hour, minute, second, false);

		t = t + TimeDelta(6, Time::Hour);
	}
	std::cout << "  ending at " << t.ToDateString() << "  " << t.ToTimeString() << "\n";
}
