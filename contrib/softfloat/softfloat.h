#ifndef _SOFTFLOAT_H
#define _SOFTFLOAT_H
#include <SDL_stdinc.h>


class sfloat
{
  private:

	const static Uint32 SIGNMASK = 0x80000000;
	const static Uint32 EXPMASK = 0x7fffff;
	const static Uint32 EXPOFFSET = 0x400002;		// multiple of 2 and 3 for easier sqrt/cuberoot
	
	Uint32 mant, exp;

	sfloat(Uint32 _exp, Uint32 _mant) { exp = _exp; mant = _mant; }
	sfloat() {}
	void FromDouble(double a);
	void FromSint64(Sint64 a);

	sfloat &SetSign(Uint32 sign) { if(mant) exp |= sign; return *this; }
	sfloat SubWorker(const sfloat &b) const;
	sfloat AddWorker(const sfloat &b) const;
	bool AbsLess(const sfloat &b) const;
	
  public:

	sfloat(double a) { FromDouble(a); }
	sfloat(float a) { FromDouble((double)a); }
	sfloat(Sint64 a) { FromSint64(a); }
	sfloat(int a) { FromSint64((Sint64)a); }

	double ToDouble() const;
	float ToFloat() const;
	Sint32 ToInt32() const;
	Sint64 ToInt64() const;

	bool operator==(const sfloat &b) const { return exp == b.exp && mant == b.mant; }
	bool operator!=(const sfloat &b) const { return exp != b.exp || mant != b.mant; }
	bool operator>(const sfloat &b) const;
	bool operator<(const sfloat &b) const;
	bool operator>=(const sfloat &b) const { return *this > b || *this == b; }
	bool operator<=(const sfloat &b) const { return *this < b || *this == b; }

	sfloat operator+(const sfloat &b) const;
	sfloat operator-(const sfloat &b) const;
	sfloat operator*(const sfloat &b) const;
	sfloat operator/(const sfloat &b) const;

	sfloat operator-() const {
		if (mant) return sfloat(exp ^ SIGNMASK, mant);
		else return sfloat(0,0);
	}

	sfloat Abs() const { return sfloat(exp & ~SIGNMASK, mant); }
	sfloat Shift(int s) const { return sfloat(exp+s, mant); }

	sfloat Sqrt() const;
	sfloat CubeRoot() const;
};


#endif
