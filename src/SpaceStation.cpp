#include "SpaceStation.h"
#include "Ship.h"
#include "Planet.h"
#include "gameconsts.h"
#include "galaxy/StarSystem.h"
#include "Serializer.h"
#include "Frame.h"
#include "Pi.h"
#include "CityOnPlanet.h"
#include "Player.h"
#include "Polit.h"
#include "LmrModel.h"
#include "LuaVector.h"
#include "Polit.h"
#include "Space.h"
#include "Lang.h"
#include "StringF.h"
#include <algorithm>
#include "Game.h"
#include "graphics/Graphics.h"

#define ARG_STATION_BAY1_STAGE 6
#define ARG_STATION_BAY1_POS   10

void SpaceStationType::_ReadStageDurations(const char *key, int *outNumStages, double **durationArray) {
	lua_State *L = LmrGetLuaState();

	LUA_DEBUG_START(L);

	model->PushAttributeToLuaStack(key);
	assert(lua_istable(L, -1));

	int num = lua_rawlen(L, -1);
	*outNumStages = num;
	if (num == 0) {
		*durationArray = 0;
	} else {
		*durationArray = new double[num];
		for (int i=1; i<=num; i++) {
			lua_pushinteger(L, i);
			lua_gettable(L, -2);
			(*durationArray)[i-1] = lua_tonumber(L, -1);
			lua_pop(L, 1);
		}
	}
	if (outNumStages <= 0) {
		Error("Space station %s must have atleast 1 docking and 1 undocking animation stage.",
				modelName);
	}

	lua_pop(L, 1);

	LUA_DEBUG_END(L, 0);
}
// read from lua model definition
void SpaceStationType::ReadStageDurations() {
	_ReadStageDurations("dock_anim_stage_duration", &numDockingStages, &dockAnimStageDuration);
	_ReadStageDurations("undock_anim_stage_duration", &numUndockStages, &undockAnimStageDuration);
}

bool SpaceStationType::GetShipApproachWaypoints(int port, int stage, positionOrient_t &outPosOrient) const
{
	lua_State *L = LmrGetLuaState();

	LUA_DEBUG_START(L);

	lua_pushcfunction(L, pi_lua_panic);
	model->PushAttributeToLuaStack("ship_approach_waypoints");
	if (!lua_isfunction(L, -1)) {
		printf("no function\n");
		lua_pop(L, 2);
		LUA_DEBUG_END(L, 0);
		return false;
	}

	lua_pushinteger(L, port+1);
	lua_pushinteger(L, stage);
	lua_pcall(L, 2, 1, -4);
	bool gotOrient;
	if (lua_istable(L, -1)) {
		gotOrient = true;
		lua_pushinteger(L, 1);
		lua_gettable(L, -2);
		outPosOrient.pos = *LuaVector::CheckFromLua(L, -1);
		lua_pop(L, 1);

		lua_pushinteger(L, 2);
		lua_gettable(L, -2);
		outPosOrient.xaxis = *LuaVector::CheckFromLua(L, -1);
		lua_pop(L, 1);

		lua_pushinteger(L, 3);
		lua_gettable(L, -2);
		outPosOrient.yaxis = *LuaVector::CheckFromLua(L, -1);
		lua_pop(L, 1);
	} else {
		gotOrient = false;
	}
	lua_pop(L, 2);

	LUA_DEBUG_END(L, 0);

	return gotOrient;
}

/* when ship is on rails it returns true and fills outPosOrient.
 * when ship has been released (or docked) it returns false.
 * Note station animations may continue for any number of stages after
 * ship has been released and is under player control again */
