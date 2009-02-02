#include <SDL_opengl.h>
#include <malloc.h>
#include "sbre_int.h"
#include "sbre.h"			// for subobject
#include "../glfreetype.h"


/*
uint16 PFUNC_MATANIM
	uint16 animfunc
	uint16 dr1, dg1, db1, sr1, sg1, sb1, sh1, er1, eg1, eb1
	uint16 dr2, dg2, db2, sr2, sg2, sb2, sh2, er2, eg2, eb2
*/

static int PrimFuncMatAnim (uint16 *pData, Model *pMod, RState *pState)
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
uint16 PFUNC_MATFIXED
	uint16 dr, dg, db
	uint16 sr, sg, sb
	uint16 shiny
	uint16 er, eg, eb
*/

static int PrimFuncMatFixed (uint16 *pData, Model *pMod, RState *pState)
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
uint16 PFUNC_MATVAR
	uint16 index
*/

static int PrimFuncMatVar (uint16 *pData, Model *pMod, RState *pState)
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
uint16 PFUNC_ZBIAS
	uint16 pos			// to test if nearer - 0x8000 = reset
	uint16 norm
	uint16 units		// integer units. not used
*/

static int PrimFuncZBias (uint16 *pData, Model *pMod, RState *pState)
{
	if (pData[1] & 0x8000) glDepthRange (pState->dn+SBRE_ZBIAS, pState->df);
	else {
		Vector tv;
		VecSub (&pState->campos, pState->pVtx+pData[1], &tv);
		if (VecDot (&tv, pState->pVtx+pData[2]) > 0.0f)
			glDepthRange (pState->dn, pState->df-SBRE_ZBIAS);
	}
	return 4;
}

