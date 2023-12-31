// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "perlin.h"
#include <math.h>

/* Simplex.cpp
 *
 * Copyright 2007 Eliot Eshelman
 * battlestartux@6by9.net
 *
 *
 *  This file is part of Battlestar Tux.
 *
 *  Battlestar Tux is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  Battlestar Tux is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Battlestar Tux; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

/*
 * I noticed this file was GPL2, not 2+, and thus incompatible with the GPL3.
 * I emailed the author asking for permission to use it under GPL3. His
 * response:
 *
 * From: Eliot Eshelman <eliot.eshelman@6by9.net>
 * To: Robert Norris <rob@eatenbyagrue.org>
 * Date: Sat, 16 Jul 2011 09:32:36 -0400
 * Subject: Re: License for Battlestar TUX
 * Message-ID: <CA+5Z5RTS2_wNLz-eBRi93Zi2VW3zEGPsVLWMrT=grEVPpKLFEQ@mail.gmail.com>
 *
 * Hi Rob,
 *
 * I'm glad you've found Simplex useful.
 *
 * You are correct - the headers need updating. The project is GPLv3. You may
 * re-use Simplex.cpp in your GPL3 project.
 *
 * If you feel this covers you, please go ahead and continue to use the file.
 * If it would be more appropriate for me to provide an updated file, I can do
 * so.
 *
 * Best,
 * Eliot
 */

inline int fastfloor(const double x) { return long(x > 0 ? x : x - 1); }

//static double dot( const int* g, const double x, const double y ) { return g[0]*x + g[1]*y; }
inline double dot(const double *g, const double x, const double y, const double z) { return g[0] * x + g[1] * y + g[2] * z; }
//static double dot( const int* g, const double x, const double y, const double z, const double w ) { return g[0]*x + g[1]*y + g[2]*z + g[3]*w; }

// The gradients are the midpoints of the vertices of a cube.
static const double grad3[12][3] = {
	{ 1, 1, 0 }, { -1, 1, 0 }, { 1, -1, 0 }, { -1, -1, 0 },
	{ 1, 0, 1 }, { -1, 0, 1 }, { 1, 0, -1 }, { -1, 0, -1 },
	{ 0, 1, 1 }, { 0, -1, 1 }, { 0, 1, -1 }, { 0, -1, -1 }
};

// Permutation table.  The same list is repeated twice.
static const unsigned char perm[512] = {
	151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142,
	8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117,
	35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71,
	134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41,
	55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89,
	18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226,
	250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182,
	189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43,
	172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97,
	228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239,
	107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
	138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180,

	151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142,
	8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117,
	35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71,
	134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41,
	55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89,
	18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226,
	250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182,
	189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43,
	172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97,
	228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239,
	107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
	138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
};

static const unsigned char mod12[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
	11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5, 6, 7,
	8, 9, 10, 11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5,
	6, 7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2, 3, 4,
	5, 6, 7, 8, 9, 10, 11, 0, 1, 2, 3
};

