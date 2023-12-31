// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SHIPAICMD_H
#define _SHIPAICMD_H

#include "GameSaveError.h"
#include "JsonFwd.h"

#include "DynamicBody.h"
#include "FixedGuns.h"
#include "FrameId.h"
#include "ship/Propulsion.h"

class Ship;
class Space;
class SpaceStation;

class AICommand {
public:
	// This enum is solely to make the serialization work
	enum CmdName { // <enum scope='AICommand::CmdName' name=ShipAICmdName public>
		CMD_NONE,
		CMD_DOCK,
		CMD_FLYTO,
		CMD_FLYAROUND,
		CMD_KILL,
		CMD_KAMIKAZE,
		CMD_HOLDPOSITION,
		CMD_FORMATION
	};

	AICommand(DynamicBody *dBody, CmdName name) :
		m_dBody(dBody),
		m_cmdName(name)
	{
		m_dBody->AIMessage(DynamicBody::AIERROR_NONE);
	}
	virtual ~AICommand() {}

	virtual bool TimeStepUpdate() = 0;
	bool ProcessChild(); // returns false if child is active
	virtual void GetStatusText(char *str)
	{
		if (m_child)
			m_child->GetStatusText(str);
		else
			strcpy(str, "AI state unknown");
	}

	// Serialisation functions
	static AICommand *LoadFromJson(const Json &jsonObj);
	AICommand(const Json &jsonObj, CmdName name);
	virtual void SaveToJson(Json &jsonObj);
	virtual void PostLoadFixup(Space *space);

	// Signal functions
	virtual void OnDeleted(const Body *body)
	{
		if (m_child) m_child->OnDeleted(body);
	}

	CmdName GetType() const { return m_cmdName; }

protected:
	DynamicBody *m_dBody;
	Propulsion *m_prop;

	std::unique_ptr<AICommand> m_child;
	bool m_is_flyto = false;
	CmdName m_cmdName;

	int m_dBodyIndex; // deserialisation
};

class AICmdDock : public AICommand {
public:
	virtual bool TimeStepUpdate();
	AICmdDock(DynamicBody *dBody, SpaceStation *target);

	virtual void GetStatusText(char *str);
	virtual void SaveToJson(Json &jsonObj);
	AICmdDock(const Json &jsonObj);
	virtual void PostLoadFixup(Space *space);

	virtual void OnDeleted(const Body *body);

private:
	enum EDockingStates {
		eDockGetDataStart = 0, // 0: get data for docking start pos
		eDockFlyToStart = 1,   // 1: Fly to docking start pos
		eDockGetDataEnd = 2,   // 2: get data for docking end pos
		eDockFlyToEnd = 3,	   // 3: Fly to docking end pos
		eDockingComplete = 4,
		eInvalidDockingStage = 5
	};

	SpaceStation *m_target;
	vector3d m_dockpos; // offset in target's frame
	vector3d m_dockdir;
	vector3d m_dockupdir;
	EDockingStates m_state; // see TimeStepUpdate()
	int m_targetIndex;		// used during deserialisation

	void IncrementState()
	{
		switch (m_state) {
		case eDockGetDataStart: m_state = eDockFlyToStart; break;
		case eDockFlyToStart: m_state = eDockGetDataEnd; break;
		case eDockGetDataEnd: m_state = eDockFlyToEnd; break;
		case eDockFlyToEnd: m_state = eDockingComplete; break;
		case eDockingComplete: m_state = eInvalidDockingStage; break;
		case eInvalidDockingStage: break;
		}
	}
};

class AICmdFlyTo : public AICommand {
public:
	virtual bool TimeStepUpdate();
	AICmdFlyTo(DynamicBody *dBody, FrameId targframeId, const vector3d &posoff, double endvel, bool tangent);
	AICmdFlyTo(DynamicBody *dBody, Body *target);

	virtual void GetStatusText(char *str);
	virtual void SaveToJson(Json &jsonObj);
	AICmdFlyTo(const Json &jsonObj);
	virtual void PostLoadFixup(Space *space);

