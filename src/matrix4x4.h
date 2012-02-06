#ifndef _MATRIX4X4_H
#define _MATRIX4X4_H

#include <math.h>
#include <stdio.h>
#include "vector3.h"

template <typename T>
class matrix4x4 {
	private:
	T cell[16];
	public:
	matrix4x4 () {}
	matrix4x4 (T val) {
		cell[0] = cell[1] = cell[2] = cell[3] = cell[4] = cell[5] = cell[6] = 
		cell[7] = cell[8] = cell[9] = cell[10] = cell[11] = cell[12] = cell[13] = 
		cell[14] = cell[15] = val;
	}
	matrix4x4 (const T *vals) {
		memcpy(cell, vals, sizeof(T)*16);
	}
	void SetTranslate(const vector3<T> v) { cell[12] = v.x; cell[13] = v.y; cell[14] = v.z; }
	vector3<T> GetTranslate() const { return vector3<T>(cell[12], cell[13], cell[14]); }
	void SetRotationOnly(const matrix4x4& m) {
		for (int i=0; i<12; i++) cell[i] = m.cell[i];
	}
	// row-major 3x3 matrix
	void LoadFrom3x3Matrix(const T *r) {
		cell[0] = r[0]; cell[4] = r[1]; cell[8] = r[2]; cell[12] = 0;
		cell[1] = r[3]; cell[5] = r[4]; cell[9] = r[5]; cell[13] = 0;
		cell[2] = r[6]; cell[6] = r[7]; cell[10] = r[8]; cell[14] = 0;
		cell[3] = 0;    cell[7] = 0;    cell[11] = 0;    cell[15] = 1;
	}
	// row-major
	void SaveTo3x3Matrix(T *r) const {
		r[0] = cell[0]; r[1] = cell[4]; r[2] = cell[8];
		r[3] = cell[1]; r[4] = cell[5]; r[5] = cell[9];
		r[6] = cell[2]; r[7] = cell[6]; r[8] = cell[10];
	}
	static matrix4x4 Identity () {
		matrix4x4 m = matrix4x4(0.0);
		m.cell[0] = m.cell[5] = m.cell[10] = m.cell[15] = 1.0f;
		return m;
	}
	//glscale equivalent
	void Scale(T x, T y, T z) {
		*this = (*this) * ScaleMatrix (x, y, z);
	}
	void Scale(T s) {
		*this = (*this) * ScaleMatrix (s, s, s);
	}
	static matrix4x4 ScaleMatrix(T x, T y, T z) {
		matrix4x4 m;
		m[0] = x; m[1] = m[2] = m[3] = 0;
		m[5] = y; m[4] = m[6] = m[7] = 0;
		m[10] = z; m[8] = m[9] = m[11] = 0;
		m[12] = m[13] = m[14] = 0; m[15] = 1;
		return m;
	}
	static matrix4x4 ScaleMatrix(T scale) {
		matrix4x4 m;
		m[0] = scale; m[1] = m[2] = m[3] = 0;
		m[5] = scale; m[4] = m[6] = m[7] = 0;
		m[10] = scale; m[8] = m[9] = m[11] = 0;
		m[12] = m[13] = m[14] = 0; m[15] = 1;
		return m;
	}
	static matrix4x4 MakeRotMatrix(const vector3<T> &rx, const vector3<T> &ry, const vector3<T> &rz) {
		matrix4x4 m;
		m[0] = rx.x; m[4] = rx.y; m[8] = rx.z; m[12] = 0;
		m[1] = ry.x; m[5] = ry.y; m[9] = ry.z; m[13] = 0;
		m[2] = rz.x; m[6] = rz.y; m[10] = rz.z; m[14] = 0;
		m[3] = 0; m[7] = 0; m[11] = 0; m[15] = 1;
		return m;
	}
	static matrix4x4 MakeInvRotMatrix(const vector3<T> &rx, const vector3<T> &ry, const vector3<T> &rz) {
		matrix4x4 m;
		m[0] = rx.x; m[4] = ry.x; m[8] = rz.x; m[12] = 0;
		m[1] = rx.y; m[5] = ry.y; m[9] = rz.y; m[13] = 0;
		m[2] = rx.z; m[6] = ry.z; m[10] = rz.z; m[14] = 0;
		m[3] = 0; m[7] = 0; m[11] = 0; m[15] = 1;
		return m;
	}
	static matrix4x4 FrustumMatrix (T left, T right, T bottom, T top, T znear, T zfar) {
		assert((znear > T(0)) && (zfar > T(0)));
		// these expressions come from the documentation for glFrustum
		const T sx = (T(2) * znear) / (right - left);
		const T sy = (T(2) * znear) / (top - bottom);
		const T A = (right + left) / (right - left);
		const T B = (top + bottom) / (top - bottom);
		const T C = -(zfar + znear) / (zfar - znear);
		const T D = -(T(2) * zfar * znear) / (zfar - znear);
		matrix4x4 m;
		m[ 0] = sx; m[ 4] =  0; m[ 8] =  A; m[12] = 0;
		m[ 1] =  0; m[ 5] = sy; m[ 9] =  B; m[13] = 0;
		m[ 2] =  0; m[ 6] =  0; m[10] =  C; m[14] = D;
		m[ 3] =  0; m[ 7] =  0; m[11] = -1; m[15] = 0;
		return m;
	}
	//glRotate equivalent (except radians instead of degrees)
	void Rotate (T ang, T x, T y, T z) {
		*this = (*this) * RotateMatrix (ang, x, y, z);
	}
	// (x,y,z) must be normalized
	static matrix4x4 RotateMatrix (T ang, T x, T y, T z) {
		matrix4x4 m;
		T c = cos(ang);
		T s = sin(ang);
		m[0] = x*x*(1-c)+c;
		m[1] = y*x*(1-c)+z*s;
		m[2] = x*z*(1-c)-y*s;
		m[3] = 0;
		m[4] = x*y*(1-c)-z*s;
		m[5] = y*y*(1-c)+c;
		m[6] = y*z*(1-c)+x*s;
		m[7] = 0;
		m[8] = x*z*(1-c)+y*s;
		m[9] = y*z*(1-c)-x*s;
		m[10] = z*z*(1-c)+c;
		m[11] = 0;
		m[12] = 0;
		m[13] = 0;
		m[14] = 0;
		m[15] = 1;
		return m;
	}
	void RotateZ (T radians) { *this = (*this) * RotateZMatrix (radians); }
	void RotateY (T radians) { *this = (*this) * RotateYMatrix (radians); }
	void RotateX (T radians) { *this = (*this) * RotateXMatrix (radians); }
	static matrix4x4 RotateXMatrix (T radians) {
		matrix4x4 m;
		T cos_r = cosf (float(radians));
		T sin_r = sinf (float(radians));
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
	static matrix4x4 RotateYMatrix (T radians) {
		matrix4x4 m;
		T cos_r = cosf (float(radians));
		T sin_r = sinf (float(radians));
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
	static matrix4x4 RotateZMatrix (T radians) {
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
	void Renormalize() {
		vector3<T> x(cell[0], cell[4], cell[8]);
		vector3<T> y(cell[1], cell[5], cell[9]);
		vector3<T> z(cell[2], cell[6], cell[10]);
		x = x.Normalized();
		z = x.Cross(y).Normalized();
		y = z.Cross(x).Normalized();
		cell[0] = x.x; cell[4] = x.y; cell[8] = x.z;
		cell[1] = y.x; cell[5] = y.y; cell[9] = y.z;
		cell[2] = z.x; cell[6] = z.y; cell[10] = z.z;
	}
	void ClearToRotOnly() {
		cell[12] = 0;
		cell[13] = 0;
		cell[14] = 0;
	}
	T& operator [] (const size_t i) { return cell[i]; }
	const T& operator[] (const size_t i) const { return cell[i]; }
	const T* Data() const { return cell; }
	T* Data() { return cell; }
	friend matrix4x4 operator+ (const matrix4x4 &a, const matrix4x4 &b) {
		matrix4x4 m;
		for (int i=0; i<16; i++) m.cell[i] = a.cell[i] + b.cell[i];
		return m;
	}
	friend matrix4x4 operator- (const matrix4x4 &a, const matrix4x4 &b) {
		matrix4x4 m;
		for (int i=0; i<16; i++) m.cell[i] = a.cell[i] - b.cell[i];
		return m;
	}
	friend matrix4x4 operator* (const matrix4x4 &a, const matrix4x4 &b) {
		matrix4x4 m;
		m.cell[0] = a.cell[0]*b.cell[0] + a.cell[4]*b.cell[1] + a.cell[8]*b.cell[2] + a.cell[12]*b.cell[3];
		m.cell[1] = a.cell[1]*b.cell[0] + a.cell[5]*b.cell[1] + a.cell[9]*b.cell[2] + a.cell[13]*b.cell[3];
		m.cell[2] = a.cell[2]*b.cell[0] + a.cell[6]*b.cell[1] + a.cell[10]*b.cell[2] + a.cell[14]*b.cell[3];
		m.cell[3] = a.cell[3]*b.cell[0] + a.cell[7]*b.cell[1] + a.cell[11]*b.cell[2] + a.cell[15]*b.cell[3];

		m.cell[4] = a.cell[0]*b.cell[4] + a.cell[4]*b.cell[5] + a.cell[8]*b.cell[6] + a.cell[12]*b.cell[7];
		m.cell[5] = a.cell[1]*b.cell[4] + a.cell[5]*b.cell[5] + a.cell[9]*b.cell[6] + a.cell[13]*b.cell[7];
		m.cell[6] = a.cell[2]*b.cell[4] + a.cell[6]*b.cell[5] + a.cell[10]*b.cell[6] + a.cell[14]*b.cell[7];
		m.cell[7] = a.cell[3]*b.cell[4] + a.cell[7]*b.cell[5] + a.cell[11]*b.cell[6] + a.cell[15]*b.cell[7];

		m.cell[8] = a.cell[0]*b.cell[8] + a.cell[4]*b.cell[9] + a.cell[8]*b.cell[10] + a.cell[12]*b.cell[11];
		m.cell[9] = a.cell[1]*b.cell[8] + a.cell[5]*b.cell[9] + a.cell[9]*b.cell[10] + a.cell[13]*b.cell[11];
		m.cell[10] = a.cell[2]*b.cell[8] + a.cell[6]*b.cell[9] + a.cell[10]*b.cell[10] + a.cell[14]*b.cell[11];
		m.cell[11] = a.cell[3]*b.cell[8] + a.cell[7]*b.cell[9] + a.cell[11]*b.cell[10] + a.cell[15]*b.cell[11];

		m.cell[12] = a.cell[0]*b.cell[12] + a.cell[4]*b.cell[13] + a.cell[8]*b.cell[14] + a.cell[12]*b.cell[15];
		m.cell[13] = a.cell[1]*b.cell[12] + a.cell[5]*b.cell[13] + a.cell[9]*b.cell[14] + a.cell[13]*b.cell[15];
		m.cell[14] = a.cell[2]*b.cell[12] + a.cell[6]*b.cell[13] + a.cell[10]*b.cell[14] + a.cell[14]*b.cell[15];
		m.cell[15] = a.cell[3]*b.cell[12] + a.cell[7]*b.cell[13] + a.cell[11]*b.cell[14] + a.cell[15]*b.cell[15];
		return m;
	}
	friend vector3<T> operator * (const matrix4x4 &a, const vector3<T> &v) {
		vector3<T> out;
		out.x = a.cell[0]*v.x + a.cell[4]*v.y + a.cell[8]*v.z + a.cell[12];
		out.y = a.cell[1]*v.x + a.cell[5]*v.y + a.cell[9]*v.z + a.cell[13];
		out.z = a.cell[2]*v.x + a.cell[6]*v.y + a.cell[10]*v.z + a.cell[14];
		return out;
	}
	// scam for doing a transpose operation
	friend vector3<T> operator * (const vector3<T> &v, const matrix4x4 &a) {
		vector3<T> out;
		out.x = a.cell[0]*v.x + a.cell[1]*v.y + a.cell[2]*v.z;
		out.y = a.cell[4]*v.x + a.cell[5]*v.y + a.cell[6]*v.z;
		out.z = a.cell[8]*v.x + a.cell[9]*v.y + a.cell[10]*v.z;
		return out;
	}
	friend matrix4x4 operator* (const matrix4x4 &a, T v) {
		matrix4x4 m;
		for (int i=0; i<16; i++) m[i] = a.cell[i] * v;
		return m;
	}
	friend matrix4x4 operator* (T v, const matrix4x4 &a) {
		return (a*v);
	}
	vector3<T> ApplyRotationOnly (const vector3<T> &v) const {
		vector3<T> out;
		out.x = cell[0]*v.x + cell[4]*v.y + cell[8]*v.z;
		out.y = cell[1]*v.x + cell[5]*v.y + cell[9]*v.z;
		out.z = cell[2]*v.x + cell[6]*v.y + cell[10]*v.z;
		return out;
	}
	//gltranslate equivalent
	void Translate(const vector3<T> &t) {
		Translate(t.x, t.y, t.z);
	}
	void Translate(T x, T y, T z) {
		matrix4x4 m = Identity ();
		m[12] = x;
		m[13] = y;
		m[14] = z;
		*this = (*this) * m;
	}
	static matrix4x4 Translation(const vector3<T> &v) {
		return Translation(v.x, v.y, v.z);
	}
	static matrix4x4 Translation(T x, T y, T z) {
		matrix4x4 m = Identity ();
		m[12] = x;
		m[13] = y;
		m[14] = z;
		return m;
	}
	matrix4x4 InverseOf () const {
		matrix4x4 m;
		// this only works for matrices containing only rotation and transform
		m[0] = cell[0]; m[1] = cell[4]; m[2] = cell[8];
		m[4] = cell[1]; m[5] = cell[5]; m[6] = cell[9];
		m[8] = cell[2]; m[9] = cell[6]; m[10] = cell[10];
		m[12] = -(cell[0]*cell[12] + cell[1]*cell[13] + cell[2]*cell[14]);
		m[13] = -(cell[4]*cell[12] + cell[5]*cell[13] + cell[6]*cell[14]);
		m[14] = -(cell[8]*cell[12] + cell[9]*cell[13] + cell[10]*cell[14]);
		m[3] = m[7] = m[11] = 0;
		m[15] = 1.0f;

		return m;
	}
	void Print () const {
		for (int i=0; i<4; i++) {
			printf ("%.2f %.2f %.2f %.2f\n", cell[i], cell[i+4], cell[i+8], cell[i+12]);
		}
		printf ("\n");
	}
};

typedef matrix4x4<float> matrix4x4f;
typedef matrix4x4<double> matrix4x4d;

#endif /* _MATRIX4X4_H */
