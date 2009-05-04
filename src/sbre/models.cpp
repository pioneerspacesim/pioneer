#include "sbre_int.h"
#include "sbre_models.h"


enum AxisIndex {
	A_X = 0, A_Y, A_Z, A_NX, A_NY, A_NZ,
};

static CompoundVertex dummyvtx2[] = {
	{ VTYPE_CROSS, { 0, 1, 2, -1, -1 } },			// dummy
};

static PlainVertex tetravtx1[] = {
	{ VTYPE_PLAIN, { 0.0f, 50.0f, 0.0f } },			// 6
	{ VTYPE_PLAIN, { 50.0f, -30.0f, -30.0f } },
	{ VTYPE_PLAIN, { -50.0f, -30.0f, -30.0f } },
	{ VTYPE_PLAIN, { 0.0f, -30.0f, 50.0f } },
	{ VTYPE_PLAIN, { 0.0f, -30.0f, 0.0f } },		// 10, wheel base
};
static CompoundVertex tetravtx2[] = {
	{ VTYPE_CROSS, { 0, 1, 2, -1, -1 } },			// dummy
};
static uint16 tetradata[] = {
	PTYPE_MATFIXED, 100, 0, 100, 0, 0, 0, 100, 0, 0, 0,
	PTYPE_TRIFLAT, 6, 7, 8,
	PTYPE_TRIFLAT, 6, 8, 9,
	PTYPE_TRIFLAT, 6, 9, 7,
	PTYPE_TRIFLAT, 9, 8, 7,
	PTYPE_SUBOBJECT, 0x8000, SUB_NOSEWHEEL, 10, 3, 4, 100,
	PTYPE_END,
};	
Model tetramodel = { 1.0f, 66.0f, 11, tetravtx1, 20, 0, tetravtx2, 0,
	{ { 0, tetradata, 0, 0, 0 } } };


static PlainVertex circlevtx1[] = {
	{ VTYPE_PLAIN, { 0.0f, 0.0f, 0.0f } },
};
static CompoundVertex circlevtx2[] = {
	{ VTYPE_NORM, { 6, 7, 8, -1, -1 } },		// dummy
};
static uint16 circledata[] = {
	PTYPE_MATANIM, AFUNC_THRUSTPULSE,
		0, 0, 0, 0, 0, 0, 100, 50, 50, 100,
		0, 0, 0, 0, 0, 0, 100, 0, 0, 50,
	PTYPE_CIRCLE, 0, 12, 6, 2, 1, 2000,
	PTYPE_END,
};	
Model circlemodel = { 1.0f, 20.0f, 7, circlevtx1, 20, 0, circlevtx2, 1,
	{ { 0, circledata, 0, 0, 0 } } };


