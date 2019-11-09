// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "CustomSystem.h"

CustomSystem::CustomSystem() :
	sBody(0),
	numStars(0),
	seed(0),
	want_rand_explored(true),
	faction(0),
	govType(Polit::GOV_INVALID),
	want_rand_lawlessness(true)
{
	for (int i = 0; i < 4; ++i)
		primaryType[i] = GalaxyEnums::BodyType::TYPE_GRAVPOINT;
}

CustomSystem::~CustomSystem()
{
	delete sBody;
}

CustomSystemBody::CustomSystemBody() :
	aspectRatio(fixed(1, 1)),
	averageTemp(1),
	want_rand_offset(true),
	latitude(0.0),
	longitude(0.0),
	volatileGas(0),
	ringStatus(WANT_RANDOM_RINGS),
	seed(0),
	want_rand_seed(true)
{
}

CustomSystemBody::~CustomSystemBody()
{
	for (std::vector<CustomSystemBody *>::iterator
			 it = children.begin();
		 it != children.end(); ++it) {
		delete (*it);
	}
}
