#include <SDL_opengl.h>
#include <malloc.h>
#include "sbre_int.h"
#include "sbre_anim.h"
#include "sbre.h"			// for subobject
#include "../glfreetype.h"


/*
Uint16 PFUNC_MATANIM
	Uint16 animfunc
	Uint16 dr1, dg1, db1, sr1, sg1, sb1, sh1, er1, eg1, eb1
	Uint16 dr2, dg2, db2, sr2, sg2, sb2, sh2, er2, eg2, eb2
*/

static int PrimFuncMatAnim (Uint16 *pData, Model *pMod, RState *pState)
{
	float anim = 0.01f * ResolveAnim (pState->pObjParam, pData[1]);
	float ianim = 0.01f - anim;

	int i;
	float pDiff[4] = { 0, 0, 0, 1.0f }, shiny;
	float pSpec[4] = { 0, 0, 0, 1.0f };
	float pEmis[4] = { 0, 0, 0, 1.0f };
	for (i=0; i<3; i++) pDiff[i] = pData[i+2] * anim + pData[i+12] * ianim;
	for (i=0; i<3; i++) pSpec[i] = pData[i+5] * anim + pData[i+15] * ianim;
	for (i=0; i<3; i++) pEmis[i] = pData[i+9] * anim + pData[i+19] * ianim;
	shiny = pData[8] * anim + pData[18] * ianim;

	glMaterialfv (GL_FRONT, GL_AMBIENT_AND_DIFFUSE, pDiff);
	glMaterialfv (GL_FRONT, GL_SPECULAR, pSpec);
	glMaterialfv (GL_FRONT, GL_EMISSION, pEmis);
	glMaterialf (GL_FRONT, GL_SHININESS, shiny);

	return 22;
}

/*
Uint16 PFUNC_MATFIXED
	Uint16 dr, dg, db
	Uint16 sr, sg, sb
	Uint16 shiny
	Uint16 er, eg, eb
*/

static int PrimFuncMatFixed (Uint16 *pData, Model *pMod, RState *pState)
{
	int i;
	float pDiff[4] = { 0, 0, 0, 1.0f }, shiny;
	float pSpec[4] = { 0, 0, 0, 1.0f };
	float pEmis[4] = { 0, 0, 0, 1.0f };
	for (i=0; i<3; i++) pDiff[i] = pData[i+1] * 0.01f;
	for (i=0; i<3; i++) pSpec[i] = pData[i+4] * 0.01f;
	for (i=0; i<3; i++) pEmis[i] = pData[i+8] * 0.01f;
	shiny = pData[7] * 0.01f;

	glMaterialfv (GL_FRONT, GL_AMBIENT_AND_DIFFUSE, pDiff);
	glMaterialfv (GL_FRONT, GL_SPECULAR, pSpec);
	glMaterialfv (GL_FRONT, GL_EMISSION, pEmis);
	glMaterialf (GL_FRONT, GL_SHININESS, shiny);

	return 11;
}

/*
Uint16 PFUNC_MATVAR
	Uint16 index
*/

static int PrimFuncMatVar (Uint16 *pData, Model *pMod, RState *pState)
{
	int i;
	float pDiff[4] = { 0, 0, 0, 1.0f }, shiny;
	float pSpec[4] = { 0, 0, 0, 1.0f };
	float pEmis[4] = { 0, 0, 0, 1.0f };
	ObjParams *pParam = pState->pObjParam;
	for (i=0; i<3; i++) pDiff[i] = pParam->pColor[pData[1]].pDiff[i];
	for (i=0; i<3; i++) pSpec[i] = pParam->pColor[pData[1]].pSpec[i];
	for (i=0; i<3; i++) pEmis[i] = pParam->pColor[pData[1]].pEmis[i];
	shiny = pParam->pColor[pData[1]].shiny;

	glMaterialfv (GL_FRONT, GL_AMBIENT_AND_DIFFUSE, pDiff);
	glMaterialfv (GL_FRONT, GL_SPECULAR, pSpec);
	glMaterialfv (GL_FRONT, GL_EMISSION, pEmis);
	glMaterialf (GL_FRONT, GL_SHININESS, shiny);

	return 2;
}

