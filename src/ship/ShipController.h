// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

/*
 * Ship movement controller class
 * Controls thrusters, autopilot according to player input or AI
 */
#include "JsonFwd.h"

class Body;
class Ship;
class Space;

enum FlightControlState { // <enum scope='FlightControlState' name=ShipControllerFlightControlState public>
	CONTROL_MANUAL,
	CONTROL_FIXSPEED,
	CONTROL_FIXHEADING_FORWARD,
	CONTROL_FIXHEADING_BACKWARD,
	CONTROL_FIXHEADING_NORMAL,
	CONTROL_FIXHEADING_ANTINORMAL,
	CONTROL_FIXHEADING_RADIALLY_INWARD,
	CONTROL_FIXHEADING_RADIALLY_OUTWARD,
	CONTROL_FIXHEADING_KILLROT,
	CONTROL_AUTOPILOT,

	CONTROL_STATE_COUNT // <enum skip>
};

// only AI
class ShipController {
public:
	//needed for serialization
	enum Type {
		AI = 0,
		PLAYER = 1
	};
	ShipController() {}
	virtual ~ShipController() {}
	virtual Type GetType() { return AI; }
	virtual void SaveToJson(Json &jsonObj, Space *s) {}
	virtual void LoadFromJson(const Json &jsonObj) {}
	virtual void PostLoadFixup(Space *) {}
	virtual void StaticUpdate(float timeStep);
	virtual void SetFlightControlState(FlightControlState s) {}
	virtual FlightControlState GetFlightControlState() const { return CONTROL_MANUAL; }
	virtual double GetCruiseSpeed() const { return 0.0; }
	virtual void ChangeCruiseSpeed(double delta) {}
	virtual Body *GetFollowTarget() const { return nullptr; }
	Ship *m_ship;
};
