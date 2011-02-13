#ifndef _SHIP_H
#define _SHIP_H

#include "libs.h"
#include "DynamicBody.h"
#include "ShipType.h"
#include "MarketAgent.h"
#include "ShipFlavour.h"
// only for SBodyPath
#include "StarSystem.h"
#include "BezierCurve.h"
#include <list>

class SpaceStation;
class HyperspaceCloud;
class AICommand;

struct shipstats_t {
	int max_capacity;
	int used_capacity;
	int used_cargo;
	int free_capacity;
	int total_mass; // cargo, equipment + hull
	float hull_mass_left; // effectively hitpoints
	float hyperspace_range;
	float hyperspace_range_max;
	float shield_mass;
	float shield_mass_left;
};

struct AIPath {
	BezierCurve path;
	double endTime;
	double startTime;
	Frame *frame;

	void Save(Serializer::Writer &wr) {
		wr.Double(endTime);
		if (endTime == 0.0) return;
		wr.Double(startTime);
		wr.Int32(Serializer::LookupFrame(frame));
		path.Save(wr);
	}
	void Load(Serializer::Reader &rd)
	{
		endTime = rd.Double();
		if (endTime == 0.0) return;
		startTime = rd.Double();
		frame = (Frame *)rd.Int32();
		path.Load(rd);
	}
	void PostLoadFixup() {
		if (endTime == 0.0) return;
		frame = (Frame *)Serializer::LookupFrame((size_t)frame);
	}
};

class Ship: public DynamicBody, public MarketAgent {
public:
	OBJDEF(Ship, DynamicBody, SHIP);
	Ship(ShipType::Type shipType);
	Ship() {}
	virtual void SetDockedWith(SpaceStation *, int port);
	/** Use GetDockedWith() to determine if docked */
	SpaceStation *GetDockedWith() { return m_dockedWith; }
	int GetDockingPort() const { return m_dockedWithPort; }
	void SetNavTarget(Body* const target);
	Body *GetNavTarget() const { return m_navTarget; }
	void SetCombatTarget(Body* const target);
	Body *GetCombatTarget() const { return m_combatTarget; }
	virtual void Render(const vector3d &viewCoords, const matrix4x4d &viewTransform);

	void SetThrusterState(int axis, double level) { m_thrusters[axis] = Clamp(level, -1.0, 1.0); }
	void SetThrusterState(const vector3d &levels);
	vector3d GetThrusterState() const { return m_thrusters; }
	void SetAngThrusterState(int axis, double level) { m_angThrusters[axis] = Clamp(level, -1.0, 1.0); }
	void SetAngThrusterState(const vector3d &levels);
	vector3d GetAngThrusterState() const { return m_angThrusters; }
	void ClearThrusterState();

	vector3d GetMaxThrust(const vector3d &dir);
	void SetGunState(int idx, int state);
	const ShipType &GetShipType() const;
	const shipstats_t *CalcStats();
	void UpdateMass();
	virtual bool SetWheelState(bool down); // returns success of state change, NOT state itself
	void Blastoff();
	bool Undock();
	virtual void TimeStepUpdate(const float timeStep);
	virtual void StaticUpdate(const float timeStep);

	virtual void NotifyDeleted(const Body* const deletedBody);
	virtual bool OnCollision(Object *o, Uint32 flags, double relVel);
	virtual bool OnDamage(Object *attacker, float kgDamage);

