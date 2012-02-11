#ifndef _COLOR_H
#define _COLOR_H

class Color {
public:
	float r,g,b,a;
	Color() : r(0.f), g(0.f), b(0.f), a(1.f) {}
	Color(float v_) : r(v_), g(v_), b(v_), a(v_) { }
	Color(float r_, float g_, float b_): r(r_), g(g_), b(b_), a(1.f) {}
	Color(float r_, float g_, float b_, float a_): r(r_), g(g_), b(b_), a(a_) {}
	Color(float *v) : r(v[0]), g(v[1]), b(v[2]), a(v[3]) { } //XXX asking for trouble
	operator float *() { return &r; }
	operator const float *() const { return &r; }
	Color &operator*=(const float v) { r*=v; g*=v; b*=v; a*=v; return *this; }
};

#endif /* _COLOR_H */
