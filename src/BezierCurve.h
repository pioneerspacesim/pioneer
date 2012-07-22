#ifndef _BEZIERCURVE_H
#define _BEZIERCURVE_H

#include "vector3.h"
#include "Serializer.h"

#ifdef _MSC_VER
	#include "win32/Gamma.h"
	#define lgamma LogGamma
#endif // _MSC_VER

class BezierCurve {
public:
	std::vector<vector3d> p;

	BezierCurve() {}
	BezierCurve(unsigned int numPoints) {
		p.resize(numPoints);
	}

	vector3d Eval(const double t) {
		int n_points = p.size();

		vector3d out(0.0);
		for (int i=0; i<n_points; i++) {
			double c = pow(1.0f-t, n_points-(i+1)) * pow(t,i) * BinomialCoeff(n_points-1, i);
			out += p[i] * c;
		}
		return out;
	}
	BezierCurve DerivativeOf() {
		int n_points = p.size()-1;
		BezierCurve out(n_points);
		for (int i=0; i<n_points; i++) {
			out.p[i] = double(n_points) * (p[i+1] - p[i]);
		}
		return out;
	}
	void Save(Serializer::Writer &wr) {
		wr.Int32(p.size());
		for (std::vector<vector3d>::size_type i=0; i<p.size(); i++) wr.Vector3d(p[i]);
	}
	void Load(Serializer::Reader &rd) {
		p.resize(rd.Int32());
		for (std::vector<vector3d>::size_type i=0; i<p.size(); i++) p[i] = rd.Vector3d();
	}

private:
	inline double BinomialCoeff(int n, int k)
	{
		// See Wikipedia page for algorithm
		// http://en.wikipedia.org/wiki/Binomial_coefficient#Binomial_coefficient_in_programming_languages

		// normal restrictions for binomial coefficients
		assert(k <= n && n >= 0 && k >= 0);

		// results are symmetric; use smaller k value to minimise loop iterations
		if (k > n-k)
			k = n-k;

		if (n > 30) {
			// for 32-bit (unsigned) ints, n > 30 leads to overflow
			// (upper bound of 30 found with a little test program)
			// so here we swap to a floating point alternative based on lgamma()
			// (see Wikipedia page linked above for explanation)
			return exp(lgamma(n+1) - lgamma(k+1) - lgamma(n-k+1));
		} else {
			unsigned int c = 1;
			for (int i = 0; i < k; ++i)
				c = (c * (n - i)) / (i + 1);
			return double(c);
		}
	}
};

#endif /* _BEZIERCURVE_H */