static PlainVertex cylvtx1[] = {
	{ VTYPE_PLAIN, { 100.0f, 20.0f, 0.0f } },
	{ VTYPE_PLAIN, { -100.0f, -10.0f, 0.0f } },
	{ VTYPE_PLAIN, { 0.0f, 0.0f, 0.0f } },
};
static CompoundVertex cylvtx2[] = {
	{ VTYPE_NORM, { 6, 7, 8, -1, -1 } },
};
static uint16 cyldata[] = {
	PTYPE_MATFIXED, 100, 0, 100, 0, 0, 0, 100, 0, 0, 0,
//	PTYPE_CYLINDER, 0, 8, 6, 7, 20, 2000,
	PTYPE_TUBE, 0, 8, 6, 7, 20, 2000, 1000,
	PTYPE_END,
};	
Model cylmodel = { 1.0f, 120.0f, 9, cylvtx1, 20, 1, cylvtx2, 1,
	{ { 0, cyldata, 0, 0, 0 } } };


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
	{ VTYPE_CROSS, { 0, 22, -1, -1, -1 } },					// gear z axis
};
static uint16 nwunitdata[] = {
	PTYPE_MATFIXED, 20, 20, 20, 0, 0, 0, 100, 0, 0, 0,
	PTYPE_QUADFLAT | RFLAG_XREF, 8, 6, 7, 9,	// flap internal

	PTYPE_MATVAR, 2,
	PTYPE_QUADFLAT | RFLAG_XREF, 6, 20, 21, 7,	// flaps
	PTYPE_QUADFLAT | RFLAG_XREF, 6, 7, 21, 20,

//	PTYPE_CYLINDER, 3, 8, 6, 7, 0, 1000,

	PTYPE_SUBOBJECT, 0x8000, SUB_NOSEWHEEL, 14, 22, 23, 100,

	PTYPE_END,
};	
Model nwunitmodel = { 1.0f, 7.0f, 15, nwunitvtx1, 20, 4, nwunitvtx2, 0, 
	{ { 0, nwunitdata, 0, 0, 0 } } };


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
static uint16 nosewheeldata[] = {
	PTYPE_MATFIXED, 50, 50, 50, 50, 50, 50, 200, 0, 0, 0,
	PTYPE_CYLINDER, 0, 8, 6, 8, 2, 40,
	PTYPE_CYLINDER, 1, 8, 7, 8, 2, 50,
	PTYPE_MATFIXED, 30, 30, 30, 0, 0, 0, 100, 0, 0, 0,
	PTYPE_CYLINDER | RFLAG_XREF, 2, 8, 9, 10, 2, 100,
	PTYPE_END,
};	
Model nosewheelmodel = { 1.0f, 7.0f, 11, nosewheelvtx1, 20, 0, nosewheelvtx2, 3,
	{ { 0, nosewheeldata, 0, 0, 0 } } };


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
	{ VTYPE_CROSS, { 0, 22, -1, -1, -1 } },					// gear z axis
};
static uint16 mwunitdata[] = {
	PTYPE_MATFIXED, 20, 20, 20, 0, 0, 0, 100, 0, 0, 0,
	PTYPE_QUADFLAT | RFLAG_XREF, 8, 6, 7, 9,	// flap internal

	PTYPE_MATVAR, 2,
	PTYPE_QUADFLAT | RFLAG_XREF, 6, 20, 21, 7,	// flaps
	PTYPE_QUADFLAT | RFLAG_XREF, 6, 7, 21, 20,

//	PTYPE_CYLINDER, 3, 8, 6, 7, 0, 1000,

	PTYPE_SUBOBJECT, 0x8000, SUB_MAINWHEEL, 14, 22, 23, 100,

	PTYPE_END,
};	
Model mwunitmodel = { 1.0f, 8.0f, 15, mwunitvtx1, 20, 4, mwunitvtx2, 0,
	{ { 0, mwunitdata, 0, 0, 0 } } };


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
static uint16 mainwheeldata[] = {

	PTYPE_MATFIXED, 50, 50, 50, 100, 100, 100, 200, 0, 0, 0,
	PTYPE_CYLINDER, 0, 8, 6, 8, 2, 40,
	PTYPE_CYLINDER, 1, 8, 7, 8, 2, 50,
	PTYPE_CYLINDER, 4, 4, 13, 14, 0, 50,
	PTYPE_MATFIXED, 30, 30, 30, 0, 0, 0, 100, 0, 0, 0,
	PTYPE_CYLINDER | RFLAG_XREF, 2, 8, 9, 10, 2, 100,
	PTYPE_CYLINDER | RFLAG_XREF, 3, 8, 11, 12, 2, 100,
	PTYPE_END,
};	
Model mainwheelmodel = { 1.0f, 8.0f, 15, mainwheelvtx1, 20, 0, mainwheelvtx2, 5,
	{ { 0, mainwheeldata, 0, 0, 0 } } };


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
static uint16 nacelledata[] = {
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
Model nacellemodel = { 1.0f, 30.0f, 16, nacellevtx1, 20, 0, nacellevtx2, 2, 
	{ { 0, nacelledata, 0, 0, 0 } } };


// do wings as subobjects

static PlainVertex ship1vtx1[] = {
	{ VTYPE_PLAIN, { -5.0f, 10.0f, -30.0f } },		// 6, top four body verts
	{ VTYPE_PLAIN, { 5.0f, 10.0f, -30.0f } },
	{ VTYPE_PLAIN, { -5.0f, 10.0f, 30.0f } },
	{ VTYPE_PLAIN, { 5.0f, 10.0f, 30.0f } },

	{ VTYPE_PLAIN, { -11.16025f, -0.6698729f, -25.0f } },		// 10, right four body verts
	{ VTYPE_PLAIN, { -6.160254f, -9.330127f, -35.0f } },
	{ VTYPE_PLAIN, { -11.16025f, -0.6698729f, 35.0f } },
	{ VTYPE_PLAIN, { -6.160254f, -9.330127f, 30.0f } },

	{ VTYPE_PLAIN, { 11.16025f, -0.6698729f, -25.0f } },		// 14, left four body verts
	{ VTYPE_PLAIN, { 6.160254f, -9.330127f, -35.0f } },
	{ VTYPE_PLAIN, { 11.16025f, -0.6698729f, 35.0f } },
	{ VTYPE_PLAIN, { 6.160254f, -9.330127f, 30.0f } },

	{ VTYPE_PLAIN, { -5.0f, -0.6698729f, -60.0f } },		// 18, front two verts
	{ VTYPE_PLAIN, { 5.0f, -0.6698729f, -60.0f } },	

	{ VTYPE_PLAIN, { 0.0f, 10.0f, 0.0f } },				// 20, top wing
	{ VTYPE_PLAIN, { -1.0f, 0.0f, 0.0f } },	
	{ VTYPE_PLAIN, { 0.0f, 1.0f, 0.0f } },	

	{ VTYPE_PLAIN, { -8.660254f, -5.0f, 0.0f } },				// 23, right wing
	{ VTYPE_PLAIN, { 0.5f, -0.8660254f, 0.0f } },
	{ VTYPE_PLAIN, { -0.8660254f, -0.5f, 0.0f } },

	{ VTYPE_PLAIN, { 8.660254f, -5.0f, 0.0f } },				// 26, left wing
	{ VTYPE_PLAIN, { 0.5f, 0.8660254f, 0.0f } },
	{ VTYPE_PLAIN, { 0.8660254f, -0.5f, 0.0f } },

	{ VTYPE_PLAIN, { -0.0f, 0.0f, 40.0f } },					// 29, main thruster
	{ VTYPE_PLAIN, { -11.0f, 0.0f, -35.0f } },					// 30, retro
	{ VTYPE_PLAIN, { 11.0f, 0.0f, -35.0f } },	
	{ VTYPE_PLAIN, { -9.0f, 5.0f, -30.0f } },						// 32, right
	{ VTYPE_PLAIN, { -12.0f, -5.0f, 30.0f } },
	{ VTYPE_PLAIN, { 12.0f, -5.0f, -30.0f } },					// 34, left
	{ VTYPE_PLAIN, { 9.0f, 5.0f, 30.0f } },
	{ VTYPE_PLAIN, { 0.0f, 12.0f, -30.0f } },					// 36, top
	{ VTYPE_PLAIN, { 0.0f, 12.0f, 30.0f } },
	{ VTYPE_PLAIN, { 0.0f, -12.0f, -30.0f } },					// 38, bottom
	{ VTYPE_PLAIN, { 0.0f, -12.0f, 30.0f } },

	{ VTYPE_PLAIN, { 0.0f, -9.330127f, -30.0f } },				// 40, nosewheel
	{ VTYPE_PLAIN, { 0.0f, -9.330127f, 13.0f } },				// 41, mainwheel

};
static CompoundVertex ship1vtx2[] = {
	{ VTYPE_ANIMLIN, { 25, 3, -1, -1, 0 } },			// 50, right wing yaxis	
	{ VTYPE_CROSS, { 50, 5, -1, -1, 0 } },				// right wing xaxis	

	{ VTYPE_ANIMLIN, { 28, 0, -1, -1, 0 } },			// 52, left wing yaxis	
	{ VTYPE_CROSS, { 52, 5, -1, -1, -1 } },				// right wing xaxis	

	{ VTYPE_NORM, { 16, 14, 7, -1, -1 } },				// 54, left text normal
	{ VTYPE_NORM, { 12, 8, 6, -1, -1 } },				// 55, right text normal

};
static uint16 ship1data[] = {
	PTYPE_MATVAR, 0,
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

	PTYPE_SUBOBJECT, 0x8000, SUB_WING1, 20, 22, 5, 100,
	PTYPE_SUBOBJECT, 0x8000, SUB_WING1, 23, 50, 5, 100,
	PTYPE_SUBOBJECT, 0x8000, SUB_WING1, 26, 52, 5, 100,

	PTYPE_MATFIXED, 20, 20, 20, 0, 0, 0, 100, 0, 0, 0,
	PTYPE_ZBIAS, 16, 54, 5,
	PTYPE_TEXT, 0, 0x8000, 16, 54, 2, 700, 200, 1000,
	PTYPE_ZBIAS, 10, 55, 5,
	PTYPE_TEXT, 0, 0x8000, 10, 55, 5, 100, 200, 1000,

	PTYPE_ZBIAS, 40, 4, 5,
	PTYPE_SUBOBJECT, 0, SUB_NWUNIT, 40, 4, 2, 200,
	PTYPE_SUBOBJECT, 0, SUB_MWUNIT, 41, 4, 2, 200,

//	PTYPE_TEXT, -1, -1, 12, 0, 1, 5000,
	PTYPE_ZBIAS, 0x8000, 0, 0,

	PTYPE_END,
};
static Thruster ship1thruster[] = {
	{ 29, 2 | THRUST_NOANG, 50.0f },
	{ 30, 5 | THRUST_NOANG, 35.0f },		// retros
	{ 31, 5 | THRUST_NOANG, 35.0f },
	{ 32, 3, 25.0f }, { 33, 3, 25.0f },	// right
	{ 34, 0, 25.0f }, { 35, 0, 25.0f },	// left
	{ 36, 1, 25.0f }, { 37, 1, 25.0f },	// top
	{ 38, 4, 25.0f }, { 39, 4, 25.0f },	// bottom
};
Model ship1model = { 1.0f, 40.0f, 42, ship1vtx1, 50, 6, ship1vtx2, 0,
	{ { 0, ship1data, 0, 11, ship1thruster } } };


static PlainVertex wing1vtx1[] = {
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
static CompoundVertex wing1vtx2[] = {
	{ VTYPE_CROSS, { 19, 14, -1, -1, -1 } },		// 30, norm 0
	{ VTYPE_CROSS, { 15, 16, -1, -1, -1 } },		// norm 1
	{ VTYPE_CROSS, { 16, 17, -1, -1, -1 } },	// norm 3
	{ VTYPE_CROSS, { 18, 19, -1, -1, -1 } },	// norm 2
};
static uint16 wing1data[] = {
	PTYPE_MATVAR, 0,
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
Model wing1model = { 25.0f, 2.0f, 24, wing1vtx1, 30, 4, wing1vtx2, 2, 
	{ { 0, wing1data, 0, 0, 0 } } };




static PlainVertex ship2vtx1[] = {
	{ VTYPE_PLAIN, { 0.0f, 0.0f, -35.0f } },			// 6, nose point
	{ VTYPE_DIR, { 0.0f, 1.0f, -0.2f } },			// nose normal
	{ VTYPE_PLAIN, { -6.0f, 0.0f, -18.0f } },			// 8, r edge forward mid
	{ VTYPE_DIR, { -0.2f, 1.0f, -0.1f } },			// norm
	{ VTYPE_PLAIN, { -12.0f, 0.0f, 2.0f } },		// 10, r edge back mid
	{ VTYPE_DIR, { -0.2f, 1.0f, -0.1f } },			// norm
	{ VTYPE_PLAIN, { -7.5f, 0.0f, 25.0f } },		// 12, r edge back
	{ VTYPE_DIR, { 0.0f, 1.0f, 0.2f } },			// norm

	{ VTYPE_PLAIN, { 0.0f, 3.0f, -15.0f } },			// 14, cockpit front
	{ VTYPE_DIR, { 0.0f, 1.0f, 0.08f } },		// norm
	{ VTYPE_PLAIN, { -1.5f, 3.0f, -13.5f } },			// 16, cockpit right
	{ VTYPE_DIR, { 0.0f, 1.0f, 0.08f } },		// norm
	{ VTYPE_PLAIN, { 0.0f, 3.0f, -10.0f } },			// 18, cockpit back
	{ VTYPE_DIR, { 0.0f, 1.0f, 0.08f } },		// norm
	{ VTYPE_PLAIN, { 1.5f, 3.0f, -13.5f } },		// 20, cockpit left
	{ VTYPE_DIR, { 0.0f, 1.0f, 0.08f } },		// norm

	{ VTYPE_PLAIN, { -6.0f, 3.0f, 5.0f } },			// 22, inner right
	{ VTYPE_DIR, { -0.2f, 1.0f, -0.2f } },			// norm
	{ VTYPE_PLAIN, { 0.0f, 3.0f, 5.0f } },			// 24, inner mid
	{ VTYPE_DIR, { 0.2f, 1.0f, -0.2f } },		// norm

	{ VTYPE_PLAIN, { -2.0f, 2.0f, -23.0f } },			// 26, fwd midpoint
	{ VTYPE_DIR, { 0.0f, 1.0f, -0.1f } },		// norm
	{ VTYPE_PLAIN, { -5.0f, 2.5f, -5.0f } },			// 28, right midpoint
	{ VTYPE_DIR, { -0.08f, 1.0f, -0.04f } },		// norm
	{ VTYPE_PLAIN, { -7.0f, 2.0f, 14.0f } },		// 30, rear right midpoint
	{ VTYPE_DIR, { -0.04f, 1.0f, 0.1f } },		// norm

	{ VTYPE_PLAIN, { -3.0f, 3.0f, -5.0f } },			// 32, central midpoint
	{ VTYPE_PLAIN, { 0.0f, 4.0f, -12.5f } },			// 33, cockpit midpoint
	{ VTYPE_PLAIN, { -3.75f, 4.0f, 20.0f } },		// 34, nacelle midpoint

	{ VTYPE_PLAIN, { -7.5f, 0.0f, 30.0f } },		// 35, nacelle outer
	{ VTYPE_PLAIN, { 0.0f, 0.0f, 30.0f } },		// 36, nacelle inner

	// edge tangents
	{ VTYPE_PLAIN, { 6.0f, 4.0f, 3.0f } },		// 37, edge to mid
	{ VTYPE_PLAIN, { 6.0f, 0.0f, 3.0f } },		//
	{ VTYPE_PLAIN, { 0.0f, 4.0f, -20.0f } },			// 39, rear to mid
	{ VTYPE_PLAIN, { 2.5f, 0.0f, -20.0f } },		//

	{ VTYPE_PLAIN, { 0.0f, 0.0f, -20.0f } },			// 41, mid to nose
	{ VTYPE_PLAIN, { 0.0f, -4.0f, -20.0f } },
	{ VTYPE_PLAIN, { -6.0f, 0.0f, -3.0f } },			// 43, mid to edge
	{ VTYPE_PLAIN, { -6.0f, -4.0f, -3.0f } },
	{ VTYPE_PLAIN, { -2.5f, 0.0f, 20.0f } },			// 45, mid to rear
	{ VTYPE_PLAIN, { 0.0f, -4.0f, 20.0f } },

	{ VTYPE_PLAIN, { -1.5f, 0.0f, 0.0f } },			// 47, cockpit CW tangents
	{ VTYPE_PLAIN, { 1.5f, 0.0f, 0.0f } },
	{ VTYPE_PLAIN, { 0.0f, 0.0f, -1.5f } },
	{ VTYPE_PLAIN, { 0.0f, 0.0f, 1.5f } },
	{ VTYPE_PLAIN, { 0.0f, 0.0f, -3.5f } },			// 51
	{ VTYPE_PLAIN, { 0.0f, 0.0f, 3.5f } },

	{ VTYPE_PLAIN, { -10.0f, 0.0f, 20.0f } },			// 53, rear edge tangents
	{ VTYPE_PLAIN, { 10.0f, 0.0f, 0.0f } },
	{ VTYPE_PLAIN, { 10.0f, 0.0f, -20.0f } },			// 55, CCW
	{ VTYPE_PLAIN, { -10.0f, 0.0f, 0.0f } },

	{ VTYPE_PLAIN, { 0.0f, 5.0f, 0.0f } },			// 57, nacelle tangents
	{ VTYPE_PLAIN, { 0.0f, -5.0f, 0.0f } },
	{ VTYPE_PLAIN, { 0.0f, 0.0f, -12.0f } },
	{ VTYPE_PLAIN, { 0.0f, 0.0f, 12.0f } },

	{ VTYPE_PLAIN, { -3.75f, 4.0f, 30.0f } },			// 61, nacelle rear midpoint
	{ VTYPE_PLAIN, { -4.0f, 0.0f, 0.0f } },			// and tangents
	{ VTYPE_PLAIN, { 4.0f, 0.0f, 0.0f } },			// 

	// underside points
	{ VTYPE_PLAIN, { -5.0f, 0.0f, -5.0f } },			// 64, upper outer vent
	{ VTYPE_PLAIN, { 0.0f, 0.0f, -5.0f } },			// 65, upper inner vent
	{ VTYPE_PLAIN, { -5.0f, -2.0f, -3.0f } },			// 66, lower outer vent
	{ VTYPE_PLAIN, { 0.0f, -2.0f, -3.0f } },			// 67, lower inner vent
	{ VTYPE_PLAIN, { -5.0f, -2.0f, 30.0f } },		// 68, nacelle outer underside
	{ VTYPE_PLAIN, { 0.0f, -2.0f, 30.0f } },		// 69, nacelle inner underside
	{ VTYPE_PLAIN, { -13.0f, 0.0f, 14.0f } },		// 70, rear underside centre
	{ VTYPE_PLAIN, { -7.5f, 0.0f, -3.0f } },			// 71, vent outer edge 

	{ VTYPE_PLAIN, { -3.75f, 0.7f, 30.0f } },			// 72, engine midpoint

	{ VTYPE_PLAIN, { 0.0f, 0.0f, -15.0f } },			// 73, nose gear pos
	{ VTYPE_PLAIN, { -3.75f, -2.0f, 15.0f } },			// 74, rear right gear
	{ VTYPE_PLAIN, { 3.75f, -2.0f, 15.0f } },			// 75, rear left gear

	{ VTYPE_PLAIN, { -3.75f, 0.7f, 32.0f } },			// 76, engine end

	{ VTYPE_PLAIN, { -4.5f, -0.3f, -4.7f } },			// 77, retro vent
	{ VTYPE_PLAIN, { -0.5f, -0.3f, -4.7f } },			// 
	{ VTYPE_PLAIN, { -4.5f, -1.7f, -3.3f } },			// 
	{ VTYPE_PLAIN, { -0.5f, -1.7f, -3.3f } },			// 

	// main & retro thrusters
	{ VTYPE_PLAIN, { -3.75f, 0.7f, 32.0f } },			// 81
	{ VTYPE_PLAIN, { 3.75f, 0.7f, 32.0f } },			
	{ VTYPE_PLAIN, { -2.5f, -1.0f, -5.0f } },
	{ VTYPE_PLAIN, { 2.5f, -1.0f, -5.0f } },

	// vertical thrusters
	{ VTYPE_PLAIN, { -9.0f, 1.5f, 10.0f } },			// 85
	{ VTYPE_PLAIN, { -9.0f, -0.5f, 10.0f } },
	{ VTYPE_PLAIN, { 9.0f, 1.5f, 10.0f } },			// 
	{ VTYPE_PLAIN, { 9.0f, -0.5f, 10.0f } },			// 
	{ VTYPE_PLAIN, { 0.0f, 3.5f, -8.0f } },			// 
	{ VTYPE_PLAIN, { 0.0f, -0.5f, -25.0f } },

	// horizontal thrusters
	{ VTYPE_PLAIN, { -8.0f, 0.0f, 28.0f } },			// 91
	{ VTYPE_PLAIN, { 8.0f, 0.0f, 28.0f } },
	{ VTYPE_PLAIN, { -3.5f, 0.0f, -25.0f } },
	{ VTYPE_PLAIN, { 3.5f, 0.0f, -25.0f } },

	// text norms
	{ VTYPE_DIR, { -2.0f, -2.5f, 0.0f } },			// 95
	{ VTYPE_DIR, { 2.0f, -2.5f, 0.0f } },
	{ VTYPE_PLAIN, { 5.0f, -2.0f, 13.5f } },		// 97, 98, reg number text points
	{ VTYPE_PLAIN, { -5.0f, -2.0f, 13.5f } },
	
};
static CompoundVertex ship2vtx2[] = {
	{ VTYPE_NORM, { 77, 78, 80, -1, -1 } },			// 120, retro norm
};
static uint16 ship2data[] = {
	PTYPE_MATVAR, 0,
	PTYPE_COMPSMOOTH | RFLAG_XREF, 0x8000, 5, 26, 27, 6, 7,		// front edge
		COMP_HERM_NOTAN, 8, 9,
		COMP_HERMITE, 16, 1, 37, 38,
		COMP_HERMITE, 14, 1, 49, 48,
		COMP_HERMITE, 6, 7, 41, 42,
		COMP_END,
	PTYPE_COMPSMOOTH | RFLAG_XREF, 0x8000, 5, 28, 29, 8, 9,		// mid edge
		COMP_HERM_NOTAN, 10, 11,
		COMP_HERMITE, 22, 1, 37, 38,
		COMP_HERM_NOTAN, 16, 1,
		COMP_HERMITE, 8, 9, 43, 44,
		COMP_END,
	PTYPE_COMPSMOOTH | RFLAG_XREF, 0x8000, 5, 30, 31, 10, 11,		// rear edge
		COMP_HERMITE, 12, 13, 53, 54, 
		COMP_HERMITE, 22, 1, 39, 40,
		COMP_HERMITE, 10, 11, 43, 44,
		COMP_END,
	PTYPE_COMPFLAT | RFLAG_XREF, 0x8000, 5, 32, 1, 16, 1,		// centre
		COMP_HERM_NOTAN, 22, 1,
		COMP_HERMITE, 24, 1, 59, 60,
		COMP_HERM_NOTAN, 18, 1,
		COMP_HERMITE, 16, 1, 47, 51, 
		COMP_END,
	PTYPE_COMPSMOOTH | RFLAG_XREF, 0x8000, 5, 34, 1, 22, 23,		// nacelle
		COMP_HERMITE, 12, 3, 45, 46,
		COMP_HERM_NOTAN, 35, 3,
		COMP_HERMITE, 61, 1, 57, 63,
		COMP_HERMITE, 36, 0, 63, 58,
		COMP_HERM_NOTAN, 24, 25,
		COMP_HERMITE, 22, 23, 59, 60,
		COMP_END,
	PTYPE_MATVAR, 2,
	PTYPE_COMPFLAT | RFLAG_XREF, 0x8000, 5, 70, 4, 12, 4,		// rear underside
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
	PTYPE_COMPFLAT | RFLAG_XREF, 7, 5, 72, 2, 36, 2,		// engine back face
		COMP_HERMITE, 61, 2, 57, 62, 
		COMP_HERMITE, 35, 2, 62, 58,
		COMP_LINE, 68, 2,
		COMP_LINE, 69, 2,
		COMP_LINE, 36, 2,
		COMP_END,

	PTYPE_MATFIXED, 30, 30, 30, 30, 30, 30, 200, 0, 0, 0,
	PTYPE_COMPSMOOTH, 0x8000, 5, 33, 1, 16, 0,		// cockpit
		COMP_HERMITE, 18, 2, 52, 48,
		COMP_HERMITE, 20, 0, 48, 51,
		COMP_HERMITE, 14, 5, 49, 47,
		COMP_HERMITE, 16, 3, 47, 50, 
		COMP_END,

	PTYPE_ZBIAS, 72, 2, 2,
	PTYPE_MATFIXED, 30, 30, 30, 30, 30, 30, 200, 0, 0, 0,
	PTYPE_TUBE | RFLAG_XREF, 8, 12, 72, 76, 1, 250, 200,
	PTYPE_MATANIM, AFUNC_THRUSTPULSE,
		0, 0, 0, 0, 0, 0, 100, 50, 50, 100,
		0, 0, 0, 0, 0, 0, 100, 0, 0, 50,
	PTYPE_CIRCLE | RFLAG_XREF, 9, 12, 72, 2, 1, 200,

	PTYPE_ZBIAS, 77, 120, 2,
//	PTYPE_MATFIXED, 30, 30, 30, 0, 0, 0, 100, 0, 0, 0,
	PTYPE_QUADFLAT | RFLAG_XREF, 77, 78, 80, 79,

	PTYPE_MATFIXED, 20, 20, 20, 0, 0, 0, 100, 0, 0, 0,
	PTYPE_ZBIAS, 98, 95, 2,
	PTYPE_TEXT | TFLAG_XCENTER, 0, 0x8000, 98, 95, 5, 0, 50, 250,
	PTYPE_ZBIAS, 97, 96, 2,
	PTYPE_TEXT | TFLAG_XCENTER, 0, 0x8000, 97, 96, 2, 0, 50, 250,

	PTYPE_ZBIAS, 73, 4, 2,
	PTYPE_SUBOBJECT, 0, SUB_NWUNIT, 73, 4, 2, 100,
	PTYPE_ZBIAS, 74, 4, 2,
	PTYPE_SUBOBJECT, 0, SUB_NWUNIT, 74, 4, 2, 64,
	PTYPE_SUBOBJECT, 0, SUB_NWUNIT, 75, 4, 2, 64,
	
	PTYPE_ZBIAS, 0x8000, 0, 0,
	PTYPE_END,

};	
static Thruster ship2thruster[] = {
	{ 81, 2 | THRUST_NOANG, 30.0f },
	{ 82, 2 | THRUST_NOANG, 30.0f },
	{ 83, 5 | THRUST_NOANG, 20.0f },
	{ 84, 5 | THRUST_NOANG, 20.0f },

	{ 85, 1, 15.0f }, { 86, 4, 15.0f },
	{ 87, 1, 15.0f }, { 88, 4, 15.0f },
	{ 89, 1, 15.0f }, { 90, 4, 15.0f },

	{ 91, 3, 15.0f }, { 92, 0, 15.0f },
	{ 93, 3, 15.0f }, { 94, 0, 15.0f },
};
Model ship2model = { 1.0f, 35.0f, 100, ship2vtx1, 120, 1, ship2vtx2, 10,
	{ 
		{ 0, ship2data, 0, 14, ship2thruster }
	} };




static PlainVertex station1vtx1[] = {
	{ VTYPE_PLAIN, { -15.0f, -20.0f, 30.0f } },			// 6, front octagon
	{ VTYPE_PLAIN, { 15.0f, -20.0f, 30.0f } },
	{ VTYPE_PLAIN, { 30.0f, -20.0f, 15.0f } },
	{ VTYPE_PLAIN, { 30.0f, -20.0f, -15.0f } },
	{ VTYPE_PLAIN, { 15.0f, -20.0f, -30.0f } },
	{ VTYPE_PLAIN, { -15.0f, -20.0f, -30.0f } },

	{ VTYPE_PLAIN, { -15.0f, 20.0f, 30.0f } }, 		// 12, back octagon
	{ VTYPE_PLAIN, { 15.0f, 20.0f, 30.0f } },
	{ VTYPE_PLAIN, { 30.0f, 20.0f, 15.0f } },
	{ VTYPE_PLAIN, { 30.0f, 20.0f, -15.0f } },
	{ VTYPE_PLAIN, { 15.0f, 20.0f, -30.0f } },
	{ VTYPE_PLAIN, { -15.0f, 20.0f, -30.0f } },

	{ VTYPE_PLAIN, { -10.0f, -20.0f, 5.0f } },			// 18, inlet front
	{ VTYPE_PLAIN, { 10.0f, -20.0f, 5.0f } },
	{ VTYPE_PLAIN, { 10.0f, -20.0f, -5.0f } },
	{ VTYPE_PLAIN, { -10.0f, -20.0f, -5.0f } },

	{ VTYPE_PLAIN, { -10.0f, 0.0f, 5.0f } },			// 22, inlet rear
	{ VTYPE_PLAIN, { 10.0f, 0.0f, 5.0f } },
	{ VTYPE_PLAIN, { 10.0f, 0.0f, -5.0f } },
	{ VTYPE_PLAIN, { -10.0f, 0.0f, -5.0f } },


	{ VTYPE_PLAIN, { 30.0f, -10.0f, 10.0f } },			// 26, strut inner
	{ VTYPE_PLAIN, { 30.0f, -10.0f, -10.0f } },
	{ VTYPE_PLAIN, { 30.0f, 10.0f, -10.0f } },
	{ VTYPE_PLAIN, { 30.0f, 10.0f, 10.0f } },

	{ VTYPE_PLAIN, { 100.0f, -10.0f, 10.0f } },			// 30, strut outer
	{ VTYPE_PLAIN, { 100.0f, -10.0f, -10.0f } },
	{ VTYPE_PLAIN, { 100.0f, 10.0f, -10.0f } },
	{ VTYPE_PLAIN, { 100.0f, 10.0f, 10.0f } },

	{ VTYPE_PLAIN, { 0.0f, -25.0f, 0.0f } },			// 34, ring start, end
	{ VTYPE_PLAIN, { 0.0f, 25.0f, 0.0f } },		

	{ VTYPE_PLAIN, { -9.0f, -10.0f, -4.5f } },			// 36, inlet middle (for docking)
	{ VTYPE_PLAIN, { 9.0f, -10.0f, -4.5f } },
	{ VTYPE_PLAIN, { 9.0f, -10.0f, 4.5f } },
	{ VTYPE_PLAIN, { -9.0f, -10.0f, 4.5f } },


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
static uint16 station1data[] = {
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

	PTYPE_TUBE | RFLAG_XREF, 0, 38, 34, 35, 2, 11500, 10000,

	PTYPE_SETCFLAG, 0x10,
	PTYPE_QUADFLAT | RFLAG_INVISIBLE, 39, 38, 37, 36,
	PTYPE_SETCFLAG, 0,

//	PTYPE_QUADFLAT | RFLAG_XREF,
//	PTYPE_SUBOBJECT, 0x8000, SUB_NOSEWHEEL, 10, 0, 4, 100,

	PTYPE_END,
};	
Model station1model = { 10.0f, 120.0f, 40, station1vtx1, 100, 0, station1vtx2, 1, 
	{ { 0, station1data, 0, 0, 0 } } };


static PlainVertex ship3vtx1[] = {
	{ VTYPE_PLAIN, { -4.0f, -5.0f, -20.0f } },		// 6, nose pair
	{ VTYPE_PLAIN, { 4.0f, -5.0f, -20.0f } },		//

	{ VTYPE_PLAIN, { -6.0f, 4.0f, -10.0f } },		// 8, mid vertices
	{ VTYPE_PLAIN, { 6.0f, 4.0f, -10.0f } },		//

	{ VTYPE_PLAIN, { -14.0f, -5.0f, -10.0f } },		// 10, front quarter
	{ VTYPE_PLAIN, { -10.0f, 5.0f, 10.0f } },		// back mid
	{ VTYPE_PLAIN, { -30.0f, -5.0f, 10.0f } },		// back end

//	{ VTYPE_PLAIN, { 20.0f, 1.0f, 0.0f } },		// 13, curve midpoint
//	{ VTYPE_DIR, { 2.0f, 2.0f, 1.0f } },		// norm

	{ VTYPE_PLAIN, { -18.75f, 1.25f, 2.1875f } },		// 13, curve midpoint
	{ VTYPE_DIR, { -0.707f, 1.0f, -0.707f } },		// norm

	{ VTYPE_PLAIN, { -15.0f, 0.0f, 10.0f } },		// 15, back midpoint
	{ VTYPE_PLAIN, { -30.0f, -5.0f, 10.0f } },		// underside midpoint

	// CW tangents
	{ VTYPE_PLAIN, { 4.0f, -1.0f, -20.0f } },		// 17, 11->10 start
	{ VTYPE_PLAIN, { -8.0f, -9.0f, 0.0f } },			// 11->10 end

	{ VTYPE_PLAIN, { -16.0f, 0.0f, 0.0f } },			// 19, 10->12 start
	{ VTYPE_PLAIN, { 0.0f, 0.0f, 20.0f } },		// 10-12 end

	{ VTYPE_PLAIN, { 0.0f, 10.0f, 0.0f } },			// 21, 12->11 start
	{ VTYPE_PLAIN, { 20.0f, 0.0f, 0.0f } },		// 12-11 end

	// CCW tangents
	{ VTYPE_PLAIN, { 8.0f, 9.0f, 0.0f } },			// 23, 10->11 start
	{ VTYPE_PLAIN, { -4.0f, 1.0f, 20.0f } },		// 10->11 end

	{ VTYPE_PLAIN, { 0.0f, 0.0f, -20.0f } },			// 25, 12-10 start
	{ VTYPE_PLAIN, { 16.0f, 0.0f, 0.0f } },		// 12->10 end

	{ VTYPE_PLAIN, { -20.0f, 0.0f, 0.0f } },			// 27, 11-12 start
	{ VTYPE_PLAIN, { 0.0f, -10.0f, 0.0f } },		// 11->12 end

	{ VTYPE_PLAIN, { 10.0f, 5.0f, 10.0f } },		// 29, back mid, left side
	{ VTYPE_PLAIN, { 14.0f, -5.0f, -10.0f } },		// 30 front quarter, left side
	{ VTYPE_PLAIN, { -10.0f, -5.0f, 10.0f } },		// back mid, right underside
	{ VTYPE_PLAIN, { 10.0f, -5.0f, 10.0f } },		// back mid, left underside

	{ VTYPE_PLAIN, { -12.0f, 0.0f, 10.0f } },		// 33, back thruster
	{ VTYPE_PLAIN, { -12.0f, 0.0f, 13.0f } },		// back thruster end

	{ VTYPE_PLAIN, { 0.0f, -5.0f, -13.0f } },		// 35, nose gear
	{ VTYPE_PLAIN, { -15.0f, -5.0f, 3.0f } },		// 36, back gear
	{ VTYPE_PLAIN, { 15.0f, -5.0f, 3.0f } },		// 37, back gear

	// thruster jets
	{ VTYPE_PLAIN, { -12.0f, 0.0f, 13.0f } },		// 38, main
	{ VTYPE_PLAIN, { -15.0f, -3.0f, -9.0f } },		// retro

	{ VTYPE_PLAIN, { -30.0f, -4.0f, 9.0f } },		// 40, corner clusters
	{ VTYPE_PLAIN, { -29.0f, -5.5f, 9.0f } },		// down
	{ VTYPE_PLAIN, { -29.0f, -4.0f, 9.0f } },		// up
	{ VTYPE_PLAIN, { -10.0f, 0.0f, -11.0f } },		// lateral front

};
static CompoundVertex ship3vtx2[] = {
	{ VTYPE_NORM, { 15, 8, 10, -1, -1 } },		// 100, mid curve norm
	{ VTYPE_NORM, { 9, 8, 11, -1, -1 } },		// 101, top curve norm
};
static uint16 ship3data[] = {
	PTYPE_MATVAR, 0,
	PTYPE_QUADFLAT, 6, 8, 9, 7,
	PTYPE_QUADFLAT, 9, 8, 11, 29,
	PTYPE_TRIFLAT | RFLAG_XREF, 8, 6, 10,
	PTYPE_QUADFLAT, 29, 11, 31, 32,

	PTYPE_COMPFLAT | RFLAG_XREF, 0, 5, 8, 100, 8, 100,		// mid curve
		COMP_LINE, 10, 100,
		COMP_HERMITE, 11, 100, 23, 24,
		COMP_LINE, 8, 100,
		COMP_END,
	PTYPE_COMPSMOOTH | RFLAG_XREF, 1, 5, 13, 14, 11, 101,		// top curve
		COMP_HERMITE, 10, 5, 17, 18,
		COMP_HERMITE, 12, 3, 19, 20,
		COMP_HERMITE, 11, 101, 21, 22,
		COMP_STEPS, 10, 
		COMP_END,
	PTYPE_COMPFLAT | RFLAG_XREF, 2, 5, 15, 2, 11, 2,		// back curve
		COMP_HERMITE, 12, 2, 27, 28,
		COMP_LINE, 31, 2,
		COMP_LINE, 11, 2,
		COMP_END,
	PTYPE_MATVAR, 2,
	PTYPE_COMPFLAT | RFLAG_XREF, 3, 5, 16, 4, 12, 4,		// underside curve
		COMP_HERMITE, 10, 4, 25, 26,
		COMP_LINE, 31, 4,
		COMP_LINE, 12, 4,
		COMP_END,
	PTYPE_QUADFLAT, 10, 6, 7, 30,
	PTYPE_QUADFLAT, 32, 31, 10, 30,

	PTYPE_ZBIAS, 33, 2, 5,
	PTYPE_MATFIXED, 30, 30, 30, 30, 30, 30, 200, 0, 0, 0,
	PTYPE_TUBE | RFLAG_XREF, 4, 12, 33, 34, 1, 300, 250,
	PTYPE_MATANIM, AFUNC_THRUSTPULSE,
		0, 0, 0, 0, 0, 0, 100, 50, 50, 100,
		0, 0, 0, 0, 0, 0, 100, 0, 0, 50,
	PTYPE_CIRCLE | RFLAG_XREF, 5, 12, 33, 2, 1, 250,

//	PTYPE_ZBIAS, 120, 5,
//	PTYPE_MATFIXED, 30, 30, 30, 0, 0, 0, 100, 0, 0, 0,
//	PTYPE_QUADFLAT | RFLAG_XREF, 77, 78, 80, 79,

/*	PTYPE_MATFIXED, 20, 20, 20, 0, 0, 0, 100, 0, 0, 0,
	PTYPE_ZBIAS, 95, 5,
	PTYPE_TEXT, 0, 0x8000, 68, 95, 2, 1900, 50, 250,
	PTYPE_ZBIAS, 96, 5,
	PTYPE_TEXT, 0, 0x8000, 97, 96, 5, 400, 50, 250,
*/
	PTYPE_ZBIAS, 35, 4, 5,
	PTYPE_SUBOBJECT, 0, SUB_NWUNIT, 35, 4, 2, 100,
	PTYPE_SUBOBJECT, 0, SUB_MWUNIT, 36, 4, 2, 100,
	PTYPE_SUBOBJECT, 0, SUB_MWUNIT, 37, 4, 2, 100,

	PTYPE_ZBIAS, 0x8000, 0, 0,
	PTYPE_END,
};
static uint16 ship3data_low[] = {
	PTYPE_MATVAR, 0,
	PTYPE_QUADFLAT, 6, 8, 9, 7,
	PTYPE_QUADFLAT, 9, 8, 11, 29,
	PTYPE_TRIFLAT | RFLAG_XREF, 8, 6, 10,
	PTYPE_QUADFLAT, 29, 11, 31, 32,

	PTYPE_COMPFLAT | RFLAG_XREF, 0x8000, 1, 8, 100, 8, 100,		// mid curve
		COMP_LINE, 10, 100,
		COMP_HERMITE, 11, 100, 23, 24,
		COMP_LINE, 8, 100,
		COMP_END,
	PTYPE_COMPSMOOTH | RFLAG_XREF, 0x8000, 1, 13, 14, 11, 101,		// top curve
		COMP_HERMITE, 10, 5, 17, 18,
		COMP_HERMITE, 12, 3, 19, 20,
		COMP_HERMITE, 11, 101, 21, 22,
		COMP_STEPS, 10, 
		COMP_END,
	PTYPE_COMPFLAT | RFLAG_XREF, 0x8000, 1, 15, 2, 11, 2,		// back curve
		COMP_HERMITE, 12, 2, 27, 28,
		COMP_LINE, 31, 2,
		COMP_LINE, 11, 2,
		COMP_END,
	PTYPE_MATVAR, 2,
	PTYPE_COMPFLAT | RFLAG_XREF, 0x8000, 1, 16, 4, 12, 4,		// underside curve
		COMP_HERMITE, 10, 4, 25, 26,
		COMP_LINE, 31, 4,
		COMP_LINE, 12, 4,
		COMP_END,
	PTYPE_QUADFLAT, 10, 6, 7, 30,
	PTYPE_QUADFLAT, 32, 31, 10, 30,

	PTYPE_ZBIAS, 33, 2, 5,
	PTYPE_MATFIXED, 30, 30, 30, 30, 30, 30, 200, 0, 0, 0,
	PTYPE_TUBE | RFLAG_XREF, 0x8000, 6, 33, 34, 1, 300, 250,
	PTYPE_MATANIM, AFUNC_THRUSTPULSE,
		0, 0, 0, 0, 0, 0, 100, 50, 50, 100,
		0, 0, 0, 0, 0, 0, 100, 0, 0, 50,
	PTYPE_CIRCLE | RFLAG_XREF, 0x8000, 6, 33, 2, 1, 250,

	PTYPE_ZBIAS, 35, 4, 5,
	PTYPE_SUBOBJECT, 0, SUB_NWUNIT, 35, 4, 2, 100,
	PTYPE_SUBOBJECT, 0, SUB_MWUNIT, 36, 4, 2, 100,
	PTYPE_SUBOBJECT, 0, SUB_MWUNIT, 37, 4, 2, 100,

	PTYPE_ZBIAS, 0x8000, 0, 0,
	PTYPE_END,
};
static Thruster ship3thruster[] = {
	{ 38, 2 | THRUST_NOANG | THRUST_XREF, 30.0f },
	{ 39, 5 | THRUST_NOANG | THRUST_XREF, 20.0f },
	{ 40, 3 | THRUST_XREF, 15.0f },
	{ 41, 4 | THRUST_XREF, 15.0f },
	{ 42, 1 | THRUST_XREF, 15.0f },
	{ 43, 3 | THRUST_XREF, 15.0f },
};
Model ship3model = { 1.0f, 35.0f, 44, ship3vtx1, 100, 2, ship3vtx2, 6, 
	{
		{ 20, ship3data_low, 0, 6, ship3thruster },
		{ 0, ship3data, 0, 6, ship3thruster }
	} };




static PlainVertex ship4vtx1[] = {
	{ VTYPE_PLAIN, { 4.0f, -3.0f, -35.0f } },			// 6, nose vertices
	{ VTYPE_PLAIN, { -4.0f, -3.0f, -35.0f } },
	{ VTYPE_PLAIN, { -1.0f, -7.0f, -32.0f } },		
	{ VTYPE_PLAIN, { 1.0f, -7.0f, -32.0f } },

	{ VTYPE_PLAIN, { 6.0f, 8.0f, -20.0f } },			// 10, nose section back
	{ VTYPE_PLAIN, { -6.0f, 8.0f, -20.0f } },				// and extrusion area
	{ VTYPE_PLAIN, { -10.0f, 4.0f, -20.0f } },			
	{ VTYPE_PLAIN, { -10.0f, -4.0f, -20.0f } },			
	{ VTYPE_PLAIN, { -6.0f, -8.0f, -20.0f } },
	{ VTYPE_PLAIN, { 6.0f, -8.0f, -20.0f } },
	{ VTYPE_PLAIN, { 10.0f, -4.0f, -20.0f } },			
	{ VTYPE_PLAIN, { 10.0f, 4.0f, -20.0f } },

	//midpoints
	{ VTYPE_PLAIN, { 0.0f, 0.0f, -20.0f } },			// 18
	{ VTYPE_PLAIN, { 0.0f, 0.0f, -16.0f } },			// 
	{ VTYPE_PLAIN, { 0.0f, 0.0f, 4.0f } },			// 
	{ VTYPE_PLAIN, { 0.0f, 0.0f, 8.0f } },			// 
	{ VTYPE_PLAIN, { 0.0f, 0.0f, 26.0f } },		// 

	{ VTYPE_PLAIN, { -0.3826834f, 0.9238795f, 0.0f } },		// 23, tube norm

	{ VTYPE_PLAIN, { -12.5f, 2.0f, 10.0f } },			// 24, top engine
	{ VTYPE_PLAIN, { -12.5f, 2.0f, 30.0f } },
	{ VTYPE_PLAIN, { -12.5f, 2.0f, 13.0f } },
	{ VTYPE_PLAIN, { -12.5f, 2.0f, 27.0f } },

	{ VTYPE_PLAIN, { -11.5f, -6.0f, 10.0f } },			// 28, bottom engine
	{ VTYPE_PLAIN, { -11.5f, -6.0f, 30.0f } },
	{ VTYPE_PLAIN, { -11.5f, -6.0f, 13.0f } },
	{ VTYPE_PLAIN, { -11.5f, -6.0f, 27.0f } },

	{ VTYPE_PLAIN, { -10.0f, -4.0f, -16.0f } },			// 32, right text pos
	{ VTYPE_PLAIN, { 10.0f, -4.0f, 4.0f } },			// left text pos

	{ VTYPE_PLAIN, { -5.0f, -8.0f, -13.0f } },			// 34, gear pos
	{ VTYPE_PLAIN, { 5.0f, -8.0f, -13.0f } },
	{ VTYPE_PLAIN, { -11.5f, -8.309f, 25.0f } },			// 36, gear pos
	{ VTYPE_PLAIN, { 11.5f, -8.309f, 25.0f } },
	{ VTYPE_PLAIN, { -11.5f, -8.309f, 13.0f } },			// 38, gear pos
	{ VTYPE_PLAIN, { 11.5f, -8.309f, 13.0f } },

	{ VTYPE_PLAIN, { -0.05f, 8.0f, 15.0f } },			// 40, dish pos
	
	// 41, extrusion as at 10 but rev order
	{ VTYPE_PLAIN, { 10.0f, 4.0f, -20.0f } },
	{ VTYPE_PLAIN, { 10.0f, -4.0f, -20.0f } },			
	{ VTYPE_PLAIN, { 6.0f, -8.0f, -20.0f } },
	{ VTYPE_PLAIN, { -6.0f, -8.0f, -20.0f } },
	{ VTYPE_PLAIN, { -10.0f, -4.0f, -20.0f } },			
	{ VTYPE_PLAIN, { -10.0f, 4.0f, -20.0f } },			
	{ VTYPE_PLAIN, { -6.0f, 8.0f, -20.0f } },
	{ VTYPE_PLAIN, { 6.0f, 8.0f, -20.0f } },

};
static CompoundVertex ship4vtx2[] = {
	{ VTYPE_ANIMROTATE, { 1, 3, -1, -1, AFUNC_LIN4SEC } },			// dummy
};
static uint16 ship4data[] = {
	PTYPE_MATVAR, 0,

	PTYPE_QUADFLAT, 6, 7, 11, 10, 					// front section
	PTYPE_TRIFLAT | RFLAG_XREF, 7, 12, 11,
	PTYPE_TRIFLAT | RFLAG_XREF, 7, 13, 12,
	PTYPE_TRIFLAT | RFLAG_XREF, 7, 14, 13, 
	PTYPE_TRIFLAT | RFLAG_XREF, 7, 9, 14,
	PTYPE_QUADFLAT, 7, 6, 9, 8, 
	PTYPE_QUADFLAT, 8, 9, 15, 14,

	PTYPE_QUADFLAT, 10, 11, 14, 15, 
	PTYPE_QUADFLAT | RFLAG_XREF, 11, 12, 13, 14, 

	PTYPE_EXTRUSION, 0, 8, 19, 20, 1, 100, 41,
	PTYPE_EXTRUSION, 1, 8, 21, 22, 1, 100, 41,

	PTYPE_TUBE | RFLAG_XREF, 2, 8, 24, 25, 23, 250, 200,
	PTYPE_TUBE | RFLAG_XREF, 3, 8, 28, 29, 23, 250, 200,

	PTYPE_MATANIM, AFUNC_THRUSTPULSE,
		0, 0, 0, 0, 0, 0, 100, 50, 50, 100,
		0, 0, 0, 0, 0, 0, 100, 0, 0, 50,
	PTYPE_CIRCLE | RFLAG_XREF, 4, 8, 26, 5, 23, 200,
	PTYPE_CIRCLE | RFLAG_XREF, 5, 8, 27, 2, 23, 200,
	PTYPE_CIRCLE | RFLAG_XREF, 6, 8, 30, 5, 23, 200,
	PTYPE_CIRCLE | RFLAG_XREF, 7, 8, 31, 2, 23, 200,

	PTYPE_MATFIXED, 30, 30, 30, 10, 10, 10, 100, 0, 0, 0,
	PTYPE_EXTRUSION, 8, 8, 18, 19, 1, 85, 41,
	PTYPE_EXTRUSION, 9, 8, 20, 21, 1, 85, 41,

	PTYPE_MATFIXED, 20, 20, 20, 0, 0, 0, 100, 0, 0, 0,
	PTYPE_ZBIAS, 32, 3, 5,
	PTYPE_TEXT, 0, 0x8000, 32, 3, 5, 0, 250, 400,
	PTYPE_ZBIAS, 33, 0, 5,
	PTYPE_TEXT, 0, 0x8000, 33, 0, 2, 0, 250, 400,

	PTYPE_ZBIAS, 34, 4, 5,
	PTYPE_SUBOBJECT, 0, SUB_MWUNIT, 34, 4, 2, 60,
	PTYPE_SUBOBJECT, 0, SUB_MWUNIT, 35, 4, 2, 60,
	PTYPE_SUBOBJECT, 0, SUB_MWUNIT, 36, 4, 2, 50,
	PTYPE_SUBOBJECT, 0, SUB_MWUNIT, 37, 4, 2, 50,
	PTYPE_SUBOBJECT, 0, SUB_MWUNIT, 38, 4, 2, 50,
	PTYPE_SUBOBJECT, 0, SUB_MWUNIT, 39, 4, 2, 50,

	PTYPE_ZBIAS, 40, 1, 5,
	PTYPE_SUBOBJECT, 0x8000, SUB_DISH, 40, 1, 100, 200,

	PTYPE_ZBIAS, 0x8000, 0, 0,
	PTYPE_END,
};	
static Thruster ship4thruster[] = {
	{ 25, 2 | THRUST_NOANG | THRUST_XREF, 30.0f },
	{ 29, 2 | THRUST_NOANG | THRUST_XREF, 30.0f },
	{ 24, 5 | THRUST_NOANG | THRUST_XREF, 20.0f },
	{ 28, 5 | THRUST_NOANG | THRUST_XREF, 20.0f },
//	{ 40, 0 | THRUST_XREF, 15.0f },
//	{ 41, 4 | THRUST_XREF, 15.0f },
//	{ 42, 1 | THRUST_XREF, 15.0f },
//	{ 43, 0 | THRUST_XREF, 15.0f },
};
Model ship4model = { 1.0f, 40.0f, 49, ship4vtx1, 100, 1, ship4vtx2, 10,
	{ { 0, ship4data, 0, 4, ship4thruster } } };


static PlainVertex dishvtx1[] = {
	{ VTYPE_PLAIN, { 0.0f, 3.0f, 1.2f } },			// 6, dish
	{ VTYPE_PLAIN, { 1.0f, 2.0f, 1.2f } },
	{ VTYPE_PLAIN, { 0.0f, 1.0f, 1.2f } },
	{ VTYPE_PLAIN, { -1.0f, 2.0f, 1.2f } },
	{ VTYPE_PLAIN, { 0.0f, 2.0f, 0.2f } },

	{ VTYPE_PLAIN, { 0.0f, 2.2f, 0.0f } },			// 11, stand
	{ VTYPE_PLAIN, { 0.0f, 0.6f, 0.0f } },
	{ VTYPE_PLAIN, { 0.0f, 0.0f, 0.0f } },

	{ VTYPE_PLAIN, { 0.0f, 2.0f, 1.7f } },			// 14, antenna

	{ VTYPE_PLAIN, { 1.5f, 0.0f, 0.0f } },			// 15, tangents
	{ VTYPE_PLAIN, { -1.5f, 0.0f, 0.0f } },
	{ VTYPE_PLAIN, { 0.0f, 1.5f, 0.0f } },
	{ VTYPE_PLAIN, { 0.0f, -1.5f, 0.0f } },
	
};
static CompoundVertex dishvtx2[] = {
	{ VTYPE_CROSS, { 0, 1, 2, -1, -1 } },			// dummy
};
static uint16 dishdata[] = {
	PTYPE_MATFIXED, 50, 50, 50, 100, 100, 100, 200, 0, 0, 0,
	PTYPE_COMPSMOOTH, 0, 5, 10, 5, 6, 1,
		COMP_HERMITE, 7, 0, 15, 18,
		COMP_HERMITE, 8, 4, 18, 16,
		COMP_HERMITE, 9, 3, 16, 17,
		COMP_HERMITE, 6, 1, 17, 15,
		COMP_END,
	PTYPE_COMPSMOOTH, 1, 5, 10, 2, 6, 4,
		COMP_HERMITE, 9, 0, 16, 18,
		COMP_HERMITE, 8, 1, 18, 15,
		COMP_HERMITE, 7, 3, 15, 17,
		COMP_HERMITE, 6, 4, 17, 16,
		COMP_END,
	PTYPE_CYLINDER, 4, 6, 10, 14, 0, 10,

	PTYPE_MATVAR, 0,
	PTYPE_CYLINDER, 2, 6, 11, 12, 0, 20,
	PTYPE_CYLINDER, 3, 6, 12, 13, 0, 70,

	PTYPE_END,
};	
Model dishmodel = { 1.0f, 4.0f, 19, dishvtx1, 40, 0, dishvtx2, 5,
	{ { 0, dishdata, 0, 0, 0 } } };


static PlainVertex ship5vtx1[] = {
	{ VTYPE_PLAIN, { 1.0f, 0.0f, -20.0f } },			// 6, right nose vertex
	{ VTYPE_PLAIN, { 0.0f, 1.0f, -0.4f } },
	{ VTYPE_PLAIN, { 0.0f, -1.0f, -0.4f } },

	{ VTYPE_PLAIN, { -1.0f, 0.0f, -20.0f } },		// 9, left nose vertex
	{ VTYPE_PLAIN, { 0.0f, 1.0f, -0.4f } },
	{ VTYPE_PLAIN, { 0.0f, -1.0f, -0.4f } },

	{ VTYPE_PLAIN, { 2.0f, 2.0f, -10.0f } },			// 12, nose section back
	{ VTYPE_PLAIN, { -2.0f, 2.0f, -10.0f } },
	{ VTYPE_PLAIN, { -3.0f, 0.0f, -10.0f } },			
	{ VTYPE_PLAIN, { -2.0f, -2.0f, -10.0f } },			
	{ VTYPE_PLAIN, { 2.0f, -2.0f, -10.0f } },
	{ VTYPE_PLAIN, { 3.0f, 0.0f, -10.0f } },

	{ VTYPE_PLAIN, { 0.0f, 0.0f, -10.0f } },			// 18, extrusion start/end
	{ VTYPE_PLAIN, { 0.0f, 0.0f, 15.0f } },

	// tangents

	{ VTYPE_PLAIN, { -1.0f, 0.0f, -10.0f } },			// 20, 12->6 and 16->6 s
	{ VTYPE_PLAIN, { -1.0f, -4.0f, -10.0f } },		// 12->6 e
	{ VTYPE_PLAIN, { 1.0f, 4.0f, 10.0f } },		// 6-12 e
	{ VTYPE_PLAIN, { 1.0f, 0.0f, 10.0f } },		// 6->12 and 6->16 s

	{ VTYPE_PLAIN, { 1.0f, 0.0f, -10.0f } },			// 24, 13->9
	{ VTYPE_PLAIN, { 1.0f, -4.0f, -10.0f } },
	{ VTYPE_PLAIN, { -1.0f, 4.0f, 10.0f } },
	{ VTYPE_PLAIN, { -1.0f, 0.0f, 10.0f } },			// 9->13 and 9->

	{ VTYPE_PLAIN, { -1.0f, 4.0f, -10.0f } },			// 28, 16->6 e
	{ VTYPE_PLAIN, { 1.0f, -4.0f, 10.0f } },		// 6-16 e

	{ VTYPE_PLAIN, { 1.0f, 4.0f, -10.0f } },		// 30, 15->9 e
	{ VTYPE_PLAIN, { -1.0f, -4.0f, 10.0f } },		// 9-15 e
	
	{ VTYPE_PLAIN, { 0.0f, 1.333f, -15.0f } },		// 32, nose top midpoint
	{ VTYPE_PLAIN, { 0.0f, 1.0f, -0.2f } },

	{ VTYPE_PLAIN, { 0.0f, -1.333f, -15.0f } },		// 34, nose bottom midpoint
	{ VTYPE_PLAIN, { 0.0f, -1.0f, -0.2f } },


	// wing positions
	{ VTYPE_PLAIN, { -2.5f, 1.0f, 5.0f } },			// 36
	{ VTYPE_PLAIN, { -2.5f, -1.0f, 5.0f } },
	{ VTYPE_PLAIN, { 2.5f, -1.0f, 5.0f } },
	{ VTYPE_PLAIN, { 2.5f, 1.0f, 5.0f } },

	// wing normals
	{ VTYPE_DIR, { -2.0f, 1.0f, 0.0f } },			// 40
	{ VTYPE_DIR, { -2.0f, -1.0f, 0.0f } },
	{ VTYPE_DIR, { 2.0f, -1.0f, 0.0f } },
	{ VTYPE_DIR, { 2.0f, 1.0f, 0.0f } },

	// 44, for extrusion
	{ VTYPE_PLAIN, { 3.0f, 0.0f, -10.0f } },
	{ VTYPE_PLAIN, { 2.0f, -2.0f, -10.0f } },
	{ VTYPE_PLAIN, { -2.0f, -2.0f, -10.0f } },			
	{ VTYPE_PLAIN, { -3.0f, 0.0f, -10.0f } },			
	{ VTYPE_PLAIN, { -2.0f, 2.0f, -10.0f } },			
	{ VTYPE_PLAIN, { 2.0f, 2.0f, -10.0f } },

};
static CompoundVertex ship5vtx2[] = {
	{ VTYPE_NORM, { 9, 14, 13, -1, -1 } },			// 100, nose side normals
	{ VTYPE_NORM, { 9, 15, 14, -1, -1 } },

};
static uint16 ship5data[] = {
	PTYPE_MATFIXED, 100, 0, 100, 0, 0, 0, 100, 0, 0, 0,
//	PTYPE_TRIFLAT | RFLAG_XREF, 9, 14, 13,
//	PTYPE_TRIFLAT | RFLAG_XREF, 9, 15, 14,
//	PTYPE_QUADFLAT, 6, 9, 13, 12,
//	PTYPE_QUADFLAT, 9, 6, 16, 15,

	PTYPE_EXTRUSION, 0, 6, 18, 19, 1, 100, 44,

	PTYPE_COMPSMOOTH, 1, 5, 32, 33, 13, 1,
		COMP_LINE, 12, 1,
		COMP_HERMITE, 6, 7, 20, 21,
		COMP_LINE, 9, 10,
		COMP_HERMITE, 13, 1, 26, 27,
		COMP_END,
	PTYPE_COMPSMOOTH, 2, 5, 34, 35, 16, 4,
		COMP_LINE, 15, 4,
		COMP_HERMITE, 9, 11, 24, 30,
		COMP_LINE, 6, 8,
		COMP_HERMITE, 16, 4, 29, 23,
		COMP_END,

	PTYPE_COMPFLAT | RFLAG_XREF, 3, 5, 14, 100, 13, 100,
		COMP_HERMITE, 9, 100, 24, 25,
		COMP_LINE, 14, 100,
		COMP_LINE, 13, 100,
		COMP_END,
	PTYPE_COMPFLAT | RFLAG_XREF, 4, 5, 14, 101, 9, 101,
		COMP_HERMITE, 15, 101, 31, 27,
		COMP_LINE, 14, 101,
		COMP_LINE, 9, 101,
		COMP_END,

	PTYPE_SUBOBJECT | SUBOBJ_THRUST, 0x8000, SUB_WING2, 36, 40, 5, 70,
	PTYPE_SUBOBJECT | SUBOBJ_THRUST, 0x8000, SUB_WING2, 37, 41, 5, 70,
	PTYPE_SUBOBJECT | SUBOBJ_THRUST, 0x8000, SUB_WING2, 38, 42, 5, 70,
	PTYPE_SUBOBJECT | SUBOBJ_THRUST, 0x8000, SUB_WING2, 39, 43, 5, 70,

	PTYPE_END,
};	
Model ship5model = { 1.0f, 25.0f, 50, ship5vtx1, 100, 2, ship5vtx2, 5, 
	{ { 0, ship5data, 0, 0, 0 } } };


static PlainVertex wing2vtx1[] = {
	{ VTYPE_PLAIN, { 0.0f, 0.0f, -3.5f } },			// 6, bottom front
	{ VTYPE_PLAIN, { 0.0f, 0.0f, 3.5f } },			// bottom back
	{ VTYPE_PLAIN, { 0.0f, 20.0f, -3.5f } },			// top front
	{ VTYPE_PLAIN, { 0.0f, 20.0f, 3.5f } },		// top back

	{ VTYPE_DIR, { 0.0f, 0.0f, -1.0f } },			// 10, front norm
	{ VTYPE_DIR, { -3.0f, 0.0f, 1.0f } },			// back norm

	{ VTYPE_PLAIN, { -0.8f, 10.0f, 0.0f } },			// 12, sidecentre
	{ VTYPE_DIR, { -1.0f, 0.0f, -0.20f } },			// sidenorm

	{ VTYPE_PLAIN, { 2.8f, 0.0f, 0.0f } },			// 14, front tan, forward
	{ VTYPE_PLAIN, { -2.8f, 0.0f, 0.0f } },			// front tan, backward

	{ VTYPE_PLAIN, { -1.0f, 0.0f, -3.0f } },			// 16, back tan, forward
	{ VTYPE_PLAIN, { 1.0f, 0.0f, 3.0f } },		// back tan, backward

	{ VTYPE_PLAIN, { -0.3826834f, 0.9238795f, 0.0f } },			// 18, tube norm
	{ VTYPE_PLAIN, { 0.0f, 21.5f, -5.0f } },		// tube start
	{ VTYPE_PLAIN, { 0.0f, 21.5f, 5.0f } },		// 
	{ VTYPE_PLAIN, { 0.0f, 22.0f, -4.0f } },			// 
	{ VTYPE_PLAIN, { 0.0f, 22.0f, 4.0f } },		// 
	
};
static CompoundVertex wing2vtx2[] = {
	{ VTYPE_CROSS, { 19, 14, -1, -1, -1 } },		// dummy
};
static uint16 wing2data[] = {
	PTYPE_MATFIXED, 100, 0, 100, 0, 0, 0, 100, 0, 0, 0,
	PTYPE_COMPSMOOTH | RFLAG_XREF, 0, 10, 12, 13, 6, 10,		// side
		COMP_HERMITE, 7, 11, 15, 17, 
		COMP_HERM_NOTAN, 9, 11,
		COMP_HERMITE, 8, 10, 16, 14,
		COMP_HERM_NOTAN, 6, 10, 
		COMP_END,

	PTYPE_TUBE, 1, 8, 19, 20, 18, 162, 140,

	PTYPE_END,
};
static Thruster wing2thruster[] = {
	{ 20, 2, 25.0f },
	{ 19, 5, 20.0f },
};
Model wing2model = { 1.0f, 25.0f, 23, wing2vtx1, 30, 0, wing2vtx2, 2,
	{ { 0, wing2data, 0, 2, wing2thruster } } };

static PlainVertex metalFrameTowerVtx1[] = {
	{ VTYPE_PLAIN, { -1, 0, 1 } },
	{ VTYPE_PLAIN, { 1, 0, 1 } },
	{ VTYPE_PLAIN, { 1, 0, -1 } },
	{ VTYPE_PLAIN, { -1, 0, -1 } },
	{ VTYPE_PLAIN, { -1, 10, 1 } },
	{ VTYPE_PLAIN, { 1, 10, 1 } },
	{ VTYPE_PLAIN, { 1, 10, -1 } },
	{ VTYPE_PLAIN, { -1, 10, -1 } },
};
static uint16 metalFrameTowerData[] = {
	PTYPE_CYLINDER, 0x8000, 4, 6, 10, 0, 10,
	PTYPE_CYLINDER, 0x8000, 4, 7, 11, 0, 10,
	PTYPE_CYLINDER, 0x8000, 4, 8, 12, 0, 10,
	PTYPE_CYLINDER, 0x8000, 4, 9, 13, 0, 10,
	PTYPE_CYLINDER, 0x8000, 4, 6, 11, 0, 10,
	PTYPE_CYLINDER, 0x8000, 4, 7, 10, 0, 10,
	PTYPE_CYLINDER, 0x8000, 4, 7, 12, 0, 10,
	PTYPE_CYLINDER, 0x8000, 4, 8, 11, 0, 10,
	PTYPE_CYLINDER, 0x8000, 4, 8, 13, 0, 10,
	PTYPE_CYLINDER, 0x8000, 4, 9, 12, 0, 10,
	PTYPE_CYLINDER, 0x8000, 4, 6, 13, 0, 10,
	PTYPE_CYLINDER, 0x8000, 4, 9, 10, 0, 10,
	PTYPE_CYLINDER, 0x8000, 4, 10, 11, 1, 10,
	PTYPE_CYLINDER, 0x8000, 4, 11, 12, 1, 10,
	PTYPE_CYLINDER, 0x8000, 4, 12, 13, 1, 10,
	PTYPE_CYLINDER, 0x8000, 4, 13, 10, 1, 10,
	PTYPE_END,
};
Model metalFrameTowerModel = { 0.1f, 20.0f, 14, metalFrameTowerVtx1, 14, 4, dummyvtx2, 0,
	{ { 0, metalFrameTowerData, 0, 0, 0 } } };

static PlainVertex starport1vtx1[] = {
	{ VTYPE_PLAIN, { 0,0,0 } }, // 6: pad centre
	{ VTYPE_PLAIN, { 0,.01,0 } },
	{ VTYPE_PLAIN, { -0.1,.01,-0.1 } },
	{ VTYPE_PLAIN, { 0, 0, -2 } }, // 9: hermite norm
	{ VTYPE_PLAIN, { 0, 0, 2 } }, // 10: hermite norm
	{ VTYPE_PLAIN, { 0.5, 0, 0 } }, // 11: pad vtx
	{ VTYPE_PLAIN, { -0.5, 0, 0 } }, // 12: pad vtx

	{ VTYPE_PLAIN, { 0, 0, 2 } }, // 13: pad2 centre
	{ VTYPE_PLAIN, { 0.5, 0, 2 } }, // 14: pad2 vtx
	{ VTYPE_PLAIN, { -0.5, 0, 2 } }, // 15: pad2 vtx
	{ VTYPE_PLAIN, { -0.1,.01, 2-0.1 } },
};
/*uint16 PFUNC_COMPSMOOTH
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
*/
static uint16 starport1data[] = {
	PTYPE_MATFIXED, 30, 30, 30, 0, 0, 0, 0, 0, 0, 0,
	PTYPE_SETCFLAG, 0x10,
	PTYPE_COMPFLAT, 0x8000, 20, 6, 1, 11, 1,
		COMP_HERMITE, 12, 1, 9, 10,
		COMP_HERMITE, 11, 1, 10, 9,
		COMP_END,
	PTYPE_SETCFLAG, 0x11,
	PTYPE_COMPFLAT, 0x8000, 20, 13, 1, 14, 1,
		COMP_HERMITE, 15, 1, 9, 10,
		COMP_HERMITE, 14, 1, 10, 9,
		COMP_END,
	PTYPE_SETCFLAG, 0,
	PTYPE_MATFIXED, 100, 100, 100, 0, 0, 0, 0, 0, 0, 0,
	PTYPE_ZBIAS, 6, 1, 0,
	PTYPE_TEXT | TFLAG_XCENTER | TFLAG_YCENTER, 0, 10, 6, 1, 0, 0, 0, 20,
	PTYPE_TEXT | TFLAG_XCENTER | TFLAG_YCENTER, 0, 11, 13, 1, 0, 0, 0, 20,
	PTYPE_ZBIAS, 0x8000, 0, 0,
	PTYPE_SUBOBJECT, 0x8000, 100, 0, 1, 2, 100,
	PTYPE_SUBOBJECT, 0x8000, 100, 3, 1, 2, 100,
	PTYPE_END,
};
Model starport1model = { 100.0f, 55.0f, 17, starport1vtx1, 17, 0, dummyvtx2, 1,
	{ { 0, starport1data, 0, 0, 0 } } };

static PlainVertex tombstonevtx1[] = {
	{ VTYPE_PLAIN, { 0.6f, 1.0f, -0.1f } }, // front quad
	{ VTYPE_PLAIN, { 0.6f, -1.0f, -0.1f } },
	{ VTYPE_PLAIN, { -0.6f, -1.0f, -0.1f } },
	{ VTYPE_PLAIN, { -0.6f, 1.0f, -0.1f } },
	{ VTYPE_PLAIN, { 0, 1, 0.1 } }, // cylinder
	{ VTYPE_PLAIN, { 0, 1, -0.1 } },
	{ VTYPE_PLAIN, { 0.6f, 1.0f, 0.1f } }, // rear quad
	{ VTYPE_PLAIN, { 0.6f, -1.0f, 0.1f } },
	{ VTYPE_PLAIN, { -0.6f, -1.0f, 0.1f } },
	{ VTYPE_PLAIN, { -0.6f, 1.0f, 0.1f } },
	{ VTYPE_PLAIN, { 0, 0.5, -0.1 } }, // text start
	{ VTYPE_PLAIN, { 0, 0.5, 0.1 } }, // text start
};
static uint16 tombstonedata[] = {
	PTYPE_MATFIXED, 50, 50, 50, 0, 0, 0, 100, 0, 0, 0,
	PTYPE_QUADFLAT, 6, 7, 8, 9,
	PTYPE_QUADFLAT, 15, 14, 13, 12,
	PTYPE_QUADFLAT, 6, 12, 13, 7,
	PTYPE_QUADFLAT, 9, 8, 14, 15,
	PTYPE_QUADFLAT, 8, 7, 13, 14,
	PTYPE_CYLINDER, 0x8000, 16, 10, 11, 1, 60,
	PTYPE_MATFIXED, 100, 100, 100, 0, 0, 0, 100, 100, 100, 0,
	PTYPE_ZBIAS, 16, 5, 0,
	PTYPE_TEXT | TFLAG_XCENTER, 0, 0x8000, 16, 5, 0, 0, 0, 10,
	PTYPE_ZBIAS, 17, 2, 0,
	PTYPE_TEXT | TFLAG_XCENTER, 0, 0x8000, 17, 2, 3, 0, 0, 10,
	PTYPE_END
};
Model tombstonemodel = { 10.0f, 2.0f, 18, tombstonevtx1, 18, 0, dummyvtx2, 1,
	{ { 0, tombstonedata, 0, 0, 0 } } };

static PlainVertex cargovtx1[] = {
	{ VTYPE_PLAIN, { 0, 0.8, 0 } },
	{ VTYPE_PLAIN, { 0, -0.8, 0 } },
};

static uint16 cargodata[] = {
	PTYPE_MATFIXED, 50, 50, 50, 0, 0, 0, 100, 0, 0, 0,
	PTYPE_TUBE, 0x8000, 16, 1, 4, 0, 100, 90,
	PTYPE_CYLINDER, 0x8000, 16, 6, 7, 0, 90,
	PTYPE_MATFIXED, 100, 0, 0, 0, 0, 0, 100, 0, 0, 0,
	PTYPE_ZBIAS, 1, 1, 0,
	PTYPE_TEXT | TFLAG_XCENTER | TFLAG_YCENTER, 0, 0x8000, 6, 1, 0, 0, 0, 10,
	PTYPE_ZBIAS, 4, 4, 0,
	PTYPE_TEXT | TFLAG_XCENTER | TFLAG_YCENTER, 0, 0x8000, 7, 4, 3, 0, 0, 10,
	PTYPE_END
};

static uint16 cargodata_lod2[] = {
	PTYPE_MATFIXED, 50, 50, 50, 0, 0, 0, 100, 0, 0, 0,
	PTYPE_CYLINDER, 0x8000, 8, 1, 4, 0, 100,
	PTYPE_END
};

Model cargomodel = { 1.0f, 2.0f, 8, cargovtx1, 8, 0, dummyvtx2, 1,
	{ 
		{ 20, cargodata_lod2, 0, 0, 0 } ,
		{ 0, cargodata, 0, 0, 0  },
	} };


