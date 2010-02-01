#include "SpaceStation.h"
#include "Ship.h"
#include "Planet.h"
#include "gameconsts.h"
#include "StarSystem.h"
#include "Serializer.h"
#include "Frame.h"
#include "Pi.h"
#include "Mission.h"
#include "CityOnPlanet.h"
#include "Shader.h"
#include "Player.h"
#include "Polit.h"
#include "LmrModel.h"


#define ARG_STATION_BAY1_STAGE 6
#define ARG_STATION_BAY1_POS   10


struct SpaceStationType {
	LmrModel *model;
	const char *modelName;
	float angVel;
	enum DOCKMETHOD { SURFACE, ORBITAL } dockMethod;
	int numDockingPorts;
	int numDockingStages;
	int numUndockStages;
	struct positionOrient_t {
		vector3d pos;
		vector3d xaxis;
		vector3d yaxis;
	};
	float *dockAnimStageDuration;
	float *undockAnimStageDuration;

	void _ReadStageDurations(const char *key, int *outNumStages, float **durationArray) {
		lua_State *L = LmrGetLuaState();
		model->PushAttributeToStack(key);
		assert(lua_istable(L, -1));

		int num = lua_objlen(L, -1);
		*outNumStages = num;
		if (num == 0) {
			*durationArray = 0;
		} else {
			*durationArray = new float[num];
			for (int i=1; i<=num; i++) {
				lua_pushinteger(L, i);
				lua_gettable(L, -2);
				(*durationArray)[i-1] = lua_tonumber(L, -1);
				lua_pop(L, 1);
			}
		}
	}
	// read from lua model definition
	void ReadStageDurations() {
		_ReadStageDurations("dock_anim_stage_duration", &numDockingStages, &dockAnimStageDuration);
		_ReadStageDurations("undock_anim_stage_duration", &numUndockStages, &undockAnimStageDuration);
	}

	/* when ship is on rails it returns true and fills outPosOrient.
	 * when ship has been released (or docked) it returns false.
	 * Note station animations may continue for any number of stages after
	 * ship has been released and is under player control again */
	bool GetDockAnimPositionOrient(int stage, float t, const vector3d &from, positionOrient_t &outPosOrient) const
	{
		if ((stage < 0) && ((-stage) > numUndockStages)) return false;
		if ((stage > 0) && (stage > numDockingStages)) return false;
		lua_State *L = LmrGetLuaState();
		// It's a function of form function(stage, t, from)
		model->PushAttributeToStack("ship_dock_anim");
		if (!lua_isfunction(L, -1)) {
			fprintf(stderr, "Error: Spacestation model %s needs ship_dock_anim method\n", model->GetName());
			Pi::Quit();
		}
		lua_pushinteger(L, stage);
		lua_pushnumber(L, (double)t);
		vector3f *_from = MyLuaVec::pushVec(L);
		*_from = vector3f(from);
		lua_call(L, 3, 1);
		bool gotOrient;
		if (lua_istable(L, -1)) {
			gotOrient = true;
			lua_pushinteger(L, 1);
			lua_gettable(L, -2);
			outPosOrient.pos = vector3f(*MyLuaVec::checkVec(L, -1));
			lua_pop(L, 1);

			lua_pushinteger(L, 2);
			lua_gettable(L, -2);
			outPosOrient.xaxis = vector3f(*MyLuaVec::checkVec(L, -1));
			lua_pop(L, 1);

			lua_pushinteger(L, 3);
			lua_gettable(L, -2);
			outPosOrient.yaxis = vector3f(*MyLuaVec::checkVec(L, -1));
			lua_pop(L, 1);
		} else {
			gotOrient = false;
		}
		lua_pop(L, 1);
		return gotOrient;
	}


};

static bool stationTypesInitted = false;
static std::vector<SpaceStationType> surfaceStationTypes;
static std::vector<SpaceStationType> orbitalStationTypes;

/* Must be called after LmrModel init is called */
void SpaceStation::Init()
{
	if (stationTypesInitted) return;
	stationTypesInitted = true;
	for (int is_orbital=0; is_orbital<2; is_orbital++) {
		std::vector<LmrModel*> models;
		if (is_orbital) LmrGetModelsWithTag("orbital_station", models);
		else LmrGetModelsWithTag("surface_station", models);

		for (std::vector<LmrModel*>::iterator i = models.begin();
				i != models.end(); ++i) {
			SpaceStationType t;
			t.modelName = (*i)->GetName();
			t.model = LmrLookupModelByName(t.modelName);
			t.dockMethod = (SpaceStationType::DOCKMETHOD) is_orbital;
			t.numDockingPorts = (*i)->GetIntAttribute("num_docking_ports");
			t.ReadStageDurations();
			printf("%s: %d docking ports\n", t.modelName, t.numDockingPorts);
			if (is_orbital) {
				t.angVel = (*i)->GetFloatAttribute("angular_velocity");
				orbitalStationTypes.push_back(t);
			}
			else surfaceStationTypes.push_back(t);
		}
	}
}

