#ifndef __SBRE_H__
#define __SBRE_H__
#include "jjvector.h"


enum animsrc
{
	ASRC_GEAR = 0,	
	ASRC_SECFRAC,
	ASRC_MINFRAC,
	ASRC_HOURFRAC,
	ASRC_DAYFRAC,
	// stage 1,2,3 of docking animation
	ASRC_STATION_S1_BAY1,
	ASRC_STATION_S1_BAY2,
	ASRC_STATION_S1_BAY3,
	ASRC_STATION_S1_BAY4,
	ASRC_STATION_S2_BAY1,
	ASRC_STATION_S2_BAY2,
	ASRC_STATION_S2_BAY3,
	ASRC_STATION_S2_BAY4,
};

enum animflag
{
	AFLAG_GEAR = 0, 
};

#define ANIM_MAX 20

struct ObjParams
{
	float pAnim[ANIM_MAX];
	unsigned short pFlag[ANIM_MAX];

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

struct CollMesh
{
	int nv, ni;
	float *pVertex;
	int *pIndex;
	int *pFlag;

	int maxv, maxi;
	int cflag;
};

// if you don't call SetDepthRange, z bias and LOD will fail
// sd is screen depth in pixels, dn and df are like glDepthRange params
void sbreSetDepthRange (float sd, float dn, float df);
void sbreSetZBias (float zbias);
void sbreSetDirLight (float *pColor, float *pDir);
void sbreSetWireframe (int val);
void sbreRenderModel(const double pos[3], const double orient[16], int model, ObjParams *pParam,
	float s=1.0f, Vector *pCompos=0);
void sbreRenderModel (Vector *pPos, Matrix *pOrient, int model, ObjParams *pParam,
	float s=1.0f, Vector *pCompos=0);
void sbreCompilerLoadModels();
struct SbreModelNotFoundException {};
int sbreLookupModelByName(const char *name) throw (SbreModelNotFoundException);
float sbreGetModelRadius(int modelNum);

// will preserve and realloc pointers in pCMesh
// maxv/maxi should match allocated sizes
void sbreGenCollMesh (CollMesh *pCMesh, int model, ObjParams *pParam, float s=1.0f);
void sbreRenderCollMesh (CollMesh *pCMesh, Vector *pPos, Matrix *pOrient);

#endif /* __SBRE_H__ */

