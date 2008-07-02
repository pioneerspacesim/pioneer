#ifndef __SBRE_INT_H__
#define __SBRE_INT_H__
#include "jjtypes.h"
#include "jjvector.h"
#include "sbre.h"

//*****************************************************************************

enum vtxtype
{
	VTYPE_PLAIN = 0,
	VTYPE_DIR,
	VTYPE_CROSS,
	VTYPE_NORM,
	VTYPE_ANIMLIN,
	VTYPE_ANIMCUBIC,
	VTYPE_ANIMHERM,
	VTYPE_ANIMROTATE,
};

struct PlainVertex
{
	uint32 type;
	Vector pos;
};

struct CompoundVertex
{
	uint16 type;
	uint16 pParam[5];
};

//**************************************************************************

struct Thruster
{
	uint16 pos;			// index into vertices
	uint16 dir;
	float power;
	int detail;			// 0 - min, 1 - mid, 2 - max
};


struct Model
{
	float scale;
	float radius;			// scale multiplies this too

	int numPVtx;
	PlainVertex *pPVtx;

	int cvStart;
	int numCVtx;
	CompoundVertex *pCVtx;

	int numCache;				// number of cached primitives

	struct {
		float pixrad;			// size in screen pixels below which LOD applies
		uint16 *pData1;			// pixrad <= 0.0f is top LOD - kinda backward
		uint16 *pData2;
		int numThrusters;
		Thruster *pThruster;
	} pLOD[4];

	int *pNumVtx, *pNumIdx;		// caches
	Vector **ppVCache;
	uint16 **ppICache;
};


//*****************************************************************************
// AnimFuncs

enum animmod
{
	AMOD_CLIP = 0,		// just clip result to 0-1
	AMOD_MOD1,			// fmod(1), then clip
	AMOD_REF,			// fmod(2), reflect around 1, then clip
};

struct AnimFunc
{
	int src;
	int mod;
	float order0;
	float order1;
	float order2;
	float order3;
};

enum animfunc
{
	AFUNC_GEAR = 0,
	AFUNC_GFLAP,
	AFUNC_THRUSTPULSE,
	AFUNC_LIN4SEC,
};

const AnimFunc pAFunc[] =
{
	{ ASRC_GEAR, AMOD_CLIP, -1.0f, 2.0f, 0.0f, 0.0f },
	{ ASRC_GEAR, AMOD_CLIP, 0.0f, 2.0f, 0.0f, 0.0f },
	{ ASRC_MINFRAC, AMOD_REF, 0.0f, 30.0f, 0.0f, 0.0f },
	{ ASRC_MINFRAC, AMOD_MOD1, 0.0f, 15.0f, 0.0f, 0.0f },
};

//*****************************************************************************
// stuff from simtriang.cpp

#define TRIANG_MAXPOINTS 64
#define TRIANG_MAXSTEPS 5

void ResolveLinearInterp (Vector *p0, Vector *p1, float t, Vector *pRes);
void ResolveQuadraticSpline (Vector *p0, Vector *p1, Vector *p2, float t, Vector *pRes);
void ResolveCubicSpline (Vector *p0, Vector *p1, Vector *p2, Vector *p3, float t, Vector *pRes);
void ResolveHermiteSpline (Vector *p0, Vector *p1, Vector *n0, Vector *n1, float t, Vector *pRes);
void ResolveHermiteNormal (Vector *p0, Vector *p1, Vector *n0, Vector *n1, float t, Vector *pRes);

void TriangAddPoint (Vector *pPos, Vector *pNorm);
void Triangulate (Vector *pCPos, Vector *pCNorm, int steps,
	Vector **ppVtx, int *pNV, uint16 **ppIndex, int *pNI);

//******************************************************************************
// Random rendering crap

struct RState
{
	Model *pModel;		// original model
	Vector *pVtx;		// transformed vertices
	Vector campos;		// camera pos relative to model
	Vector objpos;		// object pos relative to camera
	Matrix objorient;	// object orient relative to camera
	float scale;		// current GL modelview scale
	ObjParams *pObjParam;	// dynamic object parameters
	float dn, df;		// near/far depth range
	Vector compos;		// object relative centre of mass

	// Collision output stuff
	CollMesh *pCMesh;
	void (*pCallback)(int nv, int ni, Vector *pVertex, uint16 *pIndex, uint16 flags, RState *);
};



enum primtype
{
	PTYPE_END = 0,
	PTYPE_MATANIM,
	PTYPE_MATFIXED,
	PTYPE_MATVAR,
	PTYPE_ZBIAS,
	PTYPE_TRIFLAT,
	PTYPE_QUADFLAT,
	PTYPE_COMPFLAT,
	PTYPE_COMPSMOOTH,
	PTYPE_CIRCLE,
	PTYPE_CYLINDER,
	PTYPE_TUBE,
	PTYPE_SUBOBJECT,
	PTYPE_TEXT,
	PTYPE_EXTRUSION,
	PTYPE_SETCFLAG,
};

extern int (*pPrimFuncTable[])(uint16 *, Model *, RState *);
extern int (*pCollFuncTable[])(uint16 *, Model *, RState *);

static const int RFLAG_XREF = 0x8000;
static const int RFLAG_INVISIBLE = 0x4000;
static const int SUBOBJ_THRUST = 0x4000;

static const int THRUST_XREF = 0x8000;
static const int THRUST_NOANG = 0x4000;

//static const float SBRE_ZBIAS = 0.00002f;
static const float SBRE_AMB = 0.3f;

extern float SBRE_ZBIAS;

enum comptype
{
	COMP_END = 0,
	COMP_LINE,
	COMP_HERMITE,
	COMP_HERM_NOTAN,
	COMP_HERM_AUTOTAN,
	COMP_STEPS,
};

const int pCompSize[] = { 1, 3, 5, 3, 3, 2 };

const char pModelString[1][256] = {
	"Bollocks",
};

void RenderThrusters (RState *pState, int numThrusters, Thruster *pThrusters);
float ResolveAnim (ObjParams *pObjParam, uint16 type);
void GenCollMeshInternal (Vector *pPos, Matrix *pOrient, int model, ObjParams *pParam, float s, CollMesh *pCMesh);


#endif /* __SBRE_INT_H__ */