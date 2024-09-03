// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _MATRIX4X4_H
#define _MATRIX4X4_H

#include "matrix3x3.h"
#include "vector3.h"

#include <cassert>
#include <math.h>
#include <stdio.h>
#include <cassert>
#include <type_traits>

template <typename T>
class matrix4x4 {
private:
	T cell[16];
	using other_float_t = typename std::conditional<std::is_same<T, float>::value, double, float>::type;

public:
	matrix4x4() {}
	explicit matrix4x4(T val)
	{
		cell[0] = cell[1] = cell[2] = cell[3] = cell[4] = cell[5] = cell[6] =
			cell[7] = cell[8] = cell[9] = cell[10] = cell[11] = cell[12] = cell[13] =
				cell[14] = cell[15] = val;
	}
	explicit matrix4x4(const T *vals)
	{
		memcpy(cell, vals, sizeof(T) * 16);
	}
	matrix4x4(const matrix3x3<T> &m)
	{
		LoadFrom3x3Matrix(m.Data());
	}
	matrix4x4(const matrix3x3<T> &m, const vector3<T> &v)
	{
		LoadFrom3x3Matrix(m.Data());
		SetTranslate(v);
	}
	explicit matrix4x4(const matrix4x4<other_float_t> &m)
	{
		for (int i = 0; i < 16; i++)
			cell[i] = T(m[i]);
	}

