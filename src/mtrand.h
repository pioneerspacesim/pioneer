// mtrand.h
// C++ include file for MT19937, with initialization improved 2002/1/26.
// Coded by Takuji Nishimura and Makoto Matsumoto.
// Ported to C++ by Jasper Bedaux 2003/1/1 (see http://www.bedaux.net/mtrand/).
// The generators returning floating point numbers are based on
// a version by Isaku Wada, 2002/01/09
//
// Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// 3. The names of its contributors may not be used to endorse or promote
//    products derived from this software without specific prior written
//    permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Any feedback is very welcome.
// http://www.math.keio.ac.jp/matumoto/emt.html
// email: matumoto@math.keio.ac.jp
//
// Feedback about the C++ port should be sent to Jasper Bedaux,
// see http://www.bedaux.net/mtrand/ for e-mail address and info.

#ifndef MTRAND_H
#define MTRAND_H

#include "fixed.h"
#include "DeleteEmitter.h" // for lua
#include <assert.h>

class MTRand_int32 { // Mersenne Twister random number generator
public:
// default constructor: uses default seed only if this is the first instance
  MTRand_int32() { seed(5489UL); }
// constructor with 32 bit int as seed
  MTRand_int32(unsigned long s) { seed(s); }
// constructor with array of size 32 bit ints as seed
  MTRand_int32(const unsigned long* array, int size) { seed(array, size);  }
// the two seed functions
  void seed(unsigned long); // seed with 32 bit integer
  void seed(const unsigned long*, int size); // seed with array
// 2007-02-11: made the destructor virtual; thanks "double more" for pointing this out
  virtual ~MTRand_int32() {} // destructor
protected: // used by derived classes, otherwise not accessible; use the ()-operator
  unsigned long rand_int32(); // generate 32 bit random integer
private:
  unsigned long operator()() { return rand_int32(); }
  static const int n = 624, m = 397; // compile time constants
// the variables below are static (no duplicates can exist)
  unsigned long state[n]; // state vector array
  int p; // position in state array
// private functions used to generate the pseudo random numbers
  unsigned long twiddle(unsigned long, unsigned long); // used by gen_state()
  void gen_state(); // generate new state
// make copy constructor and assignment operator unavailable, they don't make sense
  MTRand_int32(const MTRand_int32&); // copy constructor not defined
  void operator=(const MTRand_int32&); // assignment operator not defined
};

// inline for speed, must therefore reside in header file
inline unsigned long MTRand_int32::twiddle(unsigned long u, unsigned long v) {
  return (((u & 0x80000000UL) | (v & 0x7FFFFFFFUL)) >> 1)
    ^ ((v & 1UL) ? 0x9908B0DFUL : 0x0UL);
}

inline unsigned long MTRand_int32::rand_int32() { // generate 32 bit random int
  if (p == n) gen_state(); // new state vector needed
// gen_state() is split off to be non-inline, because it is only called once
// in every 624 calls and otherwise irand() would become too big to get inlined
  unsigned long x = state[p++];
  x ^= (x >> 11);
  x ^= (x << 7) & 0x9D2C5680UL;
  x ^= (x << 15) & 0xEFC60000UL;
  return x ^ (x >> 18);
}

class MTRand : public MTRand_int32, public DeleteEmitter {
public:
  MTRand() : MTRand_int32() {}
  MTRand(unsigned long seed_) : MTRand_int32(seed_) {}
  MTRand(const unsigned long* seed_, int size) : MTRand_int32(seed_, size) {}
  ~MTRand() {}
  double NDouble(int p_) {
    double o = Double(1.0);
    while (--p_ > 0) o *= Double(1.0);
    return o;
  }

  // interval [min, max)
  double Double(double min, double max) {
    return Double(max-min)+min;
  }
  // interval [0, max)
  double Double(double max) {
    return max*Double(); } // divided by 2^32
  // interval [0, 1)
  double Double() {
    return double(rand_int32()) * (1. / 4294967296.); } // divided by 2^32
  // interval [min, max]
  double Double_closed(double min, double max) {
    return Double_closed(max-min)+min;
  }
  // interval [0, max]
  double Double_closed(double max) {
    return max*Double_closed();
  }
  // [0, 1]
  double Double_closed() {
    return double(rand_int32()) * (1. / 4294967295.); } // divided by 2^32 - 1
  // (0, 1)
  double Double_open() {
    return (double(rand_int32()) + .5) * (1. / 4294967296.); } // divided by 2^32
  // generates 53 bit resolution doubles in the half-open interval [0, 1)
  double Double53() {
    return (double(rand_int32() >> 5) * 67108864. + double(rand_int32() >> 6)) * (1. / 9007199254740992.);
  }
  // [0,1)
  fixed Fixed() {
	  assert(fixed::FRAC == 32);
	  return fixed(rand_int32());
  }
  fixed NFixed(int p_) {
	  fixed o = Fixed();
	  while (--p_ > 0) o *= Fixed();
	  return o;
  }
  // [min,max]
  int Int32(int min, int max) {
	  return (rand_int32()%(1+max-min))+min;
  }
  // [0,max)
  unsigned int Int32(int max) { assert(max > 0); return rand_int32()%max; }
  unsigned int Int32() { return rand_int32(); }
private:
  MTRand(const MTRand&); // copy constructor not defined
  void operator=(const MTRand&); // assignment operator not defined
};

#endif // MTRAND_H
