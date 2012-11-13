// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _MATRIX3X3_H
#define _MATRIX3X3_H

#include <math.h>
#include <stdio.h>
#include "vector3.h"

template <typename T>
class matrix3x3 {
	private:
	T cell[9];
	public:
	matrix3x3 () {}
	matrix3x3 (const T *vals) {
		memcpy(cell, vals, sizeof(T)*9);
	}
	vector3<T> VectorX() { return vector3<T>(cell[0], cell[3], cell[6]); }
	vector3<T> VectorY() { return vector3<T>(cell[1], cell[4], cell[7]); }
	vector3<T> VectorZ() { return vector3<T>(cell[2], cell[5], cell[8]); }

	static matrix3x3 Identity () {
		matrix3x3 m;
		m.cell[1] = m.cell[2] = m.cell[3] = m.cell[5] = m.cell[6] = m.cell[7] = 0.0f;
		m.cell[0] = m.cell[4] = m.cell[8] = 1.0f;
		return m;
	}
	void Scale(T x, T y, T z) {
		*this = (*this) * BuildScale(x, y, z);
	}
	void Scale(T s) {
		*this = (*this) * BuildScale(s);
	}
	static matrix3x3 BuildScale(T x, T y, T z) {
		matrix3x3 m;
		m.cell[1] = m.cell[2] = m.cell[3] = m.cell[5] = m.cell[6] = m.cell[7] = 0.0f;
		m.cell[0] = x; m.cell[4] = y; m.cell[8] = z;
		return m;
	}
	static matrix3x3 BuildScale(T scale) {
		matrix3x3 m;
		m.cell[1] = m.cell[2] = m.cell[3] = m.cell[5] = m.cell[6] = m.cell[7] = 0.0f;
		m.cell[0] = m.cell[4] = m.cell[8] = s;
		return m;
	}
	static matrix3x3 BuildFromVectors(const vector3<T> &rx, const vector3<T> &ry, const vector3<T> &rz) {
		matrix3x3 m;
		m[0] = rx.x; m[4] = ry.x; m[8] = rz.x;
		m[1] = rx.y; m[5] = ry.y; m[9] = rz.y;
		m[2] = rx.z; m[6] = ry.z; m[10] = rz.z;
		return m;
	}

