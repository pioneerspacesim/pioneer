// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include <cstring>
#include <string>
#include <string_view>
#include <vector>
#include <iterator>
#include <cstdint>

#ifdef _MSC_VER
#ifndef __MINGW32__
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif
#endif

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

// Utility class to split a string based on a provided set of delimiters
// Makes find_first_of / find_last_of more ergonomic to use
struct SplitString {
	struct iter {
		using value_type = std::string_view;
		using reference = std::string_view;

		// "end" iterator
		iter() : m_parent(nullptr), m_str() {};
		// "live" iterator
		iter(SplitString *parent) :
			m_parent(parent),
			m_str(parent->m_orig),
			m_next(parent->step(m_str))
		{
		}

		value_type operator*()
		{
			if (m_next != std::string_view::npos)
				return m_parent->m_reverse ? m_str.substr(m_next + 1) : m_str.substr(0, m_next);
			else
				return m_str;
		}

		iter &operator++()
		{
			m_parent->trim(m_str, m_next);
			m_next = m_parent->step(m_str);
			return *this;
		}

		bool operator!=(const iter &rhs) { return !(*this == rhs); }
		bool operator==(const iter &rhs) {
			return (m_str.empty() && rhs.m_str.empty()) ||
				(m_parent == rhs.m_parent && m_str.size() == rhs.m_str.size());
		}

	private:
		SplitString *m_parent;
		std::string_view m_str;
		size_t m_next = std::string_view::npos;
	};

	SplitString(std::string_view source, std::string_view delim) :
		m_orig(source), m_delim(delim)
	{}

	SplitString(std::string_view source, std::string_view delim, bool reverse) :
		m_orig(source), m_delim(delim), m_reverse(reverse)
	{}

	iter begin() { return iter(this); }
	iter end() { return iter(); }

	// Split the input string to a vector of fragments using the specified type
	template<typename T = std::string_view>
	std::vector<T> to_vector() {
		std::vector<T> out;
		for (auto str : *this) {
			out.push_back(T(str));
		}

		return out;
	}

	// Apply the given predicate to each fragment of the string and push the result into the given container
	template<typename Container, typename Predicate>
	void to_vector(Container &c, Predicate p) {
		for (auto str : *this) {
			c.push_back(p(str));
		}
	}

private:
	friend struct iter;

	// find the boundary for the next token
	size_t step(std::string_view str);
	// remove previous substring if present
	void trim(std::string_view &str, size_t next);

	std::string_view m_orig;
	std::string_view m_delim;
	bool m_reverse = false;
};

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
