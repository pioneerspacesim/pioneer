// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

// A deterministic random number generator for use by the rest of the
// engine. It *should* give the same output for a given input regardless
// of the processor architecture. If it doesn't then it needs to be fixed!

// Based on: http://www.javamex.com/tutorials/random_numbers/xorshift.shtml

#ifndef RAND_H
#define RAND_H

#include <assert.h>
#include <cmath>
#include <cstdint>
#include <initializer_list>

#include "RefCounted.h"
#include "fixed.h"

extern "C" {
#include "jenkins/lookup3.h"
}

#define PCG_LITTLE_ENDIAN 1
#include "pcg-cpp/pcg_random.hpp"

// A deterministic random number generator
class Random : public RefCounted {
	pcg32 mPCG;

	// For storing second rand from Normal
	bool cached;
	double z1;

public:
	//
	// Constructors
	//

	// Construct a new random generator using the given seed
	Random(const Uint32 initialSeed = 0xabcd1234)
	{
		seed(initialSeed);
	}

	// Construct a new generator given an array of 32-bit seeds.
	Random(const Uint32 *const seeds, size_t length)
	{
		seed(seeds, length);
	}

	// Construct a new random generator from an array of 64-bit
	// seeds.
	Random(const Uint64 *const seeds, size_t length)
	{
		seed(reinterpret_cast<const Uint32 *>(seeds), length * 2);
	}

	// Construct a new generator given an array of 32-bit seeds.
	Random(std::initializer_list<uint32_t> seeds)
	{
		seed(seeds);
	}

	//
	// Seed functions
	//

	// Seed the RNG using the hash of the given array of seeds.
	void seed(const Uint32 *const seeds, size_t length)
	{
		const Uint32 hash = lookup3_hashword(seeds, length, 0);
		mPCG.seed(hash);
		cached = false;
	}

	// Seed using an array of 64-bit integers
	void seed(const Uint64 *const seeds, size_t length)
	{
		seed(reinterpret_cast<const Uint32 *>(seeds), length * 2);
	}

	// Seed using an initializer_list of 32-bit integers
	void seed(std::initializer_list<uint32_t> list) {
		seed(&*list.begin(), list.size());
	}

	// Seed using a single 32-bit integer
	void seed(const Uint32 value)
	{
		seed(&value, 1);
	}

	//
	// Number generators.
	//
	// Starting from a given seed value the generator will return the
	// same sequence of numbers. Unless otherwise stated the numbers
	// are 32-bit and each call consumes one
	//

	// Get the next integer from the sequence
	// interval [0, 2**32)
	inline Uint32 Int32()
	{
		return mPCG();
	}

	// Pick an integer like you're rolling a "choices" sided die,
	// a 6 sided die would return a number between 0 and 5.
	// interval [0, choices)
	inline Uint32 Int32(const int choices)
	{
		return Int32() % choices;
	}

	// Pick a number between min and max, inclusive.
	// interval [min, max]
	inline int Int32(const int min, const int max)
	{
		return (Int32() % (1 + max - min)) + min;
	}

	// Pick a number in the half open interval [0, 1)
	inline double Double()
	{
		return double(Int32()) * (1. / 4294967296.); // divided by 2^32
	}

	// Pick a number in the closed interval [0, 1]
	inline double Double_closed()
	{
		return double(Int32()) * (1. / 4294967295.); // divided by 2^32 - 1
	}

	// Pick a number in the open interval (0, 1)
	inline double Double_open()
	{
		return (double(Int32()) + .5) * (1. / 4294967296.); // divided by 2^32
	}

	// Pick a 53-bit resolution double in the half open interval [0, 1)
	// This method consumes two 32-bit numbers from the sequence.
	inline double Double53()
	{
		return (double(Int32() >> 5) * 67108864. + double(Int32() >> 6)) * (1. / 9007199254740992.);
	}

	// Pick a number in the half-open interval [min, limit)
	inline double Double(double min, double limit)
	{
		return Double(limit - min) + min;
	}

	// Pick a number in the half-open interval [0, limit)
	inline double Double(double limit)
	{
		return limit * Double();
	}

	// Pick a number between 0 and max inclusive
	// interval [0, max]
	inline double Double_closed(double max)
	{
		return max * Double_closed();
	}

	// Pick a number between min and max inclusive
	// interval [min, max]
	inline double Double_closed(double min, double max)
	{
		return Double_closed(max - min) + min;
	}

	// interval [0,1)
	inline double NDouble(int p)
	{
		double o = Double(1.0);
		while (--p > 0)
			o *= Double(1.0);
		return o;
	}

	// Normal distribution with zero mean, and unit variance
	inline double Normal()
	{
		return Normal(0.0, 1.0);
	}

	// Normal distribution with unit variance
	inline double Normal(const double mean)
	{
		return Normal(mean, 1.0);
	}

	//Normal (Gauss) distribution
	inline double Normal(const double mean, const double stddev)
	{
		//https://en.wikipedia.org/wiki/Box-Muller_transform#Polar_form
		double u, v, s, z0;

		if (cached) {
			z0 = z1;
			cached = false;
		} else {
			do {
				u = Double_closed(-1, 1);
				v = Double_closed(-1, 1);
				s = u * u + v * v;
			} while (s >= 1.0);

			s = sqrt((-2.0 * log(s)) / s);
			z0 = u * s;
			z1 = v * s;
			cached = true;
		}

		return mean + z0 * stddev;
	}

	inline int Poisson(const double lambda)
	{
		int k = 0;
		double p = Double();
		const double target = exp(-lambda);
		while (p > target) {
			k += 1;
			p *= Double();
		}
		return k;
	}

	// Pick a fixed-point integer half open interval [0,1)
	inline fixed Fixed()
	{
		return fixed(Int32());
	}

	// interval [0,1)
	inline fixed NFixed(int p)
	{
		fixed o = Fixed();
		while (--p > 0)
			o *= Fixed();
		return o;
	}

	// Returns an approximation of a normal distribution in the bounded interval
	// (-1, 1)
	inline fixed NormFixed()
	{
		// Because addition is fully commutative, the compiler can order these
		// Fixed() calls in any order it wants without changing the result
		fixed o = Fixed() + Fixed() + Fixed();
		return o * fixed(10, 15) - fixed(1, 1);
	}

	// interval (mean - maxdev, mean + maxdev)
	// this is an approximation of a gaussian distribution with cross-platform determinism
	inline fixed NormFixed(fixed mean, fixed maxdev)
	{
		return mean + maxdev * NormFixed();
	}

	const pcg32 &GetPCG() const { return mPCG; }

private:
	Random(const Random &); // copy constructor not defined
	void operator=(const Random &); // assignment operator not defined
};

#endif // RAND_H
