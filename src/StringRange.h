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
	StringRange(const char *begin_, size_t size)
		: begin(begin_), end(begin_ + size)
	{
		assert(begin_);
	}

	const char *begin;
	const char *end;

	bool Empty() const { return (begin == end); }
	size_t Size() const { return (end - begin); }

	const char &operator[](size_t idx) const { return begin[idx]; }
	const char &operator*() const { return *begin; }
	std::string ToString() const { return begin ? std::string(begin, Size()) : std::string(); }

	int Compare(const char *b) const;

	friend bool operator==(const StringRange &a, const char *b) { return (a.Compare(b) == 0); }
	friend bool operator!=(const StringRange &a, const char *b) { return (a.Compare(b) != 0); }
	friend bool operator<=(const StringRange &a, const char *b) { return (a.Compare(b) <= 0); }
	friend bool operator>=(const StringRange &a, const char *b) { return (a.Compare(b) >= 0); }
	friend bool operator< (const StringRange &a, const char *b) { return (a.Compare(b) <  0); }
	friend bool operator> (const StringRange &a, const char *b) { return (a.Compare(b) >  0); }

	friend bool operator==(const char *a, const StringRange &b) { return (b.Compare(a) == 0); }
	friend bool operator!=(const char *a, const StringRange &b) { return (b.Compare(a) != 0); }
	friend bool operator<=(const char *a, const StringRange &b) { return (b.Compare(a) >= 0); }
	friend bool operator>=(const char *a, const StringRange &b) { return (b.Compare(a) <= 0); }
	friend bool operator< (const char *a, const StringRange &b) { return (b.Compare(a) >  0); }
	friend bool operator> (const char *a, const StringRange &b) { return (b.Compare(a) <  0); }

	const char *FindChar(char c) const;
	const char *RFindChar(char c) const;

	const char *FindSpace() const;
	const char *RFindSpace() const;

	const char *FindNewline() const;

	const char *FindNonSpace() const;
	const char *RFindNonSpace() const;

	const char *FindNextLine() const;

	StringRange StripSpace() const;

	StringRange ReadLine();
private:
	static bool is_space(char c) {
		// MSVC's isspace() apparently asserts if you give it non-ASCII chars
		// (maybe changing locale would help)
		return (c == ' ') || (c == '\t') || (c == '\v') || (c == '\r') || (c == '\n');
	}
};

inline StringRange StringRange::ReadLine()
{
	StringRange line(begin, FindNextLine());
	begin = line.end;
	return line;
}

inline int StringRange::Compare(const char *b) const
{
	const char *a = begin;
	while (a != end && *b && *a == *b) { ++a; ++b; }
	if (a != end && *b) { return (*a < *b ? -1 : 1); }
	if (a != end) { return 1; }
	if (*b) { return -1; }
	return 0;
}

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

inline const char *StringRange::FindNextLine() const
{
	const char *x = FindNewline(), *y = end;
	if ((x != y) && (*x == '\r')) ++x;
	if ((x != y) && (*x == '\n')) ++x;
	return x;
}

inline const char *StringRange::FindNonSpace() const
{
	const char *x = begin, *y = end;
	while ((x != y) && is_space(*x)) ++x;
	return x;
}

inline const char *StringRange::RFindNonSpace() const
{
	const char *x = begin, *y = end;
	while ((x != y) && is_space(*--y)) {}
	return (y == end ? y : y + 1);
}

inline StringRange StringRange::StripSpace() const
{
	// can't just do StringRange(FindNonSpace(), RFindNonSpace())
	// because if the string is *all* space then that will break the
	// invariant that begin <= end
	StringRange ss(FindNonSpace(), end);
	ss.end = ss.RFindNonSpace();
	return ss;
}

#endif
