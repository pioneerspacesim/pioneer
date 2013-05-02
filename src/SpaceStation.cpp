// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "SpaceStation.h"
#include "CityOnPlanet.h"
#include "FileSystem.h"
#include "Frame.h"
#include "Game.h"
#include "gameconsts.h"
#include "Lang.h"
#include "LuaEvent.h"
#include "LuaVector.h"
#include "Pi.h"
#include "Planet.h"
#include "Player.h"
#include "Polit.h"
#include "Polit.h"
#include "Serializer.h"
#include "Ship.h"
#include "Space.h"
#include "StringF.h"
#include "galaxy/StarSystem.h"
#include "graphics/Graphics.h"
#include <algorithm>

#define ARG_STATION_BAY1_STAGE 6
#define ARG_STATION_BAY1_POS   10

void SpaceStation::Init()
{
	SpaceStationType::Init();
}

void SpaceStation::Uninit()
{
	SpaceStationType::Uninit();
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
	for (std::vector<ShipOnSale>::iterator i = m_shipsOnSale.begin();
			i != m_shipsOnSale.end(); ++i) {
		wr.String((*i).id);
		wr.String((*i).regId);
		(*i).skin.Save(wr);
	}
	wr.Int32(m_shipDocking.size());
	for (uint32_t i=0; i<m_shipDocking.size(); i++) {
		wr.Int32(space->GetIndexForBody(m_shipDocking[i].ship));
		wr.Int32(m_shipDocking[i].stage);
		wr.Float(float(m_shipDocking[i].stagePos));
		wr.Vector3d(m_shipDocking[i].fromPos);
		wr.WrQuaternionf(m_shipDocking[i].fromRot);
	}
	// store each of the bay groupings
	wr.Int32(mBayGroups.size());
	for (uint32_t i=0; i<mBayGroups.size(); i++) {
		wr.Int32(mBayGroups[i].minShipSize);
		wr.Int32(mBayGroups[i].maxShipSize);
		wr.Bool(mBayGroups[i].inUse);
		wr.Int32(mBayGroups[i].bayIDs.size());
		for (uint32_t j=0; j<mBayGroups[i].bayIDs.size(); j++) {
			wr.Int32(mBayGroups[i].bayIDs[j]);
		}
	}

	wr.Bool(m_bbCreated);
	wr.Double(m_lastUpdatedShipyard);
	wr.Int32(space->GetIndexForSystemBody(m_sbody));
	wr.Int32(m_numPoliceDocked);

	wr.Double(m_doorAnimationStep);
	wr.Double(m_doorAnimationState);

	m_navLights->Save(wr);
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
		ShipType::Id id(rd.String());
		std::string regId(rd.String());
		SceneGraph::ModelSkin skin;
		skin.Load(rd);
		ShipOnSale sos(id, regId, skin);
		m_shipsOnSale.push_back(sos);
	}
	const int32_t numShipDocking = rd.Int32();
	m_shipDocking.reserve(numShipDocking);
	for (int i=0; i<numShipDocking; i++) {
		m_shipDocking.push_back(shipDocking_t());
		shipDocking_t &sd = m_shipDocking.back();
		sd.shipIndex = rd.Int32();
		sd.stage = rd.Int32();
		sd.stagePos = rd.Float();
		sd.fromPos = rd.Vector3d();
		sd.fromRot = rd.RdQuaternionf();
	}
	assert(m_shipDocking.size() == m_type->numDockingPorts);
	// retrieve each of the bay groupings
	const int32_t numBays = rd.Int32();
	mBayGroups.reserve(numBays);
	for (int32_t i=0; i<numBays; i++) {
		mBayGroups.push_back(SpaceStationType::SBayGroup());
		SpaceStationType::SBayGroup &bay = mBayGroups.back();
		bay.minShipSize = rd.Int32();
		bay.maxShipSize = rd.Int32();
		bay.inUse = rd.Bool();
		const int32_t numBayIds = rd.Int32();
		bay.bayIDs.reserve(numBayIds);
		for (int32_t j=0; j<numBayIds; j++) {
			const int32_t ID = rd.Int32();
			bay.bayIDs.push_back(ID);
		}
	}

	m_bbCreated = rd.Bool();
	m_lastUpdatedShipyard = rd.Double();
	m_sbody = space->GetSystemBodyByIndex(rd.Int32());
	m_numPoliceDocked = rd.Int32();

	m_doorAnimationStep = rd.Double();
	m_doorAnimationState = rd.Double();

	InitStation();

	m_navLights->Load(rd);
}