/*
Uint16 PFUNC_ZBIAS
	Uint16 offset		// to test if nearer - 0x8000 = reset
	Uint16 units		// integer units. not used
*/

static int PrimFuncZBias (Uint16 *pData, Model *pMod, RState *pState)
{
	if (pData[1] & 0x8000) glDepthRange (pState->dn+SBRE_ZBIAS, pState->df);
	else if (VecDot (pState->pVtx+pData[1], &pState->campos) > 0.0f)
		glDepthRange (pState->dn, pState->df-SBRE_ZBIAS);
	return 3;
}

static int PrimFuncTriFlat (Uint16 *pData, Model *pMod, RState *pState)
{
	Vector *pVtx = pState->pVtx;
	Vector *pVec;
	Vector norm, tv1, tv2;
	VecSub (pVtx+pData[1], pVtx+pData[2], &tv1);
	VecSub (pVtx+pData[3], pVtx+pData[2], &tv2);
	VecCross (&tv2, &tv1, &norm);
	VecNorm (&norm, &norm);

	glBegin (GL_TRIANGLES);
	glNormal3f (norm.x, norm.y, norm.z);
	pVec = pVtx + pData[1];
	glVertex3f (pVec->x, pVec->y, pVec->z);
	pVec = pVtx + pData[2];
	glVertex3f (pVec->x, pVec->y, pVec->z);
	pVec = pVtx + pData[3];
	glVertex3f (pVec->x, pVec->y, pVec->z);

	if (pData[0] & RFLAG_XREF) {
		glNormal3f (-norm.x, norm.y, norm.z);
		pVec = pVtx + pData[3];
		glVertex3f (-pVec->x, pVec->y, pVec->z);
		pVec = pVtx + pData[2];
		glVertex3f (-pVec->x, pVec->y, pVec->z);
		pVec = pVtx + pData[1];
		glVertex3f (-pVec->x, pVec->y, pVec->z);
	}
	glEnd ();
	return 4;
}

static int PrimFuncQuadFlat (Uint16 *pData, Model *pMod, RState *pState)
{
	Vector *pVtx = pState->pVtx;
	Vector *pVec;
	Vector norm, tv1, tv2;
	VecSub (pVtx+pData[1], pVtx+pData[2], &tv1);
	VecSub (pVtx+pData[3], pVtx+pData[2], &tv2);
	VecCross (&tv2, &tv1, &norm);
	VecNorm (&norm, &norm);

	glBegin (GL_TRIANGLE_FAN);
	glNormal3f (norm.x, norm.y, norm.z);
	pVec = pVtx + pData[1];
	glVertex3f (pVec->x, pVec->y, pVec->z);
	pVec = pVtx + pData[2];
	glVertex3f (pVec->x, pVec->y, pVec->z);
	pVec = pVtx + pData[3];
	glVertex3f (pVec->x, pVec->y, pVec->z);
	pVec = pVtx + pData[4];
	glVertex3f (pVec->x, pVec->y, pVec->z);
	glEnd ();

	if (pData[0] & RFLAG_XREF) {
		glBegin (GL_TRIANGLE_FAN);
		glNormal3f (-norm.x, norm.y, norm.z);
		pVec = pVtx + pData[3];
		glVertex3f (-pVec->x, pVec->y, pVec->z);
		pVec = pVtx + pData[2];
		glVertex3f (-pVec->x, pVec->y, pVec->z);
		pVec = pVtx + pData[1];
		glVertex3f (-pVec->x, pVec->y, pVec->z);
		pVec = pVtx + pData[4];
		glVertex3f (-pVec->x, pVec->y, pVec->z);
		glEnd ();
	}
	return 5;
}