	void SetTranslate(const vector3<T> &v)
	{
		cell[12] = v.x;
		cell[13] = v.y;
		cell[14] = v.z;
	}
	vector3<T> GetTranslate() const { return vector3<T>(cell[12], cell[13], cell[14]); }
	void SetRotationOnly(const matrix4x4 &m)
	{
		for (int i = 0; i < 12; i++)
			cell[i] = m.cell[i];
	}
	matrix3x3<T> GetOrient() const
	{
		matrix3x3<T> m;
		m[0] = cell[0];
		m[1] = cell[4];
		m[2] = cell[8];
		m[3] = cell[1];
		m[4] = cell[5];
		m[5] = cell[9];
		m[6] = cell[2];
		m[7] = cell[6];
		m[8] = cell[10];
		return m;
	}
	// row-major 3x3 matrix
	void LoadFrom3x3Matrix(const T *r)
	{
		cell[0] = r[0];
		cell[4] = r[1];
		cell[8] = r[2];
		cell[12] = 0;
		cell[1] = r[3];
		cell[5] = r[4];
		cell[9] = r[5];
		cell[13] = 0;
		cell[2] = r[6];
		cell[6] = r[7];
		cell[10] = r[8];
		cell[14] = 0;
		cell[3] = 0;
		cell[7] = 0;
		cell[11] = 0;
		cell[15] = 1;
	}
	// row-major
	void SaveTo3x3Matrix(T *r) const
	{
		r[0] = cell[0];
		r[1] = cell[4];
		r[2] = cell[8];
		r[3] = cell[1];
		r[4] = cell[5];
		r[5] = cell[9];
		r[6] = cell[2];
		r[7] = cell[6];
		r[8] = cell[10];
	}
	// matrix4x4 is column-major
	// but it seems more natural to write the matrix by row
	template <std::size_t N>
	static matrix4x4 FromRowMajor(const T (&a)[N])
	{
		static_assert(N == 16, "Incorrect number of elements specified");
		matrix4x4 m;
		auto &c = m.cell;
		c[0] = a[0];  c[4] = a[1];  c[8]  = a[2];  c[12] = a[3];
		c[1] = a[4];  c[5] = a[5];  c[9]  = a[6];  c[13] = a[7];
		c[2] = a[8];  c[6] = a[9];  c[10] = a[10]; c[14] = a[11];
		c[3] = a[12]; c[7] = a[13]; c[11] = a[14]; c[15] = a[15];
		return m;
	}
	static matrix4x4 Identity()
	{
		matrix4x4 m = matrix4x4(0.0);
		m.cell[0] = m.cell[5] = m.cell[10] = m.cell[15] = 1.0f;
		return m;
	}
	//glscale equivalent
	void Scale(T x, T y, T z)
	{
		*this = (*this) * ScaleMatrix(x, y, z);
	}
	void Scale(T s)
	{
		*this = (*this) * ScaleMatrix(s, s, s);
	}
	static matrix4x4 ScaleMatrix(T x, T y, T z)
	{
		matrix4x4 m;
		m[0] = x;
		m[1] = m[2] = m[3] = 0;
		m[5] = y;
		m[4] = m[6] = m[7] = 0;
		m[10] = z;
		m[8] = m[9] = m[11] = 0;
		m[12] = m[13] = m[14] = 0;
		m[15] = 1;
		return m;
	}
	static matrix4x4 ScaleMatrix(T scale)
	{
		matrix4x4 m;
		m[0] = scale;
		m[1] = m[2] = m[3] = 0;
		m[5] = scale;
		m[4] = m[6] = m[7] = 0;
		m[10] = scale;
		m[8] = m[9] = m[11] = 0;
		m[12] = m[13] = m[14] = 0;
		m[15] = 1;
		return m;
	}
	static matrix4x4 MakeRotMatrix(const vector3<T> &rx, const vector3<T> &ry, const vector3<T> &rz)
	{
		matrix4x4 m;
		m[0] = rx.x;
		m[4] = rx.y;
		m[8] = rx.z;
		m[12] = 0;
		m[1] = ry.x;
		m[5] = ry.y;
		m[9] = ry.z;
		m[13] = 0;
		m[2] = rz.x;
		m[6] = rz.y;
		m[10] = rz.z;
		m[14] = 0;
		m[3] = 0;
		m[7] = 0;
		m[11] = 0;
		m[15] = 1;
		return m;
	}
	static matrix4x4 MakeInvRotMatrix(const vector3<T> &rx, const vector3<T> &ry, const vector3<T> &rz)
	{
		matrix4x4 m;
		m[0] = rx.x;
		m[4] = ry.x;
		m[8] = rz.x;
		m[12] = 0;
		m[1] = rx.y;
		m[5] = ry.y;
		m[9] = rz.y;
		m[13] = 0;
		m[2] = rx.z;
		m[6] = ry.z;
		m[10] = rz.z;
		m[14] = 0;
		m[3] = 0;
		m[7] = 0;
		m[11] = 0;
		m[15] = 1;
		return m;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Matrix Construction Functions
	// NOTE: all matrix functions here are optimized for reverse-Z depth buffers.
	// Compared to "standard" DirectX or OpenGL matricies they invert the Z value
	// so it ranges from 1.0 at the near plane to 0.0 at the far plane.
	///////////////////////////////////////////////////////////////////////////////

	// Construct a perspective projection matrix based on arbitrary left/right/top/bottom
	// plane positions.
	// This method is slower than the others, but supports view frustrums that are not
	// aligned with the Z-axis. Unless you know what you're doing, you shouldn't use this.
	//
	// @param left - the minimum x-value of the view volume at the near plane
	// @param right - the maximum x-value of the view volume at the near plane
	// @param bottom - the maximum y-value of the view volume at the near plane
	// @param top - the maximum y-value of the view volume at the near plane
	// @param znear - the near clipping plane
	// @param zfar - the far clipping plane
	static matrix4x4 FrustumMatrix(T left, T right, T bottom, T top, T znear, T zfar)
	{
		assert((znear > T(0)) && (zfar > T(0)));
		// these expressions come from the documentation for glFrustum
		const T sx = (T(2) * znear) / (right - left);
		const T sy = (T(2) * znear) / (top - bottom);
		const T A = (right + left) / (right - left);
		const T B = (top + bottom) / (top - bottom);
		const T C = (zfar) / (zfar - znear) - 1;
		const T D = (zfar * znear) / (zfar - znear);
		matrix4x4 m;

		// http://glprogramming.com/red/appendixf.html
		// OpenGL 'Red Book' on Perspective Projection
		// Presented here in row-major notation (because that's what matrix4x4f uses internally)
		T perspective[16] = {
			sx, 0, 0, 0,
			0, sy, 0, 0,
			A, B, C, -1,
			0, 0, D, 0
		};
		return matrix4x4(&perspective[0]);
	}

	// Construct a perspective projection matrix based field of view and aspect ratio.
	// This method is the optimized case when you know your screen aspect ratio and
	// field of view and aren't interested in fancy math. Use this function or
	// InfinitePerspectiveMatrix if at all possible.
	//
	// @param fovR - the camera FOV in radians
	// @param aspect - the aspect ratio (width / height) of the viewport
	// @param znear - the near clipping plane
	// @param zfar - the far clipping plane
	// @param fovX - whether the field of view is horizontal or vertical (default)
	static matrix4x4 PerspectiveMatrix(T fovR, T aspect, T znear, T zfar, bool fovX = false)
	{
		assert((znear > T(0)) && (zfar > znear));

		const T e = 1 / tan(fovR / T(2));
		const T x = fovX ? e : e / aspect;
		const T y = fovX ? e * aspect : e;
		const T z = (znear) / (zfar - znear);
		const T w = (zfar * znear) / (zfar - znear);

		// Based on: http://www.terathon.com/gdc07_lengyel.pdf
		// Unlike gluProject / FrustumMatrix, this projection matrix can only be
		//  symmetric about the Z axis.
		// This is what you want in 99% of cases, and simplifies the math a good deal.
		T perspective[16] = {
			x, 0, 0, 0,
			0, y, 0, 0,
			0, 0, z, -1,
			0, 0, w, 0
		};

		return matrix4x4(&perspective[0]);
	}

	// Construct an infinite far-plane perspective projection matrix.
	// Unless you specifically want to clip objects beyond a specific distance,
	// this projection will work for any object at any distance.
	//
	// @param fovR - the camera FOV in radians
	// @param aspect - the aspect ratio (width / height) of the viewport
	// @param znear - the near clipping plane
	// @param fovX - whether the field of view is horizontal or vertical (default)
	static matrix4x4 InfinitePerspectiveMatrix(T fovR, T aspect, T znear, bool fovX = false)
	{
		assert(znear > T(0));

		const T e = 1 / tan(fovR / T(2));
		const T x = fovX ? e : e / aspect;
		const T y = fovX ? e / aspect : e;
		const T w = znear;

		// Based on: http://dev.theomader.com/depth-precision/
		// An 'infinite far-plane' projection matrix. There is no concept of a zFar value,
		// and it can handle everything up to and including homogeneous coordinates with w=0.
		T perspective[16] = {
			x, 0, 0, 0,
			0, y, 0, 0,
			0, 0, 0, -1,
			0, 0, w, 0
		};

		return matrix4x4(&perspective[0]);
	}

	///////////////////////////////////////////////////////////////////////////////
	// set a orthographic frustum with 6 params similar to glOrtho()
	// (left, right, bottom, top, near, far)
	//
	// Derived from:
	// [1] https://docs.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixorthorh
	// [2] https://thxforthefish.com/posts/reverse_z/
	//
	// Specifically, the `tz` and `c` terms are based on a right-handed DirectX-style
	// (Z=0..1) matrix [1], multiplied by the given reversing matrix [2].
	// If looking at the sources, keep in mind that [1] is presented in row-major order
	// and [2] is presented in column-major order, and thus multiplication occurs in
	// column-column fashion.
	///////////////////////////////////////////////////////////////////////////////
	static matrix4x4 OrthoFrustum(T left, T right, T bottom, T top, T znear, T zfar)
	{
		assert((znear >= T(-1)) && (zfar >= T(0)));
		T a = T(2) / (right - left);
		T b = T(2) / (top - bottom);
		T c = T(1) / (zfar - znear);

		T tx = (right + left) / (left - right);
		T ty = (top + bottom) / (bottom - top);
		T tz = (zfar) / (zfar - znear);

		T ortho[16] = {
			a, 0, 0, 0,
			0, b, 0, 0,
			0, 0, c, 0,
			tx, ty, tz, 1
		};
		matrix4x4 m(&ortho[0]);
		return m;
	}

	// Optimized form that takes width/height and near/far planes
	static matrix4x4 OrthoMatrix(T width, T height, T znear, T zfar)
	{
		assert((znear >= T(-1)) && (zfar > T(0)));
		T a = T(2) / width;
		T b = T(2) / height;
		T c = T(1) / (zfar - znear);

		T tz = (zfar) / (zfar - znear);

		T ortho[16] = {
			a, 0, 0, 0,
			0, b, 0, 0,
			0, 0, c, 0,
			0, 0, tz, 1
		};
		matrix4x4 m(&ortho[0]);
		return m;
	}

	//glRotate equivalent (except radians instead of degrees)
	void Rotate(T ang, T x, T y, T z)
	{
		*this = (*this) * RotateMatrix(ang, x, y, z);
	}
	// (x,y,z) must be normalized
	static matrix4x4 RotateMatrix(T ang, T x, T y, T z)
	{
		matrix4x4 m;
		T c = cos(ang);
		T s = sin(ang);
		m[0] = x * x * (1 - c) + c;
		m[1] = y * x * (1 - c) + z * s;
		m[2] = x * z * (1 - c) - y * s;
		m[3] = 0;
		m[4] = x * y * (1 - c) - z * s;
		m[5] = y * y * (1 - c) + c;
		m[6] = y * z * (1 - c) + x * s;
		m[7] = 0;
		m[8] = x * z * (1 - c) + y * s;
		m[9] = y * z * (1 - c) - x * s;
		m[10] = z * z * (1 - c) + c;
		m[11] = 0;
		m[12] = 0;
		m[13] = 0;
		m[14] = 0;
		m[15] = 1;
		return m;
	}
	void RotateZ(T radians) { *this = (*this) * RotateZMatrix(radians); }
	void RotateY(T radians) { *this = (*this) * RotateYMatrix(radians); }
	void RotateX(T radians) { *this = (*this) * RotateXMatrix(radians); }
	static matrix4x4 RotateXMatrix(T radians)
	{
		matrix4x4 m;
		T cos_r = cosf(float(radians));
		T sin_r = sinf(float(radians));
		m[0] = 1.0f;
		m[1] = 0;
		m[2] = 0;
		m[3] = 0;

		m[4] = 0;
		m[5] = cos_r;
		m[6] = -sin_r;
		m[7] = 0;

		m[8] = 0;
		m[9] = sin_r;
		m[10] = cos_r;
		m[11] = 0;

		m[12] = 0;
		m[13] = 0;
		m[14] = 0;
		m[15] = 1.0f;
		return m;
	}
	static matrix4x4 RotateYMatrix(T radians)
	{
		matrix4x4 m;
		T cos_r = cosf(float(radians));
		T sin_r = sinf(float(radians));
		m[0] = cos_r;
		m[1] = 0;
		m[2] = sin_r;
		m[3] = 0;

		m[4] = 0;
		m[5] = 1;
		m[6] = 0;
		m[7] = 0;

		m[8] = -sin_r;
		m[9] = 0;
		m[10] = cos_r;
		m[11] = 0;

		m[12] = 0;
		m[13] = 0;
		m[14] = 0;
		m[15] = 1.0f;
		return m;
	}
	static matrix4x4 RotateZMatrix(T radians)
	{
		matrix4x4 m;
		T cos_r = cosf(float(radians));
		T sin_r = sinf(float(radians));
		m[0] = cos_r;
		m[1] = -sin_r;
		m[2] = 0;
		m[3] = 0;

		m[4] = sin_r;
		m[5] = cos_r;
		m[6] = 0;
		m[7] = 0;

		m[8] = 0;
		m[9] = 0;
		m[10] = 1.0f;
		m[11] = 0;

		m[12] = 0;
		m[13] = 0;
		m[14] = 0;
		m[15] = 1.0f;
		return m;
	}
	void Renormalize()
	{
		vector3<T> x(cell[0], cell[4], cell[8]);
		vector3<T> y(cell[1], cell[5], cell[9]);
		vector3<T> z(cell[2], cell[6], cell[10]);
		x = x.Normalized();
		z = x.Cross(y).Normalized();
		y = z.Cross(x).Normalized();
		cell[0] = x.x;
		cell[4] = x.y;
		cell[8] = x.z;
		cell[1] = y.x;
		cell[5] = y.y;
		cell[9] = y.z;
		cell[2] = z.x;
		cell[6] = z.y;
		cell[10] = z.z;
	}
	void ClearToRotOnly()
	{
		cell[12] = 0;
		cell[13] = 0;
		cell[14] = 0;
	}
	T &operator[](const size_t i) { return cell[i]; }
	const T &operator[](const size_t i) const { return cell[i]; }
	const T *Data() const { return cell; }
	T *Data() { return cell; }
	friend matrix4x4 operator+(const matrix4x4 &a, const matrix4x4 &b)
	{
		matrix4x4 m;
		for (int i = 0; i < 16; i++)
			m.cell[i] = a.cell[i] + b.cell[i];
		return m;
	}
	friend matrix4x4 operator-(const matrix4x4 &a, const matrix4x4 &b)
	{
		matrix4x4 m;
		for (int i = 0; i < 16; i++)
			m.cell[i] = a.cell[i] - b.cell[i];
		return m;
	}
	friend matrix4x4 operator-(const matrix4x4 &a)
	{
		matrix4x4 m;
		for (int i = 0; i < 16; ++i) {
			m.cell[i] = -a.cell[i];
		}
		return m;
	}
	friend matrix4x4 operator*(const matrix4x4 &a, const matrix4x4 &b)
	{
		matrix4x4 m;
		m.cell[0] = a.cell[0] * b.cell[0] + a.cell[4] * b.cell[1] + a.cell[8] * b.cell[2] + a.cell[12] * b.cell[3];
		m.cell[1] = a.cell[1] * b.cell[0] + a.cell[5] * b.cell[1] + a.cell[9] * b.cell[2] + a.cell[13] * b.cell[3];
		m.cell[2] = a.cell[2] * b.cell[0] + a.cell[6] * b.cell[1] + a.cell[10] * b.cell[2] + a.cell[14] * b.cell[3];
		m.cell[3] = a.cell[3] * b.cell[0] + a.cell[7] * b.cell[1] + a.cell[11] * b.cell[2] + a.cell[15] * b.cell[3];

		m.cell[4] = a.cell[0] * b.cell[4] + a.cell[4] * b.cell[5] + a.cell[8] * b.cell[6] + a.cell[12] * b.cell[7];
		m.cell[5] = a.cell[1] * b.cell[4] + a.cell[5] * b.cell[5] + a.cell[9] * b.cell[6] + a.cell[13] * b.cell[7];
		m.cell[6] = a.cell[2] * b.cell[4] + a.cell[6] * b.cell[5] + a.cell[10] * b.cell[6] + a.cell[14] * b.cell[7];
		m.cell[7] = a.cell[3] * b.cell[4] + a.cell[7] * b.cell[5] + a.cell[11] * b.cell[6] + a.cell[15] * b.cell[7];

		m.cell[8] = a.cell[0] * b.cell[8] + a.cell[4] * b.cell[9] + a.cell[8] * b.cell[10] + a.cell[12] * b.cell[11];
		m.cell[9] = a.cell[1] * b.cell[8] + a.cell[5] * b.cell[9] + a.cell[9] * b.cell[10] + a.cell[13] * b.cell[11];
		m.cell[10] = a.cell[2] * b.cell[8] + a.cell[6] * b.cell[9] + a.cell[10] * b.cell[10] + a.cell[14] * b.cell[11];
		m.cell[11] = a.cell[3] * b.cell[8] + a.cell[7] * b.cell[9] + a.cell[11] * b.cell[10] + a.cell[15] * b.cell[11];

		m.cell[12] = a.cell[0] * b.cell[12] + a.cell[4] * b.cell[13] + a.cell[8] * b.cell[14] + a.cell[12] * b.cell[15];
		m.cell[13] = a.cell[1] * b.cell[12] + a.cell[5] * b.cell[13] + a.cell[9] * b.cell[14] + a.cell[13] * b.cell[15];
		m.cell[14] = a.cell[2] * b.cell[12] + a.cell[6] * b.cell[13] + a.cell[10] * b.cell[14] + a.cell[14] * b.cell[15];
		m.cell[15] = a.cell[3] * b.cell[12] + a.cell[7] * b.cell[13] + a.cell[11] * b.cell[14] + a.cell[15] * b.cell[15];
		return m;
	}
	friend vector3<T> operator*(const matrix4x4 &a, const vector3<T> &v)
	{
		vector3<T> out;
		out.x = a.cell[0] * v.x + a.cell[4] * v.y + a.cell[8] * v.z + a.cell[12];
		out.y = a.cell[1] * v.x + a.cell[5] * v.y + a.cell[9] * v.z + a.cell[13];
		out.z = a.cell[2] * v.x + a.cell[6] * v.y + a.cell[10] * v.z + a.cell[14];
		return out;
	}
	// scam for doing a transpose operation
	friend vector3<T> operator*(const vector3<T> &v, const matrix4x4 &a)
	{
		vector3<T> out;
		out.x = a.cell[0] * v.x + a.cell[1] * v.y + a.cell[2] * v.z;
		out.y = a.cell[4] * v.x + a.cell[5] * v.y + a.cell[6] * v.z;
		out.z = a.cell[8] * v.x + a.cell[9] * v.y + a.cell[10] * v.z;
		return out;
	}

	// Transform a vector by the affine inverse of a matrix4x4
	// internally this does a transpose operation, and thus only works on
	// Euclidean (translation + rotation) matricies.
	vector3<T> InvTransform(const vector3<T> &inVec)
	{
		// Formula derivation from songho (https://songho.ca/opengl):
		//
		// M = [ R | T ]
		//     [ --+-- ]    (R denotes 3x3 rotation/reflection matrix)
		//     [ 0 | 1 ]    (T denotes 1x3 translation matrix)
		//
		// y = M*x  ->  y = R*x + T  ->  x = R^-1*(y - T)  ->  x = R^T*y - R^T*T
		// (R is orthogonal,  R^-1 = R^T)

		// thanks to https://stackoverflow.com/a/2625420
		// "Depending on your situation, it may be faster to compute the result of
		// inv(A) * x instead of actually forming inv(A)..."
		//
		// inv(A) * [x] = [ inv(M) * (x - b) ]
		//          [1] = [        1         ]
		//

		vector3<T> v = inVec - GetTranslate();
		vector3<T> out;
		out.x = cell[0] * v.x + cell[1] * v.y + cell[2] * v.z;
		out.y = cell[4] * v.x + cell[5] * v.y + cell[6] * v.z;
		out.z = cell[8] * v.x + cell[9] * v.y + cell[10] * v.z;
		return out;
	}

	friend matrix4x4 operator*(const matrix4x4 &a, T v)
	{
		matrix4x4 m;
		for (int i = 0; i < 16; i++)
			m[i] = a.cell[i] * v;
		return m;
	}
	friend matrix4x4 operator*(T v, const matrix4x4 &a)
	{
		return (a * v);
	}
	vector3<T> ApplyRotationOnly(const vector3<T> &v) const
	{
		vector3<T> out;
		out.x = cell[0] * v.x + cell[4] * v.y + cell[8] * v.z;
		out.y = cell[1] * v.x + cell[5] * v.y + cell[9] * v.z;
		out.z = cell[2] * v.x + cell[6] * v.y + cell[10] * v.z;
		return out;
	}
	//gltranslate equivalent
	void Translate(const vector3<T> &t)
	{
		Translate(t.x, t.y, t.z);
	}
	void Translate(T x, T y, T z)
	{
		matrix4x4 m = Identity();
		m[12] = x;
		m[13] = y;
		m[14] = z;
		*this = (*this) * m;
	}
	static matrix4x4 Translation(const vector3<T> &v)
	{
		return Translation(v.x, v.y, v.z);
	}
	static matrix4x4 Translation(T x, T y, T z)
	{
		matrix4x4 m = Identity();
		m[12] = x;
		m[13] = y;
		m[14] = z;
		return m;
	}
	matrix4x4 Inverse() const
	{
		matrix4x4 m;
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
	matrix4x4 Transpose() const
	{
		matrix4x4 m;
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
	void Print() const
	{
		for (int i = 0; i < 4; i++) {
			printf("%.12f %.12f %.12f %.12f\n", cell[i], cell[i + 4], cell[i + 8], cell[i + 12]);
		}
		printf("\n");
	}

	//convenience accessors for getting right/up/back vectors
	//from rotation matrices
	vector3<T> Right() const
	{
		return vector3<T>(cell[0], cell[1], cell[2]);
	}

	vector3<T> Up() const
	{
		return vector3<T>(cell[4], cell[5], cell[6]);
	}

	vector3<T> Back() const
	{
		return vector3<T>(cell[8], cell[9], cell[10]);
	}
};

typedef matrix4x4<float> matrix4x4f;
typedef matrix4x4<double> matrix4x4d;

static const matrix4x4f matrix4x4fIdentity(matrix4x4f::Identity());
static const matrix4x4d matrix4x4dIdentity(matrix4x4d::Identity());

#endif /* _MATRIX4X4_H */