static int PrimFuncTriFlat (uint16 *pData, Model *pMod, RState *pState)
{
	if (pData[0] & RFLAG_INVISIBLE) return 4;
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

static int PrimFuncQuadFlat (uint16 *pData, Model *pMod, RState *pState)
{
	if (pData[0] & RFLAG_INVISIBLE) return 5;
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


static void RenderArray (int nv, int ni, Vector *pVertex, uint16 *pIndex, uint16 flags, RState *pState)
{
	if (pState->pCallback) {
		pState->pCallback (nv, ni, pVertex, pIndex, flags, pState);
		return;
	}

	glNormalPointer (GL_FLOAT, 2*sizeof(Vector), pVertex+1);
	glVertexPointer (3, GL_FLOAT, 2*sizeof(Vector), pVertex);
	glDrawElements (GL_TRIANGLES, ni, GL_UNSIGNED_SHORT, pIndex);

	if (flags & RFLAG_XREF)
	{
		glPushMatrix ();
		glFrontFace (GL_CW);
		const float pMV[16] = { -1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1 };
		glMultMatrixf (pMV);
		glDrawElements (GL_TRIANGLES, ni, GL_UNSIGNED_SHORT, pIndex);
		glFrontFace (GL_CCW);
		glPopMatrix ();
	}
}

/*
uint16 PFUNC_COMPSMOOTH
	uint16 cacheidx
	uint16 steps
	uint16 centpos
	uint16 centnorm
	uint16 startpos
	uint16 startnorm
		uint16 COMP_END
		uint16 COMP_LINE
			uint16 pos
			uint16 norm
		uint16 COMP_HERMITE
			uint16 pos
			uint16 norm
			uint16 tan0
			uint16 tan1

// tangents should be prescaled
*/

static int PrimFuncCompoundSmooth (uint16 *pData, Model *pMod, RState *pState)
{
	Vector *pVtx = pState->pVtx;

	int steps = pData[2];				// detail factor...
	Vector *pCPos = pVtx+pData[3];
	Vector *pCNorm = pVtx+pData[4];				// centre point
	Vector *pLPos = pVtx+pData[5];
	Vector *pLNorm = pVtx+pData[6];
	int c = 7;

	while (pData[c] != COMP_END)
	{
		if (pData[c] == COMP_STEPS)	steps = pData[c+1];
		else if (pData[c] == COMP_LINE)	
		{
			pLPos = pVtx+pData[c+1], pLNorm = pVtx+pData[c+2];
			TriangAddPoint (pLPos, pLNorm);
		}
		else
		{
			Vector t0, t1;
			if (pData[c] == COMP_HERMITE)
				{ t0 = pVtx[pData[c+3]]; t1 = pVtx[pData[c+4]]; }
			else if (pData[c] == COMP_HERM_NOTAN)
				{ VecSub (pVtx+pData[c+1], pLPos, &t0); t1 = t0; }
			else if (pData[c] == COMP_HERM_AUTOTAN)
			{
				Vector tv; VecSub (pVtx+pData[c+1], pLPos, &tv);
				VecMul (pLNorm, VecDot (&tv, pLNorm), &t0);
				VecSub (&tv, &t0, &t0);
				VecMul (pVtx+pData[c+2], VecDot (&tv, pVtx+pData[c+2]), &t1);
				VecSub (&tv, &t1, &t1);
			}
//			else { //crash? };

			// Now add points along spline
			float t, incstep = 1.0f / (steps+1);
			int i; for (i=0, t=incstep; i<steps; i++, t+=incstep)
			{
				Vector pos, norm;
				ResolveHermiteSpline (pLPos, pVtx+pData[c+1], &t0, &t1, t, &pos);
				ResolveLinearInterp (pLNorm, pVtx+pData[c+2], t, &norm);
				VecNorm (&norm, &norm);
				TriangAddPoint (&pos, &norm);
			}
			// add end point
			pLPos = pVtx+pData[c+1], pLNorm = pVtx+pData[c+2];
			TriangAddPoint (pLPos, pLNorm);
		}
		c += pCompSize[pData[c]];
	}

	int ni, nv;
	Vector *pVertex;
	uint16 *pIndex;
	if ((pData[0]&0xff) == PTYPE_COMPFLAT) steps = 0;
	Triangulate (pCPos, pCNorm, steps, &pVertex, &nv, &pIndex, &ni);

	RenderArray (nv, ni, pVertex, pIndex, pData[0], pState);
	return c+1;		// +1 for COMP_END
}

/*
uint16 PFUNC_CYLINDER
	uint16 cacheidx		-1 => uncacheable
	uint16 steps		8 => octagonal
	uint16 startvtx
	uint16 endvtx
	uint16 updir
	uint16 rad
*/

static int PrimFuncCylinder (uint16 *pData, Model *pMod, RState *pState)
{
	Vector *pVtx = pState->pVtx;

	int steps = pData[2];
	float rad = pData[6] * 0.01f;
	Vector *pVertex = (Vector *) alloca (8*steps*sizeof(Vector));
	uint16 *pIndex = (uint16 *) alloca (12*steps*sizeof(Vector));
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

	RenderArray (4*steps, ni, pVertex, pIndex, pData[0], pState);
	return 7;
}

/*
uint16 PFUNC_CIRCLE
	uint16 cacheidx		-1 => uncacheable
	uint16 steps		8 => octagonal
	uint16 vtx
	uint16 norm
	uint16 updir		
	uint16 rad
*/

static int PrimFuncCircle (uint16 *pData, Model *pMod, RState *pState)
{
	Vector *pVtx = pState->pVtx;

	int steps = pData[2];
	float rad = pData[6] * 0.01f;
	Vector *pVertex = (Vector *) alloca (2*steps*sizeof(Vector));
	uint16 *pIndex = (uint16 *) alloca (3*steps*sizeof(Vector));
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

	RenderArray (steps, ni, pVertex, pIndex, pData[0], pState);
	return 7;
}


/*
uint16 PFUNC_TUBE
	uint16 cacheidx		-1 => uncacheable
	uint16 steps		8 => octagonal
	uint16 startvtx
	uint16 endvtx
	uint16 updir
	uint16 outerrad
	uint16 innerrad
*/

static int PrimFuncTube (uint16 *pData, Model *pMod, RState *pState)
{
	Vector *pVtx = pState->pVtx;

	int steps = pData[2];
	Vector *pVertex = (Vector *) alloca (16*steps*sizeof(Vector));
	uint16 *pIndex = (uint16 *) alloca (24*steps*sizeof(Vector));
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

	RenderArray (8*steps, ni, pVertex, pIndex, pData[0], pState);
	return 8;
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

static int PrimFuncSubObject (uint16 *pData, Model *pMod, RState *pState)
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
	
	glPushAttrib (GL_LIGHTING_BIT);
	glPushMatrix ();

	// transform lin & ang thrust
	if (pData[0] & SUBOBJ_THRUST)
	{
		Vector compos;
		MatTVecMult (&m, pState->pVtx+pData[3], &compos);
		VecInv (&compos, &compos);

		ObjParams *pParam = pState->pObjParam;
		Vector oldlin = *(Vector *)pParam->linthrust;
		Vector oldang = *(Vector *)pParam->angthrust;
		MatTVecMult (&m, &oldlin, (Vector *)pParam->linthrust);
		MatTVecMult (&m, &oldang, (Vector *)pParam->angthrust);

		sbreRenderModel (&pos, &orient, pData[2], pParam, scale, &compos);
		*(Vector *)pParam->linthrust = oldlin;
		*(Vector *)pParam->angthrust = oldang;
	}
	else sbreRenderModel (&pos, &orient, pData[2], pState->pObjParam, scale);

	glPopMatrix ();
	glPopAttrib ();
	return 7;
}

static int glfinit = 0;
static FontFace *pFace = 0;

/*
uint16 PFUNC_TEXT
	uint16 anim
	uint16 textnum
	uint16 pos
	uint16 norm
	uint16 xaxis
	uint16 xoff
	uint16 yoff
	uint16 scale
*/

static int PrimFuncText (uint16 *pData, Model *pMod, RState *pState)
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
	VecInv (&v1, &v1); VecCross (&v3, &v1, &v2);
	m.x1 = v1.x; m.x2 = v2.x; m.x3 = v3.x;
	m.y1 = v1.y; m.y2 = v2.y; m.y3 = v3.y;
	m.z1 = v1.z; m.z2 = v2.z; m.z3 = v3.z;
	MatMatMult (&pState->objorient, &m, &m2);

	const char *pText;
	if (pData[2] != 0x8000) pText = pModelString[pData[2]];
	else pText = pState->pObjParam->pText[pData[1]];
	float xoff = 0, yoff = 0;

	/* Centre the damn thing if wanted!!!!!!!!!!!!!!!!!!! */
	if (pData[0] & (TFLAG_XCENTER | TFLAG_YCENTER)) {
		pFace->MeasureString(pText, xoff, yoff);
		xoff *= pData[8]*0.01f / pFace->GetHeight();
		yoff *= pData[8]*0.01f / pFace->GetHeight();
		if (!(pData[0] & TFLAG_XCENTER)) xoff = 0;
		if (!(pData[0] & TFLAG_YCENTER)) yoff = 0;
	}
	
	VecMul (&v1, pData[6]*0.01f - xoff*0.5, &tv);
	VecMul (&v2, pData[7]*0.01f - yoff*0.5, &pos);
	VecAdd (&pos, &tv, &tv);
	VecAdd (pState->pVtx+pData[3], &tv, &tv);
	VecMul (&tv, pState->scale, &tv);
	MatVecMult (&pState->objorient, &tv, &pos);
	VecAdd (&pos, &pState->objpos, &pos);

	glPushMatrix ();
	
	//void MeasureString(const char *str, float &w, float &h);

	float pMV[16];
	float s = pState->scale*pData[8]*0.01f / pFace->GetHeight();
	pMV[0] = s*m2.x1; pMV[1] = s*m2.y1; pMV[2] = s*m2.z1; pMV[3] = 0.0f;
	pMV[4] = s*m2.x2; pMV[5] = s*m2.y2; pMV[6] = s*m2.z2; pMV[7] = 0.0f;
	pMV[8] = s*m2.x3; pMV[9] = s*m2.y3; pMV[10] = s*m2.z3; pMV[11] = 0.0f;
	pMV[12] = pos.x; pMV[13] = pos.y; pMV[14] = pos.z; pMV[15] = 1.0f;
	glLoadMatrixf (pMV);

	glNormal3f (0.0f, 0.0f, 1.0f);
//	glColor4f (1.0f, 1.0f, 1.0f, 1.0f);

	pFace->RenderString(pText);

	glPopMatrix ();
	return 9;
}

/*
uint16 PFUNC_EXTRUSION
	uint16 cacheidx		-1 => uncacheable
	uint16 count
	uint16 startvtx
	uint16 endvtx
	uint16 updir
	uint16 rad
	uint16 firstvtx
*/

static int PrimFuncExtrusion (uint16 *pData, Model *pMod, RState *pState)
{
	Vector *pVtx = pState->pVtx;

	int steps = pData[2];
	float rad = pData[6] * 0.01f;
	Vector *pVertex = (Vector *) alloca (8*steps*sizeof(Vector));
	uint16 *pIndex = (uint16 *) alloca (12*steps*sizeof(Vector));
	int ni = 0;

	// generate cylinder axes

	Vector yax = pVtx[pData[5]], xax, zax;
	VecSub (pVtx+pData[4], pVtx+pData[3], &zax);	// dir = end-start
	VecNorm (&zax, &zax);
	VecCross (&yax, &zax, &xax);

	int i; for (i=0; i<steps; i++)
	{
		Vector tv, norm;
		VecMul (&xax, pVtx[pData[7]+i].x, &tv);
		VecMul (&yax, pVtx[pData[7]+i].y, &norm);
		VecAdd (&tv, &norm, &norm);
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
		FindNormal (pVertex+i*2, pVertex+(i+steps)*2, pVertex+i1*2, pVertex+i1*2+1);
		pIndex[ni++] = i; pIndex[ni++] = i+steps; pIndex[ni++] = i1;
		pIndex[ni++] = i+steps; pIndex[ni++] = i1+steps; pIndex[ni++] = i1;
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

	RenderArray (4*steps, ni, pVertex, pIndex, pData[0], pState);
	return 8;
}

/*
uint16 PFUNC_SETCFLAG
	uint16 flag
*/

static int PrimFuncSetCFlag (uint16 *pData, Model *pMod, RState *pState)
{
	return 2;
}



/*
uint16 PFUNC_WINDOWS
	uint16 
	uint16 textnum
	uint16 pos
	uint16 norm
	uint16 xaxis
	uint16 xoff
	uint16 yoff
	uint16 scale
*/


int (*pPrimFuncTable[])(uint16 *, Model *, RState *) = {
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
	PrimFuncExtrusion,
	PrimFuncSetCFlag,
};


