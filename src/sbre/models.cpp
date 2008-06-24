#include "sbre_int.h"
#include "sbre_anim.h"

const int SUB_NOSEWHEEL = 3;
const int SUB_WING = 4;
const int SUB_NACELLE = 5;
const int SUB_NWUNIT = 6;
const int SUB_MAINWHEEL = 7;
const int SUB_MWUNIT = 8;

enum AxisIndex {
	A_X = 0, A_Y, A_Z, A_NX, A_NY, A_NZ,
};


static PlainVertex tetravtx1[] = {
	{ VTYPE_PLAIN, { 0.0f, 50.0f, 0.0f } },			// 6
	{ VTYPE_PLAIN, { -50.0f, -30.0f, 30.0f } },
	{ VTYPE_PLAIN, { 50.0f, -30.0f, 30.0f } },
	{ VTYPE_PLAIN, { 0.0f, -30.0f, -50.0f } },
	{ VTYPE_PLAIN, { 0.0f, -30.0f, 0.0f } },		// 10, wheel base
};
static CompoundVertex tetravtx2[] = {
	{ VTYPE_CROSS, { 0, 1, 2, -1, -1 } },			// dummy
};
static Uint16 tetradata[] = {
	PTYPE_MATFIXED, 100, 0, 100, 0, 0, 0, 100, 0, 0, 0,
	PTYPE_TRIFLAT, 6, 7, 8,
	PTYPE_TRIFLAT, 6, 8, 9,
	PTYPE_TRIFLAT, 6, 9, 7,
	PTYPE_TRIFLAT, 9, 8, 7,
	PTYPE_SUBOBJECT, 0x8000, SUB_NOSEWHEEL, 10, 0, 4, 100,
	PTYPE_END,
};	
static Model tetramodel = { 1.0f, 11, tetravtx1, 20, 0, tetravtx2,
	0, 0, 0, 0, tetradata, 0 };


static PlainVertex circlevtx1[] = {
	{ VTYPE_PLAIN, { 0.0f, 0.0f, 0.0f } },
};
static CompoundVertex circlevtx2[] = {
	{ VTYPE_NORM, { 6, 7, 8, -1, -1 } },		// dummy
};
static Uint16 circledata[] = {
	PTYPE_MATANIM, AFUNC_THRUSTPULSE,
		0, 0, 0, 0, 0, 0, 100, 50, 50, 100,
		0, 0, 0, 0, 0, 0, 100, 0, 0, 50,
	PTYPE_CIRCLE, 0, 12, 6, 5, 1, 2000,
	PTYPE_END,
};	
static Model circlemodel = { 1.0f, 7, circlevtx1, 20, 0, circlevtx2,
	0, 0, 0, 0, circledata, 1 };


static PlainVertex cylvtx1[] = {
	{ VTYPE_PLAIN, { -100.0f, 20.0f, 0.0f } },
	{ VTYPE_PLAIN, { 100.0f, -10.0f, 0.0f } },
	{ VTYPE_PLAIN, { 0.0f, 0.0f, 0.0f } },
};
static CompoundVertex cylvtx2[] = {
	{ VTYPE_NORM, { 6, 7, 8, -1, -1 } },
};
static Uint16 cyldata[] = {
	PTYPE_MATFIXED, 100, 0, 100, 0, 0, 0, 100, 0, 0, 0,
//	PTYPE_CYLINDER, 0, 8, 6, 7, 20, 2000,
	PTYPE_TUBE, 0, 8, 6, 7, 20, 2000, 1000,
	PTYPE_END,
};	
static Model cylmodel = { 1.0f, 9, cylvtx1, 20, 1, cylvtx2,
	0, 0, 0, 0, cyldata, 1 };


static PlainVertex nwunitvtx1[] = {
	{ VTYPE_PLAIN, { 1.5f, 0.0f, 6.0f } },	// 6, flap
	{ VTYPE_PLAIN, { 1.5f, 0.0f, -1.0f } },
	{ VTYPE_PLAIN, { 0.0f, 0.0f, 6.0f } },
	{ VTYPE_PLAIN, { 0.0f, 0.0f, -1.0f } },
	{ VTYPE_PLAIN, { 1.5f, 1.5f, 6.0f } },
	{ VTYPE_PLAIN, { 1.5f, 1.5f, -1.0f } },

	{ VTYPE_PLAIN, { 0.0f, 1.5f, 0.0f } },		// 12, tangents
	{ VTYPE_PLAIN, { 1.5f, 0.0f, 0.0f } },

	{ VTYPE_PLAIN, { 0.0f, 0.0f, 0.0f } },		// 14, wheel pos
};
static CompoundVertex nwunitvtx2[] = {
	{ VTYPE_ANIMHERM, { 8, 10, 12, 13, AFUNC_GFLAP } },		// 20, flap paths
	{ VTYPE_ANIMHERM, { 9, 11, 12, 13, AFUNC_GFLAP } },		// 
	{ VTYPE_ANIMLIN, { 2, 1, -1, -1, AFUNC_GEAR } },		// gear y axis
};
static Uint16 nwunitdata[] = {
	PTYPE_MATFIXED, 20, 20, 20, 0, 0, 0, 100, 0, 0, 0,
	PTYPE_QUADFLAT | RFLAG_XREF, 8, 6, 7, 9,	// flap internal

	PTYPE_MATFIXED, 100, 0, 100, 0, 0, 0, 100, 0, 0, 0,
	PTYPE_QUADFLAT | RFLAG_XREF, 6, 20, 21, 7,	// flaps
	PTYPE_QUADFLAT | RFLAG_XREF, 6, 7, 21, 20,

//	PTYPE_CYLINDER, 3, 8, 6, 7, 0, 1000,

	PTYPE_SUBOBJECT, 0x8000, SUB_NOSEWHEEL, 14, 0, 22, 100,

	PTYPE_END,
};	
static Model nwunitmodel = { 1.0f, 15, nwunitvtx1, 20, 3, nwunitvtx2,
	0, 0, 0, 0, nwunitdata, 0 };


static PlainVertex nosewheelvtx1[] = {
	{ VTYPE_PLAIN, { 0.0f, 0.0f, 0.0f } },		// 6, strut
	{ VTYPE_PLAIN, { 0.0f, 3.0f, 0.0f } },
	{ VTYPE_PLAIN, { 0.0f, 5.0f, 0.0f } },
	{ VTYPE_PLAIN, { 0.5f, 5.0f, 0.0f } },		// 9, wheel	
	{ VTYPE_PLAIN, { 1.0f, 5.0f, 0.0f } },
};
static CompoundVertex nosewheelvtx2[] = {
	{ VTYPE_CROSS, { 0, 1, 2, -1, -1 } },		// dummy
};
static Uint16 nosewheeldata[] = {
	PTYPE_MATFIXED, 50, 50, 50, 100, 100, 100, 200, 0, 0, 0,
	PTYPE_CYLINDER, 0, 8, 6, 8, 2, 40,
	PTYPE_CYLINDER, 1, 8, 7, 8, 2, 50,
	PTYPE_MATFIXED, 30, 30, 30, 0, 0, 0, 100, 0, 0, 0,
	PTYPE_CYLINDER | RFLAG_XREF, 2, 8, 9, 10, 2, 100,
	PTYPE_END,
};	
static Model nosewheelmodel = { 1.0f, 11, nosewheelvtx1, 20, 0, nosewheelvtx2,
	0, 0, 0, 0, nosewheeldata, 4 };


