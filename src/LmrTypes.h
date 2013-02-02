// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LMRTYPES_H
#define _LMRTYPES_H

//this file might be temporary - but don't want to fight dependency issues right now

struct LmrObjParams
{
	float linthrust[3];		// 1.0 to -1.0
	float angthrust[3];		// 1.0 to -1.0

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
