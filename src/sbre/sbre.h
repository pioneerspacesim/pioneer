#ifndef __SBRE_H__
#define __SBRE_H__
#include "jjtypes.h"
#include "jjvector.h"


enum animsrc
{
	ASRC_GEAR = 0,	
	ASRC_SECFRAC,
	ASRC_MINFRAC,
	ASRC_HOURFRAC,
	ASRC_DAYFRAC,
};

enum animflag
{
	AFLAG_GEAR = 0, 
};

struct ObjParams
{
	float pAnim[10];
	Uint8 pFlag[10];

	float linthrust[3];		// 1.0 to -1.0
	float angthrust[3];		// 1.0 to -1.0

	struct {
		float pDiff[3];
		float pSpec[3];
		float pEmis[3];
		float shiny;
	} pColor [3];

	char pText[3][256];
};


void sbreSetViewport (int w, int h, int d, float zn, float zf, float dn, float df);
void sbreSetDirLight (float *pColor, float *pDir);
void sbreRenderModel (Vector *pPos, Matrix *pOrient, int model, ObjParams *pParam, float s=1.0f);

#endif /* __SBRE_H__ */
