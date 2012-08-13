#include <assert.h>
#include "softfloat.h"

void sfloat::FromSint64(Sint64 a)
{
	if (!a) { exp = 0; mant = 0; return; }				// zero special case
	exp = EXPOFFSET + 31;
	if (a < 0) { exp |= SIGNMASK; a = -a; }				// note, fails on a = LONG_MIN
	if (a < ((Sint64)1<<32)) {							// normalize mantissa
		mant = (Uint32)a;
		if (mant < ((Uint32)1<<16)) { mant <<= 16; exp -= 16; }
		if (mant < ((Uint32)1<<24)) { mant <<= 8; exp -= 8; }
		if (mant < ((Uint32)1<<28)) { mant <<= 4; exp -= 4; }
		if (mant < ((Uint32)1<<30)) { mant <<= 2; exp -= 2; }
		if (mant < ((Uint32)1<<31)) { mant <<= 1; exp -= 1; }
	}
	else {
		a >>= 1; exp += 1;
		if (a >= ((Sint64)1<<47)) { a >>= 16; exp += 16; }
		if (a >= ((Sint64)1<<39)) { a >>= 8; exp += 8; }
		if (a >= ((Sint64)1<<35)) { a >>= 4; exp += 4; }
		if (a >= ((Sint64)1<<33)) { a >>= 2; exp += 2; }
		if (a >= ((Sint64)1<<32)) { a >>= 1; exp += 1; }
		mant = (Uint32)a;
	}
}

void sfloat::FromDouble(double a)
{
	Uint64 *p = (Uint64 *)&a;
	exp = (Uint32)((*p>>52) & 0x7ff);
	if (!exp) { exp = 0; mant = 0; return; }					// zero/denormal special case
	exp = (exp + EXPOFFSET - 0x3ff) | (Uint32)(*p>>32) & (1<<31);	// exp | sign
	mant = (Uint32)(*p>>21) | (1<<31);								// add hidden bit
}

double sfloat::ToDouble() const
{
	Uint32 e = exp & EXPMASK;
	assert(e < EXPOFFSET+1024);						// outside double range
	if (!mant || e < EXPOFFSET-1022) return 0.0;
	Uint64 r = (Uint64)(mant & ~(1<<31)) << 21;
	r |= (Uint64)(e + 0x3ff - EXPOFFSET) << 52;
	r |= ((Uint64)(exp & (1<<31)) << 32);
	return *(double *)&r;
}

float sfloat::ToFloat() const
{
	Uint32 e = exp & EXPMASK;
	assert(e < EXPOFFSET+128);						// outside float range
	if (!mant || e < EXPOFFSET-126) return 0.0f;
	Uint32 r = (mant & ~(1<<31)) >> 8;
	r |= (e + 0x7f - EXPOFFSET) << 23;
	r |= (exp & (1<<31));
	return *(float *)&r;
}

Sint64 sfloat::ToInt64() const
{
	Uint32 e = exp & EXPMASK; Sint64 r = mant;
	assert(e < EXPOFFSET + 63);						// sint64 overflow
	if (EXPOFFSET > e) return 0;					// avoid C99 oversized right shifts
	if (e <= EXPOFFSET + 31) r >>= (31 + EXPOFFSET - e);
	else r <<= (e - EXPOFFSET - 31);
	if (exp & SIGNMASK) return -r;
	else return r;
}

Sint32 sfloat::ToInt32() const
{
	Uint32 e = exp & EXPMASK;
	assert(e < EXPOFFSET + 31);						// sint32 overflow
	if (EXPOFFSET > e) return 0;					// avoid C99 oversized right shifts
	Sint32 r = mant >> (31 + EXPOFFSET - e);
	if (exp & SIGNMASK) return -r;
	else return r;
}


bool sfloat::AbsLess(const sfloat &b) const
{
	Uint32 expA = exp & EXPMASK, expB = b.exp & EXPMASK;
	return expA < expB || (expA == expB && mant < b.mant);
}

