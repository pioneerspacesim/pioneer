#ifndef __JJVECTOR_H__
#define __JJVECTOR_H__
#include <math.h>

#define JJM_PI 3.141592653589793
#define JJM_FPI 3.1415926f

struct Vector
{
	float x, y, z;
};

struct DVector
{
	double x, y, z;
};

struct Quaternion
{
	float w, x, y, z;
};

struct AxisAng
{
	Vector v;
	float a;
};

struct Plane64
{
	DVector norm;
	double dist;
};

struct Matrix
{
	float x1, x2, x3;	// _11, _12, _13
	float y1, y2, y3;	// _21, _22, _23
	float z1, z2, z3;	// _31, _32, _33
};

struct DMatrix
{
	double x1, x2, x3;	// _11, _12, _13
	double y1, y2, y3;	// _21, _22, _23
	double z1, z2, z3;	// _31, _32, _33
};

const Vector zero_vector = {0, 0, 0};
const DVector zero_dvector = {0, 0, 0};
const Matrix identity_matrix = {1, 0, 0, 0, 1, 0, 0, 0, 1};
const DMatrix identity_dmatrix = {1, 0, 0, 0, 1, 0, 0, 0, 1};

// conversion functions

inline void DVecToVec (DVector *v1, Vector *v2)
{
	v2->x = (float)v1->x;
	v2->y = (float)v1->y;
	v2->z = (float)v1->z;
}

inline void VecToDVec (Vector *v1, DVector *v2)
{
	v2->x = v1->x;
	v2->y = v1->y;
	v2->z = v1->z;
}

inline void DMatToMat (DMatrix *m1, Matrix *m2)
{
	m2->x1 = (float)m1->x1;
	m2->x2 = (float)m1->x2;
	m2->x3 = (float)m1->x3;
	m2->y1 = (float)m1->y1;
	m2->y2 = (float)m1->y2;
	m2->y3 = (float)m1->y3;
	m2->z1 = (float)m1->z1;
	m2->z2 = (float)m1->z2;
	m2->z3 = (float)m1->z3;
}

inline void MatToDMat (Matrix *m1, DMatrix *m2)
{
	m2->x1 = m1->x1;
	m2->x2 = m1->x2;
	m2->x3 = m1->x3;
	m2->y1 = m1->y1;
	m2->y2 = m1->y2;
	m2->y3 = m1->y3;
	m2->z1 = m1->z1;
	m2->z2 = m1->z2;
	m2->z3 = m1->z3;
}

inline void DMatTrans (DMatrix *m1, DMatrix *m2)
{
	m2->x1 = m1->x1;
	m2->x2 = m1->y1;
	m2->x3 = m1->z1;
	m2->y1 = m1->x2;
	m2->y2 = m1->y2;
	m2->y3 = m1->z2;
	m2->z1 = m1->x3;
	m2->z2 = m1->y3;
	m2->z3 = m1->z3;
}	

inline void VecSet (float x, float y, float z, Vector *res)
{
	res->x = x;
	res->y = y;
	res->z = z;
}

inline void DVecSet (double x, double y, double z, DVector *res)
{
	res->x = x;
	res->y = y;
	res->z = z;
}

inline void VecRotate (Vector *v, Vector *res)
{
	res->x = v->y; res->y = v->z; res->z = v->x;
}

inline void VecRotate (DVector *v, DVector *res)
{
	res->x = v->y; res->y = v->z; res->z = v->x;
}

void MatToAxisAng (Matrix *m, AxisAng *aa);
void AxisAngToMat (AxisAng *aa, Matrix *m);

void MatToQuat (Matrix *m1, Quaternion *res);
void QuatToMat (Quaternion *q1, Matrix *res);


// Note: MatMult, MatTMult, VecCross and FindNormal will break if 
// called with output and input objects the same.


// Actual functions, maybe