void RenderArray (int nv, int ni, Vector *pVertex, Uint16 *pIndex, Uint16 flags)
{
	glNormalPointer (GL_FLOAT, 2*sizeof(Vector), pVertex+1);
	glVertexPointer (3, GL_FLOAT, 2*sizeof(Vector), pVertex);
	glDrawElements (GL_TRIANGLES, ni, GL_UNSIGNED_SHORT, pIndex);

	if (flags & RFLAG_XREF)
	{
		glPushMatrix ();
		glFrontFace (GL_CCW);
		const float pMV[16] = { -1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1 };
		glMultMatrixf (pMV);
		glDrawElements (GL_TRIANGLES, ni, GL_UNSIGNED_SHORT, pIndex);
		glFrontFace (GL_CW);
		glPopMatrix ();
	}
}

void CopyArrayToCache (int nv, int ni, Vector *pVertex, Uint16 *pIndex, int ci, Model *pModel)
{
	pModel->pNumIdx[ci] = ni;
	pModel->pNumVtx[ci] = nv;
	pModel->ppICache[ci] = (Uint16 *) malloc (ni*sizeof(Uint16));
	memcpy (pModel->ppICache[ci], pIndex, ni*sizeof(Uint16));
	pModel->ppVCache[ci] = (Vector *) malloc (2*nv*sizeof(Vector));
	memcpy (pModel->ppVCache[ci], pVertex, 2*nv*sizeof(Vector));
}

/*
Uint16 PFUNC_COMPSMOOTH
	Uint16 cacheidx
	Uint16 steps
	Uint16 centpos
	Uint16 centnorm
	Uint16 startpos
	Uint16 startnorm
		Uint16 COMP_END
		Uint16 COMP_LINE
			Uint16 pos
			Uint16 norm
		Uint16 COMP_HERMITE
			Uint16 pos
			Uint16 norm
			Uint16 tan0
			Uint16 tan1

// tangents should be prescaled
*/

int PrimFuncCompoundSmooth (Uint16 *pData, Model *pMod, RState *pState)
{
	Vector *pVtx = pState->pVtx;
	Model *pModel = pState->pModel;

	Uint16 ci = pData[1];
	if (ci != 0x8000 && pModel->pNumIdx[ci])
	{
		RenderArray (pModel->pNumVtx[ci], pModel->pNumIdx[ci],
			pModel->ppVCache[ci], pModel->ppICache[ci], pData[0]);
		int c; for (c=7; pData[c] != COMP_END; c+=pCompSize[pData[c]]);
		return c+1;
	}

	int steps = pData[2];				// detail factor...
	Vector *pCPos = pVtx+pData[3];
	Vector *pCNorm = pVtx+pData[4];				// centre point
	Vector *pLPos = pVtx+pData[5];
	Vector *pLNorm = pVtx+pData[6];
	int c = 7;

	while (pData[c] != COMP_END)
	{
		switch (pData[c])
		{
			case COMP_STEPS:
			steps = pData[c+1];
			c += 2; break;
			
			case COMP_LINE:
			pLPos = pVtx+pData[c+1], pLNorm = pVtx+pData[c+2];
			TriangAddPoint (pLPos, pLNorm);
			c += 3; break;
			
			case COMP_HERMITE:
			case COMP_HERM_NOTAN:
			// Now add points along spline
			float t, incstep = 1.0f / (steps+1);
			int i; for (i=0, t=incstep; i<steps; i++, t+=incstep)
			{
				Vector pos, norm;
				if (pData[c] == COMP_HERM_NOTAN) {
					Vector tan; VecSub (pVtx+pData[c+1], pLPos, &tan);
					ResolveHermiteSpline (pLPos, pVtx+pData[c+1], &tan, &tan, t, &pos);
				}
				else ResolveHermiteSpline (pLPos, pVtx+pData[c+1],
						pVtx+pData[c+3], pVtx+pData[c+4], t, &pos);
				ResolveLinearInterp (pLNorm, pVtx+pData[c+2], t, &norm);
				VecNorm (&norm, &norm);
				TriangAddPoint (&pos, &norm);
			}
			// add end point
			pLPos = pVtx+pData[c+1], pLNorm = pVtx+pData[c+2];
			TriangAddPoint (pLPos, pLNorm);
			c += pCompSize[pData[c]]; break;
		}
	}

	int ni, nv;
	Vector *pVertex;
	Uint16 *pIndex;
	if ((pData[0]&0xff) == PTYPE_COMPFLAT) steps = 0;
	Triangulate (pCPos, pCNorm, steps, &pVertex, &nv, &pIndex, &ni);

	RenderArray (nv, ni, pVertex, pIndex, pData[0]);
	if (ci != 0x8000) CopyArrayToCache (nv, ni, pVertex, pIndex, ci, pModel);
	return c+1;		// +1 for COMP_END
}

