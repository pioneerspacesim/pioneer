// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SPACESTATION_H
#define _SPACESTATION_H

#include "libs.h"
#include "Camera.h"
#include "MarketAgent.h"
#include "ModelBody.h"
#include "NavLights.h"
#include "Quaternion.h"
#include "RefList.h"
#include "Serializer.h"
#include "ShipType.h"
#include "SpaceStationType.h"
#include "scenegraph/ModelSkin.h"

#define MAX_DOCKING_PORTS		240	//256-(0x10), 0x10 is used because the collision surfaces use it as an identifying flag


class CityOnPlanet;
class CollMeshSet;
class FormController;
class Planet;
class Ship;
class SpaceStation;
class StationAdvertForm;
class SystemBody;
struct BBAdvert;
struct Mission;
namespace Graphics { class Renderer; }
namespace SceneGraph { class Animation; }

typedef StationAdvertForm* (*AdvertFormBuilder)(FormController *controller, SpaceStation *station, const BBAdvert &ad);

struct BBAdvert {
	int               ref;
	std::string       description;
	AdvertFormBuilder builder;
};

struct ShipOnSale {
	ShipOnSale(const ShipType::Id &_id, const std::string &_regId, const SceneGraph::ModelSkin &_skin) :
		id(_id), regId(_regId), skin(_skin) {}
	ShipType::Id          id;
	std::string           regId;
	SceneGraph::ModelSkin skin;
};

class SpaceStation: public ModelBody, public MarketAgent {
public:
	OBJDEF(SpaceStation, ModelBody, SPACESTATION);
	static void Init();
	static void Uninit();

	// Should point to SystemBody in Pi::currentSystem
	SpaceStation(const SystemBody *);
	SpaceStation() {}
	virtual ~SpaceStation();
	virtual vector3d GetAngVelocity() const { return vector3d(0,m_type->angVel,0); }
	virtual bool OnCollision(Object *b, Uint32 flags, double relVel);
	virtual void Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform);
	virtual void StaticUpdate(const float timeStep);
	virtual void TimeStepUpdate(const float timeStep);

	void AddEquipmentStock(Equip::Type t, int num) { m_equipmentStock[t] += num; }
	/* MarketAgent stuff */
	int GetStock(Equip::Type t) const { return m_equipmentStock[t]; }
	Sint64 GetPrice(Equip::Type t) const;
	bool CanBuy(Equip::Type t, bool verbose) const;
	bool CanSell(Equip::Type t, bool verbose) const;
	bool DoesSell(Equip::Type t) const;
	virtual const SystemBody *GetSystemBody() const { return m_sbody; }
	void ReplaceShipOnSale(int idx, const ShipOnSale &with);
	const std::vector<ShipOnSale> &GetShipsOnSale() const { return m_shipsOnSale; }
	virtual void PostLoadFixup(Space *space);
	virtual void NotifyRemoved(const Body* const removedBody);

	// should call Ship::Undock and Ship::SetDockedWith instead
	// Returns true on success, false if permission denied
	bool LaunchShip(Ship *ship, int port);
	void SetDocked(Ship *ship, int port);

	bool GetDockingClearance(Ship *s, std::string &outMsg);
	int GetDockingPortCount() const { return m_type->numDockingPorts; }
	int GetFreeDockingPort() const; // returns -1 if none free
	int GetMyDockingPort(const Ship *s) const;

	const SpaceStationType *GetStationType() const { return m_type; }
	bool IsGroundStation() const;

	sigc::signal<void> onShipsForSaleChanged;
	sigc::signal<void, BBAdvert&> onBulletinBoardAdvertDeleted;
	sigc::signal<void> onBulletinBoardChanged;
	sigc::signal<void> onBulletinBoardDeleted;

	bool AllocateStaticSlot(int& slot);

	void CreateBB();
	int AddBBAdvert(std::string description, AdvertFormBuilder builder);
	const BBAdvert *GetBBAdvert(int ref);
	bool RemoveBBAdvert(int ref);
	const std::list<const BBAdvert*> GetBBAdverts();

	// use docking bay position, if player has been granted permission
	virtual vector3d GetTargetIndicatorPosition(const Frame *relTo) const;

	// need this now because stations rotate in their frame
	virtual void UpdateInterpTransform(double alpha);

protected:
	virtual void Save(Serializer::Writer &wr, Space *space);
	virtual void Load(Serializer::Reader &rd, Space *space);
	/* MarketAgent stuff */
	void Bought(Equip::Type t);
	void Sold(Equip::Type t);

private:
	void DockingUpdate(const double timeStep);
	void PositionDockedShip(Ship *ship, int port) const;
	void DoLawAndOrder(const double timeStep);
	bool IsPortLocked(const int bay) const;
	void LockPort(const int bay, const bool lockIt);

	/* Stage 0 means docking port empty
	 * Stage 1 means docking clearance granted to ->ship
	 * Stage 2 to m_type->numDockingStages is docking animation
	 * Stage m_type->numDockingStages+1 means ship is docked
	 * Stage -1 to -m_type->numUndockStages is undocking animation
	 */
	struct shipDocking_t {
		shipDocking_t() : ship(NULL), shipIndex(0),
			stage(0), stagePos(0)
		{}

		Ship *ship;
		int shipIndex; // deserialisation
		int stage;
		vector3d fromPos; // in station model coords
		Quaterniond fromRot;
		double stagePos; // 0 -> 1.0
	};
	typedef std::vector<shipDocking_t>::const_iterator	constShipDockingIter;
	typedef std::vector<shipDocking_t>::iterator		shipDockingIter;
	std::vector<shipDocking_t> m_shipDocking;

	SpaceStationType::TBayGroups mBayGroups;

	double m_oldAngDisplacement;

	void InitStation();
	void UpdateShipyard();
	const SpaceStationType *m_type;
	const SystemBody *m_sbody;
	int m_equipmentStock[Equip::TYPE_MAX];
	std::vector<ShipOnSale> m_shipsOnSale;
	double m_lastUpdatedShipyard;
	CityOnPlanet *m_adjacentCity;
	double m_distFromPlanet;
	int m_numPoliceDocked;
	enum { NUM_STATIC_SLOTS = 4 };
	bool m_staticSlot[NUM_STATIC_SLOTS];

	std::vector<BBAdvert> m_bbAdverts;
	bool m_bbCreated, m_bbShuffled;

	SceneGraph::Animation *m_doorAnimation;
	double m_doorAnimationStep;
	double m_doorAnimationState;

	ScopedPtr<NavLights> m_navLights;
};

#endif /* _SPACESTATION_H */
