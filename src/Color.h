// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _COLOR_H
#define _COLOR_H

#include <SDL_stdinc.h>

struct lua_State;

struct Color4f {
	float r,g,b,a;
	Color4f() : r(0.f), g(0.f), b(0.f), a(1.f) {}
	Color4f(float v_) : r(v_), g(v_), b(v_), a(v_) { }
	Color4f(float r_, float g_, float b_): r(r_), g(g_), b(b_), a(1.f) {}
	Color4f(float r_, float g_, float b_, float a_): r(r_), g(g_), b(b_), a(a_) {}
	operator float *() { return &r; }
	operator const float *() const { return &r; }
	Color4f &operator*=(const float v) { r*=v; g*=v; b*=v; a*=v; return *this; }
	friend Color4f operator*(const Color4f &c, const float v) { return Color4f(c.r*v, c.g*v, c.b*v, c.a*v); }

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
};

struct Color4ub {
	Uint8 r, g, b, a;
	Color4ub(): r(0), g(0), b(0), a(255) {}
	Color4ub(Uint8 v_): r(v_), g(v_), b(v_), a(v_) {}
	Color4ub(Uint8 r_, Uint8 g_, Uint8 b_): r(r_), g(g_), b(b_), a(255) {}
	Color4ub(Uint8 r_, Uint8 g_, Uint8 b_, Uint8 a_): r(r_), g(g_), b(b_), a(a_) {}
	Color4ub(const Color4f &c): r(Uint8(c.r*255.f)), g(Uint8(c.g*255.f)), b(Uint8(c.b*255.f)), a(Uint8(c.a*255.f)) {}

	operator unsigned char*() { return &r; }
	operator const unsigned char*() const { return &r; }
	Color4ub operator+(const Color4ub &c) const { return Color4ub(c.r+r, c.g+g, c.b+b, c.a+a); }
	Color4ub &operator*=(const float f)			{ r=Uint8(r*f); g=Uint8(g*f); b=Uint8(b*f); a=Uint8(a*f); return *this; }
	Color4ub operator*(const float f) const		{ return Color4ub(Uint8(f*r), Uint8(f*g), Uint8(f*b), Uint8(f*a)); }
	Color4ub operator/(const float f) const		{ return Color4ub(Uint8(r/f), Uint8(g/f), Uint8(b/f), Uint8(a/f)); }

	friend bool operator==(const Color4ub& aIn, const Color4ub& bIn) { return ((aIn.r == bIn.r) && (aIn.g == bIn.g) && (aIn.b == bIn.b) && (aIn.a == bIn.a)); }

	Color4f ToColor4f() const { return Color4f(r/255.0f, g/255.0f, b/255.0f, a/255.0f); }

	void ToLuaTable(lua_State *l);
	static Color4ub FromLuaTable(lua_State *l, int idx);

	Uint8 GetLuminance() const;

	static const Color4ub BLACK;
	static const Color4ub WHITE;
	static const Color4ub RED;
	static const Color4ub GREEN;
	static const Color4ub BLUE;
	static const Color4ub YELLOW;
	static const Color4ub GRAY;
	static const Color4ub STEELBLUE;
};

struct Color3ub {
	Uint8 r, g, b;
	Color3ub(): r(0), g(0), b(0) {}
	Color3ub(Uint8 v_): r(v_), g(v_), b(v_) {}
	Color3ub(Uint8 r_, Uint8 g_, Uint8 b_): r(r_), g(g_), b(b_) {}
	Color3ub(const Color4f &c): r(Uint8(c.r*255.f)), g(Uint8(c.g*255.f)), b(Uint8(c.b*255.f)) {}

	operator unsigned char*() { return &r; }
	operator const unsigned char*() const { return &r; }
	Color3ub operator+(const Color3ub &c) const { return Color3ub(c.r+r, c.g+g, c.b+b); }
	Color3ub operator*(const float f) const { return Color3ub(Uint8(f*r), Uint8(f*g), Uint8(f*b)); }
	Color3ub operator/(const float f) const { return Color3ub(Uint8(r/f), Uint8(g/f), Uint8(b/f)); }

	Color4f ToColor4f() const { return Color4f(r/255.0f, g/255.0f, b/255.0f); }

	static const Color3ub BLACK;
	static const Color3ub WHITE;
	static const Color3ub RED;
	static const Color3ub GREEN;
	static const Color3ub BLUE;
	static const Color3ub YELLOW;
	static const Color3ub STEELBLUE;
};

typedef Color4ub Color;

#endif /* _COLOR_H */