/*
Uint16 PFUNC_CYLINDER
	Uint16 cacheidx		-1 => uncacheable
	Uint16 steps		8 => octagonal
	Uint16 startvtx
	Uint16 endvtx
	Uint16 updir
	Uint16 rad
*/

static int PrimFuncCylinder (Uint16 *pData, Model *pMod, RState *pState)
{
	Vector *pVtx = pState->pVtx;
	Model *pModel = pState->pModel;

	Uint16 ci = pData[1];
	if (ci != 0x8000 && pModel->pNumIdx[ci])
	{
		RenderArray (pModel->pNumVtx[ci], pModel->pNumIdx[ci],
			pModel->ppVCache[ci], pModel->ppICache[ci], pData[0]);
		return 7;
	}

	int steps = pData[2];
	float rad = pData[6] * 0.01f;
	Vector *pVertex = (Vector *) alloca (8*steps*sizeof(Vector));
	Uint16 *pIndex = (Uint16 *) alloca (12*steps*sizeof(Vector));
	int ni = 0;

	// generate cylinder axes

	Vector yax = pVtx[pData[5]], xax, zax;
	VecSub (pVtx+pData[4], pVtx+pData[3], &zax);	// dir = end-start
	VecNorm (&zax, &zax);
	VecCross (&yax, &zax, &xax);

	float angstep = 2.0f * 3.141592f / steps, ang = 0.0f;
	int i; for (i=0; i<steps; i++, ang+=angstep)
	{
		Vector tv, norm;
		VecMul (&xax, sin(ang), &tv);
		VecMul (&yax, cos(ang), &norm);
		VecAdd (&tv, &norm, &norm);
		pVertex[1+(i+steps)*2] = pVertex[1+i*2] = norm;
		pVertex[1+(i+steps*3)*2] = zax; VecInv (&zax, &tv);	// endcap
		pVertex[1+(i+steps*2)*2] = tv;						// startcap

		VecMul (&norm, rad, &tv);
		VecAdd (pVtx+pData[3], &tv, pVertex+i*2);			// start
		VecAdd (pVtx+pData[4], &tv, pVertex+(i+steps)*2);	// end
		pVertex[(i+steps*2)*2] = pVertex[i*2];				// startcap
		pVertex[(i+steps*3)*2] = pVertex[(i+steps)*2];		// endcap
	}

	// render sides
	for (i=0; i<steps; i++)
	{
		int i1 = i+1==steps?0:i+1;
		pIndex[ni++] = i; pIndex[ni++] = i+steps; pIndex[ni++] = i1+steps;
		pIndex[ni++] = i1+steps; pIndex[ni++] = i1; pIndex[ni++] = i;
	}	

	// render ends
	for (i=1; i<steps-1; i++) {
		pIndex[ni++] = steps*2;
		pIndex[ni++] = i+steps*2;
		pIndex[ni++] = i+1+steps*2;
	}
	for (i=steps-1; i>1; i--) {
		pIndex[ni++] = steps*3;
		pIndex[ni++] = i+steps*3;
		pIndex[ni++] = i-1+steps*3;
	}

	RenderArray (4*steps, ni, pVertex, pIndex, pData[0]);
	if (ci != 0x8000) CopyArrayToCache (4*steps, ni, pVertex, pIndex, ci, pModel);
	return 7;
}

