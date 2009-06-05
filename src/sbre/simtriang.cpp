#include <stdlib.h>
#include "sbre_int.h"


void ResolveLinearInterp (Vector *p0, Vector *p1, float t, Vector *pRes)
{
	Vector tv;
	VecSub (p1, p0, &tv);
	VecMul (&tv, t, &tv);
	VecAdd (&tv, p0, pRes);
}

void ResolveQuadraticSpline (Vector *p0, Vector *p1, Vector *p2, float t, Vector *pRes)
{
	float invt = 1.0f - t;
	Vector tv;
	VecMul (p0, invt*invt, pRes);
	VecMul (p1, 2.0f*t*invt, &tv);
	VecAdd (&tv, pRes, pRes);
	VecMul (p2, t*t, &tv);
	VecAdd (&tv, pRes, pRes);
}

void ResolveCubicSpline (Vector *p0, Vector *p1, Vector *p2, Vector *p3, float t, Vector *pRes)
{
	float invt = 1.0f - t;
	Vector tv1, tv2, tv;
	VecMul (p0, invt*invt*invt, &tv1);
	VecMul (p3, t*t*t, &tv);
	VecAdd (&tv, &tv1, &tv1);
	VecMul (p1, 3.0f*t*invt*invt, &tv2);
	VecMul (p2, 3.0f*t*t*invt, &tv);
	VecAdd (&tv, &tv2, &tv2);
	VecAdd (&tv1, &tv2, pRes);
}

void ResolveHermiteSpline (Vector *p0, Vector *p1, Vector *n0, Vector *n1, float t, Vector *pRes)
{
	float t2 = t*t, t3 = t*t*t;
	Vector tv1, tv2, tv;
	VecMul (p0, 2*t3-3*t2+1, &tv1);
	VecMul (n0, t3-2*t2+t, &tv);
	VecAdd (&tv, &tv1, &tv1);
	VecMul (p1, -2*t3+3*t2, &tv2);
	VecMul (n1, t3-t2, &tv);
	VecAdd (&tv, &tv2, &tv2);
	VecAdd (&tv1, &tv2, pRes);
}

void ResolveHermiteTangent (Vector *p0, Vector *p1, Vector *n0, Vector *n1, float t, Vector *pRes)
{
	float t2 = t*t;
	Vector tv1, tv2, tv;
	VecMul (p0, 6*t2-6*t, &tv1);
	VecMul (n0, 3*t2-4*t+1, &tv);
	VecAdd (&tv, &tv1, &tv1);
	VecMul (p1, -6*t2+6*t, &tv2);
	VecMul (n1, 3*t2-2*t, &tv);
	VecAdd (&tv, &tv2, &tv2);
	VecAdd (&tv1, &tv2, pRes);
}

//**************************************************************************************

struct TriangPoint
{
	Vector pos;					// base pos, norm
	Vector norm;
	int num;			// count of valid vertices
	uint16 pIndex[TRIANG_MAXSTEPS+1];		// index of each vertex, inside to outside
};

static uint16 pIndex[6*TRIANG_MAXPOINTS*(TRIANG_MAXSTEPS+1)];
static Vector pVertex[2*TRIANG_MAXPOINTS*(TRIANG_MAXSTEPS+1)];

static TriangPoint pPoint[TRIANG_MAXPOINTS];
static int numPoints = 0;


void TriangAddPoint (Vector *pPos, Vector *pNorm)
{
	if (numPoints == TRIANG_MAXPOINTS) return;
	TriangPoint *tp = pPoint + numPoints++;
	tp->pos = *pPos; tp->norm = *pNorm;
}

void Triangulate (Vector *pCPos, Vector *pCNorm, int steps,
	Vector **ppVtx, int *pNV, uint16 **ppIndex, int *pNI)
{
	// Ok. For each point, find number of increments
	// and generate intermediate values

	int nv = 0, ni = 0;
	int i; for (int i=0; i<numPoints; i++)
	{
		Vector tv;			//, tnorm, tnorm2;
		TriangPoint *pCur = pPoint+i;
		VecSub (pCPos, &pCur->pos, &tv);
//		float len = sqrt (VecDot (&tv, &tv));
//		VecCross (&pCur->norm, pCNorm, &tnorm);
//		VecCross (pCNorm, &tv, &tnorm2);
//		if (VecDot (&tnorm, &tnorm2) < 0.0f) VecInv (&tnorm, &tnorm);
	
		pVertex[nv] = pCur->pos;			// add first vertex to array
		pVertex[nv+1] = pCur->norm;
		pCur->pIndex[0] = nv>>1; nv+=2;

		Vector t0, t1;						// find tangents
		VecMul (&pCur->norm, VecDot (&tv, &pCur->norm), &t0);
		VecSub (&tv, &t0, &t0);
		VecMul (pCNorm, VecDot (&tv, pCNorm), &t1);
		VecSub (&tv, &t1, &t1);

		if (steps > TRIANG_MAXSTEPS) pCur->num = TRIANG_MAXSTEPS;
		else pCur->num = steps;
		static const float pInv[] = { 1.0f, 0.5f, 0.3333333f, 0.25f, 0.2f,
			0.1666667f, 0.1428571f, 0.125f, 0.1111111f, 0.1f };

		float t, inc = pInv[pCur->num];
		int j; for (t=inc, j=1; j<=pCur->num; j++, t+=inc)
		{
			ResolveHermiteSpline (&pCur->pos, pCPos, &t0, &t1, t, pVertex+nv);
//			ResolveHermiteTangent (&pCur->pos, pCPos, &t0, &t1, t, &tv);
//			VecCross (&tv, &tnorm, pVertex+nv+1);
			ResolveLinearInterp (&pCur->norm, pCNorm, t, pVertex+nv+1);
			pCur->pIndex[j] = nv>>1; nv+=2;
		}
	}
	pVertex[nv] = *pCPos;			// add centre vertex to array
	pVertex[nv+1] = *pCNorm;
	int cindex = nv>>1; nv+=2;

	// Now render each radial strip from the centre

	for (i=0; i<numPoints; i++)
	{
		TriangPoint *pCur = pPoint + i;
		TriangPoint *pNext = pPoint + (i+1==numPoints?0:i+1);
		pIndex[ni++] = cindex;
		pIndex[ni++] = pCur->pIndex[pCur->num];
		pIndex[ni++] = pNext->pIndex[pNext->num];

		int vc = pCur->num, vn = pNext->num;
		while (vc|vn)
		{
			if (vc) {
				pIndex[ni++] = pCur->pIndex[vc];
				pIndex[ni++] = pCur->pIndex[--vc];
				pIndex[ni++] = pNext->pIndex[vn];
			}
			if (vn) {
				pIndex[ni++] = pNext->pIndex[vn];
				pIndex[ni++] = pCur->pIndex[vc];
				pIndex[ni++] = pNext->pIndex[--vn];
			}
		}		
	}

	// return data
	*pNI = ni; *pNV = nv>>1;
	*ppIndex = pIndex;
	*ppVtx = pVertex;

	numPoints = 0;
	return;
}

