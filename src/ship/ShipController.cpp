// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ShipController.h"
#include "OS.h"
#include "Ship.h"

void ShipController::StaticUpdate(float timeStep)
{
	OS::EnableFPE();
	m_ship->AITimeStep(timeStep);
	OS::DisableFPE();
}