/*
Uint16 PFUNC_CIRCLE
	Uint16 cacheidx		-1 => uncacheable
	Uint16 steps		8 => octagonal
	Uint16 vtx
	Uint16 norm
	Uint16 updir		
	Uint16 rad
*/

static int PrimFuncCircle (Uint16 *pData, Model *pMod, RState *pState)
{
	Vector *pVtx = pState->pVtx;
	Model *pModel = pState->pModel;

	Uint16 ci = pData[1];
	if (ci != 0x8000 && pModel->pNumIdx[ci])
	{
		RenderArray (pModel->pNumVtx[ci], pModel->pNumIdx[ci],
			pModel->ppVCache[ci], pModel->ppICache[ci], pData[0]);
		return 7;
	}

	int steps = pData[2];
	float rad = pData[6] * 0.01f;
	Vector *pVertex = (Vector *) alloca (2*steps*sizeof(Vector));
	Uint16 *pIndex = (Uint16 *) alloca (3*steps*sizeof(Vector));
	int ni = 0;

	// generate axes

	Vector yax = pVtx[pData[5]], xax, zax = pVtx[pData[4]];
	VecCross (&yax, &zax, &xax);

	float angstep = 2.0f * 3.141592f / steps, ang = 0.0f;
	int i; for (i=0; i<steps; i++, ang+=angstep)
	{
		Vector tv, norm;
		VecMul (&xax, sin(ang), &tv);
		VecMul (&yax, cos(ang), &norm);
		VecAdd (&tv, &norm, &norm);
		VecMul (&norm, rad, &tv);
		VecAdd (pVtx+pData[3], &tv, pVertex+i*2);
		pVertex[i*2+1] = zax;
	}

	for (i=1; i<steps-1; i++) {
		pIndex[ni++] = 0;
		pIndex[ni++] = i+1;
		pIndex[ni++] = i;
	}

	RenderArray (steps, ni, pVertex, pIndex, pData[0]);
	if (ci != 0x8000) CopyArrayToCache (steps, ni, pVertex, pIndex, ci, pModel);
	return 7;
}


/*
Uint16 PFUNC_TUBE
	Uint16 cacheidx		-1 => uncacheable
	Uint16 steps		8 => octagonal
	Uint16 startvtx
	Uint16 endvtx
	Uint16 updir
	Uint16 outerrad
	Uint16 innerrad
*/