void SpaceStation::PostLoadFixup(Space *space)
{
	ModelBody::PostLoadFixup(space);
	for (uint32_t i=0; i<m_shipDocking.size(); i++) {
		m_shipDocking[i].ship = static_cast<Ship*>(space->GetBodyByIndex(m_shipDocking[i].shipIndex));
	}
}

SpaceStation::SpaceStation(const SystemBody *sbody): ModelBody()
{
	m_sbody = sbody;
	m_lastUpdatedShipyard = 0;
	m_numPoliceDocked = Pi::rng.Int32(3,10);
	m_bbCreated = false;
	m_bbShuffled = false;

	m_oldAngDisplacement = 0.0;

	m_doorAnimationStep = m_doorAnimationState = 0.0;

	m_shipDocking.resize(m_type->numDockingPorts);

	SetMoney(1000000000);
	SetModel(m_type->modelName.c_str());
	InitStation();
}

void SpaceStation::InitStation()
{
	m_adjacentCity = 0;
	for(int i=0; i<NUM_STATIC_SLOTS; i++) m_staticSlot[i] = false;
	Random rand(m_sbody->seed);
	bool ground = m_sbody->type == SystemBody::TYPE_STARPORT_ORBITAL ? false : true;
	if (ground) { 
		m_type = &SpaceStationType::surfaceStationTypes[ rand.Int32(SpaceStationType::surfaceStationTypes.size()) ];
	} else { 
		m_type = &SpaceStationType::orbitalStationTypes[ rand.Int32(SpaceStationType::orbitalStationTypes.size()) ];
	}

	// This SpaceStation's bay groups is an instance of...
	mBayGroups = m_type->bayGroups;

	SetStatic(ground);			// orbital stations are dynamic now

	m_navLights.Reset(new NavLights(GetModel(), 2.2f));
	m_navLights->SetEnabled(true);

	if (ground) SetClipRadius(CITY_ON_PLANET_RADIUS);		// overrides setmodel

	m_doorAnimation = GetModel()->FindAnimation("doors");
}

SpaceStation::~SpaceStation()
{
	onBulletinBoardDeleted.emit();
	if (m_adjacentCity) delete m_adjacentCity;
}

void SpaceStation::ReplaceShipOnSale(int idx, const ShipOnSale &with)
{
	m_shipsOnSale[idx] = with;
	onShipsForSaleChanged.emit();
}

// Fill the list of starships on sale. Ships that
// can't fit atmo shields are only available in
// atmosphereless environments
void SpaceStation::UpdateShipyard()
{
	bool atmospheric = false;
	if (IsGroundStation()) {
		Body *planet = GetFrame()->GetBody();
		atmospheric = planet->GetSystemBody()->HasAtmosphere();
	}

	const std::vector<ShipType::Id> &ships = atmospheric ? ShipType::playable_atmospheric_ships : ShipType::player_ships;

	unsigned int toAdd = 0, toRemove = 0;

	if (m_shipsOnSale.size() == 0)
		// fill shipyard
		toAdd = Pi::rng.Int32(20);

	else if (Pi::rng.Int32(2))
		// add one
		toAdd = 1;

	else if(m_shipsOnSale.size() > 0)
		// remove one
		toRemove = 1;

	else
		// nothing happens
		return;

	for (; toAdd > 0; toAdd--) {
		ShipType::Id id = ships[Pi::rng.Int32(ships.size())];
		std::string regId = Ship::MakeRandomLabel();
		SceneGraph::ModelSkin skin;
		skin.SetRandomColors(Pi::rng);
		skin.SetPattern(Pi::rng.Int32(0, Pi::FindModel(id)->GetNumPatterns()));
		skin.SetLabel(regId);
		ShipOnSale sos(id, regId, skin);
		m_shipsOnSale.push_back(sos);
	}

	for (; toRemove > 0; toRemove--) {
		int pos = Pi::rng.Int32(m_shipsOnSale.size());
		m_shipsOnSale.erase(m_shipsOnSale.begin() + pos);
	}

	onShipsForSaleChanged.emit();
}

void SpaceStation::NotifyRemoved(const Body* const removedBody)
{
	for (uint32_t i=0; i<m_shipDocking.size(); i++) {
		if (m_shipDocking[i].ship == removedBody) {
			m_shipDocking[i].ship = 0;
		}
	}
}