float SpaceStation::GetDesiredAngVel() const
{
	return m_type->angVel;
}

void SpaceStation::Save()
{
	using namespace Serializer::Write;
	ModelBody::Save();
	MarketAgent::Save();
	for (int i=0; i<Equip::TYPE_MAX; i++) {
		wr_int((int)m_equipmentStock[i]);
	}
	// save shipyard
	wr_int(m_shipsOnSale.size());
	for (std::vector<ShipFlavour>::iterator i = m_shipsOnSale.begin();
			i != m_shipsOnSale.end(); ++i) {
		(*i).Save();
	}
	// save bb missions
	wr_int(m_bbmissions.size());
	for (std::vector<Mission*>::iterator i = m_bbmissions.begin();
			i != m_bbmissions.end(); ++i) {
		(*i)->Save();
	}
	for (int i=0; i<MAX_DOCKING_PORTS; i++) {
		wr_int(Serializer::LookupBody(m_shipDocking[i].ship));
		wr_int(m_shipDocking[i].stage);
		wr_float(m_shipDocking[i].stagePos);
		wr_vector3d(m_shipDocking[i].fromPos);
		wr_quaternionf(m_shipDocking[i].fromRot);

		wr_float(m_openAnimState[i]);
		wr_float(m_dockAnimState[i]);
	}
	wr_double(m_lastUpdatedShipyard);
	wr_int(Serializer::LookupSystemBody(m_sbody));
}

void SpaceStation::Load()
{
	using namespace Serializer::Read;
	ModelBody::Load();
	MarketAgent::Load();
	if (IsOlderThan(11)) rd_int();
	for (int i=0; i<Equip::TYPE_MAX; i++) {
		m_equipmentStock[i] = static_cast<Equip::Type>(rd_int());
	}
	// load shityard
	int numShipsForSale = rd_int();
	for (int i=0; i<numShipsForSale; i++) {
		ShipFlavour s;
		s.Load();
		m_shipsOnSale.push_back(s);
	}
	// load bbmissions
	int numBBMissions = rd_int();
	for (int i=0; i<numBBMissions; i++) {
		Mission *m = Mission::Load();
		m_bbmissions.push_back(m);
	}
	for (int i=0; i<MAX_DOCKING_PORTS; i++) {
		m_shipDocking[i].ship = (Ship*)rd_int();
		m_shipDocking[i].stage = rd_int();
		m_shipDocking[i].stagePos = rd_float();
		m_shipDocking[i].fromPos = rd_vector3d();
		if (IsOlderThan(12)) {
			m_shipDocking[i].fromRot = Quaternionf(0.0, vector3f(1,0,0));
		} else {
			m_shipDocking[i].fromRot = rd_quaternionf();
		}

		m_openAnimState[i] = rd_float();
		m_dockAnimState[i] = rd_float();
	}
	m_lastUpdatedShipyard = rd_double();
	m_sbody = Serializer::LookupSystemBody(rd_int());
	InitStation();
}

void SpaceStation::PostLoadFixup()
{
	for (int i=0; i<MAX_DOCKING_PORTS; i++) {
		m_shipDocking[i].ship = static_cast<Ship*>(Serializer::LookupBody((size_t)m_shipDocking[i].ship));
	}
}

double SpaceStation::GetBoundingRadius() const
{
	return ModelBody::GetBoundingRadius() + CITY_ON_PLANET_RADIUS;
}

SpaceStation::SpaceStation(const SBody *sbody): ModelBody()
{
	m_sbody = sbody;
	m_lastUpdatedShipyard = 0;
	for (int i=1; i<Equip::TYPE_MAX; i++) {
		if (EquipType::types[i].slot == Equip::SLOT_CARGO) {
			m_equipmentStock[i] = Pi::rng.Int32(0,100) * Pi::rng.Int32(1,100);
		} else {
			if (EquipType::types[i].techLevel <= Pi::currentSystem->m_techlevel)
				m_equipmentStock[i] = Pi::rng.Int32(0,100);
		}
	}
	for (int i=0; i<MAX_DOCKING_PORTS; i++) {
		m_shipDocking[i].ship = 0;
		m_shipDocking[i].stage = 0;
		m_shipDocking[i].stagePos = 0;
		m_openAnimState[i] = 0;
		m_dockAnimState[i] = 0;
	}
	SetMoney(1000000000);
	InitStation();
}

