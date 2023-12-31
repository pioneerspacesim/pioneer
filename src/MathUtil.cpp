// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "MathUtil.h"
#include "Pi.h"

namespace MathUtil {

	vector3d RandomPointOnSphere(double minRadius, double maxRadius)
	{
		// see http://mathworld.wolfram.com/SpherePointPicking.html
		// or a Google search for further information
		const double dist = Pi::rng.Double(minRadius, maxRadius);
		const double z = Pi::rng.Double_closed(-1.0, 1.0);
		const double theta = Pi::rng.Double(2.0 * M_PI);
		const double r = sqrt(1.0 - z * z) * dist;
		return vector3d(r * cos(theta), r * sin(theta), z * dist);
	}

	vector3d RandomPointInCircle(double minRadius, double maxRadius)
	{
		// m: minRadius, M: maxRadius, r: random radius
		// PDF(r) = 2/(M^2 - m^2) * r  for m <= r < M
		// CDF(r) = 1/(M^2 - m^2) * (r^2 - m^2)
		// per inversion method (http://en.wikipedia.org/wiki/Inversion_method): CDF(r) := Uniform{0..1}
		// r = sqrt(Uniform{0..1} * (M^2 - m^2) + m^2) = sqrt(Uniform{m^2..M^2})
		const double r = sqrt(Pi::rng.Double(minRadius * minRadius, maxRadius * maxRadius));
		const double phi = Pi::rng.Double(2.0 * M_PI);
		return vector3d(r * cos(phi), r * sin(phi), 0.0);
	}

