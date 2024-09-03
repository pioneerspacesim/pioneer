// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _BYTERANGE_H
#define _BYTERANGE_H

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstring>

struct ByteRange {
	ByteRange() :
		begin(0),
		end(0) {}
	ByteRange(const char *begin_, const char *end_) :
		begin(begin_),
		end(end_)
	{
		assert(begin_ && end_);
		assert((end_ - begin_) >= 0);
	}
	ByteRange(const char *begin_, size_t size) :
		begin(begin_),
		end(begin_ + size)
	{
		assert(begin_);
	}

	const char *begin;
	const char *end;

	bool Empty() const { return (begin == end); }
	size_t Size() const { return (end - begin); }

	const char &operator[](size_t idx) const { return begin[idx]; }
	const char &operator*() const { return *begin; }

	size_t read(char *to, size_t sz, size_t count);
};

inline size_t ByteRange::read(char *to, size_t sz, size_t count)
{
	size_t avail = Size() / sz;
	count = std::min(count, avail);
	size_t fullsize = sz * count;
	assert(fullsize <= Size());
	memcpy(to, begin, fullsize);
	begin += fullsize;
	return count;
}

#endif