void SpaceStation::InitStation()
{
	m_adjacentCity = 0;
	MTRand rand(m_sbody->seed);
	if (m_sbody->type == SBody::TYPE_STARPORT_ORBITAL) {
		m_type = &orbitalStationTypes[ rand.Int32(orbitalStationTypes.size()) ];
	} else {
		m_type = &surfaceStationTypes[ rand.Int32(surfaceStationTypes.size()) ];
	}
	GetLmrObjParams().argFloats[ARG_STATION_BAY1_STAGE] = 1.0;
	GetLmrObjParams().argFloats[ARG_STATION_BAY1_POS] = 1.0;
	SetModel(m_type->modelName, true);
	LmrCollMesh *mesh = GetLmrCollMesh();
}

SpaceStation::~SpaceStation()
{
	for (std::vector<Mission*>::iterator i = m_bbmissions.begin();
			i != m_bbmissions.end(); ++i) {
		delete *i;
	}
	if (m_adjacentCity) delete m_adjacentCity;
}

void SpaceStation::ReplaceShipOnSale(int idx, const ShipFlavour *with)
{
	m_shipsOnSale[idx] = *with;
	onShipsForSaleChanged.emit();
}

void SpaceStation::UpdateShipyard()
{
	if (m_shipsOnSale.size() == 0) {
		// fill shipyard
		for (int i=Pi::rng.Int32(20); i; i--) {
			ShipFlavour s;
			ShipFlavour::MakeTrulyRandom(s);
			m_shipsOnSale.push_back(s);
		}
	} else if (Pi::rng.Int32(2)) {
		// add one
		ShipFlavour s;
		ShipFlavour::MakeTrulyRandom(s);
		m_shipsOnSale.push_back(s);
	} else {
		// remove one
		int pos = Pi::rng.Int32(m_shipsOnSale.size());
		m_shipsOnSale.erase(m_shipsOnSale.begin() + pos);
	}
	onShipsForSaleChanged.emit();
}

/* does not dealloc */
bool SpaceStation::BBRemoveMission(Mission *m)
{
	for (int i=m_bbmissions.size()-1; i>=0; i--) {
		if (m_bbmissions[i] == m) {
			m_bbmissions.erase(m_bbmissions.begin() + i);
			onBulletinBoardChanged.emit();
			return true;
		}
	}
	return false;
}

void SpaceStation::UpdateBB()
{
	if (m_bbmissions.size() == 0) {
		// fill bb
		for (int i=Pi::rng.Int32(20); i; i--) {
			try {
				Mission *m = Mission::GenerateRandom();
				m_bbmissions.push_back(m);
			} catch (CouldNotMakeMissionException) {

			}
		}
	} else if (Pi::rng.Int32(2)) {
		// add one
		try {
			Mission *m = Mission::GenerateRandom();
			m_bbmissions.push_back(m);
		} catch (CouldNotMakeMissionException) {

		}
	} else {
		// remove one
		int pos = Pi::rng.Int32(m_bbmissions.size());
		delete m_bbmissions[pos];
		m_bbmissions.erase(m_bbmissions.begin() + pos);
	}
	onBulletinBoardChanged.emit();
}

