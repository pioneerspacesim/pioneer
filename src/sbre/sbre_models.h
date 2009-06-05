#ifndef __SBRE_MODELS_H__
#define __SBRE_MODELS_H__
#include "sbre_int.h"

extern Model curvetest;

extern Model dishmodel, nosewheelmodel, nwunitmodel, mainwheelmodel, mwunitmodel;
extern Model wing1model, wing2model;
extern Model ship1model, ship2model, ship3model, ship4model, ship5model, ship6model;
extern Model station1model, starport1model, tombstonemodel, cargomodel;
extern Model metalFrameTowerModel;
extern Model building1, building2, house1;

// common subobject indices

const int SUB_NOSEWHEEL = 1;
const int SUB_NWUNIT = 2;
const int SUB_MAINWHEEL = 3;
const int SUB_MWUNIT = 4;
const int SUB_DISH = 5;

// other subobject indices

const int SUB_WING1 = 30;
const int SUB_WING2 = 31;

#define SBRE_MAX_MODEL 1024
#define SBRE_COMPILED_MODELS 512
extern Model *ppModel[SBRE_MAX_MODEL];


#endif // __SBRE_MODELS_H__
