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
	VTYPE_ANIMDIR,
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
};

struct Light
{
	uint8 animcolor;
	uint8 animpower;
	uint16 vtx;
	float power;
	float pColor[3];
};

struct Model
{
	float scale;

	int numPVtx;
	PlainVertex *pPVtx;

	int cvStart;
	int numCVtx;
	CompoundVertex *pCVtx;

	int numLights;
	Light *pLight;

	int numThrusters;
	Thruster *pThruster;

	uint16 *pData;

	int numCache;				// number of cached primitives
	int *pNumVtx, *pNumIdx;
	Vector **ppVCache;
	uint16 **ppICache;
};

extern Model *ppModel[];

//*****************************************************************************

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
};

extern int (*pPrimFuncTable[])(uint16 *, Model *, RState *);

static const int RFLAG_XREF = 0x8000;

static const int THRUST_XREF = 0x8000;
static const int THRUST_NOANG = 0x4000;

static const float SBRE_ZBIAS = 0.00002f;
static const float SBRE_AMB = 0.3f;

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

void RenderTransparencies (RState *pState);
float ResolveAnim (ObjParams *pObjParam, uint16 type);

#endif /* __SBRE_INT_H__ */