void SpaceStation::DoDockingAnimation(const float timeStep)
{
	matrix4x4d rot, wantRot;
	vector3d p1, p2, zaxis;
	for (int i=0; i<MAX_DOCKING_PORTS; i++) {
		shipDocking_t &dt = m_shipDocking[i];
		if (!dt.ship) continue;
		if (!dt.stage) continue;
		// docked stage is m_type->numDockingPorts + 1
		if (dt.stage > m_type->numDockingStages) continue;
		GetRotMatrix(rot);

		float stageDuration = (dt.stage > 0 ?
				m_type->dockAnimStageDuration[dt.stage-1] :
				m_type->undockAnimStageDuration[abs(dt.stage)-1]);
		dt.stagePos += timeStep / stageDuration;

		if (dt.stage == 1) {
			// SPECIAL stage! Docking granted but waiting for ship
			// to dock
			m_openAnimState[i] += 0.3*timeStep;
			m_dockAnimState[i] -= 0.3*timeStep;

			if (dt.stagePos >= 1.0) {
				if (dt.ship == (Ship*)Pi::player) Pi::onDockingClearanceExpired.emit(this);
				dt.ship = 0;
				dt.stage = 0;
			}
			continue;
		}
	
		if (dt.stagePos > 1.0f) {
			dt.stagePos = 0;
			if (dt.stage >= 0) dt.stage++;
			else dt.stage--;
			dt.fromPos = dt.ship->GetPosition();
			matrix4x4d temp;
			dt.ship->GetRotMatrix(temp);
			dt.fromRot = Quaternionf::FromMatrix4x4<double>(temp);
		}

		SpaceStationType::positionOrient_t shipOrient;
		bool onRails = m_type->GetDockAnimPositionOrient(dt.stage, dt.stagePos, dt.fromPos, shipOrient);
		
		if (onRails) {
			dt.ship->SetPosition(shipOrient.pos);
			wantRot = matrix4x4d::MakeRotMatrix(
					shipOrient.xaxis, shipOrient.yaxis,
					vector3d::Cross(shipOrient.xaxis, shipOrient.yaxis)) * rot;
			// use quaternion spherical linear interpolation to do
			// rotation smoothly
			Quaternionf wantQuat = Quaternionf::FromMatrix4x4<double>(wantRot);
			Quaternionf q = Quaternionf::Slerp(dt.fromRot, wantQuat, dt.stagePos);
			wantRot = q.ToMatrix4x4<double>();
			dt.ship->SetRotMatrix(wantRot);
		} else {
			if (dt.stage >= 0) {
				// set docked
				dt.ship->SetDockedWith(this, i);
			} else {
				if (!dt.ship->IsEnabled()) {
					// launch ship
					dt.ship->Enable();
					dt.ship->SetFlightState(Ship::FLYING);
					dt.ship->SetVelocity(GetFrame()->GetStasisVelocityAtPosition(dt.ship->GetPosition()));
					dt.ship->SetAngVelocity(GetFrame()->GetAngVelocity());
					dt.ship->SetForce(vector3d(0,0,0));
					dt.ship->SetTorque(vector3d(0,0,0));
					dt.ship->SetThrusterState(ShipType::THRUSTER_REAR, 1.0);
				}
			}
		}
		if ((dt.stage < 0) && ((-dt.stage) > m_type->numUndockStages)) {
		       dt.stage = 0;
		       dt.ship = 0;
		}
	}
	for (int i=0; i<MAX_DOCKING_PORTS; i++) {
		m_openAnimState[i] = CLAMP(m_openAnimState[i], 0.0f, 1.0f);
		m_dockAnimState[i] = CLAMP(m_dockAnimState[i], 0.0f, 1.0f);
	}
}

void SpaceStation::TimeStepUpdate(const float timeStep)
{
	if (Pi::GetGameTime() > m_lastUpdatedShipyard) {
		UpdateBB();
		UpdateShipyard();
		// update again in an hour or two
		m_lastUpdatedShipyard = Pi::GetGameTime() + 3600.0 + 3600.0*Pi::rng.Double();
	}
	DoDockingAnimation(timeStep);
}

bool SpaceStation::IsGroundStation() const
{
	return (m_type->dockMethod == SpaceStationType::SURFACE);
}

/* XXX THIS and PositionDockedShip do almost the same thing */
void SpaceStation::OrientDockedShip(Ship *ship, int port) const
{
	SpaceStationType::positionOrient_t dport;
	assert(m_type->GetDockAnimPositionOrient(m_type->numDockingStages, 1.0f, vector3d(0.0), dport));
//	const positionOrient_t *dport = &this->port[port];
	const int dockMethod = m_type->dockMethod;
	if (dockMethod == SpaceStationType::SURFACE) {
		matrix4x4d stationRot;
		GetRotMatrix(stationRot);
		vector3d port_z = vector3d::Cross(dport.xaxis, dport.yaxis);
		matrix4x4d rot = stationRot * matrix4x4d::MakeRotMatrix(dport.xaxis, dport.yaxis, port_z);
		vector3d pos = GetPosition() + stationRot*dport.pos;

		// position with wheels perfectly on ground :D
		Aabb aabb;
		ship->GetAabb(aabb);
		pos += stationRot*vector3d(0,-aabb.min.y,0);

		ship->SetPosition(pos);
		ship->SetRotMatrix(rot);
	}
}