static int PrimFuncTube (Uint16 *pData, Model *pMod, RState *pState)
{
	Vector *pVtx = pState->pVtx;
	Model *pModel = pState->pModel;

	Uint16 ci = pData[1];
	if (ci != 0x8000 && pModel->pNumIdx[ci])
	{
		RenderArray (pModel->pNumVtx[ci], pModel->pNumIdx[ci],
			pModel->ppVCache[ci], pModel->ppICache[ci], pData[0]);
		return 8;
	}

	int steps = pData[2];
	Vector *pVertex = (Vector *) alloca (16*steps*sizeof(Vector));
	Uint16 *pIndex = (Uint16 *) alloca (24*steps*sizeof(Vector));
	int ni = 0;

	// generate cylinder axes

	Vector yax = pVtx[pData[5]], xax, zax;
	VecSub (pVtx+pData[4], pVtx+pData[3], &zax);	// dir = end-start
	VecNorm (&zax, &zax);
	VecCross (&yax, &zax, &xax);

/*
0: start, outer, radial
steps: end, outer, radial
steps*2: start, inner, radial
steps*3: end, inner, radial
steps*4: start, outer, axial
steps*5: end, outer, axial
steps*6: start, inner, axial
steps*7: end, inner, axial
*/

	float angstep = 2.0f * 3.141592f / steps, ang = 0.0f;
	int i; for (i=0; i<steps; i++, ang+=angstep)
	{
		Vector tv, norm, invnorm;
		VecMul (&xax, sin(ang), &tv);
		VecMul (&yax, cos(ang), &norm);
		VecAdd (&tv, &norm, &norm); VecInv (&norm, &invnorm);
		pVertex[1+(i+steps)*2] = pVertex[1+i*2] = norm;
		pVertex[1+(i+steps*2)*2] = pVertex[1+(i+steps*3)*2] = invnorm;
		VecInv (&zax, &tv);
		pVertex[1+(i+steps*5)*2] = pVertex[1+(i+steps*7)*2] = zax;		// endcap
		pVertex[1+(i+steps*4)*2] = pVertex[1+(i+steps*6)*2] = tv;		// startcap

		VecMul (&norm, pData[6] * 0.01f, &tv);			// outer
		VecAdd (pVtx+pData[3], &tv, pVertex+i*2);			// start
		VecAdd (pVtx+pData[4], &tv, pVertex+(i+steps)*2);	// end
		pVertex[(i+steps*4)*2] = pVertex[i*2];				// startcap
		pVertex[(i+steps*5)*2] = pVertex[(i+steps)*2];		// endcap

		VecMul (&norm, pData[7] * 0.01f, &tv);			// inner 
		VecAdd (pVtx+pData[3], &tv, pVertex+(i+steps*2)*2);	// start
		VecAdd (pVtx+pData[4], &tv, pVertex+(i+steps*3)*2);	// end
		pVertex[(i+steps*6)*2] = pVertex[(i+steps*2)*2];	// startcap
		pVertex[(i+steps*7)*2] = pVertex[(i+steps*3)*2];	// endcap
	}

	// render sides
	for (i=0; i<steps; i++)
	{
		int i1 = i+1==steps?0:i+1;
		// outer radial surface
		pIndex[ni++] = i; pIndex[ni++] = i+steps; pIndex[ni++] = i1+steps;
		pIndex[ni++] = i1+steps; pIndex[ni++] = i1; pIndex[ni++] = i;
		// inner radial surface
		pIndex[ni++] = i+steps*2; pIndex[ni++] = i1+steps*2; pIndex[ni++] = i1+steps*3;
		pIndex[ni++] = i1+steps*3; pIndex[ni++] = i+steps*3; pIndex[ni++] = i+steps*2;
		// startcap
		pIndex[ni++] = i+steps*4; pIndex[ni++] = i1+steps*4; pIndex[ni++] = i1+steps*6;
		pIndex[ni++] = i1+steps*6; pIndex[ni++] = i+steps*6; pIndex[ni++] = i+steps*4;
		// endcap
		pIndex[ni++] = i+steps*5; pIndex[ni++] = i+steps*7; pIndex[ni++] = i1+steps*7;
		pIndex[ni++] = i1+steps*7; pIndex[ni++] = i1+steps*5; pIndex[ni++] = i+steps*5;
	}	

	RenderArray (8*steps, ni, pVertex, pIndex, pData[0]);
	if (ci != 0x8000) CopyArrayToCache (8*steps, ni, pVertex, pIndex, ci, pModel);
	return 8;
}


/*
Uint16 PFUNC_SUBOBJECT
	Uint16 anim
	Uint16 modelnum
	Uint16 offset
	Uint16 xaxis
	Uint16 yaxis
	Uint16 scale
*/

static int PrimFuncSubObject (Uint16 *pData, Model *pMod, RState *pState)
{
	// return immediately if object is not present
	if (pData[1] != 0x8000 && !pState->pObjParam->pFlag[pData[1]]) return 7;
	
	// build transform matrix, offset
	Vector v1, v2, v3, pos; Matrix m, orient;
	VecNorm (pState->pVtx+pData[4], &v1);
	VecNorm (pState->pVtx+pData[5], &v2);
	VecCross (&v1, &v2, &v3);
	m.x1 = v1.x; m.x2 = v2.x; m.x3 = v3.x;
	m.y1 = v1.y; m.y2 = v2.y; m.y3 = v3.y;
	m.z1 = v1.z; m.z2 = v2.z; m.z3 = v3.z;
	MatMatMult (&pState->objorient, &m, &orient);

	MatVecMult (&pState->objorient, pState->pVtx+pData[3], &pos);
	VecAdd (&pos, &pState->objpos, &pos);
	float scale = pState->scale*pData[6]*0.01f;
	
	glPushAttrib (GL_LIGHTING_BIT);
	glPushMatrix ();

	sbreRenderModel (&pos, &orient, pData[2], pState->pObjParam, scale);

	glPopMatrix ();
	glPopAttrib ();
	return 7;
}

