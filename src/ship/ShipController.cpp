// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ShipController.h"
#include "Ship.h"
#include "core/OS.h"

void ShipController::StaticUpdate(float timeStep)
{
	m_ship->AITimeStep(timeStep);
}
