#include <malloc.h>
#include "sbre_int.h"
#include "sbre.h"			// for subobject


static void ExpandVertices (CollMesh *pCMesh, int n)
{
	if (n < 100) n = 100;
	pCMesh->maxv += n; n = pCMesh->maxv;
	pCMesh->pVertex = (float *) realloc (pCMesh->pVertex, n*3*sizeof(float));
}

static void ExpandIndices (CollMesh *pCMesh, int n)
{
	if (n < 100) n = 100;
	pCMesh->maxi += n; n = pCMesh->maxi;
	pCMesh->pIndex = (int *) realloc (pCMesh->pIndex, n*sizeof(int));
	pCMesh->pFlag = (int *) realloc (pCMesh->pFlag, sizeof(int) * n/3);
}

void ResolveVtx (Vector *pIn, Vector *pOut, RState *pState)
{
	Vector tv;
	MatVecMult (&pState->objorient, pIn, &tv);
	VecMul (&tv, pState->scale, &tv);
	VecAdd (&tv, &pState->objpos, pOut);
}


static int CollFuncMatAnim (uint16 *pData, Model *pMod, RState *pState)
{
	return 22;
}
static int CollFuncMatFixed (uint16 *pData, Model *pMod, RState *pState)
{
	return 11;
}
static int CollFuncMatVar (uint16 *pData, Model *pMod, RState *pState)
{
	return 2;
}
static int CollFuncZBias (uint16 *pData, Model *pMod, RState *pState)
{
	return 4;
}

static int CollFuncTriFlat (uint16 *pData, Model *pMod, RState *pState)
{
	Vector *pVtx = pState->pVtx;
	CollMesh *pCMesh = pState->pCMesh;
	if (pCMesh->maxv - pCMesh->nv < 6) ExpandVertices (pCMesh, 6);
	if (pCMesh->maxi - pCMesh->ni < 6) ExpandIndices (pCMesh, 6);

	int ni = pCMesh->ni; int nv = pCMesh->nv;
	Vector *pOut = (Vector *)pCMesh->pVertex;
	int *pIdx = pCMesh->pIndex;
	pCMesh->pFlag[ni/3] = pCMesh->cflag;

	for (int i=0; i<3; i++) {
		ResolveVtx (pVtx+pData[i+1], pOut+nv, pState);
		pIdx[ni++] = nv++;
	}

	if (pData[0] & RFLAG_XREF)
	{
		pCMesh->pFlag[ni/3] = pCMesh->cflag;

		for (int i=0; i<3; i++) {
			Vector *pVec = pVtx + pData[3-i];
			VecSet (-pVec->x, pVec->y, pVec->z, pOut+nv);
			ResolveVtx (pOut+nv, pOut+nv, pState);
			pIdx[ni++] = nv++;
		}
	}
	pCMesh->ni = ni; pCMesh->nv = nv;
	return 4;
}

static int CollFuncQuadFlat (uint16 *pData, Model *pMod, RState *pState)
{
	Vector *pVtx = pState->pVtx;
	CollMesh *pCMesh = pState->pCMesh;
	if (pCMesh->maxv - pCMesh->nv < 8) ExpandVertices (pCMesh, 8);
	if (pCMesh->maxi - pCMesh->ni < 12) ExpandIndices (pCMesh, 12);

	int ni = pCMesh->ni; int nv = pCMesh->nv;
	Vector *pOut = (Vector *)pCMesh->pVertex;
	int *pIdx = pCMesh->pIndex;
	pCMesh->pFlag[ni/3] = pCMesh->pFlag[1+ni/3] = pCMesh->cflag;

	for (int i=0; i<4; i++) ResolveVtx (pVtx+pData[i+1], pOut+nv+i, pState);
	pIdx[ni+0] = nv; pIdx[ni+1] = nv+1; pIdx[ni+2] = nv+2;
	pIdx[ni+3] = nv; pIdx[ni+4] = nv+2; pIdx[ni+5] = nv+3;
	ni += 6; nv += 4;

	if (pData[0] & RFLAG_XREF)
	{
		pCMesh->pFlag[ni/3] = pCMesh->pFlag[1+ni/3] = pCMesh->cflag;

		for (int i=0; i<4; i++) {
			Vector *pVec = pVtx + pData[i+1];
			VecSet (-pVec->x, pVec->y, pVec->z, pOut+nv+i);
			ResolveVtx (pOut+nv+i, pOut+nv+i, pState);
		}
		pIdx[ni+0] = nv; pIdx[ni+1] = nv+2; pIdx[ni+2] = nv+1;
		pIdx[ni+3] = nv; pIdx[ni+4] = nv+3; pIdx[ni+5] = nv+2;
		ni += 6; nv += 4;
	}
	pCMesh->ni = ni; pCMesh->nv = nv;
	return 5;
}


