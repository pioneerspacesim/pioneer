// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "RandomColor.h"
#include "MathUtil.h"
#include "utils.h"
#include <algorithm>

namespace RandomColorGenerator {
	//static
	std::map<ColorScheme, RandomColor::DefinedColor> RandomColor::ColorDictionary;
	static Random *pRNG = nullptr;

	RandomColor::RandomColor()
	{
		// Populate the color dictionary
		LoadColorBounds();
	}

	/// Gets a new random color.
	/// <param name="scheme">Which color schemed to use when generating the color.</param>
	/// <param name="luminosity">The desired luminosity of the color.</param>
	//static
	Color RandomColor::GetColor(ColorScheme scheme, Luminosity luminosity)
	{
		int H, S, B;

		// First we pick a hue (H)
		H = PickHue(scheme);

		// Then use H to determine saturation (S)
		S = PickSaturation(H, luminosity, scheme);

		// Then use S and H to determine brightness (B).
		B = PickBrightness(H, S, luminosity);

		// Then we return the HSB color in the desired format
		return HsvToColor(H, S, B);
	}

	/// Generates multiple random colors.
	/// <param name="scheme">Which color scheme to use when generating the color.</param>
	/// <param name="luminosity">The desired luminosity of the color.</param>
	/// <param name="count">How many colors to generate</param>
	//static
	std::vector<Color> RandomColor::GetColors(Random &rand, ColorScheme scheme, Luminosity luminosity, int count)
	{
		pRNG = &rand;
		std::vector<Color> ret;
		ret.resize(count);
		for (int i = 0; i < count; i++) {
			ret[i] = GetColor(scheme, luminosity);
		}
		pRNG = nullptr;
		return ret;
	}

	//static
	int RandomColor::PickHue(ColorScheme scheme)
	{
		Range hueRange(GetHueRange(scheme));
		int hue = RandomWithin(hueRange);

		// Instead of storing red as two separate ranges,
		// we group them, using negative numbers
		if (hue < 0) hue = 360 + hue;

		return hue;
	}

	//static
	int RandomColor::PickSaturation(int hue, Luminosity luminosity, ColorScheme scheme)
	{
		if (luminosity == Luminosity::LUMINOSITY_RANDOM) {
			return RandomWithin(0, 100);
		}

		if (scheme == ColorScheme::SCHEME_MONOCHROME) {
			return 0;
		}

		Range saturationRange = GetColorInfo(hue).SaturationRange;

		int sMin = saturationRange.Lower;
		int sMax = saturationRange.Upper;

		switch (luminosity) {
		case Luminosity::LUMINOSITY_BRIGHT:
			sMin = 55;
			break;

		case Luminosity::LUMINOSITY_DARK:
			sMin = sMax - 10;
			break;

		case Luminosity::LUMINOSITY_LIGHT:
			sMax = 55;
			break;

		case Luminosity::LUMINOSITY_RANDOM:
			// TODO: is this correct? just leave sMin/sMax?
			break;
		}

		return RandomWithin(sMin, sMax);
	}

	//static
	int RandomColor::PickBrightness(int H, int S, Luminosity luminosity)
	{
		auto bMin = GetMinimumBrightness(H, S);
		auto bMax = 100;

		switch (luminosity) {
		case Luminosity::LUMINOSITY_DARK:
			bMax = bMin + 20;
			break;

		case Luminosity::LUMINOSITY_LIGHT:
			bMin = (bMax + bMin) / 2;
			break;

		case Luminosity::LUMINOSITY_RANDOM:
			bMin = 0;
			bMax = 100;
			break;

		case Luminosity::LUMINOSITY_BRIGHT:
			// TODO: is this correct? just leave min/max?
			break;
		}

		return RandomWithin(bMin, bMax);
	}

	//static
	int RandomColor::GetMinimumBrightness(int H, int S)
	{
		auto lowerBounds = GetColorInfo(H).LowerBounds;

		for (size_t i = 0; i < std::min(lowerBounds.size(), lowerBounds.size() - 1U); i++) {
			auto s1 = lowerBounds[i].x;
			auto v1 = lowerBounds[i].y;

			auto s2 = lowerBounds[i + 1].x;
			auto v2 = lowerBounds[i + 1].y;

			if (S >= s1 && S <= s2) {
				auto m = (v2 - v1) / (s2 - s1);
				auto b = v1 - m * s1;

				return static_cast<int>(m * S + b);
			}
		}

		return 0;
	}

	//static
	Range RandomColor::GetHueRange(ColorScheme colorInput)
	{
		std::map<ColorScheme, DefinedColor>::iterator out = ColorDictionary.find(colorInput);
		if (out != ColorDictionary.end()) {
			return out->second.HueRange;
		}

		return Range(0, 360);
	}

	//static
	RandomColor::DefinedColor RandomColor::GetColorInfo(int hue)
	{
		// Maps red colors to make picking hue easier
		if (hue >= 334 && hue <= 360) {
			hue -= 360;
		}

		for (auto c : ColorDictionary) {
			if (hue >= c.second.HueRange[0] && hue <= c.second.HueRange[1]) {
				return c.second;
			}
		}

		return DefinedColor();
	}

