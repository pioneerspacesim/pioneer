
#include "sbre_int.h"
#include "sbre_models.h"

Model * ppModel[SBRE_MAX_MODEL] =
{
	// 0, current test object
	&ship6model,
	// 1, common subobjects
	&nosewheelmodel,
	&nwunitmodel,
	&mainwheelmodel,
	&mwunitmodel,
	&dishmodel,
	0, 0, 0, 0,
	// 10
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	// 20
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	// 30, single-use subobjects
	&wing1model,
	&wing2model,
	0, 0, 0, 0, 0, 0, 0, 0,
	// 40
	&building1, &building2, &house1, 0, 0, 0, 0, 0, 0, 0,
	// 50
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	// 60, JJ ships
	&ship1model,
	&ship2model,
	&ship3model,
	&ship4model,
	&ship5model,
	&station1model,
	&ship6model, 0, 0, 0,
	// 70
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	// 80
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	// 90, other people's ships
	&starport1model,&tombstonemodel,&cargomodel,0,0,0,0,0,0,0,
	// 100, more sub-objects
	&metalFrameTowerModel

};