static PlainVertex mwunitvtx1[] = {
	{ VTYPE_PLAIN, { 1.5f, 0.0f, 6.0f } },	// 6, flap
	{ VTYPE_PLAIN, { 1.5f, 0.0f, -1.0f } },
	{ VTYPE_PLAIN, { 0.0f, 0.0f, 6.0f } },
	{ VTYPE_PLAIN, { 0.0f, 0.0f, -1.0f } },
	{ VTYPE_PLAIN, { 1.5f, 1.5f, 6.0f } },
	{ VTYPE_PLAIN, { 1.5f, 1.5f, -1.0f } },

	{ VTYPE_PLAIN, { 0.0f, 1.5f, 0.0f } },		// 12, tangents
	{ VTYPE_PLAIN, { 1.5f, 0.0f, 0.0f } },

	{ VTYPE_PLAIN, { 0.0f, 0.0f, 0.0f } },		// 14, wheel pos
};
static CompoundVertex mwunitvtx2[] = {
	{ VTYPE_ANIMHERM, { 8, 10, 12, 13, AFUNC_GFLAP } },		// 20, flap paths
	{ VTYPE_ANIMHERM, { 9, 11, 12, 13, AFUNC_GFLAP } },		// 
	{ VTYPE_ANIMLIN, { 2, 1, -1, -1, AFUNC_GEAR } },		// gear y axis
};
static Uint16 mwunitdata[] = {
	PTYPE_MATFIXED, 20, 20, 20, 0, 0, 0, 100, 0, 0, 0,
	PTYPE_QUADFLAT | RFLAG_XREF, 8, 6, 7, 9,	// flap internal

	PTYPE_MATFIXED, 100, 0, 100, 0, 0, 0, 100, 0, 0, 0,
	PTYPE_QUADFLAT | RFLAG_XREF, 6, 20, 21, 7,	// flaps
	PTYPE_QUADFLAT | RFLAG_XREF, 6, 7, 21, 20,

//	PTYPE_CYLINDER, 3, 8, 6, 7, 0, 1000,

	PTYPE_SUBOBJECT, 0x8000, SUB_MAINWHEEL, 14, 0, 22, 100,

	PTYPE_END,
};	
static Model mwunitmodel = { 1.0f, 15, mwunitvtx1, 20, 3, mwunitvtx2,
	0, 0, 0, 0, mwunitdata, 0 };


static PlainVertex mainwheelvtx1[] = {
	{ VTYPE_PLAIN, { 0.0f, 0.0f, 0.0f } },		// 6, strut
	{ VTYPE_PLAIN, { 0.0f, 3.0f, 0.0f } },
	{ VTYPE_PLAIN, { 0.0f, 5.0f, 0.0f } },
	{ VTYPE_PLAIN, { 0.5f, 5.0f, 1.1f } },		// 9, wheel 1	
	{ VTYPE_PLAIN, { 1.0f, 5.0f, 1.1f } },
	{ VTYPE_PLAIN, { 0.5f, 5.0f, -1.1f } },	// 11, wheel 2
	{ VTYPE_PLAIN, { 1.0f, 5.0f, -1.1f } },
	{ VTYPE_PLAIN, { 0.0f, 5.0f, 1.4f } },		// 13, crossbar
	{ VTYPE_PLAIN, { 0.0f, 5.0f, -1.4f } },
};
static CompoundVertex mainwheelvtx2[] = {
	{ VTYPE_CROSS, { 0, 1, 2, -1, -1 } },		// dummy
};
static Uint16 mainwheeldata[] = {

	PTYPE_MATFIXED, 50, 50, 50, 100, 100, 100, 200, 0, 0, 0,
	PTYPE_CYLINDER, 0, 8, 6, 8, 2, 40,
	PTYPE_CYLINDER, 1, 8, 7, 8, 2, 50,
	PTYPE_CYLINDER, 4, 4, 13, 14, 0, 50,
	PTYPE_MATFIXED, 30, 30, 30, 0, 0, 0, 100, 0, 0, 0,
	PTYPE_CYLINDER | RFLAG_XREF, 2, 8, 9, 10, 2, 100,
	PTYPE_CYLINDER | RFLAG_XREF, 3, 8, 11, 12, 2, 100,
	PTYPE_END,
};	
static Model mainwheelmodel = { 1.0f, 15, mainwheelvtx1, 20, 0, mainwheelvtx2,
	0, 0, 0, 0, mainwheeldata, 5 };


static PlainVertex nacellevtx1[] = {
	{ VTYPE_PLAIN, { 30.0f, 0.0f, 30.0f } },		// 6
	{ VTYPE_PLAIN, { 30.0f, 0.0f, -30.0f } },
	{ VTYPE_PLAIN, { 30.0f, 10.0f, 0.0f } },
	{ VTYPE_PLAIN, { 40.0f, 0.0f, 0.0f } },
	{ VTYPE_PLAIN, { 30.0f, -10.0f, 0.0f } },
	{ VTYPE_PLAIN, { 20.0f, 0.0f, 0.0f } },

	{ VTYPE_PLAIN, { 14.0f, 0.0f, 0.0f } },		// 12, tangents
	{ VTYPE_PLAIN, { -14.0f, 0.0f, 0.0f } },
	{ VTYPE_PLAIN, { 0.0f, 14.0f, 0.0f } },
	{ VTYPE_PLAIN, { 0.0f, -14.0f, 0.0f } },
};
static CompoundVertex nacellevtx2[] = {
	{ VTYPE_CROSS, { 0, 1, 2, -1, -1 } },		// dummy
};
static Uint16 nacelledata[] = {
	PTYPE_MATFIXED, 100, 0, 100, 0, 0, 0, 100, 0, 0, 0,
	PTYPE_COMPSMOOTH | RFLAG_XREF, 0, 5, 6, 2, 8, 1,
		COMP_HERMITE, 11, 3, 13, 15,
		COMP_HERMITE, 10, 4, 15, 12,
		COMP_HERMITE, 9, 0, 12, 14,
		COMP_HERMITE, 8, 1, 14, 13,
		COMP_END,
	PTYPE_COMPSMOOTH | RFLAG_XREF, 1, 5, 7, 5, 8, 1,
		COMP_HERMITE, 9, 0, 12, 15,
		COMP_HERMITE, 10, 4, 15, 13,
		COMP_HERMITE, 11, 3, 13, 14,
		COMP_HERMITE, 8, 1, 14, 12,
		COMP_END,
	PTYPE_END,
};
static Model nacellemodel = { 1.0f, 16, nacellevtx1, 20, 0, nacellevtx2,
	0, 0, 0, 0, nacelledata, 2 };


// do wings as subobjects

