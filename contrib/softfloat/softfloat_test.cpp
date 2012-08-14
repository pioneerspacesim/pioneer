#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "softfloat.h"

// basic tests
int main()
{
	// zeros
	sfloat sfz = sfloat(0.0);
	sfloat sf = sfloat(0);
	assert(sf == sfz);
	sf = sfloat(561259) - sfloat(561259);
	assert(sf == sfz);
	sf = sfloat(561259) + sfloat(-561259);
	assert(sf == sfz);
	sf = sfloat(3135746) * sfz;
	assert(sf == sfz);
	sf = sfz / sfloat(235326);
	assert(sf == sfz);
	sf = sfz.Sqrt();
	assert(sf == sfz);
	sf = sfz.CubeRoot();
	assert(sf == sfz);

	sf = sfloat(352345);
	assert(sf == sf + sfloat(0));
	assert(sf == sf - sfloat(0));
	assert(sf == sfloat(0) + sf);
	assert(-sf == sfloat(0) - sf);
	assert(-sf == -sf + sfloat(0));
	assert(-sf == -sf - sfloat(0));

	printf("All zero tests passed\n");

	// integer load/store
	Sint64 i64 = 0x56000000000; assert(i64 == sfloat(i64).ToInt64());
	i64 = -0x23000000000; assert(i64 == sfloat(i64).ToInt64());
	i64 = 0x560000; assert(i64 == sfloat(i64).ToInt64());
	i64 = -0x230000; assert(i64 == sfloat(i64).ToInt64());
	i64 = 0x560000; assert(i64 == sfloat(i64).ToInt32());
	i64 = -0x230000; assert(i64 == sfloat(i64).ToInt32());
	i64 = 0x7fffffff00000000; assert(i64 == sfloat(i64).ToInt64());
	i64 = -0x7fffffff00000000; assert(i64 == sfloat(i64).ToInt64());
	i64 = 0x40000000; assert(i64 == sfloat(i64).ToInt64());
	i64 = 0x80000000; assert(i64 == sfloat(i64).ToInt64());
	i64 = 0x100000000; assert(i64 == sfloat(i64).ToInt64());

	printf("Integer load/store tests passed\n");

	double d = (double)0x1; assert(d == sfloat(d).ToDouble());
	d = (double)0x2345678; assert(d == sfloat(d).ToDouble());
	d = (double)0x2345678 / 0x10000000; assert(d == sfloat(d).ToDouble());

	float f = (float)0x1; assert(f == sfloat(f).ToFloat());
	f = (float)0x2345678; assert(f == sfloat(f).ToFloat());
	f = (float)0x2345678 / 0x10000000; assert(f == sfloat(f).ToFloat());

	printf("Floating point load/store tests passed\n");
	
	assert(sfloat(2.0) > sfloat(1.01));
	assert(sfloat(1.01) > sfloat(1.0));
	assert(sfloat(1.0) < sfloat(1.01));
	assert(sfloat(-1.01) < sfloat(-1.0));
	assert(sfloat(1.0) > sfloat(-1.01));
	assert(sfloat(-1.0) < sfloat(1.0));

	printf("Comparison tests passed\n");	

	assert((sfloat(1.0) + sfloat(0.25)).ToDouble() == 1.25);		// sign tests
	assert((sfloat(1.0) - sfloat(0.25)).ToDouble() == 0.75);
	assert((sfloat(1.0) + sfloat(-0.25)).ToDouble() == 0.75);
	assert((sfloat(1.0) - sfloat(-0.25)).ToDouble() == 1.25);
	assert((sfloat(-1.0) + sfloat(0.25)).ToDouble() == -0.75);
	assert((sfloat(-1.0) - sfloat(0.25)).ToDouble() == -1.25);
	assert((sfloat(-1.0) + sfloat(-0.25)).ToDouble() == -1.25);
	assert((sfloat(-1.0) - sfloat(-0.25)).ToDouble() == -0.75);

	// normalization
	i64 = 0x80000000;
	assert((sfloat(i64) + sfloat(i64)).ToDouble() == 4294967296.0);
	assert((sfloat(i64+1) - sfloat(i64)).ToDouble() == 1.0);

	printf("Add/sub tests passed\n");

	assert((sfloat(0.5) * sfloat(2.0)).ToDouble() == 1.0);
	assert((sfloat(2.0) * sfloat(0.5)).ToDouble() == 1.0);
	assert((sfloat(2.0) * sfloat(-0.5)).ToDouble() == -1.0);
	assert((sfloat(-2.0) * sfloat(0.5)).ToDouble() == -1.0);
	assert((sfloat(-2.0) * sfloat(-0.5)).ToDouble() == 1.0);

	assert((sfloat(0.5) / sfloat(2.0)).ToDouble() == 0.25);
	assert((sfloat(2.0) / sfloat(0.5)).ToDouble() == 4.0);
	assert((sfloat(2.0) / sfloat(-0.5)).ToDouble() == -4.0);
	assert((sfloat(-2.0) / sfloat(0.5)).ToDouble() == -4.0);
	assert((sfloat(-2.0) / sfloat(-0.5)).ToDouble() == 4.0);
	
	// normalization
	assert((sfloat(1.5) * sfloat(1.5)).ToDouble() == 2.25);
	assert((sfloat(1.0) / sfloat(1.5)).ToDouble() > 0.666666);
	assert((sfloat(1.0) / sfloat(1.5)).ToDouble() < 0.666667);
	
	printf("Mul/div tests passed\n");

	printf ("Sqrt test:\n");	
	sfloat sf1 = sfloat(1.44).Sqrt();
	printf ("calc1 = %.9f\ncalc2 = %.9f\n", sf1.ToDouble(), pow(1.44, 1/2.0));
	sf1 = sfloat(1.56).Sqrt();
	printf ("calc1 = %.9f\ncalc2 = %.9f\n", sf1.ToDouble(), pow(1.56, 1/2.0));
	sf1 = sfloat(1.89).Sqrt();
	printf ("calc1 = %.9f\ncalc2 = %.9f\n", sf1.ToDouble(), pow(1.89, 1/2.0));
	sf1 = sfloat(2.45).Sqrt();
	printf ("calc1 = %.9f\ncalc2 = %.9f\n", sf1.ToDouble(), pow(2.45, 1/2.0));

	printf ("CubeRoot test:\n");	
	sf1 = sfloat(1.44).CubeRoot();
	printf ("calc1 = %.9f\ncalc2 = %.9f\n", sf1.ToDouble(), pow(1.44, 1/3.0));
	sf1 = sfloat(1.56).CubeRoot();
	printf ("calc1 = %.9f\ncalc2 = %.9f\n", sf1.ToDouble(), pow(1.56, 1/3.0));
	sf1 = sfloat(1.89).CubeRoot();
	printf ("calc1 = %.9f\ncalc2 = %.9f\n", sf1.ToDouble(), pow(1.89, 1/3.0));
	sf1 = sfloat(2.45).CubeRoot();
	printf ("calc1 = %.9f\ncalc2 = %.9f\n", sf1.ToDouble(), pow(2.45, 1/3.0));
	sf1 = sfloat(3.72).CubeRoot();
	printf ("calc1 = %.9f\ncalc2 = %.9f\n", sf1.ToDouble(), pow(3.72, 1/3.0));
	sf1 = sfloat(4.91).CubeRoot();
	printf ("calc1 = %.9f\ncalc2 = %.9f\n", sf1.ToDouble(), pow(4.91, 1/3.0));
	sf1 = sfloat(7.13).CubeRoot();
	printf ("calc1 = %.9f\ncalc2 = %.9f\n", sf1.ToDouble(), pow(7.13, 1/3.0));

	// string initialization test

	printf ("String initialization test:\n");
	printf ("calc1 = %.9f\ncalc2 = %.9f\n", sfloat("1.").ToDouble(), 1.);
	printf ("calc1 = %.9f\ncalc2 = %.9f\n", sfloat("23.0029").ToDouble(), 23.0029);
	printf ("calc1 = %.9f\ncalc2 = %.9f\n", sfloat("-38200.15008").ToDouble(), -38200.15008);
	printf ("calc1 = %.9f\ncalc2 = %.9f\n", sfloat("1.67e-4").ToDouble(), 1.67e-4);
	printf ("calc1 = %.9f\ncalc2 = %.9f\n", sfloat("5.28e17").ToDouble(), 5.28e17);


	getchar();
	return 0;
}

