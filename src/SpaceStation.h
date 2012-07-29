#ifndef _SPACESTATION_H
#define _SPACESTATION_H

#include "libs.h"
#include "ModelBody.h"
#include "ShipType.h"
#include "MarketAgent.h"
#include "ShipFlavour.h"
#include "Quaternion.h"
#include "Serializer.h"
#include "RefList.h"
#include "Camera.h"

#define MAX_DOCKING_PORTS	4

class CollMeshSet;
class Ship;
struct Mission;
class Planet;
class CityOnPlanet;
namespace Graphics { class Renderer; }

struct SpaceStationType {
	LmrModel *model;
	const char *modelName;
	float angVel;
	enum DOCKMETHOD { SURFACE, ORBITAL } dockMethod;
	int numDockingPorts;
	int numDockingStages;
	int numUndockStages;
	double *dockAnimStageDuration;
	double *undockAnimStageDuration;
	bool dockOneAtATimePlease;

	struct positionOrient_t {
		vector3d pos;
		vector3d xaxis;
		vector3d yaxis;
	};

	void _ReadStageDurations(const char *key, int *outNumStages, double **durationArray);
	// read from lua model definition
	void ReadStageDurations();
	bool GetShipApproachWaypoints(int port, int stage, positionOrient_t &outPosOrient) const;
	/** when ship is on rails it returns true and fills outPosOrient.
	 * when ship has been released (or docked) it returns false.
	 * Note station animations may continue for any number of stages after
	 * ship has been released and is under player control again */
	bool GetDockAnimPositionOrient(int port, int stage, double t, const vector3d &from, positionOrient_t &outPosOrient, const Ship *ship) const;
};

class StationAdvertForm;
class FormController;
class SpaceStation;
struct BBAdvert;

typedef StationAdvertForm* (*AdvertFormBuilder)(FormController *controller, SpaceStation *station, const BBAdvert &ad);

struct BBAdvert {
	int               ref;
	std::string       description;
	AdvertFormBuilder builder;
};


class SystemBody;

class SpaceStation: public ModelBody, public MarketAgent {
public:
	OBJDEF(SpaceStation, ModelBody, SPACESTATION);
	static void Init();
	static void Uninit();
	enum TYPE { JJHOOP, GROUND_FLAVOURED, TYPE_MAX };

	enum Animation { // <enum scope='SpaceStation' name=SpaceStationAnimation prefix=ANIM_>
		ANIM_DOCKING_BAY_1,
		ANIM_DOCKING_BAY_2,
		ANIM_DOCKING_BAY_3,
		ANIM_DOCKING_BAY_4,
	};

	// Should point to SystemBody in Pi::currentSystem
	SpaceStation(const SystemBody *);
	SpaceStation() {}
	virtual ~SpaceStation();
	virtual double GetBoundingRadius() const;
	virtual bool OnCollision(Object *b, Uint32 flags, double relVel);
	virtual void Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform);
	/** You should call Ship::Undock() rather than this.
	 * Returns true on success, false if permission denied */
	bool LaunchShip(Ship *ship, int port);
	void OrientDockedShip(Ship *ship, int port) const;
	bool GetDockingClearance(Ship *s, std::string &outMsg);
	virtual void TimeStepUpdate(const float timeStep);
	bool IsGroundStation() const;
	float GetDesiredAngVel() const;
	void AddEquipmentStock(Equip::Type t, int num) { m_equipmentStock[t] += num; }
	/* MarketAgent stuff */
	int GetStock(Equip::Type t) const { return m_equipmentStock[t]; }
	Sint64 GetPrice(Equip::Type t) const;
	bool CanBuy(Equip::Type t, bool verbose) const;
	bool CanSell(Equip::Type t, bool verbose) const;
	bool DoesSell(Equip::Type t) const;
	virtual const SystemBody *GetSystemBody() const { return m_sbody; }
	void ReplaceShipOnSale(int idx, const ShipFlavour *with);
	const std::vector<ShipFlavour> &GetShipsOnSale() const { return m_shipsOnSale; }
	virtual void PostLoadFixup(Space *space);
	virtual void NotifyRemoved(const Body* const removedBody);
	int GetDockingPortCount() const { return m_type->numDockingPorts; }
	int GetFreeDockingPort() const; // returns -1 if none free
	int GetMyDockingPort(const Ship *s) const {
		for (int i=0; i<MAX_DOCKING_PORTS; i++) {
			if (s == m_shipDocking[i].ship) return i;
		}
		return -1;
	}
	void SetDocked(Ship *ship, int port);
	const SpaceStationType *GetSpaceStationType() const { return m_type; }
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

protected:
	virtual void Save(Serializer::Writer &wr, Space *space);
	virtual void Load(Serializer::Reader &rd, Space *space);
	/* MarketAgent stuff */
	void Bought(Equip::Type t);
	void Sold(Equip::Type t);
private:
	void DoDockingAnimation(const double timeStep);
	void DoLawAndOrder();
	void CalcLighting(Planet *planet, double &ambient, double &intensity, const std::vector<Camera::Light> &lights);

	/* Stage 0 means docking port empty
	 * Stage 1 means docking clearance granted to ->ship
	 * Stage 2 to m_type->numDockingStages is docking animation
	 * Stage m_type->numDockingStages+1 means ship is docked
	 * Stage -1 to -m_type->numUndockStages is undocking animation
	 */
	struct shipDocking_t {
		Ship *ship;
		int shipIndex; // deserialisation
		int stage;
		vector3d fromPos; // in station model coords
		Quaterniond fromRot;
		double stagePos; // 0 -> 1.0
	};
	shipDocking_t m_shipDocking[MAX_DOCKING_PORTS];

	double m_openAnimState[MAX_DOCKING_PORTS];
	double m_dockAnimState[MAX_DOCKING_PORTS];

	void InitStation();
	void PositionDockedShip(Ship *ship, int port);
	void UpdateShipyard();
	const SpaceStationType *m_type;
	const SystemBody *m_sbody;
	int m_equipmentStock[Equip::TYPE_MAX];
	std::vector<ShipFlavour> m_shipsOnSale;
	double m_lastUpdatedShipyard;
	CityOnPlanet *m_adjacentCity;
	double m_distFromPlanet;
	int m_numPoliceDocked;
	enum { NUM_STATIC_SLOTS = 4 };
	bool m_staticSlot[NUM_STATIC_SLOTS];

	std::vector<BBAdvert> m_bbAdverts;
	bool m_bbCreated, m_bbShuffled;
};

void FadeInModelIfDark(Graphics::Renderer *r, double modelRadius, double dist, double fadeInEnd, double fadeInLength, double illumination, double minIllumination);

#endif /* _SPACESTATION_H */