void SpaceStation::SetDocked(Ship *ship, int port)
{
	PositionDockedShip(ship, port);
	m_shipDocking[port].ship = ship;
	m_shipDocking[port].stage = m_type->numDockingStages+1;
}

void SpaceStation::PositionDockedShip(Ship *ship, int port)
{
	SpaceStationType::positionOrient_t dport;
	assert(m_type->GetDockAnimPositionOrient(m_type->numDockingStages, 1.0f, vector3d(0.0), dport));
//	const positionOrient_t *dport = &this->port[port];
	const int dockMethod = m_type->dockMethod;
	if (dockMethod == SpaceStationType::ORBITAL) {
		matrix4x4d rot;
		GetRotMatrix(rot);
		vector3d p = GetPosition() + rot*dport.pos;
		
		ship->SetFrame(GetFrame());
		ship->SetPosition(p);
		// duplicated from DoDockingAnimation()
		vector3d zaxis = vector3d::Cross(dport.xaxis, dport.yaxis);
		ship->SetRotMatrix(matrix4x4d::MakeRotMatrix(dport.xaxis,
					dport.yaxis, zaxis) * rot);
	} else {
		Aabb aabb;
		ship->GetAabb(aabb);

	 	matrix4x4d stationRot;
		GetRotMatrix(stationRot);
		vector3d port_z = vector3d::Cross(dport.xaxis, dport.yaxis);
		matrix4x4d rot = stationRot * matrix4x4d::MakeRotMatrix(dport.xaxis, dport.yaxis, port_z);
		// position slightly (1m) off landing surface
		vector3d pos = GetPosition() + stationRot*(dport.pos +
				dport.yaxis - 
				dport.yaxis*aabb.min.y);
		ship->SetPosition(pos);
		ship->SetRotMatrix(rot);
	}
}

void SpaceStation::LaunchShip(Ship *ship, int port)
{
	const int dockMethod = m_type->dockMethod;
	
	if (dockMethod == SpaceStationType::ORBITAL) {
		shipDocking_t &sd = m_shipDocking[port];
		sd.ship = ship;
		sd.stage = -1;
		sd.stagePos = 0;
		sd.fromPos = ship->GetPosition();
		{ matrix4x4d temp;
		  ship->GetRotMatrix(temp);
		  sd.fromRot = Quaternionf::FromMatrix4x4<double>(temp);
		}
		ship->SetFlightState(Ship::DOCKING);
	}
	else if (dockMethod == SpaceStationType::SURFACE) {
		ship->Blastoff();
	} else {
		assert(0);
	}
	PositionDockedShip(ship, port);
}

bool SpaceStation::GetDockingClearance(Ship *s, std::string &outMsg)
{
	for (int i=0; i<MAX_DOCKING_PORTS; i++) {
		if (i >= m_type->numDockingPorts) break;
		if (m_shipDocking[i].ship == s) {
			outMsg = stringf(256, "Clearance already granted. Proceed to docking bay %d.", i+1);
			return true;
		}
	}
	for (int i=0; i<MAX_DOCKING_PORTS; i++) {
		if (i >= m_type->numDockingPorts) break;
		if (m_shipDocking[i].ship != 0) continue;
		shipDocking_t &sd = m_shipDocking[i];
		sd.ship = s;
		sd.stage = 1;
		sd.stagePos = 0;
		outMsg = stringf(256, "Clearance granted. Proceed to docking bay %d.", i+1);
		return true;
	}
	outMsg = "Clearance denied. There are no free docking bays.";
	return false;
}

/* MarketAgent shite */
void SpaceStation::Bought(Equip::Type t) {
	m_equipmentStock[(int)t]++;
}
void SpaceStation::Sold(Equip::Type t) {
	m_equipmentStock[(int)t]--;
}
bool SpaceStation::CanBuy(Equip::Type t) const {
	return true;
}
bool SpaceStation::CanSell(Equip::Type t) const {
	return m_equipmentStock[(int)t] > 0;
}
bool SpaceStation::DoesSell(Equip::Type t) const {
	return Polit::IsCommodityLegal(Pi::currentSystem, t);
}

Sint64 SpaceStation::GetPrice(Equip::Type t) const {
	Sint64 mul = 100 + Pi::currentSystem->GetCommodityBasePriceModPercent(t);
	return (mul * (Sint64)EquipType::types[t].basePrice) / 100;
}