static void ArrayCallback (int nv2, int ni2, Vector *pVertex, uint16 *pIndex,
	uint16 flags, RState *pState)
{
	CollMesh *pCMesh = pState->pCMesh;
	if (pCMesh->maxv - pCMesh->nv < nv2*2) ExpandVertices (pCMesh, nv2*2);
	if (pCMesh->maxi - pCMesh->ni < ni2*2) ExpandIndices (pCMesh, ni2*2);

	int ni = pCMesh->ni; int nv = pCMesh->nv;
	Vector *pOut = (Vector *)pCMesh->pVertex;
	int *pIdx = pCMesh->pIndex;
	int *pFlag = pCMesh->pFlag;

	for (int i=0; i<nv2; i++) ResolveVtx (pVertex+i*2, pOut+nv+i, pState);
	for (int i=0; i<ni2; i++) pIdx[ni+i] = nv+pIndex[i];
	for (int i=0; i<ni2/3; i++) pFlag[ni/3+i] = pCMesh->cflag;
	ni += ni2; nv += nv2;

	if (flags & RFLAG_XREF)
	{
		for (int i=0; i<nv2; i++) {
			Vector *pVec = pVertex + i*2;
			VecSet (-pVec->x, pVec->y, pVec->z, pOut+nv+i);
			ResolveVtx (pOut+nv+i, pOut+nv+i, pState);
		}
		for (int i=0; i<ni2; i+=3) {
			pIdx[ni+i+0] = nv+pIndex[i];
			pIdx[ni+i+1] = nv+pIndex[i+2];
			pIdx[ni+i+2] = nv+pIndex[i+1];
		}
		for (int i=0; i<ni2/3; i++) pFlag[ni/3+i] = pCMesh->cflag;
		ni += ni2; nv += nv2;
	}
	pCMesh->ni = ni; pCMesh->nv = nv;
}

static int CollFuncCompoundSmooth (uint16 *pData, Model *pMod, RState *pState)
{
	pState->pCallback = ArrayCallback;
	return pPrimFuncTable[*pData & 0xff] (pData, pMod, pState);
}

static int CollFuncCylinder (uint16 *pData, Model *pMod, RState *pState)
{
	pState->pCallback = ArrayCallback;
	return pPrimFuncTable[*pData & 0xff] (pData, pMod, pState);
}

static int CollFuncCircle (uint16 *pData, Model *pMod, RState *pState)
{
	pState->pCallback = ArrayCallback;
	return pPrimFuncTable[*pData & 0xff] (pData, pMod, pState);
}

static int CollFuncTube (uint16 *pData, Model *pMod, RState *pState)
{
	pState->pCallback = ArrayCallback;
	return pPrimFuncTable[*pData & 0xff] (pData, pMod, pState);
}

static int CollFuncExtrusion (uint16 *pData, Model *pMod, RState *pState)
{
	pState->pCallback = ArrayCallback;
	return pPrimFuncTable[*pData & 0xff] (pData, pMod, pState);
}


/*
uint16 PFUNC_SUBOBJECT
	uint16 anim
	uint16 modelnum
	uint16 offset
	uint16 norm
	uint16 zaxis
	uint16 scale
*/

static int CollFuncSubObject (uint16 *pData, Model *pMod, RState *pState)
{
	// return immediately if object is not present
	if (pData[1] != 0x8000 && !pState->pObjParam->pFlag[pData[1]]) return 7;
	
	// build transform matrix, offset
	Vector v1, v2, v3, pos; Matrix m, orient;
	VecNorm (pState->pVtx+pData[4], &v2);
	VecNorm (pState->pVtx+pData[5], &v3);
	VecCross (&v2, &v3, &v1);
	m.x1 = v1.x; m.x2 = v2.x; m.x3 = v3.x;
	m.y1 = v1.y; m.y2 = v2.y; m.y3 = v3.y;
	m.z1 = v1.z; m.z2 = v2.z; m.z3 = v3.z;
	MatMatMult (&pState->objorient, &m, &orient);

	MatVecMult (&pState->objorient, pState->pVtx+pData[3], &pos);
	VecMul (&pos, pState->scale, &pos);
	VecAdd (&pos, &pState->objpos, &pos);
	float scale = pState->scale*pData[6]*0.01f;
	
	GenCollMeshInternal (&pos, &orient, pData[2], pState->pObjParam, scale, pState->pCMesh);
	return 7;
}

static int CollFuncText (uint16 *pData, Model *pMod, RState *pState)
{
	return 9;
}

/*
uint16 PFUNC_SETCFLAG
	uint16 flag
*/

static int CollFuncSetCFlag (uint16 *pData, Model *pMod, RState *pState)
{
	pState->pCMesh->cflag = pData[1];
	return 2;
}



int (*pCollFuncTable[])(uint16 *, Model *, RState *) = {
	0,		// end
	CollFuncMatAnim,
	CollFuncMatFixed,
	CollFuncMatVar,
	CollFuncZBias,
	CollFuncTriFlat,
	CollFuncQuadFlat,
	CollFuncCompoundSmooth,		// just uses steps = 0
	CollFuncCompoundSmooth,
	CollFuncCircle,
	CollFuncCylinder,
	CollFuncTube,
	CollFuncSubObject,
	CollFuncText,
	CollFuncExtrusion,
	CollFuncSetCFlag,
};


