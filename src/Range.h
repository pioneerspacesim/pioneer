// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#ifndef _RANGE_H_
#define _RANGE_H_

#include <cassert>

namespace RandomColorGenerator
{
    /// Represents a range using an upper and lower value.
    
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
