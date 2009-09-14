#ifndef _COLOR_H
#define _COLOR_H

class Color {
public:
	float r,g,b,a;
	Color() {}
	Color(float r, float g, float b, float a): r(r), g(g), b(b), a(a) {}
	operator float *() { return &r; }
};

#endif /* _COLOR_H */