	enum FlightState { FLYING, LANDED, DOCKING };
       	FlightState GetFlightState() const { return m_flightState; }
	void SetFlightState(FlightState s) { m_flightState = s; }
	float GetWheelState() const { return m_wheelState; }
	bool Jettison(Equip::Type t);
	const SBodyPath *GetHyperspaceTarget() const { return &m_hyperspace.dest; }
	int GetHyperspaceCloudTargetId() { return m_hyperspace.followHypercloudId; }
	// follow departure cloud
	void SetHyperspaceTarget(HyperspaceCloud *cloud);
	// just jump to near an SBody
	void SetHyperspaceTarget(const SBodyPath *path);
	void ClearHyperspaceTarget();
	void TryHyperspaceTo(const SBodyPath *dest);
	enum HyperjumpStatus {
		HYPERJUMP_OK,
		HYPERJUMP_CURRENT_SYSTEM,
		HYPERJUMP_NO_DRIVE,
		HYPERJUMP_OUT_OF_RANGE,
		HYPERJUMP_INSUFFICIENT_FUEL
	};
	bool CanHyperspaceTo(const SBodyPath *dest, int &outFuelRequired, double &outDurationSecs, enum HyperjumpStatus *outStatus = 0);
	void UseHyperspaceFuel(const SBodyPath *dest);
	float GetHyperspaceCountdown() const { return m_hyperspace.countdown; }
	Equip::Type GetHyperdriveFuelType() const;
	float GetWeakestThrustersForce() const;
	// 0 to 1.0 is alive, > 1.0 = death
	float GetHullTemperature() const;
	void UseECM();

	void AIFaceDirection(const vector3d &dir, double av=0.0);
	vector3d AIGetLeadDir(Body *target, vector3d& targaccel, int gunindex);
	void AISlowOrient(const matrix4x4d &dir);
	void AISlowFaceDirection(const vector3d &dir);
	void AIAccelToModelRelativeVelocity(const vector3d v);
	void AIModelCoordsMatchAngVel(vector3d desiredAngVel, float softness);
	void AIModelCoordsMatchSpeedRelTo(const vector3d v, const Ship *);
	void AITrySetBodyRelativeThrust(const vector3d &force);

	bool AIAddAvoidancePathOnWayTo(const Body *target, AIPath &);
	bool AIArePlanetsInTheWayOfGettingTo(const vector3d &target, Body **obstructor, double &outDist);
	bool AIFollowPath(AIPath &, bool pointShipAtVelocityVector = false);

	void AIClearInstructions();
	bool AIIsActive() { return m_curAICmd ? true : false; }

	void AIKamikaze(Body *target);
	void AIKill(Ship *target);
	void AIJourney(SBodyPath &dest);
	void AIDock(SpaceStation *target);
	void AIFlyTo(Body *target);
	void AIOrbit(Body *target, double alt);

	void AIBodyDeleted(const Body* const body) {};		// todo: signals

	EquipSet m_equipment;			// shouldn't be public?...

	virtual void PostLoadFixup();
	/* MarketAgent stuff */
	int GetStock(Equip::Type t) const { assert(0); return 0; }
	bool CanBuy(Equip::Type t, bool verbose) const;
	bool CanSell(Equip::Type t, bool verbose) const;
	bool DoesSell(Equip::Type t) const { return true; }
	Sint64 GetPrice(Equip::Type t) const;

	void ChangeFlavour(const ShipFlavour *f);
	const ShipFlavour *GetFlavour() const { return &m_shipFlavour; }
	float GetPercentShields() const;
	float GetPercentHull() const;
	void SetPercentHull(float);
	float GetGunTemperature(int idx) const { return m_gunTemperature[idx]; }
	
	sigc::signal<void> onDock;				// JJ: check what these are for
	sigc::signal<void> onUndock;
protected:
	virtual void Save(Serializer::Writer &wr);
	virtual void Load(Serializer::Reader &rd);
	void RenderLaserfire();

	bool AITimeStep(float timeStep);		// returns true if complete

	SpaceStation *m_dockedWith;
	int m_dockedWithPort;
	ShipFlavour m_shipFlavour;
	Uint32 m_gunState[ShipType::GUNMOUNT_MAX];
	float m_gunRecharge[ShipType::GUNMOUNT_MAX];
	float m_gunTemperature[ShipType::GUNMOUNT_MAX];
	float m_ecmRecharge;
	/* MarketAgent stuff */
	void Bought(Equip::Type t);
	void Sold(Equip::Type t);

private:
	float GetECMRechargeTime();
	void FireWeapon(int num);
	void Init();
	bool IsFiringLasers();
	void TestLanded();

	FlightState m_flightState;
	bool m_testLanded;
	float m_launchLockTimeout;
	float m_wheelState;
	float m_wheelTransition;