bool SpaceStationType::GetDockAnimPositionOrient(int port, int stage, double t, const vector3d &from, positionOrient_t &outPosOrient, const Ship *ship) const
{
	if ((stage < 0) && ((-stage) > numUndockStages)) return false;
	if ((stage > 0) && (stage > numDockingStages)) return false;

	lua_State *L = LmrGetLuaState();

	LUA_DEBUG_START(L);

	lua_pushcfunction(L, pi_lua_panic);
	// It's a function of form function(stage, t, from)
	model->PushAttributeToLuaStack("ship_dock_anim");
	if (!lua_isfunction(L, -1)) {
		Error("Spacestation model %s needs ship_dock_anim method", model->GetName());
	}
	lua_pushinteger(L, port+1);
	lua_pushinteger(L, stage);
	lua_pushnumber(L, double(t));
	LuaVector::PushToLua(L, from);
	// push model aabb as lua table: { min: vec3, max: vec3 }
	{
		Aabb aabb;
		ship->GetAabb(aabb);
		lua_createtable (L, 0, 2);
		LuaVector::PushToLua(L, aabb.max);
		lua_setfield(L, -2, "max");
		LuaVector::PushToLua(L, aabb.min);
		lua_setfield(L, -2, "min");
	}

	lua_pcall(L, 5, 1, -7);
	bool gotOrient;
	if (lua_istable(L, -1)) {
		gotOrient = true;
		lua_pushinteger(L, 1);
		lua_gettable(L, -2);
		outPosOrient.pos = *LuaVector::CheckFromLua(L, -1);
		lua_pop(L, 1);

		lua_pushinteger(L, 2);
		lua_gettable(L, -2);
		outPosOrient.xaxis = *LuaVector::CheckFromLua(L, -1);
		lua_pop(L, 1);

		lua_pushinteger(L, 3);
		lua_gettable(L, -2);
		outPosOrient.yaxis = *LuaVector::CheckFromLua(L, -1);
		lua_pop(L, 1);
	} else {
		gotOrient = false;
	}
	lua_pop(L, 2);

	LUA_DEBUG_END(L, 0);

	return gotOrient;
}

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
			t.dockMethod = SpaceStationType::DOCKMETHOD(is_orbital);
			t.numDockingPorts = (*i)->GetIntAttribute("num_docking_ports");
			t.dockOneAtATimePlease = (*i)->GetBoolAttribute("dock_one_at_a_time_please");
			t.ReadStageDurations();
			//printf("one at a time? %s\n", t.dockOneAtATimePlease ? "yes" : "no");
			//printf("%s: %d docking ports\n", t.modelName, t.numDockingPorts);
			if (is_orbital) {
				t.angVel = (*i)->GetFloatAttribute("angular_velocity");
				orbitalStationTypes.push_back(t);
			}
			else surfaceStationTypes.push_back(t);
		}
	}
	//printf(SIZET_FMT " orbital station types and " SIZET_FMT " surface station types.\n", orbitalStationTypes.size(), surfaceStationTypes.size());
}

void SpaceStation::Uninit()
{
	std::vector<SpaceStationType>::iterator i;
	for (i=surfaceStationTypes.begin(); i!=surfaceStationTypes.end(); ++i) {
		delete[] (*i).dockAnimStageDuration;
		delete[] (*i).undockAnimStageDuration;
	}
	for (i=orbitalStationTypes.begin(); i!=orbitalStationTypes.end(); ++i) {
		delete[] (*i).dockAnimStageDuration;
		delete[] (*i).undockAnimStageDuration;
	}
}

float SpaceStation::GetDesiredAngVel() const
{
	return m_type->angVel;
}

void SpaceStation::Save(Serializer::Writer &wr, Space *space)
{
	ModelBody::Save(wr, space);
	MarketAgent::Save(wr);
	wr.Int32(Equip::TYPE_MAX);
	for (int i=0; i<Equip::TYPE_MAX; i++) {
		wr.Int32(int(m_equipmentStock[i]));
	}
	// save shipyard
	wr.Int32(m_shipsOnSale.size());
	for (std::vector<ShipFlavour>::iterator i = m_shipsOnSale.begin();
			i != m_shipsOnSale.end(); ++i) {
		(*i).Save(wr);
	}
	for (int i=0; i<MAX_DOCKING_PORTS; i++) {
		wr.Int32(space->GetIndexForBody(m_shipDocking[i].ship));
		wr.Int32(m_shipDocking[i].stage);
		wr.Float(float(m_shipDocking[i].stagePos));
		wr.Vector3d(m_shipDocking[i].fromPos);
		wr.WrQuaternionf(m_shipDocking[i].fromRot);

		wr.Float(float(m_openAnimState[i]));
		wr.Float(float(m_dockAnimState[i]));
	}
	wr.Bool(m_bbCreated);
	wr.Double(m_lastUpdatedShipyard);
	wr.Int32(space->GetIndexForSystemBody(m_sbody));
	wr.Int32(m_numPoliceDocked);
}

void SpaceStation::Load(Serializer::Reader &rd, Space *space)
{
	ModelBody::Load(rd, space);
	MarketAgent::Load(rd);
	int num = rd.Int32();
	if (num > Equip::TYPE_MAX) throw SavedGameCorruptException();
	for (int i=0; i<Equip::TYPE_MAX; i++) {
		m_equipmentStock[i] = 0;
	}
	for (int i=0; i<num; i++) {
		m_equipmentStock[i] = static_cast<Equip::Type>(rd.Int32());
	}
	// load shityard
	int numShipsForSale = rd.Int32();
	for (int i=0; i<numShipsForSale; i++) {
		ShipFlavour s;
		s.Load(rd);
		m_shipsOnSale.push_back(s);
	}
	for (int i=0; i<MAX_DOCKING_PORTS; i++) {
		m_shipDocking[i].shipIndex = rd.Int32();
		m_shipDocking[i].stage = rd.Int32();
		m_shipDocking[i].stagePos = rd.Float();
		m_shipDocking[i].fromPos = rd.Vector3d();
		m_shipDocking[i].fromRot = rd.RdQuaternionf();

		m_openAnimState[i] = rd.Float();
		m_dockAnimState[i] = rd.Float();
	}
	m_bbCreated = rd.Bool();
	m_lastUpdatedShipyard = rd.Double();
	m_sbody = space->GetSystemBodyByIndex(rd.Int32());
	m_numPoliceDocked = rd.Int32();
	InitStation();
}

