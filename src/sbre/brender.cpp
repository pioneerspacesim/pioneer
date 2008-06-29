#include <SDL_opengl.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "sbre.h"
#include "sbre_int.h"
#include "sbre_models.h"

float ResolveAnim (ObjParams *pObjParam, uint16 type)
{
	const AnimFunc *pFunc = pAFunc+type;
	float anim = pObjParam->pAnim[pFunc->src];
	anim = anim*anim*anim*pFunc->order3 + anim*anim*pFunc->order2
		+ anim*pFunc->order1 + pFunc->order0;

	if (pFunc->mod == AMOD_REF) {
		anim = fmod(anim, 2.002f);
		if (anim > 1.0f) anim = 2.002f - anim;
	}
	else if (pFunc->mod == AMOD_MOD1) anim = fmod(anim, 1.001f);
	
	if (anim < 0.0f) anim = 0.0f;
	if (anim > 1.0f) anim = 1.0f;
	return anim;
}


static Vector pUnitVec[6] = {
	{ 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f },
	{ -1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f },
};

static void ResolveVertices (Model *pMod, Vector *pRes, ObjParams *pObjParam)
{
	Vector *pCur = pRes;
	Vector tv1, tv2, xax, yax;
	float anim;

	int i; for (i=0; i<6; i++) *(pCur++) = pUnitVec[i];

	PlainVertex *pPVtx = pMod->pPVtx;
	for (i=0; i<pMod->numPVtx-6; i++, pCur++, pPVtx++)
	{
		switch (pPVtx->type)
		{
		case VTYPE_PLAIN: *pCur = pPVtx->pos; break;
		case VTYPE_DIR: VecNorm (&pPVtx->pos, pCur); break;
		}
	}

	pCur = pRes + pMod->cvStart;
	CompoundVertex *pCVtx = pMod->pCVtx;
	for (i=0; i<pMod->numCVtx; i++, pCur++, pCVtx++)
	{
		switch (pCVtx->type)
		{
		case VTYPE_NORM:
			VecSub (pRes+pCVtx->pParam[2], pRes+pCVtx->pParam[1], &tv1);
			VecSub (pRes+pCVtx->pParam[0], pRes+pCVtx->pParam[1], &tv2);
			VecCross (&tv1, &tv2, pCur);
			VecNorm (pCur, pCur);
			break;

		case VTYPE_CROSS:
			VecCross (pRes+pCVtx->pParam[0], pRes+pCVtx->pParam[1], pCur);
			VecNorm (pCur, pCur);
			break;
		
		case VTYPE_ANIMLIN:
			anim = ResolveAnim (pObjParam, pCVtx->pParam[4]);
			*pCur = pRes[pCVtx->pParam[0]];
			VecSub (pRes+pCVtx->pParam[1], pCur, &tv1);
			VecMul (&tv1, anim, &tv1);
			VecAdd (&tv1, pCur, pCur);
			break;

		case VTYPE_ANIMCUBIC:
			anim = ResolveAnim (pObjParam, pCVtx->pParam[4]);
			ResolveCubicSpline (pRes+pCVtx->pParam[0], pRes+pCVtx->pParam[1],
				pRes+pCVtx->pParam[2], pRes+pCVtx->pParam[3], anim, pCur);
			break;

		case VTYPE_ANIMHERM:
			anim = ResolveAnim (pObjParam, pCVtx->pParam[4]);
			ResolveHermiteSpline (pRes+pCVtx->pParam[0], pRes+pCVtx->pParam[1],
				pRes+pCVtx->pParam[2], pRes+pCVtx->pParam[3], anim, pCur);
			break;

		case VTYPE_ANIMROTATE:
			anim = ResolveAnim (pObjParam, pCVtx->pParam[4]) * 2.0f * 3.141592f;
			xax = pRes[pCVtx->pParam[1]];
			VecCross (pRes+pCVtx->pParam[0], &xax, &yax);
			VecNorm (&xax, &xax); VecNorm (&yax, &yax);
			VecMul (&xax, sin(anim), &tv1);
			VecMul (&yax, cos(anim), &tv2);
			VecAdd (&tv1, &tv2, pCur);
			break;

		default: *pCur = zero_vector; break;
		}
	}		
}

static float g_dn, g_df, g_sd;
static int g_wireframe = 0;