inline void MatMatMult (Matrix *m1, Matrix *m2, Matrix *res)
{
	res->x1 = m1->x1*m2->x1 + m1->x2*m2->y1 + m1->x3*m2->z1;
	res->x2 = m1->x1*m2->x2 + m1->x2*m2->y2 + m1->x3*m2->z2;
	res->x3 = m1->x1*m2->x3 + m1->x2*m2->y3 + m1->x3*m2->z3;

	res->y1 = m1->y1*m2->x1 + m1->y2*m2->y1 + m1->y3*m2->z1;
	res->y2 = m1->y1*m2->x2 + m1->y2*m2->y2 + m1->y3*m2->z2;
	res->y3 = m1->y1*m2->x3 + m1->y2*m2->y3 + m1->y3*m2->z3;

	res->z1 = m1->z1*m2->x1 + m1->z2*m2->y1 + m1->z3*m2->z1;
	res->z2 = m1->z1*m2->x2 + m1->z2*m2->y2 + m1->z3*m2->z2;
	res->z3 = m1->z1*m2->x3 + m1->z2*m2->y3 + m1->z3*m2->z3;
}

inline void DMatDMatMult (DMatrix *m1, DMatrix *m2, DMatrix *res)
{
	res->x1 = m1->x1*m2->x1 + m1->x2*m2->y1 + m1->x3*m2->z1;
	res->x2 = m1->x1*m2->x2 + m1->x2*m2->y2 + m1->x3*m2->z2;
	res->x3 = m1->x1*m2->x3 + m1->x2*m2->y3 + m1->x3*m2->z3;

	res->y1 = m1->y1*m2->x1 + m1->y2*m2->y1 + m1->y3*m2->z1;
	res->y2 = m1->y1*m2->x2 + m1->y2*m2->y2 + m1->y3*m2->z2;
	res->y3 = m1->y1*m2->x3 + m1->y2*m2->y3 + m1->y3*m2->z3;

	res->z1 = m1->z1*m2->x1 + m1->z2*m2->y1 + m1->z3*m2->z1;
	res->z2 = m1->z1*m2->x2 + m1->z2*m2->y2 + m1->z3*m2->z2;
	res->z3 = m1->z1*m2->x3 + m1->z2*m2->y3 + m1->z3*m2->z3;
}

inline void MatTMatMult (Matrix *m1, Matrix *m2, Matrix *res)
{
	res->x1 = m1->x1*m2->x1 + m1->y1*m2->y1 + m1->z1*m2->z1;
	res->x2 = m1->x1*m2->x2 + m1->y1*m2->y2 + m1->z1*m2->z2;
	res->x3 = m1->x1*m2->x3 + m1->y1*m2->y3 + m1->z1*m2->z3;

	res->y1 = m1->x2*m2->x1 + m1->y2*m2->y1 + m1->z2*m2->z1;
	res->y2 = m1->x2*m2->x2 + m1->y2*m2->y2 + m1->z2*m2->z2;
	res->y3 = m1->x2*m2->x3 + m1->y2*m2->y3 + m1->z2*m2->z3;

	res->z1 = m1->x3*m2->x1 + m1->y3*m2->y1 + m1->z3*m2->z1;
	res->z2 = m1->x3*m2->x2 + m1->y3*m2->y2 + m1->z3*m2->z2;
	res->z3 = m1->x3*m2->x3 + m1->y3*m2->y3 + m1->z3*m2->z3;
}

