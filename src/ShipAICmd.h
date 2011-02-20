#ifndef _SHIPAICMD_H
#define _SHIPAICMD_H

#include "Ship.h"
#include "SpaceStation.h"
#include "Serializer.h"

class AICommand {
public:
	// This enum is solely to make the serialization work
	enum CmdName { CMD_NONE, CMD_JOURNEY, CMD_DOCK, CMD_FLYTO, CMD_KILL, CMD_KAMIKAZE };

	AICommand(Ship *ship, CmdName name) { m_ship = ship; m_cmdName = name; m_child = 0; }
	virtual ~AICommand() { if(m_child) delete m_child; }

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

class AICmdDock : public AICommand {
public:
	virtual bool TimeStepUpdate();
	AICmdDock(Ship *ship, SpaceStation *target) : AICommand (ship, CMD_DOCK) {
		m_target = target;
		m_path.endTime = 0;			// trigger path generation
	}

	virtual void Save(Serializer::Writer &wr) {
		AICommand::Save(wr);
		wr.Int32(Serializer::LookupBody(m_target));
		m_path.Save(wr);
	}
	AICmdDock(Serializer::Reader &rd) : AICommand(rd, CMD_DOCK) {
		m_target = (SpaceStation *)rd.Int32();
		m_path.Load(rd);
	}
	virtual void PostLoadFixup() {
		AICommand::PostLoadFixup();
		m_target = (SpaceStation *)Serializer::LookupBody((size_t)m_target);
		m_path.PostLoadFixup();
	}

	virtual void OnDeleted(const Body *body) {
		if ((Body *)m_target == body) m_target = 0;
		AICommand::OnDeleted(body);
	}

private:
	SpaceStation *m_target;
	AIPath m_path;
};
*/


class AICmdFlyTo : public AICommand {
public:
	virtual bool TimeStepUpdate();
	AICmdFlyTo(Ship *ship, Body *target);					// fly to vicinity
	AICmdFlyTo(Ship *ship, Body *target, double alt);		// orbit
	AICmdFlyTo(Ship *ship, Body *target, vector3d &posoff, double endvel, int headmode, bool coll);

	virtual void Save(Serializer::Writer &wr) {
		AICommand::Save(wr);
		wr.Int32(Serializer::LookupBody(m_target));
		wr.Int32(Serializer::LookupFrame(m_frame));
		wr.Vector3d(m_posoff); wr.Vector3d(m_relpos);
		wr.Double(m_endvel); wr.Double(m_orbitrad);
		wr.Int32(m_headmode);
	}
	AICmdFlyTo(Serializer::Reader &rd) : AICommand(rd, CMD_FLYTO) {
		m_target = (SpaceStation *)rd.Int32();
		m_frame = (Frame *)rd.Int32();
		m_posoff = rd.Vector3d(); m_relpos = rd.Vector3d();
		m_endvel = rd.Double();	m_orbitrad = rd.Double();
		m_headmode = rd.Int32();
	}
	virtual void PostLoadFixup() {
		AICommand::PostLoadFixup();
		m_target = Serializer::LookupBody((size_t)m_target);
		m_frame = Serializer::LookupFrame((size_t)m_frame);
	}

	virtual void OnDeleted(const Body *body) {
		if ((Body *)m_target == body) m_target = 0;
		AICommand::OnDeleted(body);
	}

protected:
	void GetAwayFromBody(Body *body, vector3d &targpos);
	void NavigateAroundBody(Body *body, vector3d &targpos);
	void CheckCollisions(bool coll);

private:
	Body *m_target;
	Frame *m_frame;		// current frame of ship, used to check for changes	
	vector3d m_posoff;	// offset in target's frame
	vector3d m_relpos;	// target position relative to ship at last frame change
	double m_endvel, m_orbitrad;
	int m_headmode;
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

#endif /* _SHIPAICMD_H */