	virtual void OnDeleted(const Body *body);

private:
	Body *m_target;		   // target for vicinity. Either this or targframe is 0
	double m_dist;		   // vicinity distance
	FrameId m_targframeId; // target frame for waypoint
	vector3d m_posoff;	   // offset in target frame
	double m_endvel;	   // target speed in direction of motion at end of path, positive only
	bool m_tangent;		   // true if path is to a tangent of the target frame's body
	int m_state;

	int m_targetIndex; // used during deserialisation
	vector3d m_reldir; // target direction relative to ship at last frame change
	FrameId m_frameId; // last frame of ship
	bool m_suicideRecovery;
};

class AICmdFlyAround : public AICommand {
public:
	virtual bool TimeStepUpdate();
	AICmdFlyAround(DynamicBody *dBody, Body *obstructor, double relalt, int mode = 2);
	AICmdFlyAround(DynamicBody *dBody, Body *obstructor, double alt, double vel, int mode = 1);

	virtual void GetStatusText(char *str);
	virtual void SaveToJson(Json &jsonObj);
	AICmdFlyAround(const Json &jsonObj);
	virtual void PostLoadFixup(Space *space);
	virtual void OnDeleted(const Body *body)
	{
		AICommand::OnDeleted(body);
		// check against obstructor?
	}
	void SetTargPos(const vector3d &targpos)
	{
		m_targpos = targpos;
		m_targmode = 0;
	}

protected:
	void Setup(Body *obstructor, double alt, double vel, int mode);
	double MaxVel(double targdist, double targalt);

private:
	Body *m_obstructor;	   // body to fly around
	int m_obstructorIndex; // deserialisation
	double m_alt, m_vel;
	int m_targmode;		// 0 targpos termination, 1 infinite, 2+ orbital termination
	vector3d m_targpos; // target position in ship space
};

class AICmdKill : public AICommand {
public:
	virtual bool TimeStepUpdate();
	AICmdKill(DynamicBody *dBody, Ship *target);
	AICmdKill(const Json &jsonObj);

	~AICmdKill();

	// don't actually need to save all this crap
	virtual void SaveToJson(Json &jsonObj);
	void PostLoadFixup(Space *space);

	virtual void OnDeleted(const Body *body);

	const Ship* GetTarget() const { return m_target; }

private:
	FixedGuns *m_fguns;
	Ship *m_target;
	double m_leadTime, m_evadeTime, m_closeTime;
	vector3d m_leadOffset, m_leadDrift, m_lastVel;
	int m_targetIndex; // used during deserialisation
};

class AICmdKamikaze : public AICommand {
public:
	virtual bool TimeStepUpdate();
	AICmdKamikaze(DynamicBody *dBody, Body *target);

	virtual void SaveToJson(Json &jsonObj);
	AICmdKamikaze(const Json &jsonObj);
	virtual void PostLoadFixup(Space *space);

	virtual void OnDeleted(const Body *body);

	const Body* GetTarget() const { return m_target; }
private:
	Body *m_target;
	int m_targetIndex; // used during deserialisation
};

class AICmdHoldPosition : public AICommand {
public:
	virtual bool TimeStepUpdate();
	AICmdHoldPosition(DynamicBody *dBody);
	AICmdHoldPosition(const Json &jsonObj);
};

class AICmdFormation : public AICommand {
public:
	virtual bool TimeStepUpdate();
	AICmdFormation(DynamicBody *dBody, DynamicBody *target, const vector3d &posoff);

	void GetStatusText(char *str);
	virtual void SaveToJson(Json &jsonObj);
	AICmdFormation(const Json &jsonObj);
	virtual void PostLoadFixup(Space *space);

	virtual void OnDeleted(const Body *body);

private:
	DynamicBody *m_target; // target frame for waypoint
	vector3d m_posoff;	   // offset in target frame

	int m_targetIndex; // used during deserialisation
};

#endif /* _SHIPAICMD_H */