inline void DMatTDMatMult (DMatrix *m1, DMatrix *m2, DMatrix *res)
{
	res->x1 = m1->x1*m2->x1 + m1->y1*m2->y1 + m1->z1*m2->z1;
	res->x2 = m1->x1*m2->x2 + m1->y1*m2->y2 + m1->z1*m2->z2;
	res->x3 = m1->x1*m2->x3 + m1->y1*m2->y3 + m1->z1*m2->z3;

	res->y1 = m1->x2*m2->x1 + m1->y2*m2->y1 + m1->z2*m2->z1;
	res->y2 = m1->x2*m2->x2 + m1->y2*m2->y2 + m1->z2*m2->z2;
	res->y3 = m1->x2*m2->x3 + m1->y2*m2->y3 + m1->z2*m2->z3;

	res->z1 = m1->x3*m2->x1 + m1->y3*m2->y1 + m1->z3*m2->z1;
	res->z2 = m1->x3*m2->x2 + m1->y3*m2->y2 + m1->z3*m2->z2;
	res->z3 = m1->x3*m2->x3 + m1->y3*m2->y3 + m1->z3*m2->z3;
}


inline void MatVecMult (Matrix *m1, Vector *v1, Vector *res)
{
	res->x = m1->x1*v1->x + m1->x2*v1->y + m1->x3*v1->z;
	res->y = m1->y1*v1->x + m1->y2*v1->y + m1->y3*v1->z;
	res->z = m1->z1*v1->x + m1->z2*v1->y + m1->z3*v1->z;
}

inline void MatDVecMult (Matrix *m1, DVector *v1, DVector *res)
{
	res->x = m1->x1*v1->x + m1->x2*v1->y + m1->x3*v1->z;
	res->y = m1->y1*v1->x + m1->y2*v1->y + m1->y3*v1->z;
	res->z = m1->z1*v1->x + m1->z2*v1->y + m1->z3*v1->z;
}

inline void DMatVecMult (DMatrix *m1, Vector *v1, DVector *res)
{
	res->x = m1->x1*v1->x + m1->x2*v1->y + m1->x3*v1->z;
	res->y = m1->y1*v1->x + m1->y2*v1->y + m1->y3*v1->z;
	res->z = m1->z1*v1->x + m1->z2*v1->y + m1->z3*v1->z;
}

inline void DMatVecMult (DMatrix *m1, Vector *v1, Vector *res)
{
	res->x = (float)(m1->x1*v1->x + m1->x2*v1->y + m1->x3*v1->z);
	res->y = (float)(m1->y1*v1->x + m1->y2*v1->y + m1->y3*v1->z);
	res->z = (float)(m1->z1*v1->x + m1->z2*v1->y + m1->z3*v1->z);
}

inline void DMatDVecMult (DMatrix *m1, DVector *v1, DVector *res)
{
	res->x = m1->x1*v1->x + m1->x2*v1->y + m1->x3*v1->z;
	res->y = m1->y1*v1->x + m1->y2*v1->y + m1->y3*v1->z;
	res->z = m1->z1*v1->x + m1->z2*v1->y + m1->z3*v1->z;
}

inline void MatTVecMult (Matrix *m1, Vector *v1, Vector *res)
{
	res->x = m1->x1*v1->x + m1->y1*v1->y + m1->z1*v1->z;
	res->y = m1->x2*v1->x + m1->y2*v1->y + m1->z2*v1->z;
	res->z = m1->x3*v1->x + m1->y3*v1->y + m1->z3*v1->z;
}

inline void MatTDVecMult (Matrix *m1, DVector *v1, DVector *res)
{
	res->x = m1->x1*v1->x + m1->y1*v1->y + m1->z1*v1->z;
	res->y = m1->x2*v1->x + m1->y2*v1->y + m1->z2*v1->z;
	res->z = m1->x3*v1->x + m1->y3*v1->y + m1->z3*v1->z;
}

inline void DMatTDVecMult (DMatrix *m1, DVector *v1, DVector *res)
{
	res->x = m1->x1*v1->x + m1->y1*v1->y + m1->z1*v1->z;
	res->y = m1->x2*v1->x + m1->y2*v1->y + m1->z2*v1->z;
	res->z = m1->x3*v1->x + m1->y3*v1->y + m1->z3*v1->z;
}

inline void VecMul (Vector *v1, double m, Vector *res)
{
	res->x = (float)(v1->x * m);
	res->y = (float)(v1->y * m);
	res->z = (float)(v1->z * m);
}

