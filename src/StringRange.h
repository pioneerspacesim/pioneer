#ifndef _STRINGRANGE_H
#define _STRINGRANGE_H

#include <string>
#include <cassert>
#include <cctype>

struct StringRange
{
	StringRange(): begin(0), end(0) {}
	StringRange(const char *begin_, const char *end_)
		: begin(begin_), end(end_)
	{
		assert(begin_ && end_);
		assert((end_ - begin_) >= 0);
	}

	operator std::string() const { return ToString(); }

	const char *begin;
	const char *end;

	bool Empty() const { return (begin == end); }
	size_t Length() const { return (end - begin); }

	const char &operator[](size_t idx) const { return begin[idx]; }
	const char &operator*() const { return *begin; }
	std::string ToString() const { return begin ? std::string(begin, Length()) : std::string(); }

	const char *FindChar(char c) const;
	const char *RFindChar(char c) const;

	const char *FindSpace() const;
	const char *RFindSpace() const;

	const char *FindNewline() const;

	const char *SkipNewline() const;

	const char *SkipSpace() const;
	const char *RSkipSpace() const;

	StringRange StripSpace() const;
};

inline const char *StringRange::FindChar(char c) const
{
	const char *x = begin, *y = end;
	while ((x != y) && (*x != c)) ++x;
	return x;
}

inline const char *StringRange::RFindChar(char c) const
{
	const char *x = begin, *y = end;
	while ((x != y) && (*--y != c)) {}
	return y;
}

inline const char *StringRange::FindSpace() const
{
	const char *x = begin, *y = end;
	while ((x != y) && !isspace(*x)) ++x;
	return x;
}

inline const char *StringRange::RFindSpace() const
{
	const char *x = begin, *y = end;
	while ((x != y) && !isspace(*--y)) {}
	return y;
}

inline const char *StringRange::FindNewline() const
{
	const char *x = begin, *y = end;
	while ((x != y) && (*x != '\r') && (*x != '\n')) ++x;
	return x;
}

inline const char *StringRange::SkipNewline() const
{
	const char *x = begin, *y = end;
	if ((x != y) && (*x == '\r')) ++x;
	if ((x != y) && (*x == '\n')) ++x;
	return x;
}

inline const char *StringRange::SkipSpace() const
{
	const char *x = begin, *y = end;
	while ((x != y) && isspace(*x)) ++x;
	return x;
}

inline const char *StringRange::RSkipSpace() const
{
	const char *x = begin, *y = end;
	while ((x != y) && isspace(*--y)) {}
	return (y == end ? y : y + 1);
}

inline StringRange StringRange::StripSpace() const
{
	return StringRange(SkipSpace(), RSkipSpace());
}

#endif
