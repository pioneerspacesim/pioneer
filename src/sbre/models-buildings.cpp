#include "sbre_int.h"
#include "sbre_models.h"

static CompoundVertex dummyvtx2[] = {
	{ VTYPE_CROSS, { 0, 1, 2, -1, -1 } },			// dummy
};

// x,y,z, 3x.5x2 building
static PlainVertex building1_vtx1[] = {
	{ VTYPE_PLAIN, { -100, 50, -150 } },
	{ VTYPE_PLAIN, { -100, 50, 150 } },
	{ VTYPE_PLAIN, { 100, 50, 150 } },
	{ VTYPE_PLAIN, { 100, 50, -150 } },
	
	{ VTYPE_PLAIN, { -100, 0, -150 } },
	{ VTYPE_PLAIN, { -100, 0, 150 } },
	{ VTYPE_PLAIN, { 100, 0, 150 } },
	{ VTYPE_PLAIN, { 100, 0, -150 } },
	// text positions
	{ VTYPE_PLAIN, { -100, 25, -50 } },
	{ VTYPE_PLAIN, { 100, 25, 50 } },
};

static uint16 building1_data[] = {
	PTYPE_MATFIXED, 50,50,50, 0,0,0, 100, 0,0,0,
	PTYPE_QUADFLAT, 6, 7, 8, 9,
	PTYPE_QUADFLAT, 7, 11, 12, 8,
	PTYPE_QUADFLAT, 8, 12, 13, 9,
	PTYPE_QUADFLAT, 9, 13, 10, 6,
	PTYPE_QUADFLAT, 6, 10, 11, 7,
	PTYPE_QUADFLAT, 13, 12, 11, 10,
	PTYPE_MATFIXED, 50,10,10, 0,0,0, 100, 0,0,0,
	PTYPE_ZBIAS, 14, 3, 0,
	PTYPE_MATFIXED, 100,100,0, 0,0,0, 100, 0,0,0,
	PTYPE_TEXT | TFLAG_XCENTER | TFLAG_YCENTER, 0, 0, 14, 3, 5, 0, 0, 1000,
	PTYPE_ZBIAS, 15, 0, 0,
	PTYPE_TEXT | TFLAG_XCENTER | TFLAG_YCENTER, 0, 0, 15, 0, 2, 0, 0, 1000,
	PTYPE_END
};

Model building1 = { 1.0f, 200.0f, 16, building1_vtx1, 16, 0, dummyvtx2, 0,
	{ { 0, building1_data, 0, 0, 0 } },
};

static PlainVertex building2_vtx1[] = {
	{ VTYPE_PLAIN, { -40, 250, -60 } },
	{ VTYPE_PLAIN, { -40, 250, 60 } },
	{ VTYPE_PLAIN, { 40, 250, 60 } },
	{ VTYPE_PLAIN, { 40, 250, -60 } },
	
	{ VTYPE_PLAIN, { -40, 0, -60 } },
	{ VTYPE_PLAIN, { -40, 0, 60 } },
	{ VTYPE_PLAIN, { 40, 0, 60 } },
	{ VTYPE_PLAIN, { 40, 0, -60 } },
	// text positions
	{ VTYPE_PLAIN, { -40, 220, 0 } },
	{ VTYPE_PLAIN, { 40, 220, 0 } },
};

Model building2 = { 1.0f, 200.0f, 16, building2_vtx1, 16, 0, dummyvtx2, 0,
	{ { 0, building1_data, 0, 0, 0 } },
};


static PlainVertex house1_vtx1[] = {
	{ VTYPE_PLAIN, { -8, 4, -10 } },
	{ VTYPE_PLAIN, { -8, 4, 10 } },
	{ VTYPE_PLAIN, { 8, 4, 10 } },
	{ VTYPE_PLAIN, { 8, 4, -10 } },
	
	{ VTYPE_PLAIN, { -8, 0, -10 } },
	{ VTYPE_PLAIN, { -8, 0, 10 } },
	{ VTYPE_PLAIN, { 8, 0, 10 } },
	{ VTYPE_PLAIN, { 8, 0, -10 } },
	// roof jizz
	{ VTYPE_PLAIN, { 0, 10, 10 } }, // 14
	{ VTYPE_PLAIN, { 0, 10, -10 } },
};

static uint16 house1_data[] = {
	PTYPE_MATFIXED, 50,50,50, 0,0,0, 100, 0,0,0,
	PTYPE_QUADFLAT, 6, 7, 8, 9,
	PTYPE_QUADFLAT, 7, 11, 12, 8,
	PTYPE_QUADFLAT, 8, 12, 13, 9,
	PTYPE_QUADFLAT, 9, 13, 10, 6,
	PTYPE_QUADFLAT, 6, 10, 11, 7,
	PTYPE_QUADFLAT, 13, 12, 11, 10,
	PTYPE_TRIFLAT, 7, 8, 14,
	PTYPE_TRIFLAT, 9, 6, 15,
	PTYPE_MATFIXED, 80,50,10, 0,0,0, 100, 0,0,0,
	PTYPE_QUADFLAT | RFLAG_XREF, 15, 14, 8, 9,
	PTYPE_END
};

Model house1 = { 1.0f, 15.0f, 16, house1_vtx1, 16, 0, dummyvtx2, 0,
	{ { 0, house1_data, 0, 0, 0 } },
};


