// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include <iostream>
#include <math.h>
#include "Random.h"

using namespace std;

// Test suite for random number generator
void test_random() {

	// The deterministic pseudo-random sequence generator
	// must produce the same numbers on all platforms.

	const Uint32 in32[]       = {1, 56789, 71, 63, 0xbadf00d};
	const Uint32 in32out32[]  = {103039557, 3621827031, 1026123746, 2947867049, 4032124254};
	const double in32outDbl[] = {0.0297203, 0.313619, 0.888806, 0.717738, 0.0670876};

	const Uint64 in64[]       = {0xdeadbeefbadf000d, 23976134785912, 1, 2, 1};
	const Uint32 in64out32[]  = {3697722658, 2452550324, 2683066595, 3947765649, 1497178833};
	const double in64outDbl[] = {0.560166, 0.684969, 0.318901, 0.699317, 0.277869};

	cout << "--------------------" << endl;
	cout << "Running random tests" << endl;
	cout << "--------------------" << endl;

	// seed the random number generator with the sequence above
	Random rnd(in32, 5);

	cout << "32-bit seeds, Uint32 out: " << endl;
	for (int i=0; i<5; ++i) cout << i+1 << ": " << (rnd.Int32() - in32out32[i] == 0 ? "pass" : "fail") << endl;

	cout << "32-bit seeds, Double out: " << endl;
	for (int i=0; i<5; ++i) cout << i+1 << ": " << (fabs(rnd.Double() - in32outDbl[i]) < 0.000001 ? "pass" : "fail") << endl; // close enough

	// now seed with 64-bit numbers
	rnd.seed(in64, 5);

	cout << "64-bit seeds, Uint32 out: " << endl;
	for (int i=0; i<5; ++i) cout << i+1 << ": " << (rnd.Int32() - in64out32[i] == 0 ? "pass" : "fail") << endl;

	cout << "64-bit seeds, Double out: " << endl;
	for (int i=0; i<5; ++i) cout << i+1 << ": " << (fabs(rnd.Double() - in64outDbl[i]) < 0.000001 ? "pass" : "fail") << endl; // close enough

	cout << "--------------------" << endl;
	cout << "End of random tests." << endl;
	cout << "--------------------" << endl;
}
