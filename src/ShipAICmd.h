// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SHIPAICMD_H
#define _SHIPAICMD_H

#include "Ship.h"
#include "SpaceStation.h"
#include "Serializer.h"
#include "Pi.h"
#include "Game.h"
#include "json/JsonUtils.h"
#include "libs.h"

class AICommand {
public:
	// This enum is solely to make the serialization work
	enum CmdName { CMD_NONE, CMD_DOCK, CMD_FLYTO, CMD_FLYAROUND, CMD_KILL, CMD_KAMIKAZE, CMD_HOLDPOSITION, CMD_FORMATION };

	AICommand(DynamicBody *dBody, CmdName name):
		m_dBody(dBody), m_cmdName(name) {
		m_dBody->AIMessage(DynamicBody::AIERROR_NONE);
		m_prop = nullptr;
		m_fguns = nullptr;
	}
	virtual ~AICommand() {}

	virtual bool TimeStepUpdate() = 0;
	bool ProcessChild();				// returns false if child is active
	virtual void GetStatusText(char *str) {
		if (m_child) m_child->GetStatusText(str);
		else strcpy(str, "AI state unknown");
	}

	// Serialisation functions
	static AICommand *LoadFromJson(const Json::Value &jsonObj);
	AICommand(const Json::Value &jsonObj, CmdName name);
	virtual void SaveToJson(Json::Value &jsonObj);
	virtual void PostLoadFixup(Space *space);

	// Signal functions
	virtual void OnDeleted(const Body *body) { if (m_child) m_child->OnDeleted(body); }

protected:
	DynamicBody *m_dBody;
	Propulsion *m_prop;
	FixedGuns *m_fguns;

	std::unique_ptr<AICommand> m_child;
	CmdName m_cmdName;

	int m_dBodyIndex; // deserialisation
};

class AICmdDock : public AICommand {
public:
	virtual bool TimeStepUpdate();
	AICmdDock(DynamicBody *dBody, SpaceStation *target);

