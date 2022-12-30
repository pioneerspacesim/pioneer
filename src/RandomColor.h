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

#ifndef _RANDOM_COLOR_H_
#define _RANDOM_COLOR_H_

// Andrew Copland (2016/03/10 )This is a port of a port! Done under the MIT license (see ^^^Above^^^).
// Originally pointed out to me by RobN in a forum thread (https://forum.pioneerspacesim.net/viewtopic.php?f=3&t=221)
// I was deterred by A) not knowing any javascript, B) the more I saw of it the more I hated it.
// Then I noticed the C# port by Nathan Jones (https://github.com/nathanpjones/randomColorSharped) which was a much more logical starting point.
// I have included a snippet of his readme below about how he himself came to port it.

//randomColorSharped
//==================
//
// This is a port to c# (.NET 4.0) of [randomColor](http://llllll.li/randomColor/), David Merfield's javascript random color generator.
// This was ported by [Nathan Jones](http://www.nathanpjones.com/) so that users of the .NET family of languages could enjoy these attractive colors.
//
// I saw this project linked on [Scott Hanselman's](http://www.hanselman.com/) excellent [Newsletter of Wonderful Things](http://www.hanselman.com/newsletter/)
// around the same time a coworker was creating an ad hoc visualization app. As we watched the data appear on screen, we had to squint as some of the colors
// were very difficult to make out against the background. This should make things easier to see and will hopefully help others as well.

#include "Color.h"
#include "Random.h"

#include <cstdlib>
#include <map>
#include <vector>

namespace RandomColorGenerator {
	// Represents a range using an upper and lower value.
	class Range {
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
		const int &operator[](const size_t index) const
		{
			if (index == 0)
				return Lower;
			else
				return Upper;
		}
	};

	enum ColorScheme {
		// Select randomly from among the other color schemes.
		SCHEME_RANDOM = 0,
		// Generates only grayscale colors.
		SCHEME_MONOCHROME,
		// Generates only red colors.
		SCHEME_RED,
		// Generates only orange colors.
		SCHEME_ORANGE,
		// Generates only yellow colors.
		SCHEME_YELLOW,
		// Generates only green colors.
		SCHEME_GREEN,
		// Generates only blue colors.
		SCHEME_BLUE,
		// Generates only purple colors.
		SCHEME_PURPLE,
		// Generates only pink colors.
		SCHEME_PINK
	};

	enum Luminosity {
		// Select randomly from among the other luminosities.
		LUMINOSITY_RANDOM = 0,
		// Generate dark colors.
		LUMINOSITY_DARK,
		// Generate light, pastel colors.
		LUMINOSITY_LIGHT,
		// Generate vibrant colors.
		LUMINOSITY_BRIGHT
	};

	class Point {
	public:
		Point() :
			x(0),
			y(0) {}
		Point(int nx, int ny) :
			x(nx),
			y(ny) {}
		int x, y;
	};

	// Generates random numbers.
	class RandomColor {
	private:
		class DefinedColor {
		public:
			DefinedColor() {}
			Range HueRange;
			std::vector<Point> LowerBounds;
			Range SaturationRange;
			Range BrightnessRange;
		};

		static std::map<ColorScheme, DefinedColor> ColorDictionary;

	public:
		RandomColor();

		/// Gets a new random color.
		/// <param name="scheme">Which color schemed to use when generating the color.</param>
		/// <param name="luminosity">The desired luminosity of the color.</param>
		static Color GetColor(ColorScheme scheme, Luminosity luminosity);

		/// Generates multiple random colors.
		/// <param name="scheme">Which color schemed to use when generating the color.</param>
		/// <param name="luminosity">The desired luminosity of the color.</param>
		/// <param name="count">How many colors to generate</param>
		static std::vector<Color> GetColors(Random &rand, ColorScheme scheme, Luminosity luminosity, int count);

		static int PickHue(ColorScheme scheme);

	private:
		static int PickSaturation(int hue, Luminosity luminosity, ColorScheme scheme);

		static int PickBrightness(int H, int S, Luminosity luminosity);

		static int GetMinimumBrightness(int H, int S);

		static Range GetHueRange(ColorScheme colorInput);

		static DefinedColor GetColorInfo(int hue);

		static int RandomWithin(Range range);
		static int RandomWithin(int lower, int upper);

		static void DefineColor(ColorScheme scheme, Point hueRange, const Point *lowerBounds, const size_t lbCount);

		static void LoadColorBounds();

	public:
		// Converts hue, saturation, and lightness to a color.
		static Color HsvToColor(int hue, int saturation, double value);
	};
} // namespace RandomColorGenerator

#endif // _RANDOM_COLOR_H_
