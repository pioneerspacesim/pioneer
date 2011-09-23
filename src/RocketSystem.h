#ifndef _ROCKETSYSTEM_H
#define _ROCKETSYSTEM_H

#include "Rocket/Core/SystemInterface.h"
#include "libs.h"

class RocketSystem : public Rocket::Core::SystemInterface {
	virtual float GetElapsedTime() { return float(SDL_GetTicks()) * 0.001; }
};

#endif