	virtual void GetStatusText(char *str) {
		if (m_child) m_child->GetStatusText(str);
		else snprintf(str, 255, "Dock: target %s, state %i", m_target->GetLabel().c_str(), m_state);
	}
	virtual void SaveToJson(Json::Value &jsonObj) {
		Space *space = Pi::game->GetSpace();
		Json::Value aiCommandObj(Json::objectValue); // Create JSON object to contain ai command data.
		AICommand::SaveToJson(aiCommandObj);
		aiCommandObj["index_for_target"] = space->GetIndexForBody(m_target);
		VectorToJson(aiCommandObj, m_dockpos, "dock_pos");
		VectorToJson(aiCommandObj, m_dockdir, "dock_dir");
		VectorToJson(aiCommandObj, m_dockupdir, "dock_up_dir");
		aiCommandObj["state"] = m_state;
		jsonObj["ai_command"] = aiCommandObj; // Add ai command object to supplied object.
	}
	AICmdDock(const Json::Value &jsonObj) : AICommand(jsonObj, CMD_DOCK) {
		if (!jsonObj.isMember("index_for_target")) throw SavedGameCorruptException();
		if (!jsonObj.isMember("dock_pos")) throw SavedGameCorruptException();
		if (!jsonObj.isMember("dock_dir")) throw SavedGameCorruptException();
		if (!jsonObj.isMember("dock_up_dir")) throw SavedGameCorruptException();
		if (!jsonObj.isMember("state")) throw SavedGameCorruptException();
		m_targetIndex = jsonObj["index_for_target"].asInt();
		JsonToVector(&m_dockpos, jsonObj, "dock_pos");
		JsonToVector(&m_dockdir, jsonObj, "dock_dir");
		JsonToVector(&m_dockupdir, jsonObj, "dock_up_dir");
		m_state = EDockingStates(jsonObj["state"].asInt());
	}
	virtual void PostLoadFixup(Space *space) {
		AICommand::PostLoadFixup(space);
		m_target = static_cast<SpaceStation *>(space->GetBodyByIndex(m_targetIndex));
		// Ensure needed sub-system:
		m_prop = dynamic_cast<Propulsion*>(m_dBody);
		assert(m_prop!=nullptr);
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
	AICmdFlyTo(DynamicBody *dBody, Frame *targframe, const vector3d &posoff, double endvel, bool tangent);
	AICmdFlyTo(DynamicBody *dBody, Body *target);

	virtual void GetStatusText(char *str) {
		if (m_child) m_child->GetStatusText(str);
		else if (m_target) snprintf(str, 255, "Intercept: %s, dist %.1fkm, state %i",
			m_target->GetLabel().c_str(), m_dist, m_state);
		else snprintf(str, 255, "FlyTo: %s, dist %.1fkm, endvel %.1fkm/s, state %i",
			m_targframe->GetLabel().c_str(), m_posoff.Length()/1000.0, m_endvel/1000.0, m_state);
	}
	virtual void SaveToJson(Json::Value &jsonObj) {
		if (m_child) { m_child.reset(); }
		Json::Value aiCommandObj(Json::objectValue); // Create JSON object to contain ai command data.
		AICommand::SaveToJson(aiCommandObj);
		aiCommandObj["index_for_target"] = Pi::game->GetSpace()->GetIndexForBody(m_target);
		aiCommandObj["dist"] = DoubleToStr(m_dist);
		aiCommandObj["index_for_target_frame"] = Pi::game->GetSpace()->GetIndexForFrame(m_targframe);
		VectorToJson(aiCommandObj, m_posoff, "pos_off");
		aiCommandObj["end_vel"] = DoubleToStr(m_endvel);
		aiCommandObj["tangent"] = m_tangent;
		aiCommandObj["state"] = m_state;
		jsonObj["ai_command"] = aiCommandObj; // Add ai command object to supplied object.
	}
	AICmdFlyTo(const Json::Value &jsonObj) : AICommand(jsonObj, CMD_FLYTO) {
		if (!jsonObj.isMember("index_for_target")) throw SavedGameCorruptException();
		if (!jsonObj.isMember("dist")) throw SavedGameCorruptException();
		if (!jsonObj.isMember("index_for_target_frame")) throw SavedGameCorruptException();
		if (!jsonObj.isMember("pos_off")) throw SavedGameCorruptException();
		if (!jsonObj.isMember("end_vel")) throw SavedGameCorruptException();
		if (!jsonObj.isMember("tangent")) throw SavedGameCorruptException();
		if (!jsonObj.isMember("state")) throw SavedGameCorruptException();
		m_targetIndex = jsonObj["index_for_target"].asInt();
		m_dist = StrToDouble(jsonObj["dist"].asString());
		m_targframeIndex = jsonObj["index_for_target_frame"].asInt();
		JsonToVector(&m_posoff, jsonObj, "pos_off");
		m_endvel = StrToDouble(jsonObj["end_vel"].asString());
		m_tangent = jsonObj["tangent"].asBool();
		m_state = jsonObj["state"].asInt();
	}
	virtual void PostLoadFixup(Space *space) {
		AICommand::PostLoadFixup(space);
		m_target = space->GetBodyByIndex(m_targetIndex);
		m_targframe = space->GetFrameByIndex(m_targframeIndex);
		m_lockhead = true;
		// Ensure needed sub-system:
		m_prop = dynamic_cast<Propulsion*>(m_dBody);
		assert(m_prop!=nullptr);
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
	AICmdFlyAround(DynamicBody *dBody, Body *obstructor, double relalt, int mode=2);
	AICmdFlyAround(DynamicBody *dBody, Body *obstructor, double alt, double vel, int mode=1);

	virtual void GetStatusText(char *str) {
		if (m_child) m_child->GetStatusText(str);
		else snprintf(str, 255, "FlyAround: alt %.1fkm, vel %.1fkm/s, mode %i",
			m_alt/1000.0, m_vel/1000.0, m_targmode);
	}
	virtual void SaveToJson(Json::Value &jsonObj) {
		if (m_child) { m_child.reset(); }
		Json::Value aiCommandObj(Json::objectValue); // Create JSON object to contain ai command data.
		AICommand::SaveToJson(aiCommandObj);
		aiCommandObj["index_for_obstructor"] = Pi::game->GetSpace()->GetIndexForBody(m_obstructor);
		aiCommandObj["vel"] = DoubleToStr(m_vel);
		aiCommandObj["alt"] = DoubleToStr(m_alt);
		aiCommandObj["targ_mode"] = m_targmode;
		jsonObj["ai_command"] = aiCommandObj; // Add ai command object to supplied object.
	}
	AICmdFlyAround(const Json::Value &jsonObj) : AICommand(jsonObj, CMD_FLYAROUND) {
		if (!jsonObj.isMember("index_for_obstructor")) throw SavedGameCorruptException();
		if (!jsonObj.isMember("vel")) throw SavedGameCorruptException();
		if (!jsonObj.isMember("alt")) throw SavedGameCorruptException();
		if (!jsonObj.isMember("targ_mode")) throw SavedGameCorruptException();
		m_obstructorIndex = jsonObj["index_for_obstructor"].asInt();
		m_vel = StrToDouble(jsonObj["vel"].asString());
		m_alt = StrToDouble(jsonObj["alt"].asString());
		m_targmode = jsonObj["targ_mode"].asInt();
	}
	virtual void PostLoadFixup(Space *space) {
		AICommand::PostLoadFixup(space);
		m_obstructor = space->GetBodyByIndex(m_obstructorIndex);
		// Ensure needed sub-system:
		m_prop = dynamic_cast<Propulsion*>(m_dBody);
		assert(m_prop!=nullptr);
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
	AICmdKill(DynamicBody *dBody, Ship *target) : AICommand (dBody, CMD_KILL) {
		m_target = target;
		m_leadTime = m_evadeTime = m_closeTime = 0.0;
		m_lastVel = m_target->GetVelocity();
		m_prop = dynamic_cast<Propulsion*>(m_dBody);
		m_fguns = dynamic_cast<FixedGuns*>(m_dBody);
		assert(m_prop!=nullptr);
		assert(m_fguns!=nullptr);
	}

	// don't actually need to save all this crap
	virtual void SaveToJson(Json::Value &jsonObj) {
		Space *space = Pi::game->GetSpace();
		Json::Value aiCommandObj(Json::objectValue); // Create JSON object to contain ai command data.
		AICommand::SaveToJson(aiCommandObj);
		aiCommandObj["index_for_target"] = space->GetIndexForBody(m_target);
		jsonObj["ai_command"] = aiCommandObj; // Add ai command object to supplied object.
	}
	AICmdKill(const Json::Value &jsonObj) : AICommand(jsonObj, CMD_KILL) {
		if (!jsonObj.isMember("index_for_target")) throw SavedGameCorruptException();
		m_targetIndex = jsonObj["index_for_target"].asInt();
	}

	virtual void PostLoadFixup(Space *space) {
		AICommand::PostLoadFixup(space);
		m_target = static_cast<Ship *>(space->GetBodyByIndex(m_targetIndex));
		m_leadTime = m_evadeTime = m_closeTime = 0.0;
		m_lastVel = m_target->GetVelocity();
		// Ensure needed sub-system:
		m_prop = dynamic_cast<Propulsion*>(m_dBody);
		m_fguns = dynamic_cast<FixedGuns*>(m_dBody);
		assert(m_prop!=nullptr);
		assert(m_fguns!=nullptr);
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
	AICmdKamikaze(DynamicBody *dBody, Body *target) : AICommand(dBody, CMD_KAMIKAZE) {
		m_target = target;
		m_prop = dynamic_cast<Propulsion*>(m_dBody);
		assert(m_prop!=nullptr);
	}

	virtual void SaveToJson(Json::Value &jsonObj) {
		Space *space = Pi::game->GetSpace();
		Json::Value aiCommandObj(Json::objectValue); // Create JSON object to contain ai command data.
		AICommand::SaveToJson(aiCommandObj);
		aiCommandObj["index_for_target"] = space->GetIndexForBody(m_target);
		jsonObj["ai_command"] = aiCommandObj; // Add ai command object to supplied object.
	}
	AICmdKamikaze(const Json::Value &jsonObj) : AICommand(jsonObj, CMD_KAMIKAZE) {
		if (!jsonObj.isMember("index_for_target")) throw SavedGameCorruptException();
		m_targetIndex = jsonObj["index_for_target"].asInt();
	}
	virtual void PostLoadFixup(Space *space) {
		AICommand::PostLoadFixup(space);
		m_target = space->GetBodyByIndex(m_targetIndex);
		// Ensure needed sub-system:
		m_prop = dynamic_cast<Propulsion*>(m_dBody);
		assert(m_prop!=nullptr);
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
	AICmdHoldPosition(DynamicBody *dBody) : AICommand(dBody, CMD_HOLDPOSITION) {
		Propulsion *prop = dynamic_cast<Propulsion*>(m_dBody);
		assert(prop!=0);
	}
	AICmdHoldPosition(const Json::Value &jsonObj) : AICommand(jsonObj, CMD_HOLDPOSITION) {
		// Ensure needed sub-system:
		m_prop = dynamic_cast<Propulsion*>(m_dBody);
		assert(m_prop!=nullptr);
	}
};

class AICmdFormation : public AICommand {
public:
	virtual bool TimeStepUpdate();
	AICmdFormation(DynamicBody *dBody, DynamicBody *target, const vector3d &posoff);

	virtual void GetStatusText(char *str) {
		if (m_child) m_child->GetStatusText(str);
		else snprintf(str, 255, "Formation: %s, dist %.1fkm",
			m_target->GetLabel().c_str(), m_posoff.Length()/1000.0);
	}
	virtual void SaveToJson(Json::Value &jsonObj) {
		if (m_child) { m_child.reset(); }
		Json::Value aiCommandObj(Json::objectValue); // Create JSON object to contain ai command data.
		AICommand::SaveToJson(aiCommandObj);
		aiCommandObj["index_for_target"] = Pi::game->GetSpace()->GetIndexForBody(m_target);
		VectorToJson(aiCommandObj, m_posoff, "pos_off");
		jsonObj["ai_command"] = aiCommandObj; // Add ai command object to supplied object.
	}
	AICmdFormation(const Json::Value &jsonObj) : AICommand(jsonObj, CMD_FORMATION) {
		if (!jsonObj.isMember("index_for_target")) throw SavedGameCorruptException();
		if (!jsonObj.isMember("pos_off")) throw SavedGameCorruptException();
		m_targetIndex = jsonObj["index_for_target"].asInt();
		JsonToVector(&m_posoff, jsonObj, "pos_off");
	}
	virtual void PostLoadFixup(Space *space) {
		AICommand::PostLoadFixup(space);
		m_target = static_cast<Ship*>(space->GetBodyByIndex(m_targetIndex));
		// Ensure needed sub-system:
		m_prop = dynamic_cast<Propulsion*>(m_dBody);
		assert(m_prop!=nullptr);
	}
	virtual void OnDeleted(const Body *body) {
		if (static_cast<Body *>(m_target) == body) m_target = 0;
		AICommand::OnDeleted(body);
	}

private:
	DynamicBody *m_target;	// target frame for waypoint
	vector3d m_posoff;	// offset in target frame

	int m_targetIndex;	// used during deserialisation
};

#endif /* _SHIPAICMD_H */
