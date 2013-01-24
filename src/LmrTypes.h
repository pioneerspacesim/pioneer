// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LMRTYPES_H
#define _LMRTYPES_H

class EquipSet;

//this file might be temporary - but don't want to fight dependency issues right now
struct LmrMaterial {
	float diffuse[4];
	float specular[4];
	float emissive[4];
	float shininess;
	// make sure save and load routines in ShipFlavour are matching
};

struct LmrObjParams
{
	enum { LMR_ANIMATION_MAX = 10 };

	const char *animationNamespace; // the namespace to look up animation names in, from LuaConstants

	double time;
	int animStages[LMR_ANIMATION_MAX];
	double animValues[LMR_ANIMATION_MAX];
	const char *label; // ignored, use model->SetLabel
	const EquipSet *equipment; // for ships
	int flightState;

	float linthrust[3];		// 1.0 to -1.0
	float angthrust[3];		// 1.0 to -1.0
	//vector3f linthrust;
	//vector3f angthrust;

	struct LmrMaterial pMat[3];

	//stuff added after newmodel
	float boundingRadius; //updated by model and passed to submodels
	unsigned int nodemask;

	LmrObjParams()
	: boundingRadius(0.f)
	, nodemask(0x1) //draw solids
	{
		std::fill(linthrust, linthrust+3, 0.f);
		std::fill(angthrust, angthrust+3, 0.f);
	}
};
typedef LmrObjParams ModelParams;
typedef LmrObjParams RenderData;

#endif
