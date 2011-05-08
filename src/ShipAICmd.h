#ifndef _SHIPAICMD_H
#define _SHIPAICMD_H

#include "Ship.h"
#include "SpaceStation.h"
#include "Serializer.h"

class AICommand {
public:
	// This enum is solely to make the serialization work
	enum CmdName { CMD_NONE, CMD_JOURNEY, CMD_DOCK, CMD_FLYTO, CMD_KILL, CMD_KAMIKAZE, CMD_HOLDPOSITION };

	AICommand(Ship *ship, CmdName name) { m_ship = ship; m_cmdName = name; m_child = 0; }
	virtual ~AICommand() { if (m_child) delete m_child; }

	virtual bool TimeStepUpdate() = 0;
	bool ProcessChild();				// returns false if child is active

	// Serialisation functions
	static AICommand *Load(Serializer::Reader &rd);
	AICommand(Serializer::Reader &rd, CmdName name);
	virtual void Save(Serializer::Writer &wr);
	virtual void PostLoadFixup();

	// Signal functions
	virtual void OnDeleted(const Body *body) { if (m_child) m_child->OnDeleted(body); }

protected:
	CmdName m_cmdName;	
	Ship *m_ship;
	AICommand *m_child;
};

/*
class AICmdJourney : public AICommand {
public:
	virtual bool TimeStepUpdate();
	AICmdJourney(Ship *ship, SBodyPath &dest) : AICommand(ship, CMD_JOURNEY) {
		m_dest = dest;
	}

	virtual void Save(Serializer::Writer &wr) {
		AICommand::Save(wr);
		m_dest.Serialize(wr);
	}
	AICmdJourney(Serializer::Reader &rd) : AICommand(rd, CMD_JOURNEY) {
		SBodyPath::Unserialize(rd, &m_dest);
	}

private:
	SBodyPath m_dest;
};
*/

class AICmdDock : public AICommand {
public:
	virtual bool TimeStepUpdate();
	AICmdDock(Ship *ship, SpaceStation *target) : AICommand(ship, CMD_DOCK) {
		m_target = target;
		m_state = 0;
	}
	virtual void Save(Serializer::Writer &wr) {
		AICommand::Save(wr);
		wr.Int32(Serializer::LookupBody(m_target));
		wr.Vector3d(m_dockpos); wr.Vector3d(m_dockdir);
		wr.Vector3d(m_dockupdir); wr.Int32(m_state);
	}
	AICmdDock(Serializer::Reader &rd) : AICommand(rd, CMD_DOCK) {
		m_target = (SpaceStation *)rd.Int32();
		m_dockpos = rd.Vector3d(); m_dockdir = rd.Vector3d();
		m_dockupdir = rd.Vector3d(); m_state = rd.Int32();
	}
	virtual void PostLoadFixup() {
		AICommand::PostLoadFixup();
		m_target = (SpaceStation *)Serializer::LookupBody((size_t)m_target);
	}
	virtual void OnDeleted(const Body *body) {
		AICommand::OnDeleted(body);
		if ((Body *)m_target == body) m_target = 0;
	}
private:
	SpaceStation *m_target;
	vector3d m_dockpos;	// offset in target's frame
	vector3d m_dockdir;
	vector3d m_dockupdir;
	int m_state;		// see TimeStepUpdate()
};


class AICmdFlyTo : public AICommand {
public:
	virtual bool TimeStepUpdate();
	AICmdFlyTo(Ship *ship, Body *target);					// fly to vicinity
	AICmdFlyTo(Ship *ship, Body *target, double alt);		// orbit
	AICmdFlyTo(Ship *ship, Frame *targframe, vector3d &posoff, double endvel, int headmode, bool coll);