static int glfinit = 0;
static FontFace *pFace = 0;

/*
Uint16 PFUNC_TEXT
	Uint16 anim
	Uint16 textnum
	Uint16 pos
	Uint16 norm
	Uint16 xaxis
	Uint16 xoff
	Uint16 yoff
	Uint16 scale
*/

static int PrimFuncText (Uint16 *pData, Model *pMod, RState *pState)
{
	if (!glfinit) {
		GLFTInit ();
		pFace = new FontFace ("font.ttf");
		glfinit = 1;
	}

	// return immediately if object is not present
//	if (pData[1] != 0x8000 && !pState->pObjParam->pFlag[pData[1]]) return 7;
	
	// build transform matrix, offset
	Vector v1, v2, v3, pos, tv; Matrix m, m2;
	VecNorm (pState->pVtx+pData[4], &v3);
	VecNorm (pState->pVtx+pData[5], &v1);
	VecInv (&v3, &v3); VecCross (&v3, &v1, &v2);
	m.x1 = v1.x; m.x2 = v2.x; m.x3 = v3.x;
	m.y1 = v1.y; m.y2 = v2.y; m.y3 = v3.y;
	m.z1 = v1.z; m.z2 = v2.z; m.z3 = v3.z;
	MatMatMult (&pState->objorient, &m, &m2);

	VecMul (&v1, pData[6]*0.01f, &tv);
	VecMul (&v2, pData[7]*0.01f, &pos);
	VecAdd (&pos, &tv, &tv);
	VecAdd (pState->pVtx+pData[3], &tv, &tv);
	MatVecMult (&pState->objorient, &tv, &pos);
	VecAdd (&pos, &pState->objpos, &pos);

	float s = pState->scale*pData[8]*0.01f / pFace->GetHeight();
	glPushMatrix ();

	float pMV[16];
	pMV[0] = s*m2.x1; pMV[1] = s*m2.y1; pMV[2] = s*m2.z1; pMV[3] = 0.0f;
	pMV[4] = s*m2.x2; pMV[5] = s*m2.y2; pMV[6] = s*m2.z2; pMV[7] = 0.0f;
	pMV[8] = s*m2.x3; pMV[9] = s*m2.y3; pMV[10] = s*m2.z3; pMV[11] = 0.0f;
	pMV[12] = pos.x; pMV[13] = pos.y; pMV[14] = pos.z; pMV[15] = 1.0f;
	glLoadMatrixf (pMV);

	glFrontFace (GL_CCW);
	glNormal3f (0.0f, 0.0f, -1.0f);
//	glColor4f (1.0f, 1.0f, 1.0f, 1.0f);

	const char *pText;
	if (pData[2] != 0x8000) pText = pModelString[pData[2]];
	else pText = pState->pObjParam->pText[pData[1]];
	pFace->RenderString(pText);

	glFrontFace (GL_CW);
	glPopMatrix ();
	return 9;
}


/*
Uint16 PFUNC_WINDOWS
	Uint16 
	Uint16 textnum
	Uint16 pos
	Uint16 norm
	Uint16 xaxis
	Uint16 xoff
	Uint16 yoff
	Uint16 scale
*/


int (*pPrimFuncTable[])(Uint16 *, Model *, RState *) = {
	0,		// end
	PrimFuncMatAnim,
	PrimFuncMatFixed,
	PrimFuncMatVar,
	PrimFuncZBias,
	PrimFuncTriFlat,
	PrimFuncQuadFlat,
	PrimFuncCompoundSmooth,		// just uses steps = 0
	PrimFuncCompoundSmooth,
	PrimFuncCircle,
	PrimFuncCylinder,
	PrimFuncTube,
	PrimFuncSubObject,
	PrimFuncText,
};


