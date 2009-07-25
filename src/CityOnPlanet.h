#ifndef _CITYONPLANET_H
#define _CITYONPLANET_H

#include "libs.h"

class Planet;
class SpaceStation;
class Frame;

namespace CityOnPlanet {
	void Render(const Planet *planet, const SpaceStation *station, const Frame *camFrame, Uint32 seed);
};

#endif /* _CITYONPLANET_H */
