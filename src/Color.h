// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _COLOR_H
#define _COLOR_H

#include <SDL_stdinc.h>

struct lua_State;

struct Color4f {
	float r, g, b, a;
	constexpr Color4f() :
		r(0.f),
		g(0.f),
		b(0.f),
		a(1.f) {}
	constexpr Color4f(float v_) :
		r(v_),
		g(v_),
		b(v_),
		a(v_) {}
	constexpr Color4f(float r_, float g_, float b_) :
		r(r_),
		g(g_),
		b(b_),
		a(1.f) {}
	constexpr Color4f(float r_, float g_, float b_, float a_) :
		r(r_),
		g(g_),
		b(b_),
		a(a_) {}
	operator float *() { return &r; }
	operator const float *() const { return &r; }
	Color4f &operator*=(const float v)
	{
		r *= v;
		g *= v;
		b *= v;
		a *= v;
		return *this;
	}
	friend Color4f operator*(const Color4f &c, const float v) { return Color4f(c.r * v, c.g * v, c.b * v, c.a * v); }

	void ToLuaTable(lua_State *l);
	static Color4f FromLuaTable(lua_State *l, int idx);

	float GetLuminance() const;

	static const Color4f BLACK;
	static const Color4f WHITE;
	static const Color4f RED;
	static const Color4f GREEN;
	static const Color4f BLUE;
	static const Color4f YELLOW;
	static const Color4f GRAY;
	static const Color4f STEELBLUE;
	static const Color4f BLANK;
};

namespace {
	static const float s_inv255 = 1.0f / 255.0f;
#define INV255(n) (Uint8(float(n) * s_inv255))
} // namespace

struct Color4ub {

	Uint8 r, g, b, a;
	constexpr Color4ub() :
		r(0),
		g(0),
		b(0),
		a(255) {}
	constexpr Color4ub(Uint8 r_, Uint8 g_, Uint8 b_) :
		r(r_),
		g(g_),
		b(b_),
		a(255) {}
	constexpr Color4ub(Uint8 r_, Uint8 g_, Uint8 b_, Uint8 a_) :
		r(r_),
		g(g_),
		b(b_),
		a(a_) {}
	constexpr Color4ub(const Color4f &c) :
		r(Uint8(c.r * 255.f)),
		g(Uint8(c.g * 255.f)),
		b(Uint8(c.b * 255.f)),
		a(Uint8(c.a * 255.f)) {}
	constexpr Color4ub(const Uint32 rgba) :
		r((rgba >> 24) & 0xff),
		g((rgba >> 16) & 0xff),
		b((rgba >> 8) & 0xff),
		a(rgba & 0xff) {}
	constexpr Color4ub(const Color4ub &c, const Uint8 a) :
		r(c.r),
		g(c.g),
		b(c.b),
		a(a) {}

	operator unsigned char *() { return &r; }
	operator const unsigned char *() const { return &r; }
	Color4ub operator+(const Color4ub &c) const { return Color4ub(c.r + r, c.g + g, c.b + b, c.a + a); }
	Color4ub &operator*=(const float f)
	{
		r = Uint8(r * f);
		g = Uint8(g * f);
		b = Uint8(b * f);
		a = Uint8(a * f);
		return *this;
	}
	Color4ub &operator*=(const Color4ub &c)
	{
		r *= INV255(c.r);
		g *= INV255(c.g);
		b *= INV255(c.b);
		a *= INV255(c.a);
		return *this;
	}
	Color4ub operator*(const float f) const { return Color4ub(Uint8(f * r), Uint8(f * g), Uint8(f * b), Uint8(f * a)); }
	Color4ub operator*(const Color4ub &c) const { return Color4ub(INV255(c.r) * r, INV255(c.g) * g, INV255(c.b) * b, INV255(c.a) * a); }
	Color4ub operator/(const float f) const { return Color4ub(Uint8(r / f), Uint8(g / f), Uint8(b / f), Uint8(a / f)); }

	friend bool operator==(const Color4ub &aIn, const Color4ub &bIn) { return ((aIn.r == bIn.r) && (aIn.g == bIn.g) && (aIn.b == bIn.b) && (aIn.a == bIn.a)); }
	friend bool operator!=(const Color4ub &aIn, const Color4ub &bIn) { return ((aIn.r != bIn.r) || (aIn.g != bIn.g) || (aIn.b != bIn.b) || (aIn.a != bIn.a)); }

	Color4f ToColor4f() const { return Color4f(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f); }

	void ToLuaTable(lua_State *l);
	static Color4ub FromLuaTable(lua_State *l, int idx);

	Uint8 GetLuminance() const;
	Color4ub Shade(float factor)
	{
		Color4ub out = *this;
		out.r = static_cast<Uint8>(r * (1.0f - factor));
		out.g = static_cast<Uint8>(g * (1.0f - factor));
		out.b = static_cast<Uint8>(b * (1.0f - factor));
		return out;
	}
	Color4ub Tint(float factor)
	{
		Color4ub out = *this;
		out.r = static_cast<Uint8>(r + (255.0f - r) * factor);
		out.g = static_cast<Uint8>(g + (255.0f - g) * factor);
		out.b = static_cast<Uint8>(b + (255.0f - b) * factor);
		return out;
	}
	Color4ub Opacity(float factor)
	{
		Color4ub out = *this;
		out.a = static_cast<Uint8>(factor <= 1.0 ? factor * 255 : uint8_t(factor));
		return out;
	}

	static const Color4ub BLACK;
	static const Color4ub WHITE;
	static const Color4ub RED;
	static const Color4ub GREEN;
	static const Color4ub BLUE;
	static const Color4ub YELLOW;
	static const Color4ub GRAY;
	static const Color4ub STEELBLUE;
	static const Color4ub BLANK;
	static const Color4ub PINK;
};

struct Color3ub {
	Uint8 r, g, b;
	constexpr Color3ub() :
		r(0),
		g(0),
		b(0) {}
	constexpr Color3ub(Uint8 v_) :
		r(v_),
		g(v_),
		b(v_) {}
	constexpr Color3ub(Uint8 r_, Uint8 g_, Uint8 b_) :
		r(r_),
		g(g_),
		b(b_) {}
	constexpr Color3ub(const Color4f &c) :
		r(Uint8(c.r * 255.f)),
		g(Uint8(c.g * 255.f)),
		b(Uint8(c.b * 255.f)) {}

	operator unsigned char *() { return &r; }
	operator const unsigned char *() const { return &r; }
	Color3ub &operator*=(const Color3ub &c)
	{
		r *= INV255(c.r);
		g *= INV255(c.g);
		b *= INV255(c.b);
		return *this;
	}
	Color3ub operator+(const Color3ub &c) const { return Color3ub(c.r + r, c.g + g, c.b + b); }
	Color3ub operator*(const float f) const { return Color3ub(Uint8(f * r), Uint8(f * g), Uint8(f * b)); }
	Color3ub operator*(const Color3ub &c) const { return Color3ub(INV255(c.r) * r, INV255(c.g) * g, INV255(c.b) * b); }
	Color3ub operator/(const float f) const { return Color3ub(Uint8(r / f), Uint8(g / f), Uint8(b / f)); }

	Color4f ToColor4f() const { return Color4f(r / 255.0f, g / 255.0f, b / 255.0f); }

	static const Color3ub BLACK;
	static const Color3ub WHITE;
	static const Color3ub RED;
	static const Color3ub GREEN;
	static const Color3ub BLUE;
	static const Color3ub YELLOW;
	static const Color3ub STEELBLUE;
	static const Color3ub BLANK;
};

typedef Color4ub Color;

#endif /* _COLOR_H */