inline void VecMul (DVector *v1, double m, Vector *res)
{
	res->x = (float)(v1->x * m);
	res->y = (float)(v1->y * m);
	res->z = (float)(v1->z * m);
}

inline void VecMul (DVector *v1, double m, DVector *res)
{
	res->x = v1->x * m;
	res->y = v1->y * m;
	res->z = v1->z * m;
}

inline void VecMul (Vector *v1, double m, DVector *res)
{
	res->x = v1->x * m;
	res->y = v1->y * m;
	res->z = v1->z * m;
}

inline void VecInv (Vector *v1, Vector *res)
{
	res->x = -v1->x;
	res->y = -v1->y;
	res->z = -v1->z;
}

inline void VecInv (DVector *v1, DVector *res)
{
	res->x = -v1->x;
	res->y = -v1->y;
	res->z = -v1->z;
}

inline void VecSub (Vector *v1, Vector *v2, Vector *res)
{
	res->x = v1->x - v2->x;
	res->y = v1->y - v2->y;
	res->z = v1->z - v2->z;
}

inline void VecSub (DVector *v1, DVector *v2, DVector *res)
{
	res->x = v1->x - v2->x;
	res->y = v1->y - v2->y;
	res->z = v1->z - v2->z;
}

inline void VecSub (DVector *v1, DVector *v2, Vector *res)
{
	res->x = (float)(v1->x - v2->x);
	res->y = (float)(v1->y - v2->y);
	res->z = (float)(v1->z - v2->z);
}

inline void VecAdd (DVector *v1, DVector *v2, Vector *res)
{
	res->x = (float)(v1->x + v2->x);
	res->y = (float)(v1->y + v2->y);
	res->z = (float)(v1->z + v2->z);
}

inline void VecAdd (DVector *v1, Vector *v2, DVector *res)
{
	res->x = v1->x + v2->x;
	res->y = v1->y + v2->y;
	res->z = v1->z + v2->z;
}

inline void VecAdd (Vector *v1, Vector *v2, Vector *res)
{
	res->x = v1->x + v2->x;
	res->y = v1->y + v2->y;
	res->z = v1->z + v2->z;
}

inline void VecAdd (Vector *v1, DVector *v2, Vector *res)
{
	res->x = (float)(v1->x + v2->x);
	res->y = (float)(v1->y + v2->y);
	res->z = (float)(v1->z + v2->z);
}

inline void VecAdd (DVector *v1, DVector *v2, DVector *res)
{
	res->x = v1->x + v2->x;
	res->y = v1->y + v2->y;
	res->z = v1->z + v2->z;
}

inline float VecDot (Vector *v1, Vector *v2)
{
	return (v1->x*v2->x + v1->y*v2->y + v1->z*v2->z);
}

inline double VecDot (DVector *v1, DVector *v2)
{
	return (v1->x*v2->x + v1->y*v2->y + v1->z*v2->z);
}

inline double VecDot (DVector *v1, Vector *v2)
{
	return (v1->x*v2->x + v1->y*v2->y + v1->z*v2->z);
}

inline float VecDot (Vector *v)
{
	return (v->x*v->x + v->y*v->y + v->z*v->z);
}

inline double VecDot (DVector *v)
{
	return (v->x*v->x + v->y*v->y + v->z*v->z);
}


inline float VecMag (Vector *v1)
{
	return (float) sqrt (VecDot (v1, v1));
}

inline double VecMag (DVector *v1)
{
	return sqrt (VecDot (v1, v1));
}

inline void VecCross (Vector *v1, Vector *v2, Vector *res)
{
	res->x = v1->y*v2->z - v1->z*v2->y;
	res->y = v1->z*v2->x - v1->x*v2->z;
	res->z = v1->x*v2->y - v1->y*v2->x;
}

