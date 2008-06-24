#ifndef __FASTMATH_H__
#define __FASTMATH_H__


extern int g_pInvTable[64];
extern int g_pInvTableLow[64];
extern int g_pInvTableHigh[64];

extern int g_pSqrtTable[64];
extern int g_pSqrtTableLow[64];
extern int g_pSqrtTableHigh[64];

extern int g_pISqrtTable[64];
extern int g_pISqrtTableLow[64];
extern int g_pISqrtTableHigh[64];


inline float FastInv (float f)
{
	int exp = 0x7e800000 - (0x7f800000 & *(int *)&f);
	int mant = g_pInvTable[(*(int *)&f >> 17) & 0x3f];
	int combo = mant | exp | (0x80000000 & *(int *)&f);
	return *(float *)&combo;
}
inline float FastInvLow (float f)
{
	int exp = 0x7e800000 - (0x7f800000 & *(int *)&f);
	int mant = g_pInvTableLow[(*(int *)&f >> 17) & 0x3f];
	int combo = mant | exp | (0x80000000 & *(int *)&f);
	return *(float *)&combo;
}
// Note: fractionally off maximum for 2^n input
inline float FastInvHigh (float f)
{
	int exp = 0x7e800000 - (0x7f800000 & *(int *)&f);
	int mant = g_pInvTableHigh[(*(int *)&f >> 17) & 0x3f];
	int combo = mant | exp | (0x80000000 & *(int *)&f);
	return *(float *)&combo;
}

inline float FastSqrt (float f)
{
	int exp = 0x7f800000 & ((*(int *)&f >> 1) + 0x1fc00000);
	int mant = g_pSqrtTable[(*(int *)&f >> 18) & 0x3f] | exp;
	return *(float *)&mant;
}
inline float FastSqrtLow (float f)
{
	int exp = 0x7f800000 & ((*(int *)&f >> 1) + 0x1fc00000);
	int mant = g_pSqrtTableLow[(*(int *)&f >> 18) & 0x3f] | exp;
	return *(float *)&mant;
}
inline float FastSqrtHigh (float f)
{
	int exp = 0x7f800000 & ((*(int *)&f >> 1) + 0x1fc00000);
	int mant = g_pSqrtTableHigh[(*(int *)&f >> 18) & 0x3f] | exp;
	return *(float *)&mant;
}

inline float FastInvSqrt (float f)
{
	int exp = 0x5e800000 - ((*(int *)&f >> 1) & 0x3f800000);
	int mant = g_pISqrtTable[(*(int *)&f >> 18) & 0x3f] + exp;
	return *(float *)&mant;
}
inline float FastInvSqrtLow (float f)
{
	int exp = 0x5e800000 - ((*(int *)&f >> 1) & 0x3f800000);
	int mant = g_pISqrtTableLow[(*(int *)&f >> 18) & 0x3f] + exp;
	return *(float *)&mant;
}
inline float FastInvSqrtHigh (float f)
{
	int exp = 0x5e800000 - ((*(int *)&f >> 1) & 0x3f800000);
	int mant = g_pISqrtTableHigh[(*(int *)&f >> 18) & 0x3f] + exp;
	return *(float *)&mant;
}


inline void VecNormFast (Vector *v1, Vector *res)
{
	float temp = FastInvSqrt (VecDot (v1, v1));
	res->x = v1->x * temp;
	res->y = v1->y * temp;
	res->z = v1->z * temp;
}


#endif