void SpaceStation::PostLoadFixup(Space *space)
{
	for (int i=0; i<MAX_DOCKING_PORTS; i++) {
		m_shipDocking[i].ship = static_cast<Ship*>(space->GetBodyByIndex(m_shipDocking[i].shipIndex));
	}
}

double SpaceStation::GetBoundingRadius() const
{
	return ModelBody::GetBoundingRadius() + CITY_ON_PLANET_RADIUS;
}

SpaceStation::SpaceStation(const SystemBody *sbody): ModelBody()
{
	m_sbody = sbody;
	m_lastUpdatedShipyard = 0;
	m_numPoliceDocked = Pi::rng.Int32(3,10);
	m_bbCreated = false;

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
	for(int i=0; i<NUM_STATIC_SLOTS; i++) m_staticSlot[i] = false;
	MTRand rand(m_sbody->seed);
	if (m_sbody->type == SystemBody::TYPE_STARPORT_ORBITAL) {
		m_type = &orbitalStationTypes[ rand.Int32(orbitalStationTypes.size()) ];
		m_hasDoubleFrame = true;
	} else {
		m_type = &surfaceStationTypes[ rand.Int32(surfaceStationTypes.size()) ];
	}

	LmrObjParams &params = GetLmrObjParams();
	params.animStages[ANIM_DOCKING_BAY_1] = 1;
	params.animValues[ANIM_DOCKING_BAY_1] = 1.0;
	// XXX the animation namespace must match that in LuaConstants
	params.animationNamespace = "SpaceStationAnimation";
	SetModel(m_type->modelName, true);
}

SpaceStation::~SpaceStation()
{
	onBulletinBoardDeleted.emit();
	if (m_adjacentCity) delete m_adjacentCity;
}

void SpaceStation::ReplaceShipOnSale(int idx, const ShipFlavour *with)
{
	m_shipsOnSale[idx] = *with;
	onShipsForSaleChanged.emit();
}

// Fill the list of starships on sale. Ships that
// can't fit atmo shields are only available in
// atmosphereless environments
void SpaceStation::UpdateShipyard()
{
	bool atmospheric = false;
	if (IsGroundStation()) {
		Body *planet = GetFrame()->m_astroBody;
		atmospheric = planet->GetSystemBody()->HasAtmosphere();
	}

	if (m_shipsOnSale.size() == 0) {
		// fill shipyard
		for (int i=Pi::rng.Int32(20); i; i--) {
			ShipFlavour s;
			ShipFlavour::MakeTrulyRandom(s, atmospheric);
			m_shipsOnSale.push_back(s);
		}
	} else if (Pi::rng.Int32(2)) {
		// add one
		ShipFlavour s;
		ShipFlavour::MakeTrulyRandom(s, atmospheric);
		m_shipsOnSale.push_back(s);
	} else {
		// remove one
		int pos = Pi::rng.Int32(m_shipsOnSale.size());
		m_shipsOnSale.erase(m_shipsOnSale.begin() + pos);
	}
	onShipsForSaleChanged.emit();
}

void SpaceStation::DoDockingAnimation(const double timeStep)
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

		double stageDuration = (dt.stage > 0 ?
				m_type->dockAnimStageDuration[dt.stage-1] :
				m_type->undockAnimStageDuration[abs(dt.stage)-1]);
		dt.stagePos += timeStep / stageDuration;

		if (dt.stage == 1) {
			// SPECIAL stage! Docking granted but waiting for ship
			// to dock
			m_openAnimState[i] += 0.3*timeStep;
			m_dockAnimState[i] -= 0.3*timeStep;

			if (dt.stagePos >= 1.0) {
				if (dt.ship == static_cast<Ship*>(Pi::player)) Pi::onDockingClearanceExpired.emit(this);
				dt.ship = 0;
				dt.stage = 0;
			}
			continue;
		}

		if (dt.stagePos > 1.0) {
			dt.stagePos = 0;
			if (dt.stage >= 0) dt.stage++;
			else dt.stage--;
			dt.fromPos = rot.InverseOf() * (dt.ship->GetPosition() - GetPosition());
			matrix4x4d temp;
			dt.ship->GetRotMatrix(temp);
			dt.fromRot = Quaterniond::FromMatrix4x4(temp);
		}

		SpaceStationType::positionOrient_t shipOrient;
		bool onRails = m_type->GetDockAnimPositionOrient(i, dt.stage, dt.stagePos, dt.fromPos, shipOrient, dt.ship);

		if (onRails) {
			dt.ship->SetPosition(GetPosition() + rot*shipOrient.pos);
			wantRot = matrix4x4d::MakeRotMatrix(
					shipOrient.xaxis, shipOrient.yaxis,
					shipOrient.xaxis.Cross(shipOrient.yaxis)) * rot;
			// use quaternion spherical linear interpolation to do
			// rotation smoothly
			Quaterniond wantQuat = Quaterniond::FromMatrix4x4(wantRot);
			Quaterniond q = Quaterniond::Nlerp(dt.fromRot, wantQuat, dt.stagePos);
			wantRot = q.ToMatrix4x4<double>();
		//	wantRot.Renormalize();
			dt.ship->SetRotMatrix(wantRot);
		} else {
			if (dt.stage >= 0) {
				// set docked
				dt.ship->SetDockedWith(this, i);
				Pi::luaOnShipDocked->Queue(dt.ship, this);
			} else {
				if (!dt.ship->IsEnabled()) {
					// launch ship
					dt.ship->Enable();
					dt.ship->SetFlightState(Ship::FLYING);
					dt.ship->SetAngVelocity(GetFrame()->GetAngVelocity());
					dt.ship->SetForce(vector3d(0,0,0));
					dt.ship->SetTorque(vector3d(0,0,0));
					if (m_type->dockMethod == SpaceStationType::SURFACE) {
						dt.ship->SetThrusterState(1, 1.0);		// up
					} else {
						dt.ship->SetVelocity(GetFrame()->GetStasisVelocityAtPosition(dt.ship->GetPosition()));
						dt.ship->SetThrusterState(2, -1.0);		// forward
					}
					Pi::luaOnShipUndocked->Queue(dt.ship, this);
				}
			}
		}
		if ((dt.stage < 0) && ((-dt.stage) > m_type->numUndockStages)) {
		       dt.stage = 0;
		       dt.ship = 0;
		}
	}
	for (int i=0; i<MAX_DOCKING_PORTS; i++) {
		m_openAnimState[i] = Clamp(m_openAnimState[i], 0.0, 1.0);
		m_dockAnimState[i] = Clamp(m_dockAnimState[i], 0.0, 1.0);
	}
}