int SpaceStation::GetMyDockingPort(const Ship *s) const
{
	for (uint32_t i=0; i<m_shipDocking.size(); i++) {
		if (s == m_shipDocking[i].ship) return i;
	}
	return -1;
}

int SpaceStation::GetFreeDockingPort() const
{
	for (unsigned int i=0; i<m_type->numDockingPorts; i++) {
		if (m_shipDocking[i].ship == 0) {
			return i;
		}
	}
	return -1;
}

void SpaceStation::SetDocked(Ship *ship, int port)
{
	m_shipDocking[port].ship = ship;
	m_shipDocking[port].stage = m_type->numDockingStages+1;

	// have to do this crap again in case it was called directly (Ship::SetDockWith())
	ship->SetFlightState(Ship::DOCKED);
	ship->SetVelocity(vector3d(0.0));
	ship->SetAngVelocity(vector3d(0.0));
	ship->ClearThrusterState();
	PositionDockedShip(ship, port);
}

bool SpaceStation::LaunchShip(Ship *ship, int port)
{
	shipDocking_t &sd = m_shipDocking[port];
	if (sd.stage < 0) return true;			// already launching
	if (IsPortLocked(port)) return false;	// another ship docking
	LockPort(port, true);

	sd.ship = ship;
	sd.stage = -1;
	sd.stagePos = 0.0;

	m_doorAnimationStep = 0.3; // open door

	const Aabb& aabb = ship->GetAabb();
	const matrix3x3d mt = ship->GetOrient();
	const vector3d up = mt.VectorY().Normalized() * aabb.min.y;
	
	sd.fromPos = (ship->GetPosition() - GetPosition() + up) * GetOrient();	// station space
	sd.fromRot = Quaterniond::FromMatrix3x3(GetOrient().Transpose() * mt);

	ship->SetFlightState(Ship::DOCKING);

	return true;
}