bool sfloat::operator>(const sfloat &b) const
{
	if ((exp ^ b.exp) & SIGNMASK) return !!(b.exp & SIGNMASK);	
	if (exp & SIGNMASK) return AbsLess(b);
	else return b.AbsLess(*this);
}

bool sfloat::operator<(const sfloat &b) const
{
	if ((exp ^ b.exp) & SIGNMASK) return !!(exp & SIGNMASK);
	if (exp & SIGNMASK) return b.AbsLess(*this);
	else return AbsLess(b);
}


sfloat sfloat::operator*(const sfloat &b) const
{
	Uint32 rexp = (exp & EXPMASK) - EXPOFFSET + (b.exp & EXPMASK);
	if (rexp & (1<<31)) { return sfloat(0, 0); }			// underflow to zero
	assert(rexp < EXPMASK-1);									// potential overflow

	Uint64 rmant = (Uint64)mant * b.mant;
	if (rmant & ((Uint64)1<<63)) { rexp++; rmant >>= 32; }
	else rmant >>= 31;

	rexp |= (exp ^ b.exp) & SIGNMASK;
	if (!rmant) rexp = 0;			// special-cased zero

	return sfloat(rexp, (Uint32)rmant);
}

sfloat sfloat::operator/(const sfloat &b) const
{
	Uint32 rexp = (exp & EXPMASK) + EXPOFFSET - (b.exp & EXPMASK);
	if (rexp & (1<<31)) { return sfloat(0, 0); }			// underflow to zero
	assert(rexp < EXPMASK);										// potential overflow
	assert(b.mant);												// divide by zero

	Uint64 rmant = (((Uint64)mant)<<31) / b.mant;
	if (!(rmant & (1<<31))) { rexp--; rmant <<= 1; }

	rexp |= (exp ^ b.exp) & SIGNMASK;
	if (!rmant) rexp = 0;			// special-cased zero

	return sfloat(rexp, (Uint32)rmant);
}

// worker func for +/-. a >= b?
sfloat sfloat::SubWorker(const sfloat &b) const
{
	Uint32 rexp = exp & EXPMASK;
	Uint32 expdiff = rexp - (b.exp & EXPMASK);
	Uint32 rmant = mant - (b.mant >> expdiff);

	if (!expdiff) {
		// now could have any number of zeros
		if (rmant < (1<<16)) { rmant <<= 16; rexp -= 16; }
		if (rmant < (1<<24)) { rmant <<= 8; rexp -= 8; }
		if (rmant < (1<<28)) { rmant <<= 4; rexp -= 4; }
		if (rmant < (1<<30)) { rmant <<= 2; rexp -= 2; }
	}

	if (!(rmant & (1<<31))) { rmant <<= 1; rexp--; }
	if (rexp & (1<<31) || !rmant) return sfloat(0,0);			// underflow
	return sfloat(rexp, rmant);
}

// always a >= b, both positive
sfloat sfloat::AddWorker(const sfloat &b) const
{
	Uint32 rexp = exp & EXPMASK;
	Uint32 expdiff = rexp - (b.exp & EXPMASK);
	Uint64 rmant = (Uint64)mant + (b.mant >> expdiff);

	if (rmant & ((Uint64)1<<32)) { rmant >>= 1; rexp++; }
	assert(rexp <= EXPMASK);										// overflow
	return sfloat(rexp, (Uint32)rmant);
}

sfloat sfloat::operator+(const sfloat &b) const
{
	Uint32 signA = exp & SIGNMASK, signB = b.exp & SIGNMASK;
	Uint32 expA = exp & EXPMASK, expB = b.exp & EXPMASK;

	if (signA ^ signB) {
		if (expA > expB || (expA == expB && mant > b.mant))
			return SubWorker(b).SetSign(signA);
		else return b.SubWorker(*this).SetSign(signB);
	}
	else {
		if (expA > expB) return AddWorker(b).SetSign(signA);
		else return b.AddWorker(*this).SetSign(signB);
	}
}