void SpaceStation::DoLawAndOrder()
{
	Sint64 fine, crimeBitset;
	Polit::GetCrime(&crimeBitset, &fine);
	if (Pi::player->GetFlightState() != Ship::DOCKED
			&& m_numPoliceDocked
			&& (fine > 1000)
			&& (GetPositionRelTo(static_cast<Body*>(Pi::player)).Length() < 100000.0)) {
		int port = GetFreeDockingPort();
		if (port != -1) {
			m_numPoliceDocked--;
			// Make police ship intent on killing the player
			Ship *ship = new Ship(ShipType::LADYBIRD);
			ship->AIKill(Pi::player);
			ship->SetFrame(GetFrame());
			ship->SetDockedWith(this, port);
			Pi::game->GetSpace()->AddBody(ship);
			{ // blue and white thang
				ShipFlavour f;
				f.type = ShipType::LADYBIRD;
				f.regid = Lang::POLICE_SHIP_REGISTRATION;
				f.price = ship->GetFlavour()->price;
				LmrMaterial m;
				m.diffuse[0] = 0.0f; m.diffuse[1] = 0.0f; m.diffuse[2] = 1.0f; m.diffuse[3] = 1.0f;
				m.specular[0] = 0.0f; m.specular[1] = 0.0f; m.specular[2] = 1.0f; m.specular[3] = 1.0f;
				m.emissive[0] = 0.0f; m.emissive[1] = 0.0f; m.emissive[2] = 0.0f; m.emissive[3] = 0.0f;
				m.shininess = 50.0f;
				f.primaryColor = m;
				m.shininess = 0.0f;
				m.diffuse[0] = 1.0f; m.diffuse[1] = 1.0f; m.diffuse[2] = 1.0f; m.diffuse[3] = 1.0f;
				f.secondaryColor = m;
				ship->ResetFlavour(&f);
			}
			ship->m_equipment.Set(Equip::SLOT_LASER, 0, Equip::PULSECANNON_DUAL_1MW);
			ship->m_equipment.Add(Equip::SHIELD_GENERATOR);
			ship->m_equipment.Add(Equip::LASER_COOLING_BOOSTER);
			ship->m_equipment.Add(Equip::ATMOSPHERIC_SHIELDING);
			ship->UpdateStats();
		}
	}
}

void SpaceStation::TimeStepUpdate(const float timeStep)
{
	bool update = false;

	// if there's no BB and there are ships here, make one
	if (!m_bbCreated && GetFreeDockingPort() != 0) {
		CreateBB();
		update = true;
	}

	// if there is and it hasn't had an update for a while, update it
	else if (Pi::game->GetTime() > m_lastUpdatedShipyard) {
		Pi::luaOnUpdateBB->Queue(this);
		update = true;
	}

	if (update) {
		UpdateShipyard();
		// update again in an hour or two
		m_lastUpdatedShipyard = Pi::game->GetTime() + 3600.0 + 3600.0*Pi::rng.Double();
	}

	DoDockingAnimation(timeStep);
	DoLawAndOrder();
}

bool SpaceStation::IsGroundStation() const
{
	return (m_type->dockMethod == SpaceStationType::SURFACE);
}