	virtual void Save(Serializer::Writer &wr) {
		if(m_child) { 
			delete m_child;				// can regen children anyway
			m_child = 0;
		}
		AICommand::Save(wr);
		wr.Int32(Serializer::LookupFrame(m_targframe));
		wr.Vector3d(m_posoff);
		wr.Double(m_endvel); wr.Double(m_orbitrad);
		wr.Int32(m_state); wr.Bool(m_coll);
	}
	AICmdFlyTo(Serializer::Reader &rd) : AICommand(rd, CMD_FLYTO) {
		m_targframe = (Frame *)rd.Int32();
		m_posoff = rd.Vector3d();
		m_endvel = rd.Double();	m_orbitrad = rd.Double();
		m_state = rd.Int32(); m_coll = rd.Bool();
	}
	virtual void PostLoadFixup() {
		AICommand::PostLoadFixup(); m_frame = 0;		// regen
		m_targframe = Serializer::LookupFrame((size_t)m_targframe);
	}

protected:
	void NavigateAroundBody(Body *body, vector3d &targpos);
	void CheckCollisions();
	void CheckSuicide();
	bool OrbitCorrection();
	void SetOrigTarg(Frame *origframe, vector3d &origpos)
		{ m_origframe = origframe; m_origpos = origpos; }

private:
	Frame *m_targframe;	// target frame for waypoint
	vector3d m_posoff;	// offset in target frame
	double m_endvel;	// target speed in direction of motion at end of path, positive only
	double m_orbitrad;	// orbital radius in metres
	int m_state;		// see TimeStepUpdate()
	bool m_coll;		// whether to bother checking for collisions

	Frame *m_frame;		// current frame of ship, used to check for changes	
	vector3d m_reldir;	// target direction relative to ship at last frame change

	Frame *m_origframe;		// original target frame, used for tangent heading 
	vector3d m_origpos;		// original target offset, used for tangent heading
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
		AICommand::Save(wr);
		wr.Int32(Serializer::LookupBody(m_target));
	}
	AICmdKill(Serializer::Reader &rd) : AICommand(rd, CMD_KILL) {
		m_target = (Ship *)rd.Int32();
	}
	virtual void PostLoadFixup() {
		AICommand::PostLoadFixup();
		m_target = (Ship *)Serializer::LookupBody((size_t)m_target);
		m_leadTime = m_evadeTime = m_closeTime = 0.0;
		m_lastVel = m_target->GetVelocity();
	}

	virtual void OnDeleted(const Body *body) {
		if ((Body *)m_target == body) m_target = 0;
		AICommand::OnDeleted(body);
	}

private:
	Ship *m_target;
	double m_leadTime, m_evadeTime, m_closeTime;
	vector3d m_leadOffset, m_leadDrift, m_lastVel;
};

class AICmdKamikaze : public AICommand {
public:
	virtual bool TimeStepUpdate();
	AICmdKamikaze(Ship *ship, Body *target) : AICommand (ship, CMD_KAMIKAZE) {
		m_target = target;
	}

	virtual void Save(Serializer::Writer &wr) {
		AICommand::Save(wr);
		wr.Int32(Serializer::LookupBody(m_target));
	}
	AICmdKamikaze(Serializer::Reader &rd) : AICommand(rd, CMD_KAMIKAZE) {
		m_target = (Body *)rd.Int32();
	}
	virtual void PostLoadFixup() {
		AICommand::PostLoadFixup();
		m_target = Serializer::LookupBody((size_t)m_target);
	}

	virtual void OnDeleted(const Body *body) {
		if ((Body *)m_target == body) m_target = 0;
		AICommand::OnDeleted(body);
	}

private:
	Body *m_target;
};

class AICmdHoldPosition : public AICommand {
public:
	virtual bool TimeStepUpdate();
	AICmdHoldPosition(Ship *ship) : AICommand(ship, CMD_HOLDPOSITION) { }
	AICmdHoldPosition(Serializer::Reader &rd) : AICommand(rd, CMD_HOLDPOSITION) { }
};
#endif /* _SHIPAICMD_H */