void sbreSetViewport (int w, int h, float d, float zn, float zf, float dn, float df)
{
	glViewport (0, 0, w, h);

	float pProjMat[16] = { 1.0f, 0.0f, 0.0f, 0.0f,
							0.0f, 1.0f, 0.0f, 0.0f,
							0.0f, 0.0f, 1.0f, 1.0f,
							0.0f, 0.0f, 1.0f, 0.0f };

	pProjMat[0] = (2.0f*d)/w;
	pProjMat[5] = (2.0f*d)/h;
	pProjMat[10] = (zf+zn)/(zf-zn);
	pProjMat[14] = (-2.0f*zn*zf)/(zf-zn);

	glMatrixMode (GL_PROJECTION);
	glLoadMatrixf (pProjMat);
	glDepthRange (dn+SBRE_ZBIAS, df);
	g_dn = dn; g_df = df; g_sd = d;
}

void sbreSetDirLight (float *pColor, float *pDir)
{
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();

	float pColor4[4] = { 0, 0, 0, 1.0f };
	float pDir4[4] = { pDir[0], pDir[1], pDir[2], 0.0f };
	for (int i=0; i<3; i++) pColor4[i] = SBRE_AMB + pColor[i] * (1.0f-SBRE_AMB);

	glEnable (GL_LIGHT0);
	glLightfv (GL_LIGHT0, GL_POSITION, pDir4);
	glLightfv (GL_LIGHT0, GL_DIFFUSE, pColor4);
	glLightfv (GL_LIGHT0, GL_SPECULAR, pColor4);
}

void sbreSetWireframe (int val)
{
	g_wireframe = val;
}

void SetGeneralState ()
{
	float ambient[4] = { SBRE_AMB, SBRE_AMB, SBRE_AMB, 1.0f };
	glLightModelfv (GL_LIGHT_MODEL_AMBIENT, ambient);

	glDisable (GL_TEXTURE_2D);
	glFrontFace (GL_CW);
	glEnable (GL_DEPTH_TEST);

	if (g_wireframe) {
		glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
		glDisable (GL_CULL_FACE);
	} else {
		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		glEnable (GL_CULL_FACE);
	}	

	glDisableClientState (GL_TEXTURE_COORD_ARRAY);
	glDisableClientState (GL_COLOR_ARRAY);
}

void SetOpaqueState ()
{
	glDisable (GL_BLEND);

	glEnable (GL_LIGHTING);
	glEnable (GL_NORMALIZE);

	glEnableClientState (GL_VERTEX_ARRAY);
	glEnableClientState (GL_NORMAL_ARRAY);
}

void SetTransState ()
{
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable (GL_LIGHTING);
	glDisable (GL_NORMALIZE);

	glEnableClientState (GL_VERTEX_ARRAY);
	glDisableClientState (GL_NORMAL_ARRAY);
}


void sbreRenderModel (Vector *pPos, Matrix *pOrient, int model, ObjParams *pParam, float s, Vector *pCompos)
{
	Model *pModel = ppModel[model];
	s *= pModel->scale;
	float pMV[16];
	pMV[0] = s*pOrient->x1; pMV[1] = s*pOrient->y1; pMV[2] = s*pOrient->z1; pMV[3] = 0.0f;
	pMV[4] = s*pOrient->x2; pMV[5] = s*pOrient->y2; pMV[6] = s*pOrient->z2; pMV[7] = 0.0f;
	pMV[8] = s*pOrient->x3; pMV[9] = s*pOrient->y3; pMV[10] = s*pOrient->z3; pMV[11] = 0.0f;
	pMV[12] = pPos->x; pMV[13] = pPos->y; pMV[14] = pPos->z; pMV[15] = 1.0f;
	glMatrixMode (GL_MODELVIEW);
	glLoadMatrixf (pMV);

	Vector *pVtx = (Vector *) alloca (sizeof(Vector)*(pModel->cvStart+pModel->numCVtx));
	ResolveVertices (pModel, pVtx, pParam);

	RState rstate;
	rstate.pVtx = pVtx;
	rstate.objpos = *pPos;
	rstate.objorient = *pOrient;
	rstate.scale = s;
	rstate.pModel = pModel;
	rstate.pObjParam = pParam;
	rstate.dn = g_dn;
	rstate.df = g_df;
	MatTVecMult (pOrient, pPos, &rstate.campos);
	VecInv (&rstate.campos, &rstate.campos);
	if (pCompos) rstate.compos = *pCompos;
	else rstate.compos = zero_vector;

	if (pModel->numCache && !pModel->ppVCache)
	{
		pModel->pNumVtx = (int *) calloc (pModel->numCache, sizeof(int));
		pModel->pNumIdx = (int *) calloc (pModel->numCache, sizeof(int));
		pModel->ppVCache = (Vector **) calloc (pModel->numCache, sizeof(Vector *));
		pModel->ppICache = (uint16 **) calloc (pModel->numCache, sizeof(uint16 *));
	}

	SetGeneralState ();
	SetOpaqueState ();

	// Find suitable LOD
	float dist = sqrt(VecDot(pPos, pPos));
	float pixrad = g_sd * pModel->radius*s / dist;

	int i; for (i=0; i<4; i++)
	{
		if (pModel->pLOD[i].pixrad <= 0.0f) break;
		if (pixrad <= pModel->pLOD[i].pixrad) break;
	}

	uint16 *pData = pModel->pLOD[i].pData1;
	if (pData) while (*pData != PTYPE_END)	{	
		pData += pPrimFuncTable[*pData & 0xff] (pData, pModel, &rstate);
	}
	pData = pModel->pLOD[i].pData2;
	if (pData) while (*pData != PTYPE_END)	{
		pData += pPrimFuncTable[*pData & 0xff] (pData, pModel, &rstate);
	}

//	glDepthRange (g_dn+SBRE_ZBIAS, g_df);
	if (pModel->pLOD[i].numThrusters)
	{
		SetTransState ();
		RenderThrusters (&rstate, pModel->pLOD[i].numThrusters,	pModel->pLOD[i].pThruster);
	}

	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	glEnable (GL_CULL_FACE);
}



