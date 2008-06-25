#ifndef __SBRE_ANIM_H__
#define __SBRE_ANIM_H__
#include "jjtypes.h"
#include "jjvector.h"
#include "sbre.h"
#include "sbre_int.h"

enum animmod
{
	AMOD_CLIP = 0,		// just clip result to 0-1
	AMOD_MOD1,			// fmod(1), then clip
	AMOD_REF,			// fmod(2), reflect around 1, then clip
};

struct AnimFunc
{
	int src;
	int mod;
	float order0;
	float order1;
	float order2;
	float order3;
};

enum animfunc
{
	AFUNC_GEAR = 0,
	AFUNC_GFLAP,
	AFUNC_THRUSTPULSE,
	AFUNC_LIN4SEC,
};

const AnimFunc pAFunc[] =
{
	{ ASRC_GEAR, AMOD_CLIP, -1.0f, 2.0f, 0.0f, 0.0f },
	{ ASRC_GEAR, AMOD_CLIP, 0.0f, 2.0f, 0.0f, 0.0f },
	{ ASRC_MINFRAC, AMOD_REF, 0.0f, 30.0f, 0.0f, 0.0f },
	{ ASRC_MINFRAC, AMOD_MOD1, 0.0f, 15.0f, 0.0f, 0.0f },
};



#endif // __SBRE_ANIM_H__