	//static
	int RandomColor::RandomWithin(Range range)
	{
		return RandomWithin(range.Lower, range.Upper);
	}
	//static
	int RandomColor::RandomWithin(int lower, int upper)
	{
		assert(pRNG);
		return pRNG->Int32(lower, upper + 1);
	}

	//static
	void RandomColor::DefineColor(ColorScheme scheme, Point hueRange, const Point *lowerBounds, const size_t lbCount)
	{
		auto sMin = lowerBounds[0].x;
		auto sMax = lowerBounds[lbCount - 1].x;
		auto bMin = lowerBounds[lbCount - 1].y;
		auto bMax = lowerBounds[0].y;

		DefinedColor defCol;
		defCol.HueRange = Range(hueRange.x, hueRange.y);
		for (size_t lb = 0; lb < lbCount; lb++) {
			defCol.LowerBounds.push_back(lowerBounds[lb]);
		}
		defCol.SaturationRange = Range(sMin, sMax);
		defCol.BrightnessRange = Range(bMin, bMax);

		ColorDictionary[scheme] = defCol;
	}

	//static
	void RandomColor::LoadColorBounds()
	{
		const Point mono[] = { { 0, 0 }, { 100, 0 } };
		DefineColor(
			ColorScheme::SCHEME_MONOCHROME,
			Point(0, 360),
			mono,
			COUNTOF(mono));

		Point red[] = { { 20, 100 }, { 30, 92 }, { 40, 89 }, { 50, 85 }, { 60, 78 }, { 70, 70 }, { 80, 60 }, { 90, 55 }, { 100, 50 } };
		DefineColor(
			ColorScheme::SCHEME_RED,
			Point(-26, 18),
			red,
			COUNTOF(red));

		Point orange[] = { { 20, 100 }, { 30, 93 }, { 40, 88 }, { 50, 86 }, { 60, 85 }, { 70, 70 }, { 100, 70 } };
		DefineColor(
			ColorScheme::SCHEME_ORANGE,
			Point(19, 46),
			orange,
			COUNTOF(orange));

		Point yellow[] = { { 25, 100 }, { 40, 94 }, { 50, 89 }, { 60, 86 }, { 70, 84 }, { 80, 82 }, { 90, 80 }, { 100, 75 } };
		DefineColor(
			ColorScheme::SCHEME_YELLOW,
			Point(47, 62),
			yellow,
			COUNTOF(yellow));

		Point green[] = { { 30, 100 }, { 40, 90 }, { 50, 85 }, { 60, 81 }, { 70, 74 }, { 80, 64 }, { 90, 50 }, { 100, 40 } };
		DefineColor(
			ColorScheme::SCHEME_GREEN,
			Point(63, 178),
			green,
			COUNTOF(green));

		Point blue[] = { { 20, 100 }, { 30, 86 }, { 40, 80 }, { 50, 74 }, { 60, 60 }, { 70, 52 }, { 80, 44 }, { 90, 39 }, { 100, 35 } };
		DefineColor(
			ColorScheme::SCHEME_BLUE,
			Point(179, 257),
			blue,
			COUNTOF(blue));

		Point purple[] = { { 20, 100 }, { 30, 87 }, { 40, 79 }, { 50, 70 }, { 60, 65 }, { 70, 59 }, { 80, 52 }, { 90, 45 }, { 100, 42 } };
		DefineColor(
			ColorScheme::SCHEME_PURPLE,
			Point(258, 282),
			purple,
			COUNTOF(purple));

		Point pink[] = { { 20, 100 }, { 30, 90 }, { 40, 86 }, { 60, 84 }, { 80, 80 }, { 90, 75 }, { 100, 73 } };
		DefineColor(
			ColorScheme::SCHEME_PINK,
			Point(283, 334),
			pink,
			COUNTOF(pink));
	}

	/// Converts hue, saturation, and lightness to a color.
	//static
	Color RandomColor::HsvToColor(int hue, int saturation, double value)
	{
		// this doesn't work for the values of 0 and 360
		// here's the hacky fix
		auto h = double(hue);
		if (is_equal_exact(h, 0.0)) {
			h = 1.0;
		}
		if (is_equal_exact(h, 360.0)) {
			h = 359.0;
		}

		// Rebase the h,s,v values
		h = h / 360.0;
		auto s = saturation / 100.0;
		auto v = value / 100.0;

		auto hInt = static_cast<int>(floor(h * 6.0));
		auto f = h * 6 - hInt;
		auto p = v * (1 - s);
		auto q = v * (1 - f * s);
		auto t = v * (1 - (1 - f) * s);
		auto r = 256.0;
		auto g = 256.0;
		auto b = 256.0;

		switch (hInt) {
		case 0:
			r = v;
			g = t;
			b = p;
			break;
		case 1:
			r = q;
			g = v;
			b = p;
			break;
		case 2:
			r = p;
			g = v;
			b = t;
			break;
		case 3:
			r = p;
			g = q;
			b = v;
			break;
		case 4:
			r = t;
			g = p;
			b = v;
			break;
		case 5:
			r = v;
			g = p;
			b = q;
			break;
		}
		auto c = Color(static_cast<Uint8>(floor(r * 255.0)),
			static_cast<Uint8>(floor(g * 255.0)),
			static_cast<Uint8>(floor(b * 255.0)),
			255);

		return c;
	}
} // namespace RandomColorGenerator