inline void VecCross (DVector *v1, DVector *v2, DVector *res)
{
	res->x = v1->y*v2->z - v1->z*v2->y;
	res->y = v1->z*v2->x - v1->x*v2->z;
	res->z = v1->x*v2->y - v1->y*v2->x;
}

inline int VecNorm (Vector *v1, Vector *res)
{
	float temp = VecDot (v1, v1);
	if (temp == 0.0f) { *res = zero_vector; return 0; }
	temp = 1.0f / float(sqrt(temp));

	res->x = v1->x * temp;
	res->y = v1->y * temp;
	res->z = v1->z * temp;
	return 1;
} 

inline int VecNorm (DVector *v1, DVector *res)
{
	double temp = VecDot (v1, v1);
	if (temp == 0.0) { *res = zero_dvector; return 0; }
	temp = 1.0 / sqrt(temp);

	res->x = v1->x * temp;
	res->y = v1->y * temp;
	res->z = v1->z * temp;
	return 1;
} 

// returns component of vec1 perpendicular to vec2
inline void VecPerp (Vector *v1, Vector *v2, Vector *res)
{
	Vector n, t;
	VecNorm (v2, &n);
	VecMul (&n, VecDot (v1, &n), &t);
	VecSub (v1, &t, res);
}


inline void VecMax (Vector *in, Vector *max, Vector *res)
{
	if (in->x < max->x) res->x = max->x; else res->x = in->x;
	if (in->y < max->y) res->y = max->y; else res->y = in->y;
	if (in->z < max->z) res->z = max->z; else res->z = in->z;
}

inline void VecMin (Vector *in, Vector *min, Vector *res)
{
	if (in->x > min->x) res->x = min->x; else res->x = in->x;
	if (in->y > min->y) res->y = min->y; else res->y = in->y;
	if (in->z > min->z) res->z = min->z; else res->z = in->z;
}

inline void VecClip (Vector *in, Vector *max, Vector *min, Vector *res)
{
	if (in->x > max->x) res->x = max->x;
	else if (in->x < min->x) res->x = min->x; else res->x = in->x;
	if (in->y > max->y) res->y = max->y;
	else if (in->y < min->y) res->y = min->y; else res->y = in->y;
	if (in->z > max->z) res->z = max->z;
	else if (in->z < min->z) res->z = min->z; else res->z = in->z;

}


// Note - obeys RH rule p2-p1-p3

inline int FindNormal (Vector *p1, Vector *p2, Vector *p3, Vector *res)
{
	res->x = (p2->y-p1->y) * (p3->z-p1->z) - (p2->z-p1->z) * (p3->y-p1->y);
	res->y = (p2->z-p1->z) * (p3->x-p1->x) - (p2->x-p1->x) * (p3->z-p1->z);
	res->z = (p2->x-p1->x) * (p3->y-p1->y) - (p2->y-p1->y) * (p3->x-p1->x);

	return VecNorm (res, res);
}

inline int FindNormal (DVector *p1, DVector *p2, DVector *p3, DVector *res)
{
	res->x = (p2->y-p1->y) * (p3->z-p1->z) - (p2->z-p1->z) * (p3->y-p1->y);
	res->y = (p2->z-p1->z) * (p3->x-p1->x) - (p2->x-p1->x) * (p3->z-p1->z);
	res->z = (p2->x-p1->x) * (p3->y-p1->y) - (p2->y-p1->y) * (p3->x-p1->x);

	return VecNorm (res, res);
}

inline int FindNormal (Vector *p1, Vector *p2, Vector *p3, DVector *res)
{
	res->x = ((double)p2->y-p1->y) * ((double)p3->z-p1->z);
	res->x -= ((double)p2->z-p1->z) * ((double)p3->y-p1->y);
	res->y = ((double)p2->z-p1->z) * ((double)p3->x-p1->x); 
	res->y -= ((double)p2->x-p1->x) * ((double)p3->z-p1->z);
	res->z = ((double)p2->x-p1->x) * ((double)p3->y-p1->y);
	res->z -= ((double)p2->y-p1->y) * ((double)p3->x-p1->x);

	return VecNorm (res, res);
}