/*
// table generator
int main()
{
	// sqrt

	// 1-1.999, 32 steps
	for (int i=0; i<32; i++) printf("0x%x, ", (Uint32)((sqrt((65+2*i) / 64.0) - 1.0) * 256.0) );
	printf("\n");
	// 2-3.999, 32 steps
	for (int i=0; i<32; i++) printf("0x%x, ", (Uint32)((sqrt((130+4*i) / 64.0) - 1.0) * 256.0) );
	printf("\n");

	// 1-1.999, 32 steps
	for (int i=0; i<32; i++) printf("%.3f, ", sqrt((65+2*i) / 64.0) );
	printf("\n");
	// 2-3.999, 32 steps
	for (int i=0; i<32; i++) printf("%.3f, ", sqrt((130+4*i) / 64.0) );
	printf("\n");

	// cuberoot

	// 1-1.999, 32 steps
	for (int i=0; i<32; i++) printf("0x%x, ", (Uint32)((pow((65+2*i) / 64.0, 1/3.0) - 1.0) * 256.0) );
	printf("\n");
	// 2-3.999, 32 steps
	for (int i=0; i<32; i++) printf("0x%x, ", (Uint32)((pow((130+4*i) / 64.0, 1/3.0) - 1.0) * 256.0) );
	printf("\n");
	// 4-7.999, 32 steps
	for (int i=0; i<32; i++) printf("0x%x, ", (Uint32)((pow((260+8*i) / 64.0, 1/3.0) - 1.0) * 256.0) );
	printf("\n");

	// 1-1.999, 32 steps
	for (int i=0; i<32; i++) printf("%.3f, ", pow((65+2*i) / 64.0, 1/3.0) );
	printf("\n");
	// 2-3.999, 32 steps
	for (int i=0; i<32; i++) printf("%.3f, ", pow((130+4*i) / 64.0, 1/3.0) );
	printf("\n");
	// 4-7.999, 32 steps
	for (int i=0; i<32; i++) printf("%.3f, ", pow((260+8*i) / 64.0, 1/3.0) );
	printf("\n");

	getchar();
}
*/