	vector3d m_thrusters;
	vector3d m_angThrusters;
	Body* m_navTarget;
	Body* m_combatTarget;
	shipstats_t m_stats;

	struct HyperspacingOut {
		int followHypercloudId;
		SBodyPath dest;
		// > 0 means active
		float countdown;
	} m_hyperspace;

	AICommand *m_curAICmd;

};



#endif /* _SHIP_H */



/* Temporary stuff to put back

//		printf("AI '%s' successfully executed %d\n", GetLabel().c_str(), m_todo.front().cmd);
		m_todo.pop_front();
		// Finished autopilot program so fall out of time accel
		if ((this == static_cast<Ship*>(Pi::player)) && (m_todo.size() == 0)) {
			// doesn't happen until next game tick, which is good
			// because AI will have set thrusters assuming a
			// particular timestep

	if (m_todo.size() != 0) {
		AIInstruction &inst = m_todo.front();
		switch (inst.cmd) {
			case DO_DOCK:
				done = AICmdDock(inst, static_cast<SpaceStation*>(inst.target));
				break;
			case DO_KAMIKAZE:
				done = AICmdKamikaze(static_cast<const Ship*>(inst.target));
				break;
			case DO_KILL:
				done = AICmdKill(inst, timeStep);
				break;
			case DO_LOW_ORBIT:
				done = AICmdOrbit(inst, 1.1);
				break;
			case DO_MEDIUM_ORBIT:
				done = AICmdOrbit(inst, 2.0);
				break;
			case DO_HIGH_ORBIT:
				done = AICmdOrbit(inst, 5.0);
				break;
			case DO_FLY_TO:
				done = AICmdFlyTo(inst);
				break;
			case DO_FOLLOW_PATH:
				done = AIFollowPath(inst, inst.frame, true);
				break;
			case DO_JOURNEY:
				done = AICmdJourney(inst);
				break;
			case DO_NOTHING: done = true; break;
		}

	for (std::list<AIInstruction>::iterator i = m_todo.begin(); i != m_todo.end(); ++i) {
		wr.Int32((int)(*i).cmd);
		switch ((*i).cmd) {
			case DO_DOCK:
			case DO_KILL:
			case DO_KAMIKAZE:
			case DO_FLY_TO:
			case DO_LOW_ORBIT:
			case DO_MEDIUM_ORBIT:
			case DO_HIGH_ORBIT:
			case DO_FOLLOW_PATH:
				wr.Int32(Serializer::LookupBody((*i).target));
				{
					int n = (*i).path.p.size();
					wr.Int32(n);
					for (int j=0; j<n; j++) {
						wr.Vector3d((*i).path.p[j]);
					}
				}
				wr.Double((*i).endTime);
				wr.Double((*i).startTime);
				wr.Int32(Serializer::LookupFrame((*i).frame));
				break;
			case DO_JOURNEY:
				(*i).journeyDest.Serialize(wr);
				break;
			case DO_NOTHING: wr.Int32(0); break;
		}
	}

	int num = rd.Int32();
	while (num-- > 0) {
		AICommand c = (AICommand)rd.Int32();
		AIInstruction inst = AIInstruction(c);
		switch (c) {
		case DO_DOCK:
		case DO_KILL:
		case DO_KAMIKAZE:
		case DO_FLY_TO:
		case DO_LOW_ORBIT:
		case DO_MEDIUM_ORBIT:
		case DO_HIGH_ORBIT:
		case DO_FOLLOW_PATH:
			{
				Body *target = (Body*)rd.Int32();
				inst.target = target;
				int n = rd.Int32();
				inst.path = BezierCurve(n);
				for (int i=0; i<n; i++) {
					inst.path.p[i] = rd.Vector3d();
				}
				inst.endTime = rd.Double();
				inst.startTime = rd.Double();
				if (rd.StreamVersion() < 19) {
					inst.frame = 0;
				} else {
					inst.frame = Serializer::LookupFrame(rd.Int32());
				}
			}
			break;
		case DO_JOURNEY:
			SBodyPath::Unserialize(rd, &inst.journeyDest);
			break;
		case DO_NOTHING:
			rd.Int32();
			break;
		}
		m_todo.push_back(inst);
	}
*/