static PlainVertex shipvtx1[] = {
	{ VTYPE_PLAIN, { 5.0f, 10.0f, 30.0f } },		// 6, top four body verts
	{ VTYPE_PLAIN, { -5.0f, 10.0f, 30.0f } },
	{ VTYPE_PLAIN, { 5.0f, 10.0f, -30.0f } },
	{ VTYPE_PLAIN, { -5.0f, 10.0f, -30.0f } },

	{ VTYPE_PLAIN, { 11.16025f, -0.6698729f, 25.0f } },		// 10, right four body verts
	{ VTYPE_PLAIN, { 6.160254f, -9.330127f, 35.0f } },
	{ VTYPE_PLAIN, { 11.16025f, -0.6698729f, -35.0f } },
	{ VTYPE_PLAIN, { 6.160254f, -9.330127f, -30.0f } },

	{ VTYPE_PLAIN, { -11.16025f, -0.6698729f, 25.0f } },		// 14, left four body verts
	{ VTYPE_PLAIN, { -6.160254f, -9.330127f, 35.0f } },
	{ VTYPE_PLAIN, { -11.16025f, -0.6698729f, -35.0f } },
	{ VTYPE_PLAIN, { -6.160254f, -9.330127f, -30.0f } },

	{ VTYPE_PLAIN, { 5.0f, -0.6698729f, 60.0f } },		// 18, front two verts
	{ VTYPE_PLAIN, { -5.0f, -0.6698729f, 60.0f } },	

	{ VTYPE_PLAIN, { 0.0f, 10.0f, 0.0f } },				// 20, top wing
	{ VTYPE_PLAIN, { 1.0f, 0.0f, 0.0f } },	
	{ VTYPE_PLAIN, { 0.0f, 1.0f, 0.0f } },	

	{ VTYPE_PLAIN, { 8.660254f, -5.0f, 0.0f } },				// 23, right wing
	{ VTYPE_PLAIN, { -0.5f, -0.8660254f, 0.0f } },
	{ VTYPE_PLAIN, { 0.8660254f, -0.5f, 0.0f } },

	{ VTYPE_PLAIN, { -8.660254f, -5.0f, 0.0f } },				// 26, left wing
	{ VTYPE_PLAIN, { -0.5f, 0.8660254f, 0.0f } },
	{ VTYPE_PLAIN, { -0.8660254f, -0.5f, 0.0f } },

	{ VTYPE_PLAIN, { 0.0f, 0.0f, -40.0f } },					// 29, main thruster
	{ VTYPE_PLAIN, { 11.0f, 0.0f, 35.0f } },					// 30, retro
	{ VTYPE_PLAIN, { -11.0f, 0.0f, 35.0f } },	
	{ VTYPE_PLAIN, { 9.0f, 5.0f, 30.0f } },						// 32, right
	{ VTYPE_PLAIN, { 12.0f, -5.0f, -30.0f } },
	{ VTYPE_PLAIN, { -12.0f, -5.0f, 30.0f } },					// 34, left
	{ VTYPE_PLAIN, { -9.0f, 5.0f, -30.0f } },
	{ VTYPE_PLAIN, { 0.0f, 12.0f, 30.0f } },					// 36, top
	{ VTYPE_PLAIN, { 0.0f, 12.0f, -30.0f } },
	{ VTYPE_PLAIN, { 0.0f, -12.0f, 30.0f } },					// 38, bottom
	{ VTYPE_PLAIN, { 0.0f, -12.0f, -30.0f } },

	{ VTYPE_PLAIN, { 0.0f, -9.330127f, 30.0f } },				// 40, nosewheel
	{ VTYPE_PLAIN, { 0.0f, -9.330127f, -13.0f } },				// 41, mainwheel

};
static CompoundVertex shipvtx2[] = {
	{ VTYPE_ANIMLIN, { 25, 0, -1, -1, 0 } },			// 50, right wing yaxis	
	{ VTYPE_CROSS, { 50, 2, -1, -1, 0 } },				// right wing xaxis	

	{ VTYPE_ANIMLIN, { 28, 3, -1, -1, 0 } },			// 52, left wing yaxis	
	{ VTYPE_CROSS, { 52, 2, -1, -1, -1 } },				// right wing xaxis	

	{ VTYPE_NORM, { 16, 14, 7, -1, -1 } },				// 54, left text normal
	{ VTYPE_NORM, { 12, 8, 6, -1, -1 } },				// 55, right text normal

};
static Uint16 shipdata[] = {
	PTYPE_MATFIXED, 100, 0, 100, 0, 0, 0, 100, 0, 0, 0,
	PTYPE_QUADFLAT, 7, 6, 8, 9,					// top
	PTYPE_QUADFLAT, 13, 11, 15, 17,				// bottom
	PTYPE_QUADFLAT | RFLAG_XREF, 8, 6, 10, 12,	// side top
	PTYPE_QUADFLAT | RFLAG_XREF, 12, 10, 11, 13,	// side bottom
	PTYPE_QUADFLAT, 9, 8, 12, 16,				// back top
	PTYPE_QUADFLAT, 16, 12, 13, 17,				// back bottom

	PTYPE_QUADFLAT, 6, 7, 19, 18, 				// front top
	PTYPE_QUADFLAT, 18, 19, 15, 11, 				// front bottom
	PTYPE_TRIFLAT | RFLAG_XREF, 6, 18, 10,		// front side top
	PTYPE_TRIFLAT | RFLAG_XREF, 10, 18, 11,		// front side bottom

	PTYPE_SUBOBJECT, 0x8000, SUB_WING, 20, 21, 22, 100,
	PTYPE_SUBOBJECT, 0x8000, SUB_WING, 23, 51, 50, 100,
	PTYPE_SUBOBJECT, 0x8000, SUB_WING, 26, 53, 52, 100,

	PTYPE_MATFIXED, 20, 20, 20, 0, 0, 0, 100, 0, 0, 0,
	PTYPE_ZBIAS, 54, 5,
	PTYPE_TEXT, 0, 0x8000, 14, 54, 5, 500, 200, 1000,
	PTYPE_ZBIAS, 55, 5,
	PTYPE_TEXT, 0, 0x8000, 12, 55, 2, 1100, 200, 1000,

	PTYPE_ZBIAS, 4, 5,
	PTYPE_SUBOBJECT, 0, SUB_NWUNIT, 40, 0, 4, 200,
	PTYPE_SUBOBJECT, 0, SUB_MWUNIT, 41, 0, 4, 200,

//	PTYPE_TEXT, -1, -1, 12, 0, 1, 5000,
	PTYPE_ZBIAS, 0x8000, 5,

	PTYPE_END,
};
static Thruster shipthruster[] = {
	{ 29, 5 | THRUST_NOANG, 50.0f },
	{ 30, 2 | THRUST_NOANG, 35.0f },		// retros
	{ 31, 2 | THRUST_NOANG, 35.0f },
	{ 32, 0, 25.0f }, { 33, 0, 25.0f },	// right
	{ 34, 3, 25.0f }, { 35, 3, 25.0f },	// left
	{ 36, 1, 25.0f }, { 37, 1, 25.0f },	// top
	{ 38, 4, 25.0f }, { 39, 4, 25.0f },	// bottom
};
static Model shipmodel = { 1.0f, 42, shipvtx1, 50, 6, shipvtx2,
	0, 0, 11, shipthruster, shipdata, 0 };


static PlainVertex wingvtx1[] = {
	{ VTYPE_PLAIN, { 0.0f, 0.0f, 1.0f } },		// 6, bottom front
	{ VTYPE_PLAIN, { 0.0f, 0.0f, -1.0f } },		// bottom back
	{ VTYPE_PLAIN, { 0.0f, 1.5f, 0.0f } },		// top front
	{ VTYPE_PLAIN, { 0.0f, 1.5f, -1.5f } },		// top back

	{ VTYPE_PLAIN, { 0.1f, 0.75f, -0.5f } },		// 10, sidecentre
	{ VTYPE_PLAIN, { 0.0f, 1.5f, -0.75f } },		// topcentre

	{ VTYPE_PLAIN, { 1.0f, 0.0f, 0.0f } },			// 12, norm, sidecentre
	{ VTYPE_PLAIN, { 0.0f, 1.0f, 0.0f } },			// norm, topcentre

	{ VTYPE_PLAIN, { 0.4f, 0.0f, -2.0f } },			// 14, tan 0->1, 0
	{ VTYPE_PLAIN, { -0.4f, 0.0f, -2.0f } },		// tan 0->1, 1
	{ VTYPE_PLAIN, { 0.0f, 1.5f, -0.5f } },			// 16, tan 1->3, 0, 1
	{ VTYPE_PLAIN, { 0.4f, 0.0f, 1.5f } },			// 17, tan 3->2, 0
	{ VTYPE_PLAIN, { -0.4f, 0.0f, 1.5f } },			// tan 3->2, 1
	{ VTYPE_PLAIN, { 0.0f, -1.5f, 1.0f } },			// 19, tan 2->0, 0, 1

	{ VTYPE_PLAIN, { 0.4f, 0.0f, -1.5f } },			// 20, tan 2->3 top, 0
	{ VTYPE_PLAIN, { -0.4f, 0.0f, -1.5f } },		// tan 2->3 top, 1
	{ VTYPE_PLAIN, { -0.4f, 0.0f, 1.5f } },			// 22, tan 3->2 top, 0
	{ VTYPE_PLAIN, { 0.4f, 0.0f, 1.5f } },			// tan 3->2 top, 1
};
static CompoundVertex wingvtx2[] = {
	{ VTYPE_CROSS, { 19, 14, -1, -1, -1 } },		// 30, norm 0
	{ VTYPE_CROSS, { 15, 16, -1, -1, -1 } },		// norm 1
	{ VTYPE_CROSS, { 16, 17, -1, -1, -1 } },	// norm 3
	{ VTYPE_CROSS, { 18, 19, -1, -1, -1 } },	// norm 2
};
static Uint16 wingdata[] = {
	PTYPE_MATFIXED, 100, 0, 100, 0, 0, 0, 100, 0, 0, 0,
	PTYPE_COMPSMOOTH | RFLAG_XREF, 0, 5, 10, 12, 6, 30,		// side
		COMP_HERMITE, 7, 31, 14, 15,
		COMP_HERMITE, 9, 32, 16, 16,
		COMP_HERMITE, 8, 33, 17, 18,
		COMP_HERMITE, 6, 30, 19, 19,
		COMP_END,
	PTYPE_COMPSMOOTH, 1, 5, 11, 13, 8, 13,					// top
		COMP_HERMITE, 9, 13, 20, 21,
		COMP_HERMITE, 8, 13, 22, 23,
		COMP_END,
	PTYPE_END,
};
static Model wingmodel = { 25.0f, 24, wingvtx1, 30, 4, wingvtx2,
	0, 0, 0, 0, wingdata, 2 };