const double F3 = 1.0 / 3.0;
const double G3 = 1.0 / 6.0; // Very nice and simple unskew factor, too
const double G3mul2 = 0.3333333333333333;
const double G3mul3 = 0.5;
// 3D raw Simplex noise
double noise(const vector3d &p)
{
	// Skew the input space to determine which simplex cell we're in
	const double s = (p.x + p.y + p.z) * F3; // Very nice and simple skew factor for 3D
	const int i = fastfloor(p.x + s);
	const int j = fastfloor(p.y + s);
	const int k = fastfloor(p.z + s);

	const double t = (i + j + k) * G3;
	const double X0 = i - t; // Unskew the cell origin back to (x,y,z) space
	const double Y0 = j - t;
	const double Z0 = k - t;
	const double x0 = p.x - X0; // The x,y,z distances from the cell origin
	const double y0 = p.y - Y0;
	const double z0 = p.z - Z0;

	// For the 3D case, the simplex shape is a slightly irregular tetrahedron.
	// Determine which simplex we are in.
	int i1, j1, k1; // Offsets for second corner of simplex in (i,j,k) coords
	int i2, j2, k2; // Offsets for third corner of simplex in (i,j,k) coords

	int x_ge_y = x0 >= y0;
	int y_ge_z = y0 >= z0;
	int x_ge_z = x0 >= z0;

	// Compute simplex offsets - all values will be 0 or 1
	i1 = x_ge_y & x_ge_z;
	j1 = y_ge_z & (!x_ge_y);
	k1 = (!x_ge_z) & (!y_ge_z);

	i2 = x_ge_y | x_ge_z;
	j2 = (!x_ge_y) | y_ge_z;
	k2 = !(x_ge_z & y_ge_z);

	// A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
	// a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
	// a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
	// c = 1/6.
	const double x1 = x0 - i1 + G3; // Offsets for second corner in (x,y,z) coords
	const double y1 = y0 - j1 + G3;
	const double z1 = z0 - k1 + G3;
	const double x2 = x0 - i2 + G3mul2; // Offsets for third corner in (x,y,z) coords
	const double y2 = y0 - j2 + G3mul2;
	const double z2 = z0 - k2 + G3mul2;
	const double x3 = x0 - 1.0 + G3mul3; // Offsets for last corner in (x,y,z) coords
	const double y3 = y0 - 1.0 + G3mul3;
	const double z3 = z0 - 1.0 + G3mul3;

	// Work out the hashed gradient indices of the four simplex corners
	const int ii = i & 255;
	const int jj = j & 255;
	const int kk = k & 255;
	const int gi0 = mod12[perm[ii + perm[jj + perm[kk]]]];
	const int gi1 = mod12[perm[ii + i1 + perm[jj + j1 + perm[kk + k1]]]];
	const int gi2 = mod12[perm[ii + i2 + perm[jj + j2 + perm[kk + k2]]]];
	const int gi3 = mod12[perm[ii + 1 + perm[jj + 1 + perm[kk + 1]]]];

	// Noise contributions from the four corners
	double n0 = 0.0, n1 = 0.0, n2 = 0.0, n3 = 0.0;

	// Calculate the contribution from the four corners
	double t0 = 0.6 - x0 * x0 - y0 * y0 - z0 * z0;
	double t1 = 0.6 - x1 * x1 - y1 * y1 - z1 * z1;
	double t2 = 0.6 - x2 * x2 - y2 * y2 - z2 * z2;
	double t3 = 0.6 - x3 * x3 - y3 * y3 - z3 * z3;

	if (t0 > 0.0) {
		t0 *= t0;
		n0 = t0 * t0 * dot(grad3[gi0], x0, y0, z0);
	}

	if (t1 > 0.0) {
		t1 *= t1;
		n1 = t1 * t1 * dot(grad3[gi1], x1, y1, z1);
	}

	if (t2 > 0.0) {
		t2 *= t2;
		n2 = t2 * t2 * dot(grad3[gi2], x2, y2, z2);
	}

	if (t3 > 0.0) {
		t3 *= t3;
		n3 = t3 * t3 * dot(grad3[gi3], x3, y3, z3);
	}

	// Add contributions from each corner to get the final noise value.
	// The result is scaled to stay just inside [-1,1]
	return 32.0 * (n0 + n1 + n2 + n3);
}

#ifdef UNIT_TEST
#include <stdio.h>
#include <stdlib.h>
int main()
{
	double x, y, z;
	double a = 0.0;
	x = 0.0;
	y = 0.0;
	z = 0.0;
	for (int i = 0; i < 10000000; i++) {
		a += noise(x, y, z);
		x += 0.1;
		y += 0.2;
		z += 0.3;
	}
	return (int)a;
}

#endif /* UNIT_TEST */