// fuck, needs to recurse too...
/*
void sbreRenderModel (Vector *pPos, Matrix *pOrient, int model, ObjParams *pParam, float s, Vector *pCompos)
{
	Model *pModel = ppModel[model];
	s *= pModel->scale;
	float pMV[16];
	pMV[0] = s*pOrient->x1; pMV[1] = s*pOrient->y1; pMV[2] = s*pOrient->z1; pMV[3] = 0.0f;
	pMV[4] = s*pOrient->x2; pMV[5] = s*pOrient->y2; pMV[6] = s*pOrient->z2; pMV[7] = 0.0f;
	pMV[8] = s*pOrient->x3; pMV[9] = s*pOrient->y3; pMV[10] = s*pOrient->z3; pMV[11] = 0.0f;
	pMV[12] = pPos->x; pMV[13] = pPos->y; pMV[14] = pPos->z; pMV[15] = 1.0f;
	glMatrixMode (GL_MODELVIEW);
	glLoadMatrixf (pMV);

	Vector *pVtx = (Vector *) alloca (sizeof(Vector)*(pModel->cvStart+pModel->numCVtx));

	ResolveVertices (pModel, pVtx, pParam);

	RState rstate;
	rstate.pVtx = pVtx;
	rstate.objpos = *pPos;
	rstate.objorient = *pOrient;
	rstate.scale = s;
	rstate.pModel = pModel;
	rstate.pObjParam = pParam;
	rstate.dn = g_dn;
	rstate.df = g_df;
	MatTVecMult (pOrient, pPos, &rstate.campos);
	VecInv (&rstate.campos, &rstate.campos);
	if (pCompos) rstate.compos = *pCompos;
	else rstate.compos = zero_vector;

	if (pModel->numCache && !pModel->ppVCache)
	{
		pModel->pNumVtx = (int *) calloc (pModel->numCache, sizeof(int));
		pModel->pNumIdx = (int *) calloc (pModel->numCache, sizeof(int));
		pModel->ppVCache = (Vector **) calloc (pModel->numCache, sizeof(Vector *));
		pModel->ppICache = (uint16 **) calloc (pModel->numCache, sizeof(uint16 *));
	}

	SetGeneralState ();
	SetOpaqueState ();

	float dist = sqrt(VecDot(pPos, pPos));
	float pixrad = g_sd * pModel->radius*s / dist;
	int i; for (i=0; i<4; i++)
	{
		if (pModel->pLOD[i].pixrad >= 1000.0f) break;
		if (pixrad <= pModel->pLOD[i].pixrad) break;
	}
	uint16 *pData = pModel->pLOD[i].pData;

	while (*pData != PTYPE_END)
	{	
		pData += pPrimFuncTable[*pData & 0xff] (pData, pModel, &rstate);
	}

//	glDepthRange (g_dn+SBRE_ZBIAS, g_df);
	SetTransState ();
	RenderTransparencies (&rstate);

	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	glEnable (GL_CULL_FACE);
}*/