static PlainVertex ship2vtx1[] = {
	{ VTYPE_PLAIN, { 0.0f, 0.0f, 35.0f } },			// 6, nose point
	{ VTYPE_DIR, { 0.0f, 1.0f, 0.2f } },			// nose normal
	{ VTYPE_PLAIN, { 6.0f, 0.0f, 18.0f } },			// 8, r edge forward mid
	{ VTYPE_DIR, { 0.2f, 1.0f, 0.1f } },			// norm
	{ VTYPE_PLAIN, { 12.0f, 0.0f, -2.0f } },		// 10, r edge back mid
	{ VTYPE_DIR, { 0.2f, 1.0f, 0.1f } },			// norm
	{ VTYPE_PLAIN, { 7.5f, 0.0f, -25.0f } },		// 12, r edge back
	{ VTYPE_DIR, { 0.0f, 1.0f, -0.2f } },			// norm

	{ VTYPE_PLAIN, { 0.0f, 3.0f, 15.0f } },			// 14, cockpit front
	{ VTYPE_DIR, { 0.0f, 1.0f, -0.08f } },		// norm
	{ VTYPE_PLAIN, { 1.5f, 3.0f, 13.5f } },			// 16, cockpit right
	{ VTYPE_DIR, { 0.0f, 1.0f, -0.08f } },		// norm
	{ VTYPE_PLAIN, { 0.0f, 3.0f, 10.0f } },			// 18, cockpit back
	{ VTYPE_DIR, { 0.0f, 1.0f, -0.08f } },		// norm
	{ VTYPE_PLAIN, { -1.5f, 3.0f, 13.5f } },		// 20, cockpit left
	{ VTYPE_DIR, { 0.0f, 1.0f, -0.08f } },		// norm

	{ VTYPE_PLAIN, { 6.0f, 3.0f, -5.0f } },			// 22, inner right
	{ VTYPE_DIR, { 0.2f, 1.0f, 0.2f } },			// norm
	{ VTYPE_PLAIN, { 0.0f, 3.0f, -5.0f } },			// 24, inner mid
	{ VTYPE_DIR, { -0.2f, 1.0f, 0.2f } },		// norm

	{ VTYPE_PLAIN, { 2.0f, 2.0f, 23.0f } },			// 26, fwd midpoint
	{ VTYPE_DIR, { 0.0f, 1.0f, 0.1f } },		// norm
	{ VTYPE_PLAIN, { 5.0f, 2.5f, 5.0f } },			// 28, right midpoint
	{ VTYPE_DIR, { 0.08f, 1.0f, 0.04f } },		// norm
	{ VTYPE_PLAIN, { 7.0f, 2.0f, -14.0f } },		// 30, rear right midpoint
	{ VTYPE_DIR, { 0.04f, 1.0f, -0.1f } },		// norm

	{ VTYPE_PLAIN, { 3.0f, 3.0f, 5.0f } },			// 32, central midpoint
	{ VTYPE_PLAIN, { 0.0f, 4.0f, 12.5f } },			// 33, cockpit midpoint
	{ VTYPE_PLAIN, { 3.75f, 4.0f, -20.0f } },		// 34, nacelle midpoint

	{ VTYPE_PLAIN, { 7.5f, 0.0f, -30.0f } },		// 35, nacelle outer
	{ VTYPE_PLAIN, { 0.0f, 0.0f, -30.0f } },		// 36, nacelle inner

	// edge tangents
	{ VTYPE_PLAIN, { -6.0f, 4.0f, -3.0f } },		// 37, edge to mid
	{ VTYPE_PLAIN, { -6.0f, 0.0f, -3.0f } },		//
	{ VTYPE_PLAIN, { 0.0f, 4.0f, 20.0f } },			// 39, rear to mid
	{ VTYPE_PLAIN, { -2.5f, 0.0f, 20.0f } },		//

	{ VTYPE_PLAIN, { 0.0f, 0.0f, 20.0f } },			// 41, mid to nose
	{ VTYPE_PLAIN, { 0.0f, -4.0f, 20.0f } },
	{ VTYPE_PLAIN, { 6.0f, 0.0f, 3.0f } },			// 43, mid to edge
	{ VTYPE_PLAIN, { 6.0f, -4.0f, 3.0f } },
	{ VTYPE_PLAIN, { 2.5f, 0.0f, -20.0f } },			// 45, mid to rear
	{ VTYPE_PLAIN, { 0.0f, -4.0f, -20.0f } },

	{ VTYPE_PLAIN, { 1.5f, 0.0f, 0.0f } },			// 47, cockpit CW tangents
	{ VTYPE_PLAIN, { -1.5f, 0.0f, 0.0f } },
	{ VTYPE_PLAIN, { 0.0f, 0.0f, 1.5f } },
	{ VTYPE_PLAIN, { 0.0f, 0.0f, -1.5f } },
	{ VTYPE_PLAIN, { 0.0f, 0.0f, 3.5f } },			// 51
	{ VTYPE_PLAIN, { 0.0f, 0.0f, -3.5f } },

	{ VTYPE_PLAIN, { 10.0f, 0.0f, -20.0f } },			// 53, rear edge tangents
	{ VTYPE_PLAIN, { -10.0f, 0.0f, 0.0f } },
	{ VTYPE_PLAIN, { -10.0f, 0.0f, 20.0f } },			// 55, CCW
	{ VTYPE_PLAIN, { 10.0f, 0.0f, 0.0f } },

	{ VTYPE_PLAIN, { 0.0f, 5.0f, 0.0f } },			// 57, nacelle tangents
	{ VTYPE_PLAIN, { 0.0f, -5.0f, 0.0f } },
	{ VTYPE_PLAIN, { 0.0f, 0.0f, 12.0f } },
	{ VTYPE_PLAIN, { 0.0f, 0.0f, -12.0f } },

	{ VTYPE_PLAIN, { 3.75f, 4.0f, -30.0f } },			// 61, nacelle rear midpoint
	{ VTYPE_PLAIN, { 4.0f, 0.0f, 0.0f } },			// and tangents
	{ VTYPE_PLAIN, { -4.0f, 0.0f, 0.0f } },			// 

	// underside points
	{ VTYPE_PLAIN, { 5.0f, 0.0f, 5.0f } },			// 64, upper outer vent
	{ VTYPE_PLAIN, { 0.0f, 0.0f, 5.0f } },			// 65, upper inner vent
	{ VTYPE_PLAIN, { 5.0f, -2.0f, 3.0f } },			// 66, lower outer vent
	{ VTYPE_PLAIN, { 0.0f, -2.0f, 3.0f } },			// 67, lower inner vent
	{ VTYPE_PLAIN, { 5.0f, -2.0f, -30.0f } },		// 68, nacelle outer underside
	{ VTYPE_PLAIN, { 0.0f, -2.0f, -30.0f } },		// 69, nacelle inner underside
	{ VTYPE_PLAIN, { 13.0f, 0.0f, -14.0f } },		// 70, rear underside centre
	{ VTYPE_PLAIN, { 7.5f, 0.0f, 3.0f } },			// 71, vent outer edge 

	{ VTYPE_PLAIN, { 3.75f, 0.7f, -30.0f } },			// 72, engine midpoint

	{ VTYPE_PLAIN, { 0.0f, 0.0f, 15.0f } },			// 73, nose gear pos
	{ VTYPE_PLAIN, { 3.75f, -2.0f, -15.0f } },			// 74, rear right gear
	{ VTYPE_PLAIN, { -3.75f, -2.0f, -15.0f } },			// 75, rear left gear

	{ VTYPE_PLAIN, { 3.75f, 0.7f, -32.0f } },			// 76, engine end

	{ VTYPE_PLAIN, { 4.5f, -0.3f, 4.7f } },			// 77, retro vent
	{ VTYPE_PLAIN, { 0.5f, -0.3f, 4.7f } },			// 
	{ VTYPE_PLAIN, { 4.5f, -1.7f, 3.3f } },			// 
	{ VTYPE_PLAIN, { 0.5f, -1.7f, 3.3f } },			// 

	// main & retro thrusters
	{ VTYPE_PLAIN, { 3.75f, 0.7f, -32.0f } },			// 81
	{ VTYPE_PLAIN, { -3.75f, 0.7f, -32.0f } },			
	{ VTYPE_PLAIN, { 2.5f, -1.0f, 5.0f } },
	{ VTYPE_PLAIN, { -2.5f, -1.0f, 5.0f } },

	// vertical thrusters
	{ VTYPE_PLAIN, { 9.0f, 1.5f, -10.0f } },			// 85
	{ VTYPE_PLAIN, { 9.0f, -0.5f, -10.0f } },
	{ VTYPE_PLAIN, { -9.0f, 1.5f, -10.0f } },			// 
	{ VTYPE_PLAIN, { -9.0f, -0.5f, -10.0f } },			// 
	{ VTYPE_PLAIN, { 0.0f, 3.5f, 8.0f } },			// 
	{ VTYPE_PLAIN, { 0.0f, -0.5f, 25.0f } },

	// horizontal thrusters
	{ VTYPE_PLAIN, { 8.0f, 0.0f, -28.0f } },			// 91
	{ VTYPE_PLAIN, { -8.0f, 0.0f, -28.0f } },
	{ VTYPE_PLAIN, { 3.5f, 0.0f, 25.0f } },
	{ VTYPE_PLAIN, { -3.5f, 0.0f, 25.0f } },

	// text norms
	{ VTYPE_DIR, { 2.0f, -2.5f, 0.0f } },			// 95
	{ VTYPE_DIR, { -2.0f, -2.5f, 0.0f } },
	{ VTYPE_PLAIN, { -5.0f, -2.0f, 3.0f } },		// 97, lower outer vent, left side
	
};
static CompoundVertex ship2vtx2[] = {
	{ VTYPE_NORM, { 77, 78, 80, -1, -1 } },			// 120, retro norm
};
static Uint16 ship2data[] = {
	PTYPE_MATFIXED, 100, 0, 100, 0, 0, 0, 100, 0, 0, 0, 
	PTYPE_COMPSMOOTH | RFLAG_XREF, 0, 5, 26, 27, 6, 7,		// front edge
		COMP_HERM_NOTAN, 8, 9,
		COMP_HERMITE, 16, 1, 37, 38,
		COMP_HERMITE, 14, 1, 49, 48,
		COMP_HERMITE, 6, 7, 41, 42,
		COMP_END,
	PTYPE_COMPSMOOTH | RFLAG_XREF, 1, 5, 28, 29, 8, 9,		// mid edge
		COMP_HERM_NOTAN, 10, 11,
		COMP_HERMITE, 22, 1, 37, 38,
		COMP_HERM_NOTAN, 16, 1,
		COMP_HERMITE, 8, 9, 43, 44,
		COMP_END,
	PTYPE_COMPSMOOTH | RFLAG_XREF, 2, 5, 30, 31, 10, 11,		// rear edge
		COMP_HERMITE, 12, 13, 53, 54, 
		COMP_HERMITE, 22, 1, 39, 40,
		COMP_HERMITE, 10, 11, 43, 44,
		COMP_END,
	PTYPE_COMPFLAT | RFLAG_XREF, 3, 5, 32, 1, 16, 1,		// centre
		COMP_HERM_NOTAN, 22, 1,
		COMP_HERMITE, 24, 1, 59, 60,
		COMP_HERM_NOTAN, 18, 1,
		COMP_HERMITE, 16, 1, 47, 51, 
		COMP_END,
	PTYPE_COMPSMOOTH | RFLAG_XREF, 5, 5, 34, 1, 22, 23,		// nacelle
		COMP_HERMITE, 12, 0, 45, 46,
		COMP_HERM_NOTAN, 35, 0,
		COMP_HERMITE, 61, 1, 57, 63,
		COMP_HERMITE, 36, 3, 63, 58,
		COMP_HERM_NOTAN, 24, 25,
		COMP_HERMITE, 22, 23, 59, 60,
		COMP_END,
	PTYPE_COMPFLAT | RFLAG_XREF, 6, 5, 70, 4, 12, 4,		// rear underside
		COMP_HERMITE, 10, 4, 56, 55, 
		COMP_LINE, 71, 4,
		COMP_LINE, 12, 4,
		COMP_END,
	PTYPE_QUADFLAT | RFLAG_XREF, 8, 6, 65, 64,			// other underside
	PTYPE_QUADFLAT | RFLAG_XREF, 8, 64, 71, 10,
	PTYPE_QUADFLAT | RFLAG_XREF, 64, 65, 67, 66,
	PTYPE_TRIFLAT | RFLAG_XREF, 71, 64, 66,
	PTYPE_QUADFLAT | RFLAG_XREF, 71, 66, 68, 12,
	PTYPE_TRIFLAT | RFLAG_XREF, 12, 68, 35,
	PTYPE_QUADFLAT | RFLAG_XREF, 66, 67, 69, 68,
	PTYPE_COMPFLAT | RFLAG_XREF, 7, 5, 72, 5, 36, 5,		// engine back face
		COMP_HERMITE, 61, 5, 57, 62, 
		COMP_HERMITE, 35, 5, 62, 58,
		COMP_LINE, 68, 5,
		COMP_LINE, 69, 5,
		COMP_LINE, 36, 5,
		COMP_END,

	PTYPE_MATFIXED, 30, 30, 30, 30, 30, 30, 200, 0, 0, 0,
	PTYPE_COMPSMOOTH, 4, 5, 33, 1, 16, 0,		// cockpit
		COMP_HERMITE, 18, 5, 52, 48,
		COMP_HERMITE, 20, 3, 48, 51,
		COMP_HERMITE, 14, 2, 49, 47,
		COMP_HERMITE, 16, 0, 47, 50, 
		COMP_END,

	PTYPE_ZBIAS, 5, 5,
	PTYPE_MATFIXED, 30, 30, 30, 30, 30, 30, 200, 0, 0, 0,
	PTYPE_TUBE | RFLAG_XREF, 8, 12, 72, 76, 1, 250, 200,
	PTYPE_MATANIM, AFUNC_THRUSTPULSE,
		0, 0, 0, 0, 0, 0, 100, 50, 50, 100,
		0, 0, 0, 0, 0, 0, 100, 0, 0, 50,
	PTYPE_CIRCLE | RFLAG_XREF, 9, 12, 72, 5, 1, 200,

	PTYPE_ZBIAS, 120, 5,
//	PTYPE_MATFIXED, 30, 30, 30, 0, 0, 0, 100, 0, 0, 0,
	PTYPE_QUADFLAT | RFLAG_XREF, 77, 78, 80, 79,

	PTYPE_MATFIXED, 20, 20, 20, 0, 0, 0, 100, 0, 0, 0,
	PTYPE_ZBIAS, 95, 5,
	PTYPE_TEXT, 0, 0x8000, 68, 95, 2, 1900, 50, 250,
	PTYPE_ZBIAS, 96, 5,
	PTYPE_TEXT, 0, 0x8000, 97, 96, 5, 400, 50, 250,

	PTYPE_ZBIAS, 4, 5,
	PTYPE_SUBOBJECT, 0, SUB_NWUNIT, 73, 0, 4, 100,
	PTYPE_SUBOBJECT, 0, SUB_NWUNIT, 74, 0, 4, 64,
	PTYPE_SUBOBJECT, 0, SUB_NWUNIT, 75, 0, 4, 64,

	PTYPE_END,

};	
static Thruster ship2thruster[] = {
	{ 81, 5 | THRUST_NOANG, 30.0f },
	{ 82, 5 | THRUST_NOANG, 30.0f },
	{ 83, 2 | THRUST_NOANG, 20.0f },
	{ 84, 2 | THRUST_NOANG, 20.0f },

	{ 85, 1, 15.0f }, { 86, 4, 15.0f },
	{ 87, 1, 15.0f }, { 88, 4, 15.0f },
	{ 89, 1, 15.0f }, { 90, 4, 15.0f },

	{ 91, 0, 15.0f }, { 92, 3, 15.0f },
	{ 93, 0, 15.0f }, { 94, 3, 15.0f },
};
static Model ship2model = { 1.0f, 98, ship2vtx1, 120, 1, ship2vtx2,
	0, 0, 14, ship2thruster, ship2data, 10 };