/* XXX THIS and PositionDockedShip do almost the same thing */
void SpaceStation::OrientDockedShip(Ship *ship, int port) const
{
	SpaceStationType::positionOrient_t dport;
	if (!m_type->GetDockAnimPositionOrient(port, m_type->numDockingStages, 1.0f, vector3d(0.0), dport, ship)) {
		Error("Space station model %s does not specify valid ship_dock_anim positions", m_type->modelName);
	}
//	const positionOrient_t *dport = &this->port[port];
	const int dockMethod = m_type->dockMethod;
	if (dockMethod == SpaceStationType::SURFACE) {
		matrix4x4d stationRot;
		GetRotMatrix(stationRot);
		vector3d port_z = dport.xaxis.Cross(dport.yaxis);
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

int SpaceStation::GetFreeDockingPort() const
{
	for (int i=0; i<m_type->numDockingPorts; i++) {
		if (m_shipDocking[i].ship == 0) {
			return i;
		}
	}
	return -1;
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
	PiVerify(m_type->GetDockAnimPositionOrient(port, m_type->numDockingStages, 1.0f, vector3d(0.0), dport, ship));
//	const positionOrient_t *dport = &this->port[port];
	const int dockMethod = m_type->dockMethod;
	if (dockMethod == SpaceStationType::ORBITAL) {
		matrix4x4d rot;
		GetRotMatrix(rot);
		vector3d p = GetPosition() + rot*dport.pos;

		ship->SetFrame(GetFrame());
		ship->SetPosition(p);
		// duplicated from DoDockingAnimation()
		vector3d zaxis = dport.xaxis.Cross(dport.yaxis);
		ship->SetRotMatrix(matrix4x4d::MakeRotMatrix(dport.xaxis,
					dport.yaxis, zaxis) * rot);
	} else {
		Aabb aabb;
		ship->GetAabb(aabb);

	 	matrix4x4d stationRot;
		GetRotMatrix(stationRot);
		vector3d port_z = dport.xaxis.Cross(dport.yaxis);
		matrix4x4d rot = stationRot * matrix4x4d::MakeRotMatrix(dport.xaxis, dport.yaxis, port_z);
		// position slightly (1m) off landing surface
		vector3d pos = GetPosition() + stationRot*(dport.pos +
				dport.yaxis -
				dport.yaxis*aabb.min.y);
		ship->SetPosition(pos);
		ship->SetRotMatrix(rot);
	}
}

bool SpaceStation::LaunchShip(Ship *ship, int port)
{
	/* XXX bad to keep duplicating this */
	if (m_type->dockOneAtATimePlease) {
		for (int i=0; i<m_type->numDockingPorts; i++) {
			if (m_shipDocking[i].ship && m_shipDocking[i].stage &&
			    (m_shipDocking[i].stage != m_type->numDockingStages+1)) {
				return false;
			}
		}
	}
	matrix4x4d rot;
	GetRotMatrix(rot);

	shipDocking_t &sd = m_shipDocking[port];
	sd.ship = ship;
	sd.stage = -1;
	sd.stagePos = 0;
	sd.fromPos = rot.InverseOf() * (ship->GetPosition() - GetPosition());
	{
		matrix4x4d temp;
		ship->GetRotMatrix(temp);
		sd.fromRot = Quaterniond::FromMatrix4x4(temp);
	}
	ship->SetFlightState(Ship::DOCKING);

	PositionDockedShip(ship, port);
	return true;
}

bool SpaceStation::GetDockingClearance(Ship *s, std::string &outMsg)
{
	for (int i=0; i<MAX_DOCKING_PORTS; i++) {
		if (i >= m_type->numDockingPorts) break;
		if ((m_shipDocking[i].ship == s) && (m_shipDocking[i].stage > 0)) {
			outMsg = stringf(Lang::CLEARANCE_ALREADY_GRANTED_BAY_N, formatarg("bay", i+1));
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
		outMsg = stringf(Lang::CLEARANCE_GRANTED_BAY_N, formatarg("bay", i+1));
		return true;
	}
	outMsg = Lang::CLEARANCE_DENIED_NO_BAYS;
	return false;
}

/* MarketAgent shite */
void SpaceStation::Bought(Equip::Type t) {
	m_equipmentStock[int(t)]++;
}
void SpaceStation::Sold(Equip::Type t) {
	m_equipmentStock[int(t)]--;
}
bool SpaceStation::CanBuy(Equip::Type t, bool verbose) const {
	return true;
}
bool SpaceStation::CanSell(Equip::Type t, bool verbose) const {
	bool result = (m_equipmentStock[int(t)] > 0);
	if (verbose && !result) {
		Pi::Message(Lang::ITEM_IS_OUT_OF_STOCK);
	}
	return result;
}
bool SpaceStation::DoesSell(Equip::Type t) const {
	return Polit::IsCommodityLegal(Pi::game->GetSpace()->GetStarSystem().Get(), t);
}

Sint64 SpaceStation::GetPrice(Equip::Type t) const {
	Sint64 mul = 100 + Pi::game->GetSpace()->GetStarSystem()->GetCommodityBasePriceModPercent(t);
	return (mul * Sint64(Equip::types[t].basePrice)) / 100;
}

bool SpaceStation::OnCollision(Object *b, Uint32 flags, double relVel)
{
	if ((flags & 0x10) && (b->IsType(Object::SHIP))) {
		Ship *s = static_cast<Ship*>(b);
		matrix4x4d rot;
		GetRotMatrix(rot);

		bool canDock = true;
		int port = -1;
		for (int i=0; i<MAX_DOCKING_PORTS; i++) {
			if (m_shipDocking[i].ship == s) { port = i; break; }
		}
		if (m_type->dockOneAtATimePlease) {
			for (int i=0; i<m_type->numDockingPorts; i++) {
				if (m_shipDocking[i].ship && m_shipDocking[i].stage != 1 &&
				    (m_shipDocking[i].stage != m_type->numDockingStages+1)) {
					canDock = false;
					break;
				}
			}
		} else {
			// for non-dockOneAtATimePlease, the ship is expected
			// to hit the right docking trigger surface for that port
			if (m_shipDocking[flags&0xf].ship != s) canDock = false;
		}
		if (port == -1) canDock = false;

		// hitting docking area of a station
		if (canDock) {
			SpaceStationType::positionOrient_t dport;
			// why stage 2? Because stage 1 is permission to dock
			// granted, stage 2 is start of docking animation.
			PiVerify(m_type->GetDockAnimPositionOrient(port, 2, 0.0f, vector3d(0.0), dport, s));

			double speed = s->GetVelocity().Length();

			// must be oriented sensibly and have wheels down
			if (IsGroundStation()) {
				matrix4x4d shiprot;
				s->GetRotMatrix(shiprot);
				matrix4x4d invShipRot = shiprot.InverseOf();

				vector3d dockingNormal = rot*dport.yaxis;

				// check player is sortof sensibly oriented for landing
				const double dot = vector3d(invShipRot[1], invShipRot[5], invShipRot[9]).Dot(dockingNormal);
				if ((dot < 0.99) || (s->GetWheelState() < 1.0)) return false;
			}

			if ((speed < MAX_LANDING_SPEED) &&
			    (!s->GetDockedWith()) &&
			    (m_shipDocking[port].stage == 1)) {
				// if there is more docking port anim to do,
				// don't set docked yet
				if (m_type->numDockingStages >= 2) {
					shipDocking_t &sd = m_shipDocking[port];
					sd.ship = s;
					sd.stage = 2;
					sd.stagePos = 0;
					sd.fromPos = rot.InverseOf() * (s->GetPosition() - GetPosition());
					matrix4x4d temp;
					s->GetRotMatrix(temp);
					sd.fromRot = Quaterniond::FromMatrix4x4(temp);
					s->Disable();
					s->ClearThrusterState();
					s->SetFlightState(Ship::DOCKING);
				} else {
					s->SetDockedWith(this, port);
					Pi::luaOnShipDocked->Queue(s, this);
				}
			}
		}
		return false;
	} else {
		return true;
	}
}

void SpaceStation::NotifyRemoved(const Body* const removedBody)
{
	for (int i=0; i<MAX_DOCKING_PORTS; i++) {
		if (m_shipDocking[i].ship == removedBody) {
			m_shipDocking[i].ship = 0;
		}
	}
}

// Calculates the ambiently and directly lit portions of the lighting model taking into account the atmosphere and sun positions at a given location
// 1. Calculates the amount of direct illumination available taking into account
//    * multiple suns 
//    * sun positions relative to up direction i.e. light is dimmed as suns set 
//    * Thickness of the atmosphere overhead i.e. as atmospheres get thicker light starts dimming earlier as sun sets, without atmosphere the light switches off at point of sunset
// 2. Calculates the split between ambient and directly lit portions taking into account
//    * Atmosphere density (optical thickness) of the sky dome overhead
//        as optical thickness increases the fraction of ambient light increases
//        this takes altitude into account automatically
//    * As suns set the split is biased towards ambient 
void SpaceStation::CalcLighting(Planet *planet, double &ambient, double &intensity, const std::vector<Camera::Light> &lights)
{
	// position relative to the rotating frame of the planet
	vector3d upDir = GetPosition();
	double dist = upDir.Length();
	upDir = upDir.Normalized();
	double pressure, density;
	planet->GetAtmosphericState(dist, &pressure, &density);
	double surfaceDensity;
	Color cl;
	planet->GetSystemBody()->GetAtmosphereFlavor(&cl, &surfaceDensity);

	// approximate optical thickness fraction as fraction of density remaining relative to earths
	double opticalThicknessFraction = density/EARTH_ATMOSPHERE_SURFACE_DENSITY;
	// tweak optical thickness curve - lower exponent ==> higher altitude before ambient level drops
	opticalThicknessFraction = pow(std::max(0.00001,opticalThicknessFraction),0.15); //max needed to avoid 0^power

	//step through all the lights and calculate contributions taking into account sun position
	double light = 0.0;
	double light_clamped = 0.0;

	for(std::vector<Camera::Light>::const_iterator l = lights.begin();
		l != lights.end(); ++l) {
			
			double sunAngle;
			// calculate the extent the sun is towards zenith
			if (l->GetBody()){
				// relative to the rotating frame of the planet
				const vector3d lightDir = (l->GetBody()->GetInterpolatedPositionRelTo(planet->GetFrame()).Normalized());
				sunAngle = lightDir.Dot(upDir);
			} else 
				// light is the default light for systems without lights
				sunAngle = 1.0;

			//0 to 1 as sunangle goes from 0.0 to 1.0
			double sunAngle2 = (Clamp(sunAngle, 0.0,1.0))/1.0;

			//0 to 1 as sunAngle goes from endAngle to startAngle

			// angle at which light begins to fade on Earth
			const double startAngle = 0.3;
			// angle at which sun set completes, which should be after sun has dipped below the horizon on Earth
			const double endAngle = -0.08;

			const double start = std::min((startAngle*opticalThicknessFraction),1.0);
			const double end = std::max((endAngle*opticalThicknessFraction),-0.2);

			sunAngle = (Clamp(sunAngle, end, start)-end)/(start-end);
			
			light += sunAngle;
			light_clamped += sunAngle2;
	}


	// brightness depends on optical depth and intensity of light from all the stars
	intensity = (Clamp((light),0.0,1.0));


	// ambient light fraction
	// alter ratio between directly and ambiently lit portions towards ambiently lit as sun sets
	double fraction = (0.4+0.4*(
						1.0-light_clamped*(Clamp((opticalThicknessFraction),0.0,1.0))
						)*0.8+0.2); //fraction goes from 0.6 to 1.0
					  
	
	// fraction of light left over to be lit directly
	intensity = (1.0-fraction)*intensity;

	// scale ambient by amount of light
	ambient = fraction*(Clamp((light),0.0,1.0));
}

// if twilight or night fade in model at close ranges by increasing scene ambient lighting to minIllumination
// dist is distance in meters to model in camera space
void FadeInModelIfDark(Graphics::Renderer *r, double modelRadius, double dist, double fadeInEnd, double fadeInLength, double illumination, double minIllumination)
{
	if (illumination <= minIllumination) {
		
		fadeInEnd = std::max(std::max(modelRadius,10.0), fadeInEnd);
		const double fadeInStart = fadeInLength+fadeInEnd;
		// 0 to 1 as dist goes from fadeInEnd to fadeInStart
		double sceneAmbient = 1.0-(Clamp(dist, fadeInEnd, fadeInStart)-fadeInEnd)/((fadeInStart-fadeInEnd));
		
		//set scene ambient to the amount needed to take illumination level to 0.2
		sceneAmbient*= minIllumination-illumination;

		r->SetAmbientColor(Color(sceneAmbient, sceneAmbient, sceneAmbient, 1.0));
	}
}
// Renders space station and adjacent city if applicable
// For orbital starports: renders as normal
// For surface starports: 
//	Lighting: Calculates available light for model and splits light between directly and ambiently lit
//            Lighting is done by manipulating global lights or setting uniforms in atmospheric models shader
//            Adds an ambient light at close ranges if dark by manipulating the global ambient level
void SpaceStation::Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	LmrObjParams &params = GetLmrObjParams();
	params.label = GetLabel().c_str();
	SetLmrTimeParams();

	for (int i=0; i<MAX_DOCKING_PORTS; i++) {
		params.animStages[ANIM_DOCKING_BAY_1 + i] = m_shipDocking[i].stage;
		params.animValues[ANIM_DOCKING_BAY_1 + i] = m_shipDocking[i].stagePos;
	}

	Body *b = GetFrame()->m_astroBody;
	assert(b);

	if (!b->IsType(Object::PLANET)) {
		// orbital spaceport -- don't make city turds or change lighting based on atmosphere
		RenderLmrModel(viewCoords, viewTransform);
	}
	
	else {
		Planet *planet = static_cast<Planet*>(b);
		
		// calculate lighting
		// available light is calculated and split between directly (diffusely/specularly) lit and ambiently lit
		const std::vector<Camera::Light> &lights = camera->GetLights();
		std::vector<Camera::Light> newLights = lights;
		int numLights = lights.size();
		
		double ambient, intensity;

		CalcLighting(planet, ambient, intensity, lights);

		for(int i = 0;i < numLights; i++) {
			Color c = lights[i].GetDiffuse();
			Color ca = lights[i].GetAmbient();
			Color cs = lights[i].GetSpecular();
			ca.r = c.r * float(ambient);
			ca.g = c.g * float(ambient);
			ca.b = c.b * float(ambient);
			c.r*=float(intensity);
			c.g*=float(intensity);
			c.b*=float(intensity);
			cs.r*=float(intensity);
			cs.g*=float(intensity);
			cs.b*=float(intensity);
			newLights[i].SetDiffuse(c);
			newLights[i].SetAmbient(ca);
			newLights[i].SetSpecular(cs);
		}
		r->SetLights(numLights, &newLights[0]);

		double overallLighting = ambient+intensity;

		// turn off global ambient color
		Color oldAmbient;
		oldAmbient = Graphics::State::GetGlobalSceneAmbientColor();

		r->SetAmbientColor(Color(0.0, 0.0, 0.0, 1.0));

		// as the camera gets close adjust scene ambient so that intensity+ambient = minIllumination
		double fadeInEnd, fadeInLength, minIllumination;
		if (Graphics::AreShadersEnabled()) {
			minIllumination = 0.125;
			fadeInEnd = 800.0;
			fadeInLength = 2000.0;
		}
		else {
			minIllumination = 0.25;
			fadeInEnd = 1500.0;
			fadeInLength = 3000.0;
		}

		FadeInModelIfDark(r, GetLmrCollMesh()->GetBoundingRadius(),
							viewCoords.Length(), fadeInEnd, fadeInLength, overallLighting, minIllumination);

		RenderLmrModel(viewCoords, viewTransform);

		// reset ambient colour as Fade-in model may change it
		r->SetAmbientColor(Color(0.0, 0.0, 0.0, 1.0));

		/* don't render city if too far away */
		if (viewCoords.Length() < 1000000.0){
			if (!m_adjacentCity) {
				m_adjacentCity = new CityOnPlanet(planet, this, m_sbody->seed);
			}
			m_adjacentCity->Render(r, camera, this, viewCoords, viewTransform, overallLighting, minIllumination);
		} 

		// restore old lights
		r->SetLights(numLights, &lights[0]);

		// restore old ambient color
		r->SetAmbientColor(oldAmbient);
	}
}

// find an empty position for a static ship and mark it as used. these aren't
// saved and are only needed to help modules place bulk ships. this isn't a
// great place for this, but its gotta be tracked somewhere
bool SpaceStation::AllocateStaticSlot(int& slot)
{
	for (int i=0; i<NUM_STATIC_SLOTS; i++) {
		if (!m_staticSlot[i]) {
			m_staticSlot[i] = true;
			slot = i;
			return true;
		}
	}

	return false;
}

void SpaceStation::CreateBB()
{
	if (m_bbCreated) return;

	// fill the shipyard equipment shop with all kinds of things
	// XXX should probably be moved out to a MarketAgent/CommodityWidget type
	//     thing, or just lua
	for (int i=1; i<Equip::TYPE_MAX; i++) {
		if (Equip::types[i].slot == Equip::SLOT_CARGO) {
			m_equipmentStock[i] = Pi::rng.Int32(0,100) * Pi::rng.Int32(1,100);
		} else {
			m_equipmentStock[i] = Pi::rng.Int32(0,100);
		}
	}

	Pi::luaOnCreateBB->Queue(this);
	m_bbCreated = true;
}


static int next_ref = 0;
int SpaceStation::AddBBAdvert(std::string description, AdvertFormBuilder builder)
{
	int ref = ++next_ref;
	assert(ref);

	BBAdvert ad;
	ad.ref = ref;
	ad.description = description;
	ad.builder = builder;

	m_bbAdverts.push_back(ad);

	onBulletinBoardChanged.emit();

	return ref;
}

const BBAdvert *SpaceStation::GetBBAdvert(int ref)
{
	for (std::vector<BBAdvert>::const_iterator i = m_bbAdverts.begin(); i != m_bbAdverts.end(); i++)
		if (i->ref == ref)
			return &(*i);
	return NULL;
}

bool SpaceStation::RemoveBBAdvert(int ref)
{
	for (std::vector<BBAdvert>::iterator i = m_bbAdverts.begin(); i != m_bbAdverts.end(); i++)
		if (i->ref == ref) {
			BBAdvert ad = (*i);
			m_bbAdverts.erase(i);
			onBulletinBoardAdvertDeleted.emit(ad);
			return true;
		}
	return false;
}

const std::list<const BBAdvert*> SpaceStation::GetBBAdverts()
{
	if (!m_bbShuffled) {
		std::random_shuffle(m_bbAdverts.begin(), m_bbAdverts.end());
		m_bbShuffled = true;
	}

	std::list<const BBAdvert*> ads;
	for (std::vector<BBAdvert>::const_iterator i = m_bbAdverts.begin(); i != m_bbAdverts.end(); i++)
		ads.push_back(&(*i));
	return ads;
}

vector3d SpaceStation::GetTargetIndicatorPosition(const Frame *relTo) const
{
	// return the next waypoint if permission has been granted for player,
	// and the docking point's position once the docking anim starts
	for (int i=0; i<MAX_DOCKING_PORTS; i++) {
		if (i >= m_type->numDockingPorts) break;
		if ((m_shipDocking[i].ship == Pi::player) && (m_shipDocking[i].stage > 0)) {

			SpaceStationType::positionOrient_t dport;
			if (!m_type->GetShipApproachWaypoints(i, m_shipDocking[i].stage+1, dport))
				PiVerify(m_type->GetDockAnimPositionOrient(i, m_type->numDockingStages,
				1.0f, vector3d(0.0), dport, m_shipDocking[i].ship));
			matrix4x4d rot;
			GetRotMatrix(rot);

			matrix4x4d m;
			Frame::GetFrameRenderTransform(GetFrame(), relTo, m);
			return m * (GetInterpolatedPosition() + (rot*dport.pos));
		}
	}
	return GetInterpolatedPositionRelTo(relTo);
}
