/// The MIT License (MIT)
///
/// Copyright (c) 2016 Andrew J Copland
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.

#pragma once

#ifndef _RANGE_H_
#define _RANGE_H_

// Description: See header of /src/RandomColor.h for licnses and origin of code discussion

#include <cassert>
#include <cstdio>

namespace RandomColorGenerator
{
    // Represents a range using an upper and lower value.
    class Range
    {
    public:
		int Lower;
        int Upper;

		Range() {}
        Range(int lower, int upper)
        {
            Lower = lower;
            Upper = upper;
        }

        // Gets the lower range for an index of 0 and the upper for an index of 1.
		const int &operator[](const size_t index) const {
			if(index==0)
                 return Lower;
			else
                 return Upper;
		}
    };
};

#endif // _RANGE_H_