static PlainVertex station1vtx1[] = {
	{ VTYPE_PLAIN, { -15.0f, 30.0f, 20.0f } },			// 6, front octagon
	{ VTYPE_PLAIN, { 15.0f, 30.0f, 20.0f } },
	{ VTYPE_PLAIN, { 30.0f, 15.0f, 20.0f } },
	{ VTYPE_PLAIN, { 30.0f, -15.0f, 20.0f } },
	{ VTYPE_PLAIN, { 15.0f, -30.0f, 20.0f } },
	{ VTYPE_PLAIN, { -15.0f, -30.0f, 20.0f } },

	{ VTYPE_PLAIN, { -15.0f, 30.0f, -20.0f } }, 		// 12, back octagon
	{ VTYPE_PLAIN, { 15.0f, 30.0f, -20.0f } },
	{ VTYPE_PLAIN, { 30.0f, 15.0f, -20.0f } },
	{ VTYPE_PLAIN, { 30.0f, -15.0f, -20.0f } },
	{ VTYPE_PLAIN, { 15.0f, -30.0f, -20.0f } },
	{ VTYPE_PLAIN, { -15.0f, -30.0f, -20.0f } },

	{ VTYPE_PLAIN, { -10.0f, 5.0f, 20.0f } },			// 18, inlet front
	{ VTYPE_PLAIN, { 10.0f, 5.0f, 20.0f } },
	{ VTYPE_PLAIN, { 10.0f, -5.0f, 20.0f } },
	{ VTYPE_PLAIN, { -10.0f, -5.0f, 20.0f } },

	{ VTYPE_PLAIN, { -10.0f, 5.0f, 0.0f } },			// 22, inlet rear
	{ VTYPE_PLAIN, { 10.0f, 5.0f, 0.0f } },
	{ VTYPE_PLAIN, { 10.0f, -5.0f, 0.0f } },
	{ VTYPE_PLAIN, { -10.0f, -5.0f, 0.0f } },


	{ VTYPE_PLAIN, { 30.0f, 10.0f, 10.0f } },			// 26, strut inner
	{ VTYPE_PLAIN, { 30.0f, -10.0f, 10.0f } },
	{ VTYPE_PLAIN, { 30.0f, -10.0f, -10.0f } },
	{ VTYPE_PLAIN, { 30.0f, 10.0f, -10.0f } },

	{ VTYPE_PLAIN, { 100.0f, 10.0f, 10.0f } },			// 30, strut outer
	{ VTYPE_PLAIN, { 100.0f, -10.0f, 10.0f } },
	{ VTYPE_PLAIN, { 100.0f, -10.0f, -10.0f } },
	{ VTYPE_PLAIN, { 100.0f, 10.0f, -10.0f } },

	{ VTYPE_PLAIN, { 0.0f, 0.0f, 25.0f } },			// 34, ring start, end
	{ VTYPE_PLAIN, { 0.0f, 0.0f, -25.0f } },		

/*	{ VTYPE_PLAIN, { 0.0f, 120.0f, 15.0f } },			// 34, ring top 
	{ VTYPE_PLAIN, { 0.0f, 100.0f, 15.0f } },
	{ VTYPE_PLAIN, { 0.0f, 100.0f, -15.0f } },
	{ VTYPE_PLAIN, { 0.0f, 120.0f, -15.0f } },
	
	{ VTYPE_PLAIN, { 103.9230f, 60.0f, 15.0f } },		// 38, ring top right
	{ VTYPE_PLAIN, { 86.60254f, 50.0f, 15.0f } },
	{ VTYPE_PLAIN, { 86.60254f, 50.0f, -15.0f } },
	{ VTYPE_PLAIN, { 103.9230f, 60.0f, -15.0f } },

	{ VTYPE_PLAIN, { 103.9230f, -60.0f, 15.0f } },		// 42, ring bottom right
	{ VTYPE_PLAIN, { 86.60254f, -50.0f, 15.0f } },
	{ VTYPE_PLAIN, { 86.60254f, -50.0f, -15.0f } },
	{ VTYPE_PLAIN, { 103.9230f, -60.0f, -15.0f } },

	{ VTYPE_PLAIN, { 0.0f, -120.0f, 15.0f } },			// 46, ring bottom
	{ VTYPE_PLAIN, { 0.0f, -100.0f, 15.0f } },
	{ VTYPE_PLAIN, { 0.0f, -100.0f, -15.0f } },
	{ VTYPE_PLAIN, { 0.0f, -120.0f, -15.0f } },

	{ VTYPE_DIR, { 0.8660254f, 0.5f, 0.0f } },		// 50, ring normals
	{ VTYPE_DIR, { -0.8660254ff, -0.5f, 0.0f } },
	{ VTYPE_DIR, { 0.8660254ff, -0.5f, 0.0f } },
	{ VTYPE_DIR, { -0.8660254ff, 0.5f, 0.0f } },

	{ VTYPE_PLAIN, { 120.0f, 0.0f, 0.0f } },		// 54, top tangents
	{ VTYPE_PLAIN, { 100.0f, 0.0f, 0.0f } },
	{ VTYPE_PLAIN, { -60.0f, 104.0f, 0.0f } },		// 56, top right tangents
	{ VTYPE_PLAIN, { -50.0f, 86.6f, 0.0f } },
	{ VTYPE_PLAIN, { -60.0f, -104.0f, 0.0f } },		// 58, bottom right tangents
	{ VTYPE_PLAIN, { -50.0f, -86.6f, 0.0f } },
	{ VTYPE_PLAIN, { -120.0f, 0.0f, 0.0f } },		// 60, bottom tangents
	{ VTYPE_PLAIN, { -100.0f, 0.0f, 0.0f } },

	{ VTYPE_PLAIN, { 60.0f, 103.9230f, 0.0f } },		// 61, outer midpoints
	{ VTYPE_PLAIN, { 120.0f, 0.0f, 0.0f } },
	{ VTYPE_PLAIN, { 60.0f, -103.9230f, 0.0f } },

	{ VTYPE_PLAIN, { 50.0f, 86.60254f, 0.0f } },		// 64, inner midpoints
	{ VTYPE_PLAIN, { 100.0f, 0.0f, 0.0f } },
	{ VTYPE_PLAIN, { 50.0f, -86.60254f, 0.0f } },

	{ VTYPE_DIR, { 0.5f, 0.8660254f, 0.0f } },			// 67, midpoint normals
	{ VTYPE_DIR, { -0.5f, -0.8660254f, 0.0f } },
	{ VTYPE_DIR, { 0.5f, -0.8660254f, 0.0f } },
	{ VTYPE_DIR, { -0.5f, 0.8660254f, 0.0f } },

	{ VTYPE_PLAIN, { 60.0f, 103.9230f, 15.0f } },		// 71, forward ring midpoints
	{ VTYPE_PLAIN, { 120.0f, 0.0f, 15.0f } },
	{ VTYPE_PLAIN, { 60.0f, -103.9230f, 15.0f } },

	{ VTYPE_PLAIN, { 60.0f, 103.9230f, -15.0f } },		// 74, back ring midpoints
	{ VTYPE_PLAIN, { 120.0f, 0.0f, -15.0f } },
	{ VTYPE_PLAIN, { 60.0f, -103.9230f, -15.0f } },

	{ VTYPE_PLAIN, { -60.0f, 104.0f, 0.0f } },		// 77, top right AC tangents
	{ VTYPE_PLAIN, { -50.0f, 86.6f, 0.0f } },
	{ VTYPE_PLAIN, { -60.0f, -104.0f, 0.0f } },		// 79, bottom right AC tangents
	{ VTYPE_PLAIN, { -50.0f, -86.6f, 0.0f } },
*/

};
static CompoundVertex station1vtx2[] = {
	{ VTYPE_CROSS, { 0, 1, 2, -1, -1 } },			// dummy
};
static Uint16 station1data[] = {
	PTYPE_MATFIXED, 100, 0, 100, 0, 0, 0, 100, 0, 0, 0,
	PTYPE_QUADFLAT, 7, 6, 18, 19,			// front face
	PTYPE_QUADFLAT | RFLAG_XREF, 9, 8, 7, 19,
	PTYPE_QUADFLAT | RFLAG_XREF, 10, 9, 19, 20,
	PTYPE_QUADFLAT, 11, 10, 20, 21,

	PTYPE_QUADFLAT | RFLAG_XREF, 13, 14, 15, 16,		// back face
	PTYPE_QUADFLAT, 12, 13, 16, 17,

	PTYPE_QUADFLAT, 6, 7, 13, 12,			// sides
	PTYPE_QUADFLAT | RFLAG_XREF, 7, 8, 14, 13,
	PTYPE_QUADFLAT | RFLAG_XREF, 8, 9, 15, 14,
	PTYPE_QUADFLAT | RFLAG_XREF, 9, 10, 16, 15,
	PTYPE_QUADFLAT, 10, 11, 17, 16,

	PTYPE_QUADFLAT, 19, 18, 22, 23,				// inlet
	PTYPE_QUADFLAT | RFLAG_XREF, 20, 19, 23, 24,
	PTYPE_QUADFLAT, 21, 20, 24, 25,
	PTYPE_QUADFLAT, 23, 22, 25, 24,

	PTYPE_QUADFLAT | RFLAG_XREF, 26, 27, 31, 30,
	PTYPE_QUADFLAT | RFLAG_XREF, 27, 28, 32, 31,
	PTYPE_QUADFLAT | RFLAG_XREF, 28, 29, 33, 32,
	PTYPE_QUADFLAT | RFLAG_XREF, 29, 26, 30, 33,

	PTYPE_TUBE | RFLAG_XREF, 0, 38, 34, 35, 1, 11500, 10000,

//	PTYPE_QUADFLAT | RFLAG_XREF,
//	PTYPE_SUBOBJECT, 0x8000, SUB_NOSEWHEEL, 10, 0, 4, 100,

	PTYPE_END,
};	
static Model station1model = { 1.0f, 36, station1vtx1, 100, 0, station1vtx2,
	0, 0, 0, 0, station1data, 1 };


