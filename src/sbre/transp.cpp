#include <malloc.h>
#include "sbre_int.h"


static const int pNumIndex[3] =
	{ (2*4+1*8)*3, (2*8+5*16)*3, (2*16+13*32)*3 };

static Vector pTVertex4pt[2*4+2];
static Vector pTVertex8pt[6*8+2];
static Vector pTVertex16pt[14*16+2];

static uint16 pTIndex4pt[(2*4+1*8)*3];
static uint16 pTIndex8pt[(2*8+5*16)*3];
static uint16 pTIndex16pt[(2*16+13*32)*3];

static Vector *ppTVertex[3] =
	{ pTVertex4pt, pTVertex8pt, pTVertex16pt };

static uint16 *ppTIndex[3] =
	{ pTIndex4pt, pTIndex8pt, pTIndex16pt };


/*
static void MakeRotationalSolid (Vector *pPos, int n, int r, Vector *pRes)
{
	int i, j, k;
	for (i=1; i<n-1; i++)
	{
		pRes[i
		float angstep = 2.0f * 3.141592f / steps, ang = 0.0f;
		for (=1; i<n; i++, ang+=angstep, pCur++)
	{
		pCur->x = sin(ang) * pPos->y;
		pCur->y = cos(ang) * pPos->y;
		pCur->z = pPos->z;
	}

	}
		*pCur = pos0; pCur++;
		*pCur = pos1; pCur++;
}
*/


static void GenerateThrusters ()
{
	Vector pos0 = { 0.0f, 0.0f, 0.0f };
	Vector pos1 = { 0.0f, 0.0f, 1.0f };
	Vector tan0 = { 0.0f, 1.0f, 0.2f };
	Vector tan1 = { 0.0f, -0.2f, 1.0f };

	int j, n;
	for (j=0, n=4; j<3; j++, n<<=1)
	{
		Vector *pCur = ppTVertex[j];
		float t, incstep = 1.0f / (n-1);
		int i; for (i=0, t=incstep; i<n-2; i++, t+=incstep)
		{
			Vector *pPos = pCur; pCur++;
			ResolveHermiteSpline (&pos0, &pos1, &tan0, &tan1, t, pPos);

			float angstep = 2.0f * 3.141592f / n, ang = angstep;
			int i; for (i=1; i<n; i++, ang+=angstep, pCur++)
			{
				pCur->x = sin(ang) * pPos->y;
				pCur->y = cos(ang) * pPos->y;
				pCur->z = pPos->z;
			}
		}
		*pCur = pos0; pCur++;
		*pCur = pos1; pCur++;

		int ni=0, k;
		uint16 *pIndex = ppTIndex[j];

		for (k=0; k<n; k++) {
			int k1 = k+1==n ? 0 : k+1;
			pIndex[ni++] = (n-2)*n; pIndex[ni++] = k; pIndex[ni++] = k1;
			pIndex[ni++] = (n-2)*n+1; pIndex[ni++] = k1+(n-3)*n; pIndex[ni++] = k+(n-3)*n;
		}
		for (i=0; i<n-3; i++)
		{
			for (k=0; k<n; k++) {
				int k1 = k+1==n ? 0 : k+1;
				pIndex[ni++] = k+i*n; pIndex[ni++] = k+i*n+n; pIndex[ni++] = k1+i*n;
				pIndex[ni++] = k1+i*n; pIndex[ni++] = k+i*n+n;	pIndex[ni++] = k1+i*n+n;
			}
		}
	}
}



