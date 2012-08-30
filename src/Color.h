#ifndef _COLOR_H
#define _COLOR_H

struct Color4f {
	float r,g,b,a;
	Color4f() : r(0.f), g(0.f), b(0.f), a(1.f) {}
	Color4f(float v_) : r(v_), g(v_), b(v_), a(v_) { }
	Color4f(float r_, float g_, float b_): r(r_), g(g_), b(b_), a(1.f) {}
	Color4f(float r_, float g_, float b_, float a_): r(r_), g(g_), b(b_), a(a_) {}
	operator float *() { return &r; }
	operator const float *() const { return &r; }
	Color4f &operator*=(const float v) { r*=v; g*=v; b*=v; a*=v; return *this; }

	float GetLuminance() const;

	static const Color4f BLACK;
	static const Color4f WHITE;
};

struct Color4ub {
	unsigned char r, g, b, a;
	Color4ub(): r(0), g(0), b(0), a(255) {}
	Color4ub(unsigned char v_): r(v_), g(v_), b(v_), a(v_) {}
	Color4ub(unsigned char r_, unsigned char g_, unsigned char b_): r(r_), g(g_), b(b_), a(255) {}
	Color4ub(unsigned char r_, unsigned char g_, unsigned char b_, unsigned char a_): r(r_), g(g_), b(b_), a(a_) {}
	Color4ub(const Color4f &c): r(c.r*255.f), g(c.g*255.f), b(c.b*255.f), a(c.a*255.f) {}

	operator unsigned char*() { return &r; }
	operator const unsigned char*() const { return &r; }

	Color4f ToColor4f() const { return Color4f(r/255.0f, g/255.0f, b/255.0f, a/255.0f); }

	static const Color4ub BLACK;
	static const Color4ub WHITE;
};

typedef Color4f Color;

#endif /* _COLOR_H */
