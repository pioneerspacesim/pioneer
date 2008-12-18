#ifndef _SPACESTATION_H
#define _SPACESTATION_H

#include "libs.h"
#include "ModelBody.h"
#include "ShipType.h"
#include "MarketAgent.h"

#define MAX_DOCKING_PORTS	4

class CollMeshSet;
class Ship;

class SpaceStation: public ModelBody, public MarketAgent {
public:
	OBJDEF(SpaceStation, ModelBody, SPACESTATION);
	enum TYPE { JJHOOP, GROUND_FLAVOURED, TYPE_MAX };
	SpaceStation(TYPE);
	SpaceStation() {}
	virtual ~SpaceStation();
	virtual bool OnCollision(Body *b, Uint32 flags);
	virtual void Render(const Frame *camFrame);
	void OrientLaunchingShip(Ship *ship, int port) const;
	void OrientDockedShip(Ship *ship, int port) const;
	void GetDockingSurface(CollMeshSet *mset, int midx);
	bool GetDockingClearance(Ship *s);
	bool IsGroundStation() const;
	struct dockingport_t {
		vector3d center;
		vector3d normal;
		vector3d horiz;
	} port[MAX_DOCKING_PORTS];
	int GetEquipmentStock(Equip::Type t) const { return m_equipmentStock[t]; }
	/* MarketAgent stuff */
	int GetPrice(Equip::Type t) const;
	bool CanBuy(Equip::Type t) const;
	bool CanSell(Equip::Type t) const;
protected:
	virtual void Save();
	virtual void Load();
	/* MarketAgent stuff */
	void Bought(Equip::Type t);
	void Sold(Equip::Type t);
private:
	void Init();
	TYPE m_type;
	int m_numPorts;
	int m_equipmentStock[Equip::TYPE_MAX];
};

#endif /* _SPACESTATION_H */