	//glRotate equivalent (except radians instead of degrees)
	void Rotate (T ang, T x, T y, T z) {
		*this = (*this) * BuildRotate(ang, x, y, z);
	}
	// (x,y,z) must be normalized
	static matrix3x3 BuildRotate(T ang, T x, T y, T z) {
		matrix3x3 m;
		T c = cos(ang);
		T s = sin(ang);
		m[0] = x*x*(1-c)+c; m[1] = x*y*(1-c)-z*s; m[2] = x*z*(1-c)+y*s;
		m[3] = y*x*(1-c)+z*s; m[4] = y*y*(1-c)+c; m[5] = y*z*(1-c)-x*s;
		m[6] = x*z*(1-c)-y*s; m[7] = y*z*(1-c)+x*s; m[8] = z*z*(1-c)+c;
		return m;
	}
	void RotateZ (T radians) { *this = (*this) * RotateZMatrix (radians); }
	void RotateY (T radians) { *this = (*this) * RotateYMatrix (radians); }
	void RotateX (T radians) { *this = (*this) * RotateXMatrix (radians); }
	static matrix3x3 RotateXMatrix (T radians) {
		matrix3x3 m;
		T c = cosf(float(radians));
		T s = sinf(float(radians));
		m[0] = 1.0f; m[1] = 0; m[2] = 0;
		m[3] = 0; m[4] = c; m[5] = s;
		m[6] = 0; m[7] = -s; m[8] = c;
		return m;
	}
	static matrix3x3 RotateYMatrix (T radians) {
		matrix3x3 m;
		T c = cosf(float(radians));
		T s = sinf(float(radians));
		m[0] = c; m[1] = 0; m[2] = -s;
		m[3] = 0; m[4] = 1.0; m[5] = 0;
		m[6] = s; m[7] = 0; m[8] = c;
		return m;
	}
	static matrix3x3 RotateZMatrix (T radians) {
		matrix3x3 m;
		T c = cosf(float(radians));
		T s = sinf(float(radians));
		m[0] = c; m[1] = s; m[2] = 0;
		m[3] = -s; m[4] = c; m[5] = 0;
		m[6] = 0; m[7] = 0; m[8] = 1.0;
		return m;
	}
	void Renormalize() {
		vector3<T> x = VectorX().Normalized();
		vector3<T> z = x.Cross(VectorY()).Normalized();
		vector3<T> y = z.Cross(x).Normalized();
		*this = *BuildFromVectors(x, y, z);
	}
	T& operator [] (const size_t i) { return cell[i]; }
	const T& operator[] (const size_t i) const { return cell[i]; }
//	const T* Data() const { return cell; }
//	T* Data() { return cell; }
//	friend matrix3x3 operator+ (const matrix3x3 &a, const matrix3x3 &b) {
//		matrix3x3 m;
//		for (int i=0; i<16; i++) m.cell[i] = a.cell[i] + b.cell[i];
//		return m;
//	}
//	friend matrix3x3 operator- (const matrix3x3 &a, const matrix3x3 &b) {
//		matrix3x3 m;
//		for (int i=0; i<16; i++) m.cell[i] = a.cell[i] - b.cell[i];
//		return m;
//	}
//	friend matrix3x3 operator- (const matrix3x3 &a) {
//		matrix3x3 m;
//		for (int i = 0; i < 16; ++i) { m.cell[i] = -a.cell[i]; }
//		return m;
//	}
	friend matrix3x3 operator* (const matrix3x3 &a, const matrix3x3 &b) {
		matrix3x3 m;
		m.cell[0] = a.cell[0]*b.cell[0] + a.cell[1]*b.cell[3] + a.cell[2]*b.cell[6];
		m.cell[1] = a.cell[0]*b.cell[1] + a.cell[1]*b.cell[4] + a.cell[2]*b.cell[7];
		m.cell[2] = a.cell[0]*b.cell[2] + a.cell[1]*b.cell[5] + a.cell[2]*b.cell[8];

		m.cell[3] = a.cell[3]*b.cell[0] + a.cell[4]*b.cell[3] + a.cell[5]*b.cell[6];
		m.cell[4] = a.cell[3]*b.cell[1] + a.cell[4]*b.cell[4] + a.cell[5]*b.cell[7];
		m.cell[5] = a.cell[3]*b.cell[2] + a.cell[4]*b.cell[5] + a.cell[5]*b.cell[8];

		m.cell[6] = a.cell[6]*b.cell[0] + a.cell[7]*b.cell[3] + a.cell[8]*b.cell[3];
		m.cell[7] = a.cell[6]*b.cell[1] + a.cell[7]*b.cell[4] + a.cell[8]*b.cell[4];
		m.cell[8] = a.cell[6]*b.cell[2] + a.cell[7]*b.cell[5] + a.cell[8]*b.cell[5];
		return m;
	}
	friend vector3<T> operator * (const matrix3x3 &a, const vector3<T> &v) {
		vector3<T> out;
		out.x = a.cell[0]*v.x + a.cell[1]*v.y + a.cell[2]*v.z;
		out.y = a.cell[3]*v.x + a.cell[4]*v.y + a.cell[5]*v.z;
		out.z = a.cell[6]*v.x + a.cell[7]*v.y + a.cell[8]*v.z;
		return out;
	}
	// V * M same as transpose(M) * V
	friend vector3<T> operator * (const vector3<T> &v, const matrix3x3 &a) {
		vector3<T> out;
		out.x = a.cell[0]*v.x + a.cell[3]*v.y + a.cell[6]*v.z;
		out.y = a.cell[1]*v.x + a.cell[4]*v.y + a.cell[7]*v.z;
		out.z = a.cell[2]*v.x + a.cell[5]*v.y + a.cell[8]*v.z;
		return out;
	}
//	friend matrix3x3 operator* (const matrix3x3 &a, T v) {
//		matrix3x3 m;
//		for (int i=0; i<16; i++) m[i] = a.cell[i] * v;
//		return m;
//	}
//	friend matrix3x3 operator* (T v, const matrix3x3 &a) {
//		return (a*v);
//	}
	matrix3x3 Transpose() const {
		matrix3x3 m;
		m[0] = cell[0]; m[1] = cell[3]; m[2] = cell[6];
		m[3] = cell[1]; m[4] = cell[4]; m[5] = cell[7];
		m[6] = cell[2]; m[7] = cell[5]; m[8] = cell[8];
		return m;
	}
	void Print () const {
		for (int i=0; i<3; i++) {
			printf ("%.2f %.2f %.2f\n", cell[3*i], cell[3*i+1], cell[3*i+2]);
		}
		printf ("\n");
	}
};

typedef matrix3x3<float> matrix3x3f;
typedef matrix3x3<double> matrix3x3d;

#endif /* _MATRIX3x3_H */
