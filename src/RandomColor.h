// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#ifndef _RANDOM_COLOR_H_
#define _RANDOM_COLOR_H_

#include "Range.h"
#include "Random.h"
#include "Color.h"

#include <vector>
#include <map>

namespace RandomColorGenerator
{
	enum ColorScheme
	{
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

	enum Luminosity
	{
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
		Point() : x(0), y(0) {}
		Point(int nx, int ny) : x(nx), y(ny) {}
		int x, y;
	};

	// Generates random numbers.
	class RandomColor
	{
	private:
		class DefinedColor
		{
		public:
			DefinedColor() {}
			Range HueRange;
			std::vector<Point> LowerBounds;
			Range SaturationRange;
			Range BrightnessRange;
		};

		static std::map<ColorScheme, DefinedColor> ColorDictionary;
		static Random _rng;

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
		static std::vector<Color> GetColors(ColorScheme scheme, Luminosity luminosity, int count);

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
};


#endif // _RANDOM_COLOR_H_