sfloat sfloat::operator-(const sfloat &b) const
{
	if (!b.mant) return *this;							// don't flip zeros
	sfloat bflip = sfloat(b.exp ^ SIGNMASK, b.mant);
	return *this + bflip;
}


sfloat sfloat::Sqrt() const
{
	const unsigned char sqrttable[64] = {
		0x01, 0x05, 0x09, 0x0d, 0x11, 0x15, 0x18, 0x1c, 0x20, 0x23, 0x27, 0x2a, 0x2d, 0x31, 0x34, 0x37,
		0x3b, 0x3e, 0x41, 0x44, 0x47, 0x4b, 0x4e, 0x51, 0x54, 0x57, 0x5a, 0x5d, 0x60, 0x62, 0x65, 0x68,
		0x6c, 0x72, 0x77, 0x7d, 0x82, 0x87, 0x8d, 0x92, 0x97, 0x9c, 0xa1, 0xa6, 0xaa, 0xaf, 0xb4, 0xb9,
		0xbd, 0xc2, 0xc6, 0xcb, 0xcf, 0xd4, 0xd8, 0xdc, 0xe1, 0xe5, 0xe9, 0xed, 0xf1, 0xf5, 0xf9, 0xfd,
	};	

	assert(!(exp & SIGNMASK));										// negative sqrt
	if (!mant) return sfloat(0,0);									// arguably unnecessary
	Uint32 index = ((mant >> 26) & 0x1f) | ((exp & 1) << 5);
	Uint32 rmant = (sqrttable[index] << 23) | (1<<31);
	Uint32 rexp = ((exp & EXPMASK) >> 1) + (EXPOFFSET >> 1);

	sfloat r = sfloat(rexp, rmant);
	const sfloat sfhalf = sfloat(EXPOFFSET-1, (1<<31));

	r = (r + *this/r) * sfhalf;
	r = (r + *this/r) * sfhalf;		// NR steps
	return r;
}

sfloat sfloat::CubeRoot() const
{
	const unsigned char cuberttable[96] = {
		0x01, 0x03, 0x06, 0x09, 0x0b, 0x0d, 0x10, 0x12, 0x14, 0x17, 0x19, 0x1b, 0x1d, 0x1f, 0x21, 0x24,
		0x26, 0x28, 0x2a, 0x2c, 0x2d, 0x2f, 0x31, 0x33, 0x35, 0x37, 0x39, 0x3a, 0x3c, 0x3e, 0x40, 0x41,
		0x44, 0x47, 0x4a, 0x4d, 0x51, 0x54, 0x57, 0x59, 0x5c, 0x5f, 0x62, 0x65, 0x68, 0x6a, 0x6d, 0x6f,
		0x72, 0x75, 0x77, 0x79, 0x7c, 0x7e, 0x81, 0x83, 0x85, 0x88, 0x8a, 0x8c, 0x8e, 0x91, 0x93, 0x95,
		0x98, 0x9c, 0xa0, 0xa4, 0xa8, 0xac, 0xb0, 0xb3, 0xb7, 0xbb, 0xbe, 0xc2, 0xc5, 0xc8, 0xcc, 0xcf,
		0xd2, 0xd5, 0xd9, 0xdc, 0xdf, 0xe2, 0xe5, 0xe8, 0xeb, 0xee, 0xf0, 0xf3, 0xf6, 0xf9, 0xfb, 0xfe,
	};	

	if (!mant) return sfloat(0,0);									// arguably unnecessary
	Uint32 index = ((mant >> 26) & 0x1f) | ((exp % 3) << 5);
	Uint32 rmant = (cuberttable[index] << 23) | (1<<31);
	Uint32 rexp = ((exp & EXPMASK) / 3) + (2 * EXPOFFSET / 3);

	sfloat r = sfloat(rexp, rmant);
	const sfloat sfthird = sfloat(1/3.0);

	r = (r.Shift(1) + *this/(r*r)) * sfthird;
	r = (r.Shift(1) + *this/(r*r)) * sfthird;	// NR steps
	r.exp |= exp & SIGNMASK;
	return r;
}