bool SpaceStation::GetDockingClearance(Ship *s, std::string &outMsg)
{
	assert(m_shipDocking.size() == m_type->numDockingPorts);
	for (uint32_t i=0; i<m_shipDocking.size(); i++) {
		if (m_shipDocking[i].ship == s) {
			outMsg = stringf(Lang::CLEARANCE_ALREADY_GRANTED_BAY_N, formatarg("bay", i+1));
			return (m_shipDocking[i].stage > 0); // grant docking only if the ship is not already docked/undocking
		}
	}
	for (uint32_t i=0; i<m_shipDocking.size(); i++) {
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

bool SpaceStation::OnCollision(Object *b, Uint32 flags, double relVel)
{
	if ((flags & 0x10) && (b->IsType(Object::SHIP))) {
		Ship *s = static_cast<Ship*>(b);

		int port = -1;
		for (uint32_t i=0; i<m_shipDocking.size(); i++) {
			if (m_shipDocking[i].ship == s) { port = i; break; }
		}
		if (port == -1) return false;					// no permission
		if (IsPortLocked(port)) {
			return false;
		}
		if (m_shipDocking[port].stage != 1) return false;	// already docking?

		SpaceStationType::positionOrient_t dport;
		// why stage 2? Because stage 1 is permission to dock
		// granted, stage 2 is start of docking animation.
		PiVerify(m_type->GetDockAnimPositionOrient(port, 2, 0.0, vector3d(0.0), dport, s));

		// must be oriented sensibly and have wheels down
		if (IsGroundStation()) {
			vector3d dockingNormal = GetOrient()*dport.yaxis;
			const double dot = s->GetOrient().VectorY().Dot(dockingNormal);
			if ((dot < 0.99) || (s->GetWheelState() < 1.0)) return false;	// <0.99 harsh?
			if (s->GetVelocity().Length() > MAX_LANDING_SPEED) return false;
		}

		// if there is more docking port anim to do, don't set docked yet
		if (m_type->numDockingStages >= 2) {
			shipDocking_t &sd = m_shipDocking[port];
			sd.ship = s;
			sd.stage = 2;
			sd.stagePos = 0;
			sd.fromPos = (s->GetPosition() - GetPosition()) * GetOrient();	// station space
			sd.fromRot = Quaterniond::FromMatrix3x3(GetOrient().Transpose() * s->GetOrient());
			LockPort(port, true);

			s->SetFlightState(Ship::DOCKING);
			s->SetVelocity(vector3d(0.0));
			s->SetAngVelocity(vector3d(0.0));
			s->ClearThrusterState();
		} else {
			s->SetDockedWith(this, port);				// bounces back to SS::SetDocked()
			LuaEvent::Queue("onShipDocked", s, this);
		}
		return false;
	} else {
		return true;
	}
}

// XXX SGModel door animation. We have one station (hoop_spacestation) with a
// door, so this is pretty much based on how it does things. This all needs
// rewriting to handle triggering animations at waypoints.
//
// Docking:
//   Stage 1 (clearance granted): open
//           (clearance expired): close
//   Docked:                      close
// 
// Undocking:
//   Stage -1 (LaunchShip): open
//   Post-launch:           close
//   

void SpaceStation::DockingUpdate(const double timeStep)
{
	vector3d p1, p2, zaxis;
	for (uint32_t i=0; i<m_shipDocking.size(); i++) {
		shipDocking_t &dt = m_shipDocking[i];
		if (!dt.ship) continue;
		// docked stage is m_type->numDockingPorts + 1 => ship docked
		if (dt.stage > m_type->numDockingStages) continue;

		double stageDuration = (dt.stage > 0 ?
				m_type->GetDockAnimStageDuration(dt.stage-1) :
				m_type->GetUndockAnimStageDuration(abs(dt.stage)-1));
		dt.stagePos += timeStep / stageDuration;

		if (dt.stage == 1) {
			// SPECIAL stage! Docking granted but waiting for ship to dock

			m_doorAnimationStep = 0.3; // open door

			if (dt.stagePos >= 1.0) {
				if (dt.ship == static_cast<Ship*>(Pi::player)) Pi::onDockingClearanceExpired.emit(this);
				dt.ship = 0;
				dt.stage = 0;
				m_doorAnimationStep = -0.3; // close door
			}
			continue;
		}

		if (dt.stagePos > 1.0) {
			// use end position of last segment for start position of new segment
			SpaceStationType::positionOrient_t dport;
			PiVerify(m_type->GetDockAnimPositionOrient(i, dt.stage, 1.0f, dt.fromPos, dport, dt.ship));
			matrix3x3d fromRot = matrix3x3d::FromVectors(dport.xaxis, dport.yaxis, dport.zaxis);
			dt.fromRot = Quaterniond::FromMatrix3x3(fromRot);
			dt.fromPos = dport.pos;

			// transition between docking stages
			dt.stagePos = 0;
			if (dt.stage >= 0) dt.stage++;
			else dt.stage--;
		}

		if (dt.stage < -m_type->shipLaunchStage && dt.ship->GetFlightState() != Ship::FLYING) {
			// launch ship
			dt.ship->SetFlightState(Ship::FLYING);
			dt.ship->SetAngVelocity(GetAngVelocity());
			if (m_type->dockMethod == SpaceStationType::SURFACE) {
				dt.ship->SetThrusterState(1, 1.0);	// up
			} else {
				dt.ship->SetThrusterState(2, -1.0);	// forward
			}
			LuaEvent::Queue("onShipUndocked", dt.ship, this);
		}
		if (dt.stage < -m_type->numUndockStages) {
			// undock animation finished, clear port
			dt.stage = 0;
			dt.ship = 0;
			LockPort(i, false);
			m_doorAnimationStep = -0.3; // close door
		}
		else if (dt.stage > m_type->numDockingStages) {
			// set docked
			dt.ship->SetDockedWith(this, i);
			LuaEvent::Queue("onShipDocked", dt.ship, this);
			LockPort(i, false);
			m_doorAnimationStep = -0.3; // close door
		}
	}
	
	m_doorAnimationState = Clamp(m_doorAnimationState + m_doorAnimationStep*timeStep, 0.0, 1.0);
	if (m_doorAnimation)
		m_doorAnimation->SetProgress(m_doorAnimationState);
}

void SpaceStation::PositionDockedShip(Ship *ship, int port) const
{
	const shipDocking_t &dt = m_shipDocking[port];
	SpaceStationType::positionOrient_t dport;
	PiVerify(m_type->GetDockAnimPositionOrient(port, dt.stage, dt.stagePos, dt.fromPos, dport, ship));
	assert(dt.ship == ship);

	ship->SetPosition(GetPosition() + GetOrient()*dport.pos);

	// Still in docking animation process?
	if (dt.stage <= m_type->numDockingStages) {
		matrix3x3d wantRot = matrix3x3d::FromVectors(dport.xaxis, dport.yaxis, dport.zaxis);
		// use quaternion spherical linear interpolation to do
		// rotation smoothly
		Quaterniond wantQuat = Quaterniond::FromMatrix3x3(wantRot);
		Quaterniond q = Quaterniond::Nlerp(dt.fromRot, wantQuat, dt.stagePos);
		wantRot = q.ToMatrix3x3<double>();
		ship->SetOrient(GetOrient() * wantRot);
	} else {
		// Note: ship bounding box is used to generate dport.pos
		ship->SetOrient(GetOrient() * matrix3x3d::FromVectors(dport.xaxis, dport.yaxis, dport.zaxis));
	}
}


void SpaceStation::StaticUpdate(const float timeStep)
{
	bool update = false;

	// if there's no BB and there are ships here, make one
	if (!m_bbCreated && GetFreeDockingPort() != 0) {
		CreateBB();
		update = true;
	}

	// if there is and it hasn't had an update for a while, update it
	else if (Pi::game->GetTime() > m_lastUpdatedShipyard) {
		LuaEvent::Queue("onUpdateBB", this);
		update = true;
	}

	if (update) {
		UpdateShipyard();
		// update again in an hour or two
		m_lastUpdatedShipyard = Pi::game->GetTime() + 3600.0 + 3600.0*Pi::rng.Double();
	}

	DoLawAndOrder(timeStep);
	DockingUpdate(timeStep);
	m_navLights->Update(timeStep);
}

void SpaceStation::TimeStepUpdate(const float timeStep)
{
	// rotate the thing
	double len = m_type->angVel * timeStep;
	if (!is_zero_exact(len)) {
		matrix3x3d r = matrix3x3d::RotateY(-len);		// RotateY is backwards
		SetOrient(r * GetOrient());
	}
	m_oldAngDisplacement = len;

	// reposition the ships that are docked or docking here
	for (unsigned int i=0; i<m_type->numDockingPorts; i++) {
		const shipDocking_t &dt = m_shipDocking[i];
		if (!dt.ship) { //free
			m_navLights->SetColor(i+1, NavLights::NAVLIGHT_GREEN);
			continue;
		}
		if (dt.stage == 1) {//reserved
			m_navLights->SetColor(i+1, NavLights::NAVLIGHT_YELLOW);
			continue;
		}
		if (dt.ship->GetFlightState() == Ship::FLYING)
			continue;
		PositionDockedShip(dt.ship, i);
		m_navLights->SetColor(i+1, NavLights::NAVLIGHT_RED); //docked
	}

	if (m_doorAnimation)
		GetModel()->UpdateAnimations();
}

void SpaceStation::UpdateInterpTransform(double alpha)
{
	double len = m_oldAngDisplacement * (1.0-alpha);
	if (!is_zero_exact(len)) {
		matrix3x3d rot = matrix3x3d::RotateY(len);		// RotateY is backwards
		m_interpOrient = rot * GetOrient();
	}
	else m_interpOrient = GetOrient();
	m_interpPos = GetPosition();
}

bool SpaceStation::IsGroundStation() const
{
	return (m_type->dockMethod == SpaceStationType::SURFACE);
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

// Renders space station and adjacent city if applicable
// For orbital starports: renders as normal
// For surface starports:
//	Lighting: Calculates available light for model and splits light between directly and ambiently lit
//            Lighting is done by manipulating global lights or setting uniforms in atmospheric models shader
void SpaceStation::Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	Body *b = GetFrame()->GetBody();
	assert(b);

	if (!b->IsType(Object::PLANET)) {
		// orbital spaceport -- don't make city turds or change lighting based on atmosphere
		RenderModel(r, camera, viewCoords, viewTransform);
	}

	else {
		std::vector<Graphics::Light> oldLights;
		Color oldAmbient;
		SetLighting(r, camera, oldLights, oldAmbient);

		Planet *planet = static_cast<Planet*>(b);
		/* don't render city if too far away */
		if (viewCoords.Length() < 1000000.0){
			if (!m_adjacentCity) {
				m_adjacentCity = new CityOnPlanet(planet, this, m_sbody->seed);
			}
			m_adjacentCity->Render(r, camera, this, viewCoords, viewTransform);
		}

		RenderModel(r, camera, viewCoords, viewTransform, false);

		ResetLighting(r, oldLights, oldAmbient);
	}
}

// find an empty position for a static ship and mark it as used. these aren't
// saved and are only needed to help modules place bulk ships. this isn't a
// great place for this, but its gotta be tracked somewhere
bool SpaceStation::AllocateStaticSlot(int& slot)
{
	// no slots at ground stations
	if (IsGroundStation())
		return false;

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

	LuaEvent::Queue("onCreateBB", this);
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
	for (std::vector<BBAdvert>::const_iterator i = m_bbAdverts.begin(); i != m_bbAdverts.end(); ++i)
		if (i->ref == ref)
			return &(*i);
	return NULL;
}

bool SpaceStation::RemoveBBAdvert(int ref)
{
	for (std::vector<BBAdvert>::iterator i = m_bbAdverts.begin(); i != m_bbAdverts.end(); ++i)
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
	for (std::vector<BBAdvert>::const_iterator i = m_bbAdverts.begin(); i != m_bbAdverts.end(); ++i)
		ads.push_back(&(*i));
	return ads;
}

vector3d SpaceStation::GetTargetIndicatorPosition(const Frame *relTo) const
{
	// return the next waypoint if permission has been granted for player,
	// and the docking point's position once the docking anim starts
	for (uint32_t i=0; i<m_shipDocking.size(); i++) {
		if (i >= m_type->numDockingPorts) break;
		if ((m_shipDocking[i].ship == Pi::player) && (m_shipDocking[i].stage > 0)) {

			SpaceStationType::positionOrient_t dport;
			if (!m_type->GetShipApproachWaypoints(i, m_shipDocking[i].stage+1, dport))
				PiVerify(m_type->GetDockAnimPositionOrient(i, m_type->numDockingStages,
				1.0f, vector3d(0.0), dport, m_shipDocking[i].ship));

			vector3d v = GetInterpPositionRelTo(relTo);
			return v + GetInterpOrientRelTo(relTo) * dport.pos;
		}
	}
	return GetInterpPositionRelTo(relTo);
}

// XXX this whole thing should be done by Lua
void SpaceStation::DoLawAndOrder(const double timeStep)
{
	Sint64 fine, crimeBitset;
	Polit::GetCrime(&crimeBitset, &fine);
	if (Pi::player->GetFlightState() != Ship::DOCKED
			&& m_numPoliceDocked
			&& (fine > 1000)
			&& (GetPositionRelTo(Pi::player).Length() < 100000.0)) {
		int port = GetFreeDockingPort();
		// at 60 Hz updates (ie, 1x time acceleration),
		// this spawns a police ship with probability ~0.83% each frame
		// This makes it unlikely (but not impossible) that police will spawn on top of each other
		// the expected number of game-time seconds between spawns: 120 (2*60 Hz)
		// variance is quite high though
		if (port != -1 && 2.0*Pi::rng.Double() < timeStep) {
			m_numPoliceDocked--;
			// Make police ship intent on killing the player
			Ship *ship = new Ship(ShipType::POLICE);
			ship->AIKill(Pi::player);
			ship->SetFrame(GetFrame());
			ship->SetDockedWith(this, port);
			Pi::game->GetSpace()->AddBody(ship);
			ship->SetLabel(Lang::POLICE_SHIP_REGISTRATION);
			ship->m_equipment.Set(Equip::SLOT_LASER, 0, Equip::PULSECANNON_DUAL_1MW);
			ship->m_equipment.Add(Equip::LASER_COOLING_BOOSTER);
			ship->m_equipment.Add(Equip::ATMOSPHERIC_SHIELDING);
			ship->UpdateStats();
		}
	}
}

bool SpaceStation::IsPortLocked(const int bay) const
{
	SpaceStationType::TBayGroups::const_iterator bayIter = mBayGroups.begin();
	for ( ; bayIter!=mBayGroups.end() ; ++bayIter ) {
		std::vector<int>::const_iterator idIter = (*bayIter).bayIDs.begin();
		for ( ; idIter!=(*bayIter).bayIDs.end() ; ++idIter ) {
			if ((*idIter)==bay) {
				return (*bayIter).inUse;
			}
		}
	}
	// is it safer to return that the is loacked?
	return true;
}

void SpaceStation::LockPort(const int bay, const bool lockIt)
{
	SpaceStationType::TBayGroups::iterator bayIter = mBayGroups.begin();
	for ( ; bayIter!=mBayGroups.end() ; ++bayIter ) {
		std::vector<int>::iterator idIter = (*bayIter).bayIDs.begin();
		for ( ; idIter!=(*bayIter).bayIDs.end() ; ++idIter ) {
			if ((*idIter)==bay) {
				(*bayIter).inUse = lockIt;
				return;
			}
		}
	}
}