static void RenderThruster (RState *pState, Thruster *pThruster, Vector *pPos, Vector *pDir)
{
//	Thruster *pThruster = pState->pModel->pThruster + index;

	Vector start, end, dir = *pDir;
	VecMul (pPos, pState->scale, &start);
	float power = -VecDot (&dir, (Vector *)pState->pObjParam->linthrust);

	if (!(pThruster->dir & THRUST_NOANG))
	{
		Vector angdir, *pAT = (Vector *)pState->pObjParam->angthrust, cpos;
		VecAdd (&pState->compos, &start, &cpos);
		VecCross (&cpos, &dir, &angdir);
//		VecNorm (&angdir, &angdir);
		float xp = angdir.x * pAT->x;
		float yp = angdir.y * pAT->y;
		float zp = angdir.z * pAT->z;
		if (xp+yp+zp > 0) {
			if (xp > yp && xp > zp && fabs(pAT->x) > power) power = fabs(pAT->x);
			else if (yp > xp && yp > zp && fabs(pAT->y) > power) power = fabs(pAT->y);
			else if (zp > xp && zp > yp && fabs(pAT->z) > power) power = fabs(pAT->z);
		}
	}

	if (power <= 0.001f) return;
	power *= pState->scale;
	float width = sqrt(power)*pThruster->power*0.6f;
	float len = power*pThruster->power;
	VecMul (&dir, len, &end);
	VecAdd (&end, &start, &end);

	// 

	Vector v1, v2, pos; Matrix m, m2;
	v1.x = dir.y; v1.y = dir.z; v1.z = dir.x;
	VecCross (&v1, &dir, &v2);	VecNorm (&v2, &v2);
	VecCross (&v2, &dir, &v1);
	m.x1 = v1.x; m.x2 = v2.x; m.x3 = dir.x;
	m.y1 = v1.y; m.y2 = v2.y; m.y3 = dir.y;
	m.z1 = v1.z; m.z2 = v2.z; m.z3 = dir.z;
	MatMatMult (&pState->objorient, &m, &m2);

	MatVecMult (&pState->objorient, &start, &pos);
	VecAdd (&pos, &pState->objpos, &pos);

	float pMV[16];
	pMV[0] = m2.x1; pMV[1] = m2.y1; pMV[2] = m2.z1; pMV[3] = 0.0f;
	pMV[4] = m2.x2; pMV[5] = m2.y2; pMV[6] = m2.z2; pMV[7] = 0.0f;
	pMV[8] = m2.x3; pMV[9] = m2.y3; pMV[10] = m2.z3; pMV[11] = 0.0f;
	pMV[12] = pos.x; pMV[13] = pos.y; pMV[14] = pos.z; pMV[15] = 1.0f;
	glPushMatrix ();
	glLoadMatrixf (pMV);

	glScalef (width*0.5f, width*0.5f, len*0.666f);
	glColor4f (0.0f, 0.4f, 1.0f, 0.6f);
	glVertexPointer (3, GL_FLOAT, sizeof(Vector), pTVertex8pt);
	glDrawElements (GL_TRIANGLES, pNumIndex[1], GL_UNSIGNED_SHORT, pTIndex8pt);

	glScalef (2.0f, 2.0f, 1.5f);
	glColor4f (0.4f, 0.0f, 1.0f, 0.6f);
	glVertexPointer (3, GL_FLOAT, sizeof(Vector), pTVertex8pt);
	glDrawElements (GL_TRIANGLES, pNumIndex[1], GL_UNSIGNED_SHORT, pTIndex8pt);

	glPopMatrix ();
	return;
}


// occlusion problems

static void RenderLight (RState *pState, int index)
{
/*	Light *pLight = pState->pModel->pLight + index;

	float *pColor = pLight->pColor;
	if (pLight->animcolor != -1) pColor = pState->pObjParam->ppColor[pLight->animcolor];

	float power = pLight->power;
	if (pLight->animpower != -1) power *= pState->pObjParam->pAnim[pLight->animpower];

	Vector xax, zax;
	VecAdd (&pState->campos, pState->pVtx+pLight->vtx, &zax);
	Vector yax = { pState->objorient.x2, pState->objorient.y2, pState->objorient.z2 };
	VecNorm (&zax, &zax); VecCross (&yax, &zax, &xax);
	VecNorm (&xax, &xax); VecCross (&zax, &xax, &yax);
*/
}


struct TransElem
{
	Vector pos, dir;
	Thruster *pThruster;
	float dist;
};

inline void SWAP (TransElem *a, TransElem *b) { TransElem t = *a; *a = *b; *b = t; }

static void QuickSort (TransElem *pA, int end)
{
	if (end <= 0) return;
	float pivotval = pA[end].dist;
	int i, j; for (i=0, j=0; i<end; i++)
		if (pA[i].dist < pivotval) { SWAP (pA+i, pA+j); j++; }
	SWAP (pA+end, pA+j);
	if (j > 1) QuickSort (pA, j-1); j++;
	if (j < end) QuickSort (pA+j, end-j);
}

static int thrustgen = 0;

void RenderThrusters (RState *pState, int numThrusters, Thruster *pThrusters) 
{
	Vector *pVtx = pState->pVtx;

	if (!thrustgen) GenerateThrusters ();
	thrustgen = 1;

	TransElem *pList = (TransElem *) alloca (numThrusters*2*sizeof(TransElem));
	Vector tv;
	int i, numElem = 0;
	for (i=0; i<numThrusters; i++)
	{
		Thruster *pThruster = pThrusters+i;
		pList[numElem].pThruster = pThruster;
		pList[numElem].pos = pVtx[pThruster->pos];
		pList[numElem].dir = pVtx[pThruster->dir&0xff];
		VecAdd (&pState->campos, &pList[numElem].pos, &tv);
		pList[numElem].dist = VecDot (&tv, &tv);

		if (pThruster->dir & THRUST_XREF)
		{
			pList[numElem+1] = pList[numElem];
			pList[numElem+1].pos.x = -pList[numElem].pos.x;
			pList[numElem+1].dir.x = -pList[numElem].dir.x;
			VecAdd (&pState->campos, &pList[numElem+1].pos, &tv);
			pList[numElem+1].dist = VecDot (&tv, &tv);
			numElem++;
		}
		numElem++;
	}

	QuickSort (pList, numElem-1);

//	for (i=numElem-1; i>=0; i--)
	for (i=0; i<numElem; i++)
	{
		RenderThruster (pState, pList[i].pThruster, &pList[i].pos, &pList[i].dir);
	}
}

