#ifndef _SPACESTATION_H
#define _SPACESTATION_H

#include "libs.h"
#include "ModelBody.h"
#include "ShipType.h"
#include "MarketAgent.h"
#include "ShipFlavour.h"

#define MAX_DOCKING_PORTS	4

class CollMeshSet;
class Ship;
class Mission;
class CityOnPlanet;

class SBody;

class SpaceStation: public ModelBody, public MarketAgent {
public:
	OBJDEF(SpaceStation, ModelBody, SPACESTATION);
	enum TYPE { JJHOOP, GROUND_FLAVOURED, TYPE_MAX };
	// Should point to SBody in Pi::currentSystem
	SpaceStation(const SBody *);
	SpaceStation() {}
	virtual ~SpaceStation();
	virtual bool OnCollision(Body *b, Uint32 flags);
	virtual void Render(const Frame *camFrame);
	void OrientLaunchingShip(Ship *ship, int port) const;
	void OrientDockedShip(Ship *ship, int port) const;
	void GetDockingSurface(const CollMeshSet *mset, int midx);
	bool GetDockingClearance(Ship *s);
	virtual void TimeStepUpdate(const float timeStep);
	bool IsGroundStation() const;
	struct dockingport_t {
		vector3d center;
		vector3d normal;
		vector3d horiz;
	} port[MAX_DOCKING_PORTS];
	int GetEquipmentStock(Equip::Type t) const { return m_equipmentStock[t]; }
	void AddEquipmentStock(Equip::Type t, int num) { m_equipmentStock[t] += num; }
	/* MarketAgent stuff */
	int GetPrice(Equip::Type t) const;
	bool CanBuy(Equip::Type t) const;
	bool CanSell(Equip::Type t) const;
	const SBody *GetSBody() const { return m_sbody; }
	void ReplaceShipOnSale(int idx, const ShipFlavour *with);
	std::vector<ShipFlavour> &GetShipsOnSale() { return m_shipsOnSale; }
	const std::vector<Mission*> &GetBBMissions() { return m_bbmissions; }
	// does not dealloc
	bool BBRemoveMission(Mission *m);
	sigc::signal<void> onShipsForSaleChanged;
	sigc::signal<void> onBulletinBoardChanged;
protected:
	virtual void Save();
	virtual void Load();
	/* MarketAgent stuff */
	void Bought(Equip::Type t);
	void Sold(Equip::Type t);
private:
	void UpdateShipyard();
	void UpdateBB();
	void Init();
	TYPE m_type;
	const SBody *m_sbody;
	int m_numPorts;
	int m_equipmentStock[Equip::TYPE_MAX];
	std::vector<ShipFlavour> m_shipsOnSale;
	std::vector<Mission*> m_bbmissions;
	double m_lastUpdatedShipyard;
	CityOnPlanet *m_adjacentCity;
};

#endif /* _SPACESTATION_H */