inline void BuildMatrix (float xr, float yr, float zr, Matrix *m1)
{
	double sx = sin (xr), cx = cos (xr);
	double sy = sin (yr), cy = cos (yr);
	double sz = sin (zr), cz = cos (zr);

	m1->x1 = float(cz*cy + sz*sx*sy);
	m1->x2 = float(sz*cx);
	m1->x3 = float(-cz*sy + sz*sx*cy);
	m1->y1 = float(-sz*cy + cz*sx*sy); 
	m1->y2 = float(cz*cx);
	m1->y3 = float(sz*sy + cz*sx*cy);
	m1->z1 = float(cx*sy);
	m1->z2 = float(-sx);
	m1->z3 = float(cx*cy);
}

inline void BuildMatrix (double xr, double yr, double zr, DMatrix *m1)
{
	double sx = sin (xr), cx = cos (xr);
	double sy = sin (yr), cy = cos (yr);
	double sz = sin (zr), cz = cos (zr);

	m1->x1 = cz*cy + sz*sx*sy;
	m1->x2 = sz*cx;
	m1->x3 = -cz*sy + sz*sx*cy;
	m1->y1 = -sz*cy + cz*sx*sy; 
	m1->y2 = cz*cx;
	m1->y3 = sz*sy + cz*sx*cy;
	m1->z1 = cx*sy;
	m1->z2 = -sx;
	m1->z3 = cx*cy;
}

inline void MakeVecRotMatrix (Vector *v, Matrix *m)
{
	float r, div;
	if ((r = VecDot (v, v)) < 1.0e-20f) {
		*m = identity_matrix;
		return;
	}
	r = float(sqrt(r));
	div = 1 / r;

	Vector n;
	n.x = v->x * div;
	n.y = v->y * div;
	n.z = v->z * div;

    float cr = cosf(r);
	float cri = 1.0f - cr;
    float sr = sinf(r);

	m->x1 = (n.x * n.x) * cri + cr;
    m->x2 = (n.x * n.y) * cri - (n.z * sr);
    m->x3 = (n.x * n.z) * cri + (n.y * sr);

    m->y1 = (n.y * n.x) * cri + (n.z * sr);
    m->y2 = (n.y * n.y) * cri + cr ;
    m->y3 = (n.y * n.z) * cri - (n.x * sr);

    m->z1 = (n.z * n.x) * cri - (n.y * sr);
    m->z2 = (n.z * n.y) * cri + (n.x * sr);
    m->z3 = (n.z * n.z) * cri + cr;
}

inline void MakeVecRotDMatrix (DVector *v, DMatrix *m)
{
	double r, div;
	if ((r = VecDot (v, v)) < 1.0e-20f) {
		*m = identity_dmatrix;
		return;
	}
	r = sqrt(r);
	div = 1 / r;

	DVector n;
	n.x = v->x * div;
	n.y = v->y * div;
	n.z = v->z * div;

    double cr = cos(r);
	double cri = 1.0 - cr;
    double sr = sin(r);

	m->x1 = (n.x * n.x) * cri + cr;
    m->x2 = (n.x * n.y) * cri - (n.z * sr);
    m->x3 = (n.x * n.z) * cri + (n.y * sr);

    m->y1 = (n.y * n.x) * cri + (n.z * sr);
    m->y2 = (n.y * n.y) * cri + cr ;
    m->y3 = (n.y * n.z) * cri - (n.x * sr);

    m->z1 = (n.z * n.x) * cri - (n.y * sr);
    m->z2 = (n.z * n.y) * cri + (n.x * sr);
    m->z3 = (n.z * n.z) * cri + cr;
}


#endif /* __JJVECTOR_H__ */
