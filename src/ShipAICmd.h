// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SHIPAICMD_H
#define _SHIPAICMD_H

#include "Ship.h"
#include "SpaceStation.h"
#include "Serializer.h"
#include "Pi.h"
#include "Game.h"

class AICommand {
public:
	// This enum is solely to make the serialization work
	enum CmdName { CMD_NONE, CMD_DOCK, CMD_FLYTO, CMD_FLYAROUND, CMD_KILL, CMD_KAMIKAZE, CMD_HOLDPOSITION, CMD_FORMATION };

	AICommand(Ship *ship, CmdName name) {
	   	m_ship = ship; m_cmdName = name;
		m_child = 0;
		m_ship->AIMessage(Ship::AIERROR_NONE);
	}
	virtual ~AICommand() { if (m_child) delete m_child; }

	virtual bool TimeStepUpdate() = 0;
	bool ProcessChild();				// returns false if child is active
	virtual void GetStatusText(char *str) {
		if (m_child) m_child->GetStatusText(str);
		else strcpy(str, "AI state unknown");
	}

	// Serialisation functions
	static AICommand *Load(Serializer::Reader &rd);
	AICommand(Serializer::Reader &rd, CmdName name);
	virtual void Save(Serializer::Writer &wr);
	virtual void PostLoadFixup(Space *space);

	// Signal functions
	virtual void OnDeleted(const Body *body) { if (m_child) m_child->OnDeleted(body); }

protected:
	CmdName m_cmdName;
	Ship *m_ship;
	AICommand *m_child;

	int m_shipIndex; // deserialisation
};

class AICmdDock : public AICommand {
public:
	virtual bool TimeStepUpdate();
	AICmdDock(Ship *ship, SpaceStation *target);

	virtual void GetStatusText(char *str) {
		if (m_child) m_child->GetStatusText(str);
		else snprintf(str, 255, "Dock: target %s, state %i", m_target->GetLabel().c_str(), m_state);
	}
	virtual void Save(Serializer::Writer &wr) {
        Space *space = Pi::game->GetSpace();
		AICommand::Save(wr);
		wr.Int32(space->GetIndexForBody(m_target));
		wr.Vector3d(m_dockpos); wr.Vector3d(m_dockdir);
		wr.Vector3d(m_dockupdir); wr.Int32(m_state);
	}
	AICmdDock(Serializer::Reader &rd) : AICommand(rd, CMD_DOCK) {
		m_targetIndex = rd.Int32();
		m_dockpos = rd.Vector3d(); m_dockdir = rd.Vector3d();
		m_dockupdir = rd.Vector3d(); m_state = EDockingStates(rd.Int32());
	}
	virtual void PostLoadFixup(Space *space) {
		AICommand::PostLoadFixup(space);
		m_target = static_cast<SpaceStation *>(space->GetBodyByIndex(m_targetIndex));
	}
	virtual void OnDeleted(const Body *body) {
		AICommand::OnDeleted(body);
		if (static_cast<Body *>(m_target) == body) m_target = 0;
	}
private:
	enum EDockingStates {
		eDockGetDataStart = 0,	// 0: get data for docking start pos
		eDockFlyToStart = 1,	// 1: Fly to docking start pos
		eDockGetDataEnd = 2,	// 2: get data for docking end pos
		eDockFlyToEnd = 3,		// 3: Fly to docking end pos
		eDockingComplete = 4,
		eInvalidDockingStage = 5
	};

	SpaceStation *m_target;
	vector3d m_dockpos;	// offset in target's frame
	vector3d m_dockdir;
	vector3d m_dockupdir;
	EDockingStates m_state;		// see TimeStepUpdate()
	int m_targetIndex;	// used during deserialisation

	void IncrementState() {
		switch(m_state) {
		case eDockGetDataStart:		m_state = eDockFlyToStart;		break;	
		case eDockFlyToStart:		m_state = eDockGetDataEnd;		break;	
		case eDockGetDataEnd:		m_state = eDockFlyToEnd;		break;	
		case eDockFlyToEnd:			m_state = eDockingComplete;		break;		
		case eDockingComplete:		m_state = eInvalidDockingStage;	break;	
		case eInvalidDockingStage:	break;	
		}
	}
};

class AICmdFlyTo : public AICommand {
public:
	virtual bool TimeStepUpdate();
	AICmdFlyTo(Ship *ship, Frame *targframe, const vector3d &posoff, double endvel, bool tangent);
	AICmdFlyTo(Ship *ship, Body *target);

