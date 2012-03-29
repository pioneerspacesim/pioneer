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
	enum CmdName { CMD_NONE, CMD_DOCK, CMD_FLYTO, CMD_FLYAROUND, CMD_KILL, CMD_KAMIKAZE, CMD_HOLDPOSITION };

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
	virtual Frame *GetRiskFrame() {
		if (m_child) return m_child->GetRiskFrame();
		return m_ship->GetFrame();			// or local frame?
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
	AICmdDock(Ship *ship, SpaceStation *target) : AICommand(ship, CMD_DOCK) {
		m_target = target;
		m_state = 0;
	}
	virtual void GetStatusText(char *str) { 
		if (m_child) m_child->GetStatusText(str);
		else snprintf(str, 255, "Dock: target %s, state %i", m_target->GetLabel().c_str(), m_state);
	}
	virtual Frame *GetRiskFrame() {
		if (m_child) return m_child->GetRiskFrame();
		return m_target->GetFrame();
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
		m_dockupdir = rd.Vector3d(); m_state = rd.Int32();
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
	SpaceStation *m_target;
	vector3d m_dockpos;	// offset in target's frame
	vector3d m_dockdir;
	vector3d m_dockupdir;
	int m_state;		// see TimeStepUpdate()
	int m_targetIndex;	// used during deserialisation
};


class AICmdFlyTo : public AICommand {
public:
	virtual bool TimeStepUpdate();
	AICmdFlyTo(Ship *ship, Body *target);					// fly to vicinity
	AICmdFlyTo(Ship *ship, Body *target, double alt);		// orbit
	AICmdFlyTo(Ship *ship, Frame *targframe, const vector3d &posoff, double endvel, bool tangent);

	virtual void GetStatusText(char *str) { 
		if (m_child) m_child->GetStatusText(str);
		else snprintf(str, 255, "FlyTo: endvel %.1f, state %i", m_endvel/1000.0, m_state);
	}
	virtual Frame *GetRiskFrame() {
		if (m_child) return m_child->GetRiskFrame();
		return m_targframe;
	}
	virtual void Save(Serializer::Writer &wr) {
        Space *space = Pi::game->GetSpace();
		if(m_child) { delete m_child; m_child = 0; }
		AICommand::Save(wr);
		wr.Int32(space->GetIndexForFrame(m_targframe));
		wr.Vector3d(m_posoff);
		wr.Double(m_endvel);
		wr.Int32(m_state);
		wr.Bool(m_tangent);
	}
	AICmdFlyTo(Serializer::Reader &rd) : AICommand(rd, CMD_FLYTO) {
		m_targframeIndex = rd.Int32();
		m_posoff = rd.Vector3d();
		m_endvel = rd.Double();
		m_state = rd.Int32();
		m_tangent = rd.Bool();
	}
	virtual void PostLoadFixup(Space *space) {
		AICommand::PostLoadFixup(space); m_frame = 0;		// regen
		m_targframe = space->GetFrameByIndex(m_targframeIndex);
	}

private:
	Frame *m_targframe;	// target frame for waypoint
	int m_targframeIndex;	// used during deserialisation
	vector3d m_posoff;	// offset in target frame
	double m_endvel;	// target speed in direction of motion at end of path, positive only
	int m_state;		// 
	bool m_tangent;		// true if path is to a tangent of the target frame's body

	Frame *m_frame;		// current frame of ship, used to check for changes	
	vector3d m_reldir;	// target direction relative to ship at last frame change
};


class AICmdFlyAround : public AICommand {
public:
	virtual bool TimeStepUpdate();
	AICmdFlyAround(Ship *ship, Body *obstructor, double alt);
	AICmdFlyAround(Ship *ship, Body *obstructor, double alt, double vel);
	AICmdFlyAround(Ship *ship, Body *obstructor, double alt, double vel, Body *target, const vector3d &posoff);
	AICmdFlyAround(Ship *ship, Body *obstructor, double alt, double vel, Frame *targframe, const vector3d &posoff);

	virtual void GetStatusText(char *str) { 
		if (m_child) m_child->GetStatusText(str);
		else snprintf(str, 255, "FlyAround: alt %.1f, targmode %i", m_alt/1000.0, m_targmode);
	}	
	virtual Frame *GetRiskFrame() {
		if (m_child) return m_child->GetRiskFrame();
		return m_obstructor->GetFrame();
	}
	virtual void Save(Serializer::Writer &wr) {
        Space *space = Pi::game->GetSpace();
		if (m_child) { delete m_child; m_child = 0; }
		AICommand::Save(wr);
		wr.Int32(space->GetIndexForBody(m_obstructor));
		wr.Double(m_vel); wr.Double(m_alt);
		wr.Int32(m_targmode);
		if (m_targmode == 2) wr.Int32(space->GetIndexForFrame(m_targframe));
		else wr.Int32(space->GetIndexForBody(m_target));
		wr.Vector3d(m_posoff);
	}
	AICmdFlyAround(Serializer::Reader &rd) : AICommand(rd, CMD_FLYAROUND) {
		m_obstructorIndex = rd.Int32();
		m_vel = rd.Double(); m_alt = rd.Double();
		m_targmode = rd.Int32();
		m_targetIndex = rd.Int32();
		m_posoff = rd.Vector3d();
	}
	virtual void PostLoadFixup(Space *space) {
		AICommand::PostLoadFixup(space);
		m_obstructor = space->GetBodyByIndex(m_obstructorIndex);
		if (m_targmode == 2) m_target = space->GetBodyByIndex(m_targetIndex);
		else m_targframe = space->GetFrameByIndex(m_targetIndex);
	}
	virtual void OnDeleted(const Body *body) {
		AICommand::OnDeleted(body);
		if (m_target == body) m_target = 0;
	}

protected:
	void Setup(Body *obstructor, double alt, double vel, int targmode, Body *target, Frame *targframe, const vector3d &posoff);
	double MaxVel(double targdist, double targalt);
	vector3d Targpos();

private:
	Body *m_obstructor;		// body to fly around 
	int m_obstructorIndex;	// deserialisation
	double m_alt, m_vel;

	int m_targmode;			// 0 = no target, 1 = target body, 2 = target waypoint
	Body *m_target;			// target body
	Frame *m_targframe;		// target frame
	int m_targetIndex;		// deserialisation
	vector3d m_posoff;		// target offset from body or frame
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
#endif /* _SHIPAICMD_H */