static PlainVertex ship3vtx1[] = {
	{ VTYPE_PLAIN, { 4.0f, -5.0f, 20.0f } },		// 6, nose pair
	{ VTYPE_PLAIN, { -4.0f, -5.0f, 20.0f } },		//

	{ VTYPE_PLAIN, { 6.0f, 4.0f, 10.0f } },		// 8, mid vertices
	{ VTYPE_PLAIN, { -6.0f, 4.0f, 10.0f } },		//

	{ VTYPE_PLAIN, { 14.0f, -5.0f, 10.0f } },		// 10, front quarter
	{ VTYPE_PLAIN, { 10.0f, 5.0f, -10.0f } },		// back mid
	{ VTYPE_PLAIN, { 30.0f, -5.0f, -10.0f } },		// back end

	{ VTYPE_PLAIN, { 20.0f, 1.0f, 0.0f } },		// 13, curve midpoint
	{ VTYPE_DIR, { 2.0f, 2.0f, 1.0f } },		// norm

	{ VTYPE_PLAIN, { 15.0f, 0.0f, -10.0f } },		// 15, back midpoint
	{ VTYPE_PLAIN, { 30.0f, -5.0f, -10.0f } },		// underside midpoint

	// CW tangents
	{ VTYPE_PLAIN, { -4.0f, -1.0f, 20.0f } },		// 17, 11->10 start
	{ VTYPE_PLAIN, { 8.0f, -9.0f, 0.0f } },			// 11->10 end

	{ VTYPE_PLAIN, { 16.0f, 0.0f, 0.0f } },			// 19, 10->12 start
	{ VTYPE_PLAIN, { 0.0f, 0.0f, -20.0f } },		// 10-12 end

	{ VTYPE_PLAIN, { 0.0f, 10.0f, 0.0f } },			// 21, 12->11 start
	{ VTYPE_PLAIN, { -20.0f, 0.0f, 0.0f } },		// 12-11 end

	// CCW tangents
	{ VTYPE_PLAIN, { -8.0f, 9.0f, 0.0f } },			// 23, 10->11 start
	{ VTYPE_PLAIN, { 4.0f, 1.0f, -20.0f } },		// 10->11 end

	{ VTYPE_PLAIN, { 0.0f, 0.0f, 20.0f } },			// 25, 12-10 start
	{ VTYPE_PLAIN, { -16.0f, 0.0f, 0.0f } },		// 12->10 end

	{ VTYPE_PLAIN, { 20.0f, 0.0f, 0.0f } },			// 27, 11-12 start
	{ VTYPE_PLAIN, { 0.0f, -10.0f, 0.0f } },		// 11->12 end

	{ VTYPE_PLAIN, { -10.0f, 5.0f, -10.0f } },		// 29, back mid, left side
	{ VTYPE_PLAIN, { -14.0f, -5.0f, 10.0f } },		// 30 front quarter, left side
	{ VTYPE_PLAIN, { 10.0f, -5.0f, -10.0f } },		// back mid, right underside
	{ VTYPE_PLAIN, { -10.0f, -5.0f, -10.0f } },		// back mid, left underside

	{ VTYPE_PLAIN, { 12.0f, 0.0f, -10.0f } },		// 33, back thruster
	{ VTYPE_PLAIN, { 12.0f, 0.0f, -13.0f } },		// back thruster end

	{ VTYPE_PLAIN, { 0.0f, -5.0f, 13.0f } },		// 35, nose gear
	{ VTYPE_PLAIN, { 15.0f, -5.0f, -3.0f } },		// 36, back gear
	{ VTYPE_PLAIN, { -15.0f, -5.0f, -3.0f } },		// 37, back gear

	// thruster jets
	{ VTYPE_PLAIN, { 12.0f, 0.0f, -13.0f } },		// 38, main
	{ VTYPE_PLAIN, { 15.0f, -3.0f, 9.0f } },		// retro

	{ VTYPE_PLAIN, { 30.0f, -4.0f, -9.0f } },		// 40, corner clusters
	{ VTYPE_PLAIN, { 29.0f, -5.5f, -9.0f } },		// down
	{ VTYPE_PLAIN, { 29.0f, -4.0f, -9.0f } },		// up
	{ VTYPE_PLAIN, { 10.0f, 0.0f, 11.0f } },		// lateral front

};
static CompoundVertex ship3vtx2[] = {
	{ VTYPE_NORM, { 15, 8, 10, -1, -1 } },		// 100, mid curve norm
	{ VTYPE_NORM, { 9, 8, 11, -1, -1 } },		// 101, top curve norm
};
static Uint16 ship3data[] = {
	PTYPE_MATFIXED, 100, 0, 100, 0, 0, 0, 100, 0, 0, 0,
	PTYPE_QUADFLAT, 6, 8, 9, 7,
	PTYPE_QUADFLAT, 9, 8, 11, 29,
	PTYPE_TRIFLAT | RFLAG_XREF, 8, 6, 10,
	PTYPE_QUADFLAT, 10, 6, 7, 30,
	PTYPE_QUADFLAT, 32, 31, 10, 30,
	PTYPE_QUADFLAT, 29, 11, 31, 32,

	PTYPE_COMPFLAT | RFLAG_XREF, 0, 5, 8, 100, 8, 100,		// mid curve
		COMP_LINE, 10, 100,
		COMP_HERMITE, 11, 100, 23, 24,
		COMP_LINE, 8, 100,
		COMP_END,
	PTYPE_COMPSMOOTH | RFLAG_XREF, 1, 5, 13, 14, 11, 101,		// top curve
		COMP_HERMITE, 10, 2, 17, 18,
		COMP_HERMITE, 12, 0, 19, 20,
		COMP_HERMITE, 11, 101, 21, 22,
		COMP_STEPS, 10, 
		COMP_END,
	PTYPE_COMPFLAT | RFLAG_XREF, 2, 5, 15, 5, 11, 5,		// back curve
		COMP_HERMITE, 12, 5, 27, 28,
		COMP_LINE, 31, 5,
		COMP_LINE, 11, 5,
		COMP_END,
	PTYPE_COMPFLAT | RFLAG_XREF, 3, 5, 16, 4, 12, 4,		// underside curve
		COMP_HERMITE, 10, 4, 25, 26,
		COMP_LINE, 31, 4,
		COMP_LINE, 12, 4,
		COMP_END,

	PTYPE_ZBIAS, 5, 5,
	PTYPE_MATFIXED, 30, 30, 30, 30, 30, 30, 200, 0, 0, 0,
	PTYPE_TUBE | RFLAG_XREF, 4, 12, 33, 34, 1, 300, 250,
	PTYPE_MATANIM, AFUNC_THRUSTPULSE,
		0, 0, 0, 0, 0, 0, 100, 50, 50, 100,
		0, 0, 0, 0, 0, 0, 100, 0, 0, 50,
	PTYPE_CIRCLE | RFLAG_XREF, 5, 12, 33, 5, 1, 250,

//	PTYPE_ZBIAS, 120, 5,
//	PTYPE_MATFIXED, 30, 30, 30, 0, 0, 0, 100, 0, 0, 0,
//	PTYPE_QUADFLAT | RFLAG_XREF, 77, 78, 80, 79,

/*	PTYPE_MATFIXED, 20, 20, 20, 0, 0, 0, 100, 0, 0, 0,
	PTYPE_ZBIAS, 95, 5,
	PTYPE_TEXT, 0, 0x8000, 68, 95, 2, 1900, 50, 250,
	PTYPE_ZBIAS, 96, 5,
	PTYPE_TEXT, 0, 0x8000, 97, 96, 5, 400, 50, 250,
*/
	PTYPE_ZBIAS, 4, 5,
	PTYPE_SUBOBJECT, 0, SUB_NWUNIT, 35, 0, 4, 100,
	PTYPE_SUBOBJECT, 0, SUB_MWUNIT, 36, 0, 4, 100,
	PTYPE_SUBOBJECT, 0, SUB_MWUNIT, 37, 0, 4, 100,

	PTYPE_END,
};
static Thruster ship3thruster[] = {
	{ 38, 5 | THRUST_NOANG | THRUST_XREF, 30.0f },
	{ 39, 2 | THRUST_NOANG | THRUST_XREF, 20.0f },
	{ 40, 0 | THRUST_XREF, 15.0f },
	{ 41, 4 | THRUST_XREF, 15.0f },
	{ 42, 1 | THRUST_XREF, 15.0f },
	{ 43, 0 | THRUST_XREF, 15.0f },
};
static Model ship3model = { 1.0f, 44, ship3vtx1, 100, 2, ship3vtx2,
	0, 0, 6, ship3thruster, ship3data, 6 };