	virtual void GetStatusText(char *str) {
		if (m_child) m_child->GetStatusText(str);
		else if (m_target) snprintf(str, 255, "Intercept: %s, dist %.1fkm, state %i",
			m_target->GetLabel().c_str(), m_dist, m_state);
		else snprintf(str, 255, "FlyTo: %s, dist %.1fkm, endvel %.1fkm/s, state %i",
			m_targframe->GetLabel().c_str(), m_posoff.Length()/1000.0, m_endvel/1000.0, m_state);
	}
	virtual void Save(Serializer::Writer &wr) {
		if(m_child) { delete m_child; m_child = 0; }
		AICommand::Save(wr);
		wr.Int32(Pi::game->GetSpace()->GetIndexForBody(m_target));
		wr.Double(m_dist);
		wr.Int32(Pi::game->GetSpace()->GetIndexForFrame(m_targframe));
		wr.Vector3d(m_posoff);
		wr.Double(m_endvel);
		wr.Bool(m_tangent);
		wr.Int32(m_state);
	}
	AICmdFlyTo(Serializer::Reader &rd) : AICommand(rd, CMD_FLYTO) {
		m_targetIndex = rd.Int32();
		m_dist = rd.Double();
		m_targframeIndex = rd.Int32();
		m_posoff = rd.Vector3d();
		m_endvel = rd.Double();
		m_tangent = rd.Bool();
		m_state = rd.Int32();
	}
	virtual void PostLoadFixup(Space *space) {
		AICommand::PostLoadFixup(space);
		m_target = space->GetBodyByIndex(m_targetIndex);
		m_targframe = space->GetFrameByIndex(m_targframeIndex);
		m_lockhead = true;
	}
	virtual void OnDeleted(const Body *body) {
		AICommand::OnDeleted(body);
		if (m_target == body) m_target = 0;
	}

private:
	Body *m_target;		// target for vicinity. Either this or targframe is 0
	double m_dist;		// vicinity distance
	Frame *m_targframe;	// target frame for waypoint
	vector3d m_posoff;	// offset in target frame
	double m_endvel;	// target speed in direction of motion at end of path, positive only
	bool m_tangent;		// true if path is to a tangent of the target frame's body
	int m_state;		

	bool m_lockhead;
	int m_targetIndex, m_targframeIndex;	// used during deserialisation
	vector3d m_reldir;	// target direction relative to ship at last frame change
	Frame *m_frame;		// last frame of ship
};


class AICmdFlyAround : public AICommand {
public:
	virtual bool TimeStepUpdate();
	AICmdFlyAround(Ship *ship, Body *obstructor, double relalt, int mode=2);
	AICmdFlyAround(Ship *ship, Body *obstructor, double alt, double vel, int mode=1);

	virtual void GetStatusText(char *str) {
		if (m_child) m_child->GetStatusText(str);
		else snprintf(str, 255, "FlyAround: alt %.1fkm, vel %.1fkm/s, mode %i",
			m_alt/1000.0, m_vel/1000.0, m_targmode);
	}
	virtual void Save(Serializer::Writer &wr) {
		if (m_child) { delete m_child; m_child = 0; }
		AICommand::Save(wr);
		wr.Int32(Pi::game->GetSpace()->GetIndexForBody(m_obstructor));
		wr.Double(m_vel); wr.Double(m_alt); wr.Int32(m_targmode);
	}
	AICmdFlyAround(Serializer::Reader &rd) : AICommand(rd, CMD_FLYAROUND) {
		m_obstructorIndex = rd.Int32();
		m_vel = rd.Double(); m_alt = rd.Double(); m_targmode = rd.Int32();
	}
	virtual void PostLoadFixup(Space *space) {
		AICommand::PostLoadFixup(space);
		m_obstructor = space->GetBodyByIndex(m_obstructorIndex);
	}
	virtual void OnDeleted(const Body *body) {
		AICommand::OnDeleted(body);
		// check against obstructor?
	}
	void SetTargPos(const vector3d &targpos) { m_targpos = targpos; m_targmode = 0; }

protected:
	void Setup(Body *obstructor, double alt, double vel, int mode);
	double MaxVel(double targdist, double targalt);

private:
	Body *m_obstructor;		// body to fly around
	int m_obstructorIndex;	// deserialisation
	double m_alt, m_vel;
	int m_targmode;			// 0 targpos termination, 1 infinite, 2+ orbital termination
	vector3d m_targpos;		// target position in ship space
};

