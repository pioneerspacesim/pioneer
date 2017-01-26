// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "Ship.h"
#include "Propulsion.h"
#include "ShipAICmd.h"
#include "Pi.h"
#include "Player.h"
#include "perlin.h"
#include "Frame.h"
#include "Planet.h"
#include "SpaceStation.h"
#include "Space.h"
#include "LuaConstants.h"
#include "LuaEvent.h"
#include "EnumStrings.h"

// returns true if command is complete
bool Ship::AITimeStep(float timeStep)
{
	// allow the launch thruster thing to happen
	if (m_launchLockTimeout > 0.0) return false;

	m_decelerating = false;
	if (!m_curAICmd) {
		if (this == Pi::player) return true;

		// just in case the AI left it on
		ClearThrusterState();
		for (int i=0; i<Guns::GUNMOUNT_MAX; i++)
			SetGunState(i,0);
		return true;
	}

	if (m_curAICmd->TimeStepUpdate()) {
		AIClearInstructions();
//		ClearThrusterState();		// otherwise it does one timestep at 10k and gravity is fatal
		LuaEvent::Queue("onAICompleted", this, EnumStrings::GetString("ShipAIError", AIMessage()));
		return true;
	}
	else return false;
}

void Ship::AIClearInstructions()
{
	if (!m_curAICmd) return;

	delete m_curAICmd;		// rely on destructor to kill children
	m_curAICmd = 0;
	m_decelerating = false;		// don't adjust unless AI is running
}

void Ship::AIGetStatusText(char *str)
{
	if (!m_curAICmd) strcpy(str, "AI inactive");
	else m_curAICmd->GetStatusText(str);
}

void Ship::AIKamikaze(Body *target)
{
	AIClearInstructions();
	m_curAICmd = new AICmdKamikaze(this, target);
}

void Ship::AIKill(Ship *target)
{
	AIClearInstructions();
	SetFuelReserve((GetFuel() < 0.5) ? GetFuel() / 2 : 0.25);

	m_curAICmd = new AICmdKill(this, target);
}

/*
void Ship::AIJourney(SystemBodyPath &dest)
{
	AIClearInstructions();
//	m_curAICmd = new AICmdJourney(this, dest);
}
*/

void Ship::AIFlyTo(Body *target)
{
	AIClearInstructions();
	SetFuelReserve((GetFuel() < 0.5) ? GetFuel() / 2 : 0.25);

	if (target->IsType(Object::SHIP)) {		// test code
		vector3d posoff(-1000.0, 0.0, 1000.0);
		m_curAICmd = new AICmdFormation(this, static_cast<Ship*>(target), posoff);
	}
	else m_curAICmd = new AICmdFlyTo(this, target);
}

void Ship::AIDock(SpaceStation *target)
{
	AIClearInstructions();
	SetFuelReserve((GetFuel() < 0.5) ? GetFuel() / 2 : 0.25);

	m_curAICmd = new AICmdDock(this, target);
}

void Ship::AIOrbit(Body *target, double alt)
{
	AIClearInstructions();
	SetFuelReserve((GetFuel() < 0.5) ? GetFuel() / 2 : 0.25);

	m_curAICmd = new AICmdFlyAround(this, target, alt);
}

void Ship::AIHoldPosition()
{
	AIClearInstructions();
	m_curAICmd = new AICmdHoldPosition(this);
}