static PlainVertex ship4vtx1[] = {
	{ VTYPE_PLAIN, { -4.0f, -3.0f, 35.0f } },			// 6, nose vertices
	{ VTYPE_PLAIN, { 4.0f, -3.0f, 35.0f } },
	{ VTYPE_PLAIN, { 1.0f, -7.0f, 32.0f } },		
	{ VTYPE_PLAIN, { -1.0f, -7.0f, 32.0f } },

	{ VTYPE_PLAIN, { -6.0f, 8.0f, 20.0f } },			// 10, nose section back
	{ VTYPE_PLAIN, { 6.0f, 8.0f, 20.0f } },
	{ VTYPE_PLAIN, { 10.0f, 4.0f, 20.0f } },			
	{ VTYPE_PLAIN, { 10.0f, -6.0f, 20.0f } },			
	{ VTYPE_PLAIN, { 8.0f, -8.0f, 20.0f } },
	{ VTYPE_PLAIN, { -8.0f, -8.0f, 20.0f } },

	{ VTYPE_PLAIN, { -7.5f, 6.0f, 20.0f } },			// 16,
	{ VTYPE_PLAIN, { 7.5f, 6.0f, 20.0f } },			
	{ VTYPE_PLAIN, { 8.0f, -7.0f, 20.0f } },
	{ VTYPE_PLAIN, { -8.0f, -7.0f, 20.0f } },

	{ VTYPE_PLAIN, { -7.5f, 6.0f, 16.0f } },			
	{ VTYPE_PLAIN, { 7.5f, 6.0f, 16.0f } },			
	{ VTYPE_PLAIN, { 8.0f, -7.0f, 16.0f } },
	{ VTYPE_PLAIN, { -8.0f, -7.0f, 16.0f } },

	{ VTYPE_PLAIN, { -6.0f, 8.0f, 16.0f } },			// 24, mid section front
	{ VTYPE_PLAIN, { 6.0f, 8.0f, 16.0f } },
	{ VTYPE_PLAIN, { 10.0f, 4.0f, 16.0f } },			
	{ VTYPE_PLAIN, { 10.0f, -6.0f, 16.0f } },			
	{ VTYPE_PLAIN, { 8.0f, -8.0f, 16.0f } },
	{ VTYPE_PLAIN, { -8.0f, -8.0f, 16.0f } },

	{ VTYPE_PLAIN, { -6.0f, 8.0f, -4.0f } },			// 30, mid section back
	{ VTYPE_PLAIN, { 6.0f, 8.0f, -4.0f } },
	{ VTYPE_PLAIN, { 10.0f, 4.0f, -4.0f } },			
	{ VTYPE_PLAIN, { 10.0f, -6.0f, -4.0f } },			
	{ VTYPE_PLAIN, { 8.0f, -8.0f, -4.0f } },
	{ VTYPE_PLAIN, { -8.0f, -8.0f, -4.0f } },

	{ VTYPE_PLAIN, { -7.5f, 6.0f, -4.0f } },			// 36,
	{ VTYPE_PLAIN, { 7.5f, 6.0f, -4.0f } },			
	{ VTYPE_PLAIN, { 8.0f, -7.0f, -4.0f } },
	{ VTYPE_PLAIN, { -8.0f, -7.0f, -4.0f } },

	{ VTYPE_PLAIN, { -7.5f, 6.0f, -8.0f } },			
	{ VTYPE_PLAIN, { 7.5f, 6.0f, -8.0f } },			
	{ VTYPE_PLAIN, { 8.0f, -7.0f, -8.0f } },
	{ VTYPE_PLAIN, { -8.0f, -7.0f, -8.0f } },

	{ VTYPE_PLAIN, { -6.0f, 8.0f, -8.0f } },			// 44, mid section front
	{ VTYPE_PLAIN, { 6.0f, 8.0f, -8.0f } },
	{ VTYPE_PLAIN, { 10.0f, 4.0f, -8.0f } },			
	{ VTYPE_PLAIN, { 10.0f, -6.0f, -8.0f } },			
	{ VTYPE_PLAIN, { 8.0f, -8.0f, -8.0f } },
	{ VTYPE_PLAIN, { -8.0f, -8.0f, -8.0f } },

	{ VTYPE_PLAIN, { -6.0f, 8.0f, -26.0f } },			// 50, mid section back
	{ VTYPE_PLAIN, { 6.0f, 8.0f, -26.0f } },
	{ VTYPE_PLAIN, { 10.0f, 4.0f, -26.0f } },			
	{ VTYPE_PLAIN, { 10.0f, -6.0f, -26.0f } },			
	{ VTYPE_PLAIN, { 8.0f, -8.0f, -26.0f } },
	{ VTYPE_PLAIN, { -8.0f, -8.0f, -26.0f } },

};
static CompoundVertex ship4vtx2[] = {
	{ VTYPE_CROSS, { 0, 1, 2, -1, -1 } },			// dummy
};
static Uint16 ship4data[] = {
	PTYPE_MATFIXED, 100, 0, 100, 0, 0, 0, 100, 0, 0, 0,

	PTYPE_QUADFLAT, 6, 7, 11, 10, 					// front section
	PTYPE_TRIFLAT | RFLAG_XREF, 7, 12, 11,
	PTYPE_TRIFLAT | RFLAG_XREF, 7, 13, 12,
	PTYPE_TRIFLAT | RFLAG_XREF, 7, 14, 13, 
	PTYPE_TRIFLAT | RFLAG_XREF, 7, 9, 14,
	PTYPE_QUADFLAT, 7, 6, 9, 8, 
	PTYPE_QUADFLAT, 8, 9, 15, 14,

	PTYPE_QUADFLAT, 10, 11, 14, 15, 
	PTYPE_QUADFLAT | RFLAG_XREF, 11, 12, 13, 14, 

	PTYPE_QUADFLAT, 29, 28, 25, 24, 				// mid section
	PTYPE_QUADFLAT | RFLAG_XREF, 28, 27, 26, 25,

	PTYPE_QUADFLAT, 24, 25, 31, 30, 
	PTYPE_QUADFLAT | RFLAG_XREF, 25, 26, 32, 31, 
	PTYPE_QUADFLAT | RFLAG_XREF, 26, 27, 33, 32, 
	PTYPE_QUADFLAT | RFLAG_XREF, 27, 28, 34, 33,
	PTYPE_QUADFLAT, 28, 29, 35, 34,

	PTYPE_QUADFLAT, 30, 31, 34, 35, 
	PTYPE_QUADFLAT | RFLAG_XREF, 31, 32, 33, 34,

	PTYPE_QUADFLAT, 49, 48, 45, 44, 				// rear section
	PTYPE_QUADFLAT | RFLAG_XREF, 48, 47, 46, 45,

	PTYPE_QUADFLAT, 44, 45, 51, 50, 
	PTYPE_QUADFLAT | RFLAG_XREF, 45, 46, 52, 51, 
	PTYPE_QUADFLAT | RFLAG_XREF, 46, 47, 53, 52, 
	PTYPE_QUADFLAT | RFLAG_XREF, 47, 48, 54, 53,
	PTYPE_QUADFLAT, 48, 49, 55, 54,

	PTYPE_QUADFLAT, 50, 51, 54, 55, 
	PTYPE_QUADFLAT | RFLAG_XREF, 51, 52, 53, 54,

	PTYPE_MATFIXED, 30, 30, 30, 10, 10, 10, 100, 0, 0, 0,

	PTYPE_QUADFLAT, 16, 17, 21, 20,								// front join
	PTYPE_QUADFLAT | RFLAG_XREF, 17, 18, 22, 21, 
	PTYPE_QUADFLAT, 18, 19, 23, 22,

	PTYPE_QUADFLAT, 36, 37, 41, 40,								// rear join
	PTYPE_QUADFLAT | RFLAG_XREF, 37, 38, 42, 41, 
	PTYPE_QUADFLAT, 38, 39, 43, 42,

//	PTYPE_SUBOBJECT, 0x8000, SUB_NOSEWHEEL, 10, 0, 4, 100,
	PTYPE_END,
};	
static Model ship4model = { 1.0f, 56, ship4vtx1, 100, 0, ship4vtx2,
	0, 0, 0, 0, ship4data, 0 };



Model *ppModel[] = {
	&ship4model,
	&tetramodel,
	&circlemodel,
	&nosewheelmodel,
	&wingmodel,
	&nacellemodel,
	&nwunitmodel,
	&mainwheelmodel,
	&mwunitmodel,
	&cylmodel,
	&ship2model,
	&shipmodel,
	&station1model,
	&ship3model,
	0,
};

