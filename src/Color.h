#ifndef _COLOR_H
#define _COLOR_H

class Color {
public:
	float r,g,b,a;
	Color() {}
	Color(float r_, float g_, float b_, float a_): r(r_), g(g_), b(b_), a(a_) {}
	operator float *() { return &r; }
	operator const float *() const { return &r; }
};

#endif /* _COLOR_H */