	// matrix4x4f utility functions
	matrix4x4f Inverse(const matrix4x4f &cell)
	{
		matrix4x4f m;
		// this only works for matrices containing only rotation and transform
		m[0] = cell[0];
		m[1] = cell[4];
		m[2] = cell[8];
		m[4] = cell[1];
		m[5] = cell[5];
		m[6] = cell[9];
		m[8] = cell[2];
		m[9] = cell[6];
		m[10] = cell[10];
		m[12] = -(cell[0] * cell[12] + cell[1] * cell[13] + cell[2] * cell[14]);
		m[13] = -(cell[4] * cell[12] + cell[5] * cell[13] + cell[6] * cell[14]);
		m[14] = -(cell[8] * cell[12] + cell[9] * cell[13] + cell[10] * cell[14]);
		m[3] = m[7] = m[11] = 0;
		m[15] = 1.0f;

		return m;
	}
	matrix4x4f InverseSlow(const matrix4x4f &cell)
	{
		matrix4x4f inv;

		inv[0] = cell[5] * cell[10] * cell[15] -
			cell[5] * cell[11] * cell[14] -
			cell[9] * cell[6] * cell[15] +
			cell[9] * cell[7] * cell[14] +
			cell[13] * cell[6] * cell[11] -
			cell[13] * cell[7] * cell[10];

		inv[4] = -cell[4] * cell[10] * cell[15] +
			cell[4] * cell[11] * cell[14] +
			cell[8] * cell[6] * cell[15] -
			cell[8] * cell[7] * cell[14] -
			cell[12] * cell[6] * cell[11] +
			cell[12] * cell[7] * cell[10];

		inv[8] = cell[4] * cell[9] * cell[15] -
			cell[4] * cell[11] * cell[13] -
			cell[8] * cell[5] * cell[15] +
			cell[8] * cell[7] * cell[13] +
			cell[12] * cell[5] * cell[11] -
			cell[12] * cell[7] * cell[9];

		inv[12] = -cell[4] * cell[9] * cell[14] +
			cell[4] * cell[10] * cell[13] +
			cell[8] * cell[5] * cell[14] -
			cell[8] * cell[6] * cell[13] -
			cell[12] * cell[5] * cell[10] +
			cell[12] * cell[6] * cell[9];

		inv[1] = -cell[1] * cell[10] * cell[15] +
			cell[1] * cell[11] * cell[14] +
			cell[9] * cell[2] * cell[15] -
			cell[9] * cell[3] * cell[14] -
			cell[13] * cell[2] * cell[11] +
			cell[13] * cell[3] * cell[10];

		inv[5] = cell[0] * cell[10] * cell[15] -
			cell[0] * cell[11] * cell[14] -
			cell[8] * cell[2] * cell[15] +
			cell[8] * cell[3] * cell[14] +
			cell[12] * cell[2] * cell[11] -
			cell[12] * cell[3] * cell[10];

		inv[9] = -cell[0] * cell[9] * cell[15] +
			cell[0] * cell[11] * cell[13] +
			cell[8] * cell[1] * cell[15] -
			cell[8] * cell[3] * cell[13] -
			cell[12] * cell[1] * cell[11] +
			cell[12] * cell[3] * cell[9];

		inv[13] = cell[0] * cell[9] * cell[14] -
			cell[0] * cell[10] * cell[13] -
			cell[8] * cell[1] * cell[14] +
			cell[8] * cell[2] * cell[13] +
			cell[12] * cell[1] * cell[10] -
			cell[12] * cell[2] * cell[9];

		inv[2] = cell[1] * cell[6] * cell[15] -
			cell[1] * cell[7] * cell[14] -
			cell[5] * cell[2] * cell[15] +
			cell[5] * cell[3] * cell[14] +
			cell[13] * cell[2] * cell[7] -
			cell[13] * cell[3] * cell[6];

		inv[6] = -cell[0] * cell[6] * cell[15] +
			cell[0] * cell[7] * cell[14] +
			cell[4] * cell[2] * cell[15] -
			cell[4] * cell[3] * cell[14] -
			cell[12] * cell[2] * cell[7] +
			cell[12] * cell[3] * cell[6];

		inv[10] = cell[0] * cell[5] * cell[15] -
			cell[0] * cell[7] * cell[13] -
			cell[4] * cell[1] * cell[15] +
			cell[4] * cell[3] * cell[13] +
			cell[12] * cell[1] * cell[7] -
			cell[12] * cell[3] * cell[5];

		inv[14] = -cell[0] * cell[5] * cell[14] +
			cell[0] * cell[6] * cell[13] +
			cell[4] * cell[1] * cell[14] -
			cell[4] * cell[2] * cell[13] -
			cell[12] * cell[1] * cell[6] +
			cell[12] * cell[2] * cell[5];

		inv[3] = -cell[1] * cell[6] * cell[11] +
			cell[1] * cell[7] * cell[10] +
			cell[5] * cell[2] * cell[11] -
			cell[5] * cell[3] * cell[10] -
			cell[9] * cell[2] * cell[7] +
			cell[9] * cell[3] * cell[6];

		inv[7] = cell[0] * cell[6] * cell[11] -
			cell[0] * cell[7] * cell[10] -
			cell[4] * cell[2] * cell[11] +
			cell[4] * cell[3] * cell[10] +
			cell[8] * cell[2] * cell[7] -
			cell[8] * cell[3] * cell[6];

		inv[11] = -cell[0] * cell[5] * cell[11] +
			cell[0] * cell[7] * cell[9] +
			cell[4] * cell[1] * cell[11] -
			cell[4] * cell[3] * cell[9] -
			cell[8] * cell[1] * cell[7] +
			cell[8] * cell[3] * cell[5];

		inv[15] = cell[0] * cell[5] * cell[10] -
			cell[0] * cell[6] * cell[9] -
			cell[4] * cell[1] * cell[10] +
			cell[4] * cell[2] * cell[9] +
			cell[8] * cell[1] * cell[6] -
			cell[8] * cell[2] * cell[5];

		float det = cell[0] * inv[0] + cell[1] * inv[4] + cell[2] * inv[8] + cell[3] * inv[12];

		if (is_equal_exact(det, 0.0f))
			return matrix4x4f::Identity();

		det = 1.0f / det;

		matrix4x4f m;
		for (int i = 0; i < 16; i++)
			m[i] = inv[i] * det;

		return m;
	}
	matrix4x4f Transpose(const matrix4x4f &cell)
	{
		matrix4x4f m;
		m[0] = cell[0];
		m[1] = cell[4];
		m[2] = cell[8];
		m[3] = cell[12];
		m[4] = cell[1];
		m[5] = cell[5];
		m[6] = cell[9];
		m[7] = cell[13];
		m[8] = cell[2];
		m[9] = cell[6];
		m[10] = cell[10];
		m[11] = cell[14];
		m[12] = cell[3];
		m[13] = cell[7];
		m[14] = cell[11];
		m[15] = cell[15];
		return m;
	}