class AICmdKill : public AICommand {
public:
	virtual bool TimeStepUpdate();
	AICmdKill(Ship *ship, Ship *target) : AICommand (ship, CMD_KILL) {
		m_target = target;
		m_leadTime = m_evadeTime = m_closeTime = 0.0;
		m_lastVel = m_target->GetVelocity();
	}

	// don't actually need to save all this crap
	virtual void Save(Serializer::Writer &wr) {
        Space *space = Pi::game->GetSpace();
		AICommand::Save(wr);
		wr.Int32(space->GetIndexForBody(m_target));
	}
	AICmdKill(Serializer::Reader &rd) : AICommand(rd, CMD_KILL) {
		m_targetIndex = rd.Int32();
	}
	virtual void PostLoadFixup(Space *space) {
		AICommand::PostLoadFixup(space);
		m_target = static_cast<Ship *>(space->GetBodyByIndex(m_targetIndex));
		m_leadTime = m_evadeTime = m_closeTime = 0.0;
		m_lastVel = m_target->GetVelocity();
	}

	virtual void OnDeleted(const Body *body) {
		if (static_cast<Body *>(m_target) == body) m_target = 0;
		AICommand::OnDeleted(body);
	}

private:
	Ship *m_target;
	double m_leadTime, m_evadeTime, m_closeTime;
	vector3d m_leadOffset, m_leadDrift, m_lastVel;
	int m_targetIndex;	// used during deserialisation
};

class AICmdKamikaze : public AICommand {
public:
	virtual bool TimeStepUpdate();
	AICmdKamikaze(Ship *ship, Body *target) : AICommand (ship, CMD_KAMIKAZE) {
		m_target = target;
	}

	virtual void Save(Serializer::Writer &wr) {
        Space *space = Pi::game->GetSpace();
		AICommand::Save(wr);
		wr.Int32(space->GetIndexForBody(m_target));
	}
	AICmdKamikaze(Serializer::Reader &rd) : AICommand(rd, CMD_KAMIKAZE) {
		m_targetIndex = rd.Int32();
	}
	virtual void PostLoadFixup(Space *space) {
		AICommand::PostLoadFixup(space);
		m_target = space->GetBodyByIndex(m_targetIndex);
	}

	virtual void OnDeleted(const Body *body) {
		if (static_cast<Body *>(m_target) == body) m_target = 0;
		AICommand::OnDeleted(body);
	}

private:
	Body *m_target;
	int m_targetIndex;	// used during deserialisation
};

class AICmdHoldPosition : public AICommand {
public:
	virtual bool TimeStepUpdate();
	AICmdHoldPosition(Ship *ship) : AICommand(ship, CMD_HOLDPOSITION) { }
	AICmdHoldPosition(Serializer::Reader &rd) : AICommand(rd, CMD_HOLDPOSITION) { }
};

class AICmdFormation : public AICommand {
public:
	virtual bool TimeStepUpdate();
	AICmdFormation(Ship *ship, Ship *target, const vector3d &posoff);

	virtual void GetStatusText(char *str) {
		if (m_child) m_child->GetStatusText(str);
		else snprintf(str, 255, "Formation: %s, dist %.1fkm",
			m_target->GetLabel().c_str(), m_posoff.Length()/1000.0);
	}
	virtual void Save(Serializer::Writer &wr) {
		if(m_child) { delete m_child; m_child = 0; }
		AICommand::Save(wr);
		wr.Int32(Pi::game->GetSpace()->GetIndexForBody(m_target));
		wr.Vector3d(m_posoff);
	}
	AICmdFormation(Serializer::Reader &rd) : AICommand(rd, CMD_FORMATION) {
		m_targetIndex = rd.Int32();
		m_posoff = rd.Vector3d();
	}
	virtual void PostLoadFixup(Space *space) {
		AICommand::PostLoadFixup(space);
		m_target = static_cast<Ship*>(space->GetBodyByIndex(m_targetIndex));
	}
	virtual void OnDeleted(const Body *body) {
		if (static_cast<Body *>(m_target) == body) m_target = 0;
		AICommand::OnDeleted(body);
	}

private:
	Ship *m_target;		// target frame for waypoint
	vector3d m_posoff;	// offset in target frame

	int m_targetIndex;	// used during deserialisation
};

#endif /* _SHIPAICMD_H */
