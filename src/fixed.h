#ifndef _FIXED_H
#define _FIXED_H

#include <SDL_stdinc.h>

class fixed {
public:
	enum { FRAC=16 };
	fixed(): v(0) {}
	fixed(int raw): v(raw) {}
	fixed(int num, int denom): v(((Sint64)num<<FRAC) / (Sint64)denom) {}

	friend fixed operator+(const fixed a, const int b) { return a+fixed(b<<FRAC); }
	friend fixed operator-(const fixed a, const int b) { return a-fixed(b<<FRAC); }
	friend fixed operator*(const fixed a, const int b) { return a*fixed(b<<FRAC); }
	friend fixed operator/(const fixed a, const int b) { return a/fixed(b<<FRAC); }
	friend fixed operator+(const int a, const fixed b) { return fixed(a<<FRAC)+b; }
	friend fixed operator-(const int a, const fixed b) { return fixed(a<<FRAC)-b; }
	friend fixed operator*(const int a, const fixed b) { return fixed(a<<FRAC)*b; }
	friend fixed operator/(const int a, const fixed b) { return fixed(a<<FRAC)/b; }
	friend bool operator==(const fixed a, const int b) { return a == fixed(b<<FRAC); }
	friend bool operator==(const int a, const fixed b) { return b == fixed(a<<FRAC); }
	friend bool operator>=(const fixed a, const int b) { return a >= fixed(b<<FRAC); }
	friend bool operator>=(const int a, const fixed b) { return b >= fixed(a<<FRAC); }
	friend bool operator<=(const fixed a, const int b) { return a <= fixed(b<<FRAC); }
	friend bool operator<=(const int a, const fixed b) { return b <= fixed(a<<FRAC); }
	friend bool operator>(const fixed a, const int b) { return a > fixed(b<<FRAC); }
	friend bool operator>(const int a, const fixed b) { return b > fixed(a<<FRAC); }
	friend bool operator<(const fixed a, const int b) { return a < fixed(b<<FRAC); }
	friend bool operator<(const int a, const fixed b) { return b < fixed(a<<FRAC); }

	fixed &operator*=(const fixed a) { (*this) = (*this)*a; return (*this); }
	fixed &operator*=(const int a) { (*this) = (*this)*a; return (*this); }
	fixed &operator/=(const fixed a) { (*this) = (*this)/a; return (*this); }
	fixed &operator/=(const int a) { (*this) = (*this)/a; return (*this); }
	fixed &operator+=(const fixed a) { (*this) = (*this)+a; return (*this); }
	fixed &operator+=(const int a) { (*this) = (*this)+a; return (*this); }
	fixed &operator-=(const fixed a) { (*this) = (*this)-a; return (*this); }
	fixed &operator-=(const int a) { (*this) = (*this)-a; return (*this); }

	friend fixed operator+(const fixed a, const fixed b) { return fixed(a.v+b.v); }
	friend fixed operator-(const fixed a, const fixed b) { return fixed(a.v-b.v); }
	friend fixed operator*(const fixed a, const fixed b) { return fixed(((Sint64)a.v*(Sint64)b.v)>>FRAC); }
	friend fixed operator/(const fixed a, const fixed b) { return fixed(((Sint64)a.v<<FRAC)/(Sint64)b.v); }
	friend bool operator==(const fixed a, const fixed b) { return a.v == b.v; }
	friend bool operator>(const fixed a, const fixed b) { return a.v > b.v; }
	friend bool operator<(const fixed a, const fixed b) { return a.v < b.v; }
	friend bool operator>=(const fixed a, const fixed b) { return a.v >= b.v; }
	friend bool operator<=(const fixed a, const fixed b) { return a.v <= b.v; }


	operator float() { return v/(float)(1<<FRAC); }
	operator double() { return v/(double)(1<<FRAC); }

	private:
	int v;
};

#endif /* _FIXED_H */
