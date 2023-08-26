// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include <cstring>
#include <string>
#include <string_view>
#include <vector>

// Pioneer String Utility Helpers

#define SIZET_FMT "%zu"

std::string format_date(double time);
std::string format_date_only(double time);
std::string format_distance(double dist, int precision = 2);
std::string format_money(double cents, bool showCents = true);
std::string format_duration(double seconds);

// find string in bigger string, ignoring case
const char *pi_strcasestr(const char *haystack, const char *needle);

inline bool starts_with(const std::string_view s, const std::string_view t)
{
	if (s.size() < t.size())
		return false;
	return std::memcmp(s.data(), t.data(), t.size()) == 0;
}

inline bool ends_with(const std::string_view s, const std::string_view t)
{
	if (s.size() < t.size())
		return false;

	return std::memcmp(s.data() + (s.size() - t.size()), t.data(), t.size()) == 0;
}

inline bool starts_with_ci(const std::string_view s, const std::string_view t)
{
	if (s.size() < t.size())
		return false;

	for (size_t i = 0; i < t.size(); i++)
		if (tolower(s.data()[i]) != tolower(t.data()[i]))
			return false;

	return true;
}

inline bool ends_with_ci(const std::string_view s, const std::string_view t)
{
	if (s.size() < t.size())
		return false;

	for (int64_t i = t.size(); i > 0; i--)
		if (tolower(s.data()[s.size() - i]) != tolower(t.data()[t.size() - i]))
			return false;

	return true;
}

inline bool compare_ci(const std::string_view s, const std::string_view t)
{
	if (s.size() != t.size())
		return false;

	for (size_t i = 0; i < s.size(); i++)
		if (tolower(s.data()[i]) != tolower(t.data()[i]))
			return false;

	return true;
}

inline std::string_view read_line(std::string_view &s)
{
	if (s.empty())
		return {};

	std::string_view out = s;

	size_t end = s.find_first_of("\r\n");
	if (end == std::string_view::npos) {
		s = {};
		return out;
	}

	out = { s.data(), end };

	size_t start = s.find_first_not_of("\r\n", end);
	if (start == std::string_view::npos) {
		s = {};
		return out;
	}

	s.remove_prefix(start);
	return out;
}

inline std::string_view strip_spaces(std::string_view &s)
{
	if (s.empty())
		return s;

	size_t start = s.find_first_not_of(" \t\r\n\v");
	if (start == std::string::npos)
		return {};

	size_t end = s.find_last_not_of(" \t\r\n\v");

	return s.substr(start, end);
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
std::string AutoToStr(int32_t val);
std::string AutoToStr(int64_t val);
std::string AutoToStr(float val);
std::string AutoToStr(double val);

// String to 'Numeric type' conversions.
int64_t StrToSInt64(const std::string &str);
uint64_t StrToUInt64(const std::string &str);
float StrToFloat(const std::string &str);
double StrToDouble(const std::string &str);
void StrToAuto(int32_t *pVal, const std::string &str);
void StrToAuto(int64_t *pVal, const std::string &str);
void StrToAuto(float *pVal, const std::string &str);
void StrToAuto(double *pVal, const std::string &str);

// Convert decimal coordinates to degree/minute/second format and return as string
std::string DecimalToDegMinSec(float dec);
