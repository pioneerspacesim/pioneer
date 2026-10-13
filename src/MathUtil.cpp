// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "MathUtil.h"
#include "Pi.h"

namespace PhysicsUtil {
	double GetPlanckBrightness(const double wavelength_nm, const int temperature)
	{
		// Planck's constant
		const double h = 6.62607015e-34; // kg*m^2*s^-1 (J*s)

		// speed of light
		const double c = 299792458; // m*s^2

		// Boltzmann's constant
		const double k = 1.380649e-23; // J/K

		double wavelength = wavelength_nm * 1e-9;

		double exponent = exp((h*c) / (wavelength * k * temperature));
		return 2 * h * pow(c, 2) / (pow(wavelength, 5) * exponent);
	}

	const vector3d nm_to_rgb[48] = {
		vector3d( 0.000000f, 0.000000f, 0.000001f ),    // 360 nm
		vector3d( 0.000006f, 0.000001f, 0.000026f ),    // 370 nm
		vector3d( 0.000160f, 0.000017f, 0.000705f ),    // 380 nm
		vector3d( 0.002362f, 0.000253f, 0.010482f ),    // 390 nm
		vector3d( 0.019110f, 0.002004f, 0.086011f ),    // 400 nm
		vector3d( 0.084736f, 0.008756f, 0.389366f ),    // 410 nm
		vector3d( 0.204492f, 0.021391f, 0.972542f ),    // 420 nm
		vector3d( 0.314679f, 0.038676f, 1.553480f ),    // 430 nm
		vector3d( 0.383734f, 0.062077f, 1.967280f ),    // 440 nm
		vector3d( 0.370702f, 0.089456f, 1.994800f ),    // 450 nm
		vector3d( 0.302273f, 0.128201f, 1.745370f ),    // 460 nm
		vector3d( 0.195618f, 0.185190f, 1.317560f ),    // 470 nm
		vector3d( 0.080507f, 0.253589f, 0.772125f ),    // 480 nm
		vector3d( 0.016172f, 0.339133f, 0.415254f ),    // 490 nm
		vector3d( 0.003816f, 0.460777f, 0.218502f ),    // 500 nm
		vector3d( 0.037465f, 0.606741f, 0.112044f ),    // 510 nm
		vector3d( 0.117749f, 0.761757f, 0.060709f ),    // 520 nm
		vector3d( 0.236491f, 0.875211f, 0.030451f ),    // 530 nm
		vector3d( 0.376772f, 0.961988f, 0.013676f ),    // 540 nm
		vector3d( 0.529826f, 0.991761f, 0.003988f ),    // 550 nm
		vector3d( 0.705224f, 0.997340f, 0.000000f ),    // 560 nm
		vector3d( 0.878655f, 0.955552f, 0.000000f ),    // 570 nm
		vector3d( 1.014160f, 0.868934f, 0.000000f ),    // 580 nm
		vector3d( 1.118520f, 0.777405f, 0.000000f ),    // 590 nm
		vector3d( 1.123990f, 0.658341f, 0.000000f ),    // 600 nm
		vector3d( 1.030480f, 0.527963f, 0.000000f ),    // 610 nm
		vector3d( 0.856297f, 0.398057f, 0.000000f ),    // 620 nm
		vector3d( 0.647467f, 0.283493f, 0.000000f ),    // 630 nm
		vector3d( 0.431567f, 0.179828f, 0.000000f ),    // 640 nm
		vector3d( 0.268329f, 0.107633f, 0.000000f ),    // 650 nm
		vector3d( 0.152568f, 0.060281f, 0.000000f ),    // 660 nm
		vector3d( 0.081261f, 0.031800f, 0.000000f ),    // 670 nm
		vector3d( 0.040851f, 0.015905f, 0.000000f ),    // 680 nm
		vector3d( 0.019941f, 0.007749f, 0.000000f ),    // 690 nm
		vector3d( 0.009577f, 0.003718f, 0.000000f ),    // 700 nm
		vector3d( 0.004553f, 0.001768f, 0.000000f ),    // 710 nm
		vector3d( 0.002175f, 0.000846f, 0.000000f ),    // 720 nm
		vector3d( 0.001045f, 0.000407f, 0.000000f ),    // 730 nm
		vector3d( 0.000508f, 0.000199f, 0.000000f ),    // 740 nm
		vector3d( 0.000251f, 0.000098f, 0.000000f ),    // 750 nm
		vector3d( 0.000126f, 0.000050f, 0.000000f ),    // 760 nm
		vector3d( 0.000065f, 0.000025f, 0.000000f ),    // 770 nm
		vector3d( 0.000033f, 0.000013f, 0.000000f ),    // 780 nm
		vector3d( 0.000018f, 0.000007f, 0.000000f ),    // 790 nm
		vector3d( 0.000009f, 0.000004f, 0.000000f ),    // 800 nm
		vector3d( 0.000005f, 0.000002f, 0.000000f ),    // 810 nm
		vector3d( 0.000003f, 0.000001f, 0.000000f ),    // 820 nm
		vector3d( 0.000002f, 0.000001f, 0.000000f )     // 830 nm
	};

	vector3d GenerateTemperatureColor(const double T) {
		// https://www.shadertoy.com/view/NlGXzz

		// [360-830 nm]
		vector3d rgb = vector3d(0.f);

		const matrix3x3d xyz2rgb = matrix3x3d::FromVectors(
			vector3d( 3.2404542,-0.9692660, 0.0556434),
			vector3d(-1.5371385, 1.8760108,-0.2040259),
			vector3d(-0.4985314, 0.0415560, 1.0572252)
		);

		for (int i = 0; i < 48; i++) {
			double wavelength = 360 + (10 * i);

			rgb += GetPlanckBrightness(wavelength, T) * (xyz2rgb * nm_to_rgb[i]);
		}

		// normalize
		double max = std::max(std::max(rgb.x, rgb.y), rgb.z);
		// black color
		if (max == 0.f) {
			rgb = vector3d(0.f);
		} else {
			rgb /= (max / 255);
		}
		rgb.x = std::max(rgb.x, 0.0);
		rgb.y = std::max(rgb.y, 0.0);
		rgb.z = std::max(rgb.z, 0.0);

		return rgb;
	}
}

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
			return matrix4x4f::Identity;

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
