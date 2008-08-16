#ifndef _FIXED_H
#define _FIXED_H

#include <SDL_stdinc.h>

// 48.16, with bad overflowing mul & div
class fixed {
public:
	enum { FRAC=16 };
	fixed(): v(0) {}
	fixed(Sint64 raw): v(raw) {}
	fixed(Sint64 num, Sint64 denom): v((num<<FRAC) / denom) {}
	
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
	friend fixed operator>>(const fixed a, const int b) { return fixed(a.v >> b); }
	friend fixed operator<<(const fixed a, const int b) { return fixed(a.v << b); }

	fixed &operator*=(const fixed a) { (*this) = (*this)*a; return (*this); }
	fixed &operator*=(const int a) { (*this) = (*this)*a; return (*this); }
	fixed &operator/=(const fixed a) { (*this) = (*this)/a; return (*this); }
	fixed &operator/=(const int a) { (*this) = (*this)/a; return (*this); }
	fixed &operator+=(const fixed a) { (*this) = (*this)+a; return (*this); }
	fixed &operator+=(const int a) { (*this) = (*this)+a; return (*this); }
	fixed &operator-=(const fixed a) { (*this) = (*this)-a; return (*this); }
	fixed &operator-=(const int a) { (*this) = (*this)-a; return (*this); }
	fixed &operator>>=(const int a) { v >>= a; return (*this); }
	fixed &operator<<=(const int a) { v <<= a; return (*this); }

	friend fixed operator+(const fixed a, const fixed b) { return fixed(a.v+b.v); }
	friend fixed operator-(const fixed a, const fixed b) { return fixed(a.v-b.v); }
	friend fixed operator*(const fixed a, const fixed b) { return fixed((a.v*b.v)>>FRAC); }
	friend fixed operator/(const fixed a, const fixed b) { return fixed((a.v<<FRAC)/b.v); }
	friend bool operator==(const fixed a, const fixed b) { return a.v == b.v; }
	friend bool operator>(const fixed a, const fixed b) { return a.v > b.v; }
	friend bool operator<(const fixed a, const fixed b) { return a.v < b.v; }
	friend bool operator>=(const fixed a, const fixed b) { return a.v >= b.v; }
	friend bool operator<=(const fixed a, const fixed b) { return a.v <= b.v; }

	/* implicit operator float() bad */
	Sint64 ToInt64() const { return v>>FRAC; }
	float ToFloat() const { return v/(float)(1<<FRAC); }
	double ToDouble() const { return v/(double)(1<<FRAC); }

	Sint64 v;
};

#endif /* _FIXED_H */
