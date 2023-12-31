// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "perlin.h"
#include "Random.h"
#include "matrix4x4.h"
#include "vector3.h"
#include <stdio.h>
#include <vector>

double testfunc()
{
	Random rng;
	double r = 0.0;
	for (int i = 0; i < 1000; i++)
		r += noise(1000.0 * rng.Double(), 1000.0 * rng.Double(), 1000.0 * rng.Double());
	return r;
}

double testfunc2()
{
	Random rng;
	double r = 0.0;
	for (int i = 0; i < 1000; i++) {
		r += 1000.0 * rng.Double();
		r += 1000.0 * rng.Double();
		r += 1000.0 * rng.Double();
	}
	return r;
}

volatile int ReadTSC()
{
	int tsc;
	__asm {
		rdtsc
		mov tsc, eax
	}
	return tsc;
}

#pragma optimize("", off)
int main(int argc, char **argv)
{
	int t1, t2, t3, t4;
	double r = 0.0;
	t1 = ReadTSC();
	r += testfunc();
	t2 = ReadTSC();
	r += testfunc();
	t3 = ReadTSC();
	r += testfunc();
	t4 = ReadTSC();

	printf("Times: %i, %i, %i: result %f\n", t2 - t1, t3 - t2, t4 - t3, r);

	t1 = ReadTSC();
	r += testfunc2();
	t2 = ReadTSC();
	r += testfunc2();
	t3 = ReadTSC();
	r += testfunc2();
	t4 = ReadTSC();

	printf("Times: %i, %i, %i: result %f\n", t2 - t1, t3 - t2, t4 - t3, r);

	getchar();

	return 0;
}
#pragma optimize("", on)