	// matrix3x3f utility functions
	matrix3x3f Transpose(const matrix3x3f &cell)
	{
		matrix3x3f m;
		m[0] = cell[0];
		m[1] = cell[3];
		m[2] = cell[6];
		m[3] = cell[1];
		m[4] = cell[4];
		m[5] = cell[7];
		m[6] = cell[2];
		m[7] = cell[5];
		m[8] = cell[8];
		return m;
	}
	matrix3x3f Inverse(const matrix3x3f &cell)
	{
// computes the inverse of a matrix m
#define cell2d(x, y) cell[((y * 3) + x)]
		const float det = cell2d(0, 0) * (cell2d(1, 1) * cell2d(2, 2) - cell2d(2, 1) * cell2d(1, 2)) -
			cell2d(0, 1) * (cell2d(1, 0) * cell2d(2, 2) - cell2d(1, 2) * cell2d(2, 0)) +
			cell2d(0, 2) * (cell2d(1, 0) * cell2d(2, 1) - cell2d(1, 1) * cell2d(2, 0));

		const float invdet = 1.0f / det;

		matrix3x3f minv; // inverse of matrix m
#define idx2d(x, y) ((y * 3) + x)
		minv[idx2d(0, 0)] = (cell2d(1, 1) * cell2d(2, 2) - cell2d(2, 1) * cell2d(1, 2)) * invdet;
		minv[idx2d(0, 1)] = (cell2d(0, 2) * cell2d(2, 1) - cell2d(0, 1) * cell2d(2, 2)) * invdet;
		minv[idx2d(0, 2)] = (cell2d(0, 1) * cell2d(1, 2) - cell2d(0, 2) * cell2d(1, 1)) * invdet;
		minv[idx2d(1, 0)] = (cell2d(1, 2) * cell2d(2, 0) - cell2d(1, 0) * cell2d(2, 2)) * invdet;
		minv[idx2d(1, 1)] = (cell2d(0, 0) * cell2d(2, 2) - cell2d(0, 2) * cell2d(2, 0)) * invdet;
		minv[idx2d(1, 2)] = (cell2d(1, 0) * cell2d(0, 2) - cell2d(0, 0) * cell2d(1, 2)) * invdet;
		minv[idx2d(2, 0)] = (cell2d(1, 0) * cell2d(2, 1) - cell2d(2, 0) * cell2d(1, 1)) * invdet;
		minv[idx2d(2, 1)] = (cell2d(2, 0) * cell2d(0, 1) - cell2d(0, 0) * cell2d(2, 1)) * invdet;
		minv[idx2d(2, 2)] = (cell2d(0, 0) * cell2d(1, 1) - cell2d(1, 0) * cell2d(0, 1)) * invdet;
		return minv;
	}

	// basic distince from a line segment as described here:
	// http://paulbourke.net/geometry/pointline/
	float DistanceFromLineSegment(const vector3f &start, const vector3f &end, const vector3f &pos, bool &isWithinLineSegment)
	{
		const float magnitude = (end - start).Length();
		float t = Dot((pos - start), (end - start)) / (magnitude * magnitude);

		if (t < 0.0f || t > 1.0f) {
			t = (t < 0.0f) ? 0.0f : t;
			t = (t > 1.0f) ? 1.0f : t;
			isWithinLineSegment = false;
		} else {
			isWithinLineSegment = true;
		}
		// convert the time t to a vector to use in intersection calculation
		vector3f tv(t, t, t);
		vector3f intersect(start + tv * (end - start));

		return (intersect - pos).Length();
	}

	float DistanceFromLine(const vector3f &start, const vector3f &end, const vector3f &pos)
	{
		const float magnitude = (end - start).Length();
		const float t = Dot((pos - start), (end - start)) / (magnitude * magnitude);

		// convert the time t to a vector to use in intersection calculation
		vector3f tv(t, t, t);
		vector3f intersect(start + tv * (end - start));

		return (intersect - pos).Length();
	}

#ifdef TEST_MATHUTIL
	bool TestDistanceFromLine()
	{
		//           P1
		//           |
		//           |
		//           |
		// E---------X---------H
		// Fill these with a set of known data...
		static const size_t TotalNum = 4;
		static const vector3f E[TotalNum] = {
			vector3f(-10.0f, 0.0f, 0.0f),
			vector3f(-10.0f, 0.0f, 0.0f),
			vector3f(-10.0f, 0.0f, 0.0f),
			vector3f(-10.0f, 0.0f, 0.0f)
		};
		static vector3f H[TotalNum] = {
			vector3f(10.0f, 0.0f, 0.0f),
			vector3f(10.0f, 0.0f, 0.0f),
			vector3f(10.0f, 0.0f, 0.0f),
			vector3f(10.0f, 0.0f, 0.0f)
		};
		static const vector3f P[TotalNum] = {
			vector3f(-20.0f, 0.0f, 0.0f), // no
			vector3f(0.0f, 4.0f, 0.0f), // yes
			vector3f(0.0f, 8.0f, 0.0f), // no
			vector3f(20.0f, 4.0f, 0.0f) // no
		};
		static const bool RES[TotalNum] = {
			false, // no
			true, // yes
			false, // no
			false // no
		};
		static const bool RESLINE[TotalNum] = {
			true, // no
			true, // yes
			false, // no
			true // no
		};

		// Just working with a fixed distance.
		static const float triggerSoundDistance = 6.0f;
		// Iterate through the generated data and see which ones pass close enough to trigger the sound
		bool success = true;
		for (int i = 0; i < TotalNum; i++) {
			bool isWithinLineSegment = false;
			const float dist = DistanceFromLineSegment(E[i], H[i], P[i], isWithinLineSegment);

			// Compare against the expected result and accumulate
			success &= (RES[i] == (isWithinLineSegment && dist <= triggerSoundDistance));
		}

		for (int i = 0; i < TotalNum; i++) {
			const float dist = DistanceFromLine(E[i], H[i], P[i]);

			// Compare against the expected result and accumulate
			success &= (RESLINE[i] == (dist <= triggerSoundDistance));
		}

		Output("TestDistanceFromLine successful? = %s\n", success ? "yes" : "no");
		return success;
	}
#endif
} // namespace MathUtil