bool SpaceStation::OnCollision(Object *b, Uint32 flags, double relVel)
{
	if (flags & 0x10) {
		//positionOrient_t *dport = &port[flags & 0xf];
		SpaceStationType::positionOrient_t dport;
		/// XXX if returns false???
		m_type->GetDockAnimPositionOrient(2, 0.0f, vector3d(0.0), dport);
		// hitting docking area of a station
		if (b->IsType(Object::SHIP)) {
			Ship *s = static_cast<Ship*>(b);
		
			double speed = s->GetVelocity().Length();
			
			// must be oriented sensibly and have wheels down
			if (IsGroundStation()) {
				matrix4x4d rot;
				s->GetRotMatrix(rot);
				matrix4x4d invRot = rot.InverseOf();
				
				matrix4x4d stationRot;
				GetRotMatrix(stationRot);
				vector3d dockingNormal = stationRot*dport.yaxis;

				// check player is sortof sensibly oriented for landing
				const double dot = vector3d::Dot(vector3d(invRot[1], invRot[5], invRot[9]), dockingNormal);
				if ((dot < 0.99) || (s->GetWheelState() != 1.0)) return true;
			}
			
			if ((speed < MAX_LANDING_SPEED) &&
			    (!s->GetDockedWith()) &&
			    (m_shipDocking[flags&0xf].ship == s) &&
			    (m_shipDocking[flags&0xf].stage == 1)) {
				// if there is more docking port anim to do,
				// don't set docked yet
			//	if (port_s2[flags & 0xf].exists) {
				if (m_type->numDockingStages >= 2) {
					shipDocking_t &sd = m_shipDocking[flags&0xf];
					sd.ship = s;
					sd.stage = 2;
					sd.stagePos = 0;
					sd.fromPos = s->GetPosition();
					matrix4x4d temp;
					s->GetRotMatrix(temp);
					sd.fromRot = Quaternionf::FromMatrix4x4<double>(temp);
					s->Disable();
					s->SetFlightState(Ship::DOCKING);
				} else {
					s->SetDockedWith(this, flags&0xf);
				}
			}
		}
		if (!IsGroundStation()) return false;
		return true;
	} else {
		return true;
	}
}

void SpaceStation::NotifyDeleted(const Body* const deletedBody)
{
	for (int i=0; i<MAX_DOCKING_PORTS; i++) {
		if (m_shipDocking[i].ship == deletedBody) {
			m_shipDocking[i].ship = 0;
		}
	}
}

static std::vector<LmrModel*> s_advertModels;

void SpaceStation::Render(const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	/* Well this is nice... */
	static int poo=0;
	if (!poo) {
		poo = 1;
		LmrGetModelsWithTag("advert", s_advertModels);
	}
	// it is silly to do this every render call
	//
	// random advert models in pFlag[16 .. 19]
	// station name in pText[0]
	// docking port in pText[1]
	MTRand rand;
	rand.seed(m_sbody->seed);
	
	LmrObjParams &params = GetLmrObjParams();
	/* random advert models */
	params.argStrings[4] = s_advertModels[rand.Int32(s_advertModels.size())]->GetName();
	params.argStrings[5] = s_advertModels[rand.Int32(s_advertModels.size())]->GetName();
	params.argStrings[6] = s_advertModels[rand.Int32(s_advertModels.size())]->GetName();
	params.argStrings[7] = s_advertModels[rand.Int32(s_advertModels.size())]->GetName();
	params.argStrings[0] = GetLabel().c_str();
	SetLmrTimeParams();

	for (int i=0; i<MAX_DOCKING_PORTS; i++) {
		params.argFloats[ARG_STATION_BAY1_STAGE + i] = m_shipDocking[i].stage;
		params.argFloats[ARG_STATION_BAY1_POS + i] = m_shipDocking[i].stagePos;
	}

	RenderLmrModel(viewCoords, viewTransform);
	
	/* don't render city if too far away */
	if (viewCoords.Length() > 1000000.0) return;

	// find planet Body*
	Planet *planet;
	{
		Body *_planet = GetFrame()->m_astroBody;
		if ((!_planet) || !_planet->IsType(Object::PLANET)) {
			// orbital spaceport -- don't make city turds
		} else {
			planet = static_cast<Planet*>(_planet);
		
			if (!m_adjacentCity) {
				m_adjacentCity = new CityOnPlanet(planet, this, m_sbody->seed);
			}
			Shader::EnableVertexProgram(Shader::VPROG_SBRE);
			m_adjacentCity->Render(this, viewCoords, viewTransform);
			Shader::DisableVertexProgram();
		}
	}
}
