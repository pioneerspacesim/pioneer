// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "SpaceStation.h"

#include "AnimationCurves.h"
#include "Camera.h"
#include "CityOnPlanet.h"
#include "EnumStrings.h"
#include "Frame.h"
#include "Game.h"
#include "GameLog.h"
#include "GameSaveError.h"
#include "JsonUtils.h"
#include "Lang.h"
#include "MathUtil.h"
#include "NavLights.h"
#include "Pi.h"
#include "Planet.h"
#include "Player.h"
#include "Quaternion.h"
#include "Ship.h"
#include "Space.h"
#include "SpaceStationType.h"
#include "StringF.h"
#include "graphics/Renderer.h"
#include "lua/LuaEvent.h"
#include "scenegraph/Animation.h"
#include "scenegraph/CollisionGeometry.h"
#include "scenegraph/MatrixTransform.h"
#include "scenegraph/Model.h"
#include "scenegraph/ModelSkin.h"
#include "utils.h"

SpaceStation::SpaceStation(const SystemBody *sbody) :
	ModelBody(),
	m_type(nullptr)
{
	m_sbody = sbody;

	m_oldAngDisplacement = 0.0;

	m_doorAnimationStep = m_doorAnimationState = 0.0;

	InitStation();
}

SpaceStation::SpaceStation(const Json &jsonObj, Space *space) :
	ModelBody(jsonObj, space),
	m_type(nullptr)
{
	GetModel()->SetLabel(GetLabel());

	try {
		Json spaceStationObj = jsonObj["space_station"];

		m_oldAngDisplacement = 0.0;

		Json shipDockingArray = spaceStationObj["ship_docking"].get<Json::array_t>();
		m_shipDocking.reserve(shipDockingArray.size());
		for (Uint32 i = 0; i < shipDockingArray.size(); i++) {
			m_shipDocking.push_back(shipDocking_t());
			shipDocking_t &sd = m_shipDocking.back();

			Json shipDockingArrayEl = shipDockingArray[i];
			if (shipDockingArrayEl.count("index_for_body"))
				sd.shipIndex = shipDockingArrayEl["index_for_body"];
			if (shipDockingArrayEl.count("stage"))
				sd.stage = shipDockingArrayEl["stage"];
			if (shipDockingArrayEl.count("stage_pos"))
				sd.stagePos = shipDockingArrayEl["stage_pos"]; // For some reason stagePos was saved as a float in pre-JSON system (saved & loaded as double here).
			if (shipDockingArrayEl.count("from_pos"))
				sd.fromPos = shipDockingArrayEl["from_pos"];
			if (shipDockingArrayEl.count("from_rot"))
				sd.fromRot = shipDockingArrayEl["from_rot"];
		}

		// retrieve each of the port details and bay IDs
		Json portArray = spaceStationObj["ports"].get<Json::array_t>();
		m_ports.reserve(portArray.size());
		for (Uint32 i = 0; i < portArray.size(); i++) {
			m_ports.push_back(SpaceStationType::SPort());
			SpaceStationType::SPort &port = m_ports.back();

			Json portArrayEl = portArray[i];
			if (portArrayEl["in_use"].is_boolean())
				port.inUse = portArrayEl["in_use"];
		}

		m_sbody = space->GetSystemBodyByIndex(spaceStationObj["index_for_system_body"]);

		m_doorAnimationStep = spaceStationObj["door_animation_step"];
		m_doorAnimationState = spaceStationObj["door_animation_state"];

		InitStation();

		m_navLights->LoadFromJson(spaceStationObj);
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}
}

void SpaceStation::Init()
{
	SpaceStationType::Init();
}

void SpaceStation::SaveToJson(Json &jsonObj, Space *space)
{
	ModelBody::SaveToJson(jsonObj, space);

	Json spaceStationObj({}); // Create JSON object to contain space station data.

	Json shipDockingArray = Json::array(); // Create JSON array to contain ship docking data.
	for (Uint32 i = 0; i < m_shipDocking.size(); i++) {
		Json shipDockingArrayEl({}); // Create JSON object to contain ship docking.
		Uint32 bodyIndex = space->GetIndexForBody(m_shipDocking[i].ship);
		if (bodyIndex != 0) {
			shipDockingArrayEl["index_for_body"] = bodyIndex;
			shipDockingArrayEl["stage"] = m_shipDocking[i].stage;
			shipDockingArrayEl["stage_pos"] = m_shipDocking[i].stagePos; // stagePos is a double but was saved as a float in pre-JSON system for some reason (saved as double here).
			shipDockingArrayEl["from_pos"] = m_shipDocking[i].fromPos;
			shipDockingArrayEl["from_rot"] = m_shipDocking[i].fromRot;
		}
		shipDockingArray.push_back(shipDockingArrayEl); // Append ship docking object to array.
	}
	spaceStationObj["ship_docking"] = shipDockingArray; // Add ship docking array to space station object.

	// store each of the port details and bay IDs
	Json portArray = Json::array(); // Create JSON array to contain port data.
	for (Uint32 i = 0; i < m_ports.size(); i++) {
		Json portArrayEl({}); // Create JSON object to contain port.

		if (m_ports[i].inUse)
			portArrayEl["in_use"] = m_ports[i].inUse;

		portArray.push_back(portArrayEl); // Append port object to array.
	}
	spaceStationObj["ports"] = portArray; // Add port array to space station object.

	spaceStationObj["index_for_system_body"] = space->GetIndexForSystemBody(m_sbody);

	spaceStationObj["door_animation_step"] = m_doorAnimationStep;
	spaceStationObj["door_animation_state"] = m_doorAnimationState;

	m_navLights->SaveToJson(spaceStationObj);

	jsonObj["space_station"] = spaceStationObj; // Add space station object to supplied object.
}

static float calculate_max_offset_squared(float portMaxShipSize, float shipBboxRad)
{
	float maxOffset = std::max((portMaxShipSize * 0.5f - shipBboxRad), portMaxShipSize / 5.f);
	return maxOffset * maxOffset;
}

void SpaceStation::PostLoadFixup(Space *space)
{
	ModelBody::PostLoadFixup(space);
	for (Uint32 i = 0; i < m_shipDocking.size(); i++) {
		auto &sd = m_shipDocking[i];
		sd.ship = static_cast<Ship *>(space->GetBodyByIndex(m_shipDocking[i].shipIndex));

		if (!sd.ship) continue;

		auto pPort = m_type->FindPortByBay(i);
		const Aabb &bbox = sd.ship->GetAabb();
		const float bboxRad = vector2f(float(bbox.max.x), float(bbox.max.z)).Length();
		sd.maxOffset = calculate_max_offset_squared(pPort->maxShipSize, bboxRad);
	}
}

void SpaceStation::InitStation()
{
	PROFILE_SCOPED()
	m_adjacentCity = 0;
	for (int i = 0; i < NUM_STATIC_SLOTS; i++)
		m_staticSlot[i] = false;
	Random rand(m_sbody->GetSeed());
	const bool ground = m_sbody->GetType() == SystemBody::TYPE_STARPORT_ORBITAL ? false : true;
	const std::string &space_station_type = m_sbody->GetSpaceStationType();
	if (space_station_type != "") {
		m_type = SpaceStationType::FindByName(space_station_type);
		if (m_type == nullptr)
			Output("WARNING: SpaceStation::InitStation wants to initialize a custom station of type %s, but no station type with that id has been found.\n", space_station_type.c_str());
	}
	if (m_type == nullptr)
		m_type = SpaceStationType::RandomStationType(rand, ground);

	if (m_shipDocking.empty()) {
		m_shipDocking.reserve(m_type->NumDockingPorts());
		for (unsigned int i = 0; i < m_type->NumDockingPorts(); i++) {
			m_shipDocking.push_back(shipDocking_t());
		}
		// only (re)set these if we've not come from the ::Load method
		m_doorAnimationStep = m_doorAnimationState = 0.0;
	}
	assert(m_shipDocking.size() == m_type->NumDockingPorts());

	// This SpaceStation's bay ports are an instance of...
	if (m_ports.size() != m_type->Ports().size()) {
		m_ports = m_type->Ports();
	} else {
		// since we might have loaded from JSON we've got a little bit of useful info in m_ports already
		// backup the current data
		auto backup = m_ports;
		// clear it all to default
		m_ports = m_type->Ports();
		// now restore the "inUse" variable only since it's the only bit that might have changed
		for (size_t p = 0; p < m_ports.size(); p++) {
			m_ports[p].inUse = backup[p].inUse;
		}
	}

	SetStatic(ground); // orbital stations are dynamic now

	// XXX hack. if we loaded a game then ModelBody::Load already restored the
	// model and we shouldn't overwrite it
	if (!GetModel())
		SetModel(m_type->ModelName().c_str());

	SceneGraph::Model *model = GetModel();

	m_navLights.reset(new NavLights(model, 2.2f));
	m_navLights->SetEnabled(true);

	if (ground) SetClipRadius(CityOnPlanet::RADIUS); // overrides setmodel

	m_doorAnimation = model->FindAnimation("doors");

	SceneGraph::ModelSkin skin;
	skin.SetDecal("pioneer");

	skin.SetRandomColors(rand);
	skin.Apply(model);
	if (model->SupportsPatterns()) {
		model->SetPattern(rand.Int32(0, model->GetNumPatterns() - 1));
	}
}

SpaceStation::~SpaceStation()
{
	if (m_adjacentCity) delete m_adjacentCity;
}

void SpaceStation::NotifyRemoved(const Body *const removedBody)
{
	for (Uint32 i = 0; i < m_shipDocking.size(); i++) {
		if (m_shipDocking[i].ship == removedBody) {
			m_shipDocking[i].ship = 0;
		}
	}
}

int SpaceStation::GetMyDockingPort(const Ship *s) const
{
	for (Uint32 i = 0; i < m_shipDocking.size(); i++) {
		if (s == m_shipDocking[i].ship) return i;
	}
	return -1;
}

int SpaceStation::NumShipsDocked() const
{
	Sint32 numShipsDocked = 0;
	for (Uint32 i = 0; i < m_shipDocking.size(); i++) {
		if (NULL != m_shipDocking[i].ship)
			++numShipsDocked;
	}
	return numShipsDocked;
}

int SpaceStation::GetFreeDockingPort(const Ship *s) const
{
	assert(s);
	for (unsigned int i = 0; i < m_type->NumDockingPorts(); i++) {
		if (m_shipDocking[i].ship == nullptr) {
			// size-of-ship vs size-of-bay check
			const SpaceStationType::SPort *const pPort = m_type->FindPortByBay(i);
			if (!pPort) continue;

			const Aabb &bbox = s->GetAabb();
			const double bboxRad = bbox.GetRadius();

			if (pPort->minShipSize < bboxRad && bboxRad < pPort->maxShipSize) {
				return i;
			}
		}
	}
	return -1;
}

void SpaceStation::SetDocked(Ship *ship, const int bay)
{
	assert(m_shipDocking.size() > Uint32(bay));
	m_shipDocking[bay].ship = ship;

	// have to do this crap again in case it was called directly (Ship::SetDockWith())
	ship->SetFlightState(Ship::DOCKED);
	ship->SetVelocity(vector3d(0.0));
	ship->SetAngVelocity(vector3d(0.0));
	PositionDockedShip(ship, bay);
	SwitchToStage(bay, DockStage::DOCKED);
}

void SpaceStation::SwapDockedShipsPort(const int oldBay, const int newBay)
{
	if (oldBay == newBay)
		return;

	// set new location
	Ship *ship = m_shipDocking[oldBay].ship;
	assert(ship);
	ship->SetDockedWith(this, newBay);

	m_shipDocking[oldBay].ship = nullptr;
	SwitchToStage(oldBay, DockStage::NONE);
}

bool SpaceStation::LaunchShip(Ship *ship, const int bay)
{
	if (IsPortLocked(bay)) return false; // another ship docking

	if (ship->ManualDocking()) {
		// no one except the player can launch without the autopilot
		assert(ship->IsType(ObjectType::PLAYER));
		auto p = static_cast<Player*>(ship);
		p->DoFixspeedTakeoff(this);
		SwitchToStage(bay, DockStage::LEAVE);
		return true;;
	}

	shipDocking_t &sd = m_shipDocking[bay];
	if (SpaceStationType::IsUndockStage(sd.stage)) return true; // already launching
	LockPort(bay, true);

	sd.ship = ship;
	sd.stagePos = 0.0;

	m_doorAnimationStep = 0.3; // open door

	sd.fromPos = (ship->GetPosition() - GetPosition()) * GetOrient(); // station space
	sd.fromRot = Quaterniond::FromMatrix3x3(GetOrient().Transpose() * ship->GetOrient());

	ship->SetFlightState(Ship::UNDOCKING);
	SwitchToStage(bay, DockStage::UNDOCK_BEGIN);

	return true;
}

// gets number of undocked ships within a given radius from the station
int SpaceStation::GetNearbyTraffic(double radius)
{
	int shipsNearby = 0;
	Space::BodyNearList traffic = Pi::game->GetSpace()->GetBodiesMaybeNear(this, radius);
	for (Body *body : traffic) {
		if (!body->IsType(ObjectType::SHIP)) continue;
		shipsNearby++;
	}
	return shipsNearby - NumShipsDocked();
}

bool SpaceStation::GetDockingClearance(Ship *s)
{
	assert(m_shipDocking.size() == m_type->NumDockingPorts());
	for (Uint32 i = 0; i < m_shipDocking.size(); i++) {
		if (m_shipDocking[i].ship == s) {
			LuaEvent::Queue("onDockingClearanceDenied", this, s,
				EnumStrings::GetString("DockingRefusedReason", int(DockingRefusedReason::ClearanceAlreadyGranted)));
			return (m_shipDocking[i].stage != DockStage::NONE); // grant docking only if the ship is not already docked/undocking
		}
	}

	const Aabb &bbox = s->GetAabb();
	const float bboxRad = vector2f(float(bbox.max.x), float(bbox.max.z)).Length();

	for (Uint32 i = 0; i < m_shipDocking.size(); i++) {
		// initial unoccupied check
		if (m_shipDocking[i].ship != 0) continue;

		// size-of-ship vs size-of-bay check
		const SpaceStationType::SPort *const pPort = m_type->FindPortByBay(i);
		if (!pPort) continue;

		// distance-to-station check
		const double shipDist = s->GetPositionRelTo(this).Length();
		double requestDist = 100000.0; //100km
		if (s->IsType(ObjectType::PLAYER) && shipDist > requestDist) {
			LuaEvent::Queue("onDockingClearanceDenied", this, s,
				EnumStrings::GetString("DockingRefusedReason", int(DockingRefusedReason::TooFarFromStation)));
			return false;
		}

		if (pPort->minShipSize < bboxRad && bboxRad < pPort->maxShipSize) {
			shipDocking_t &sd = m_shipDocking[i];
			sd.ship = s;
			sd.stagePos = 0;
			sd.maxOffset = calculate_max_offset_squared(pPort->maxShipSize, bboxRad);
			LuaEvent::Queue("onDockingClearanceGranted", this, s);
			SwitchToStage(i, DockStage::CLEARANCE_GRANTED);
			return true;
		}
	}

	LuaEvent::Queue("onDockingClearanceDenied", this, s,
		EnumStrings::GetString("DockingRefusedReason", int(DockingRefusedReason::NoBaysAvailable)));
	return false;
}

bool SpaceStation::OnCollision(Body *b, Uint32 flags, double relVel)
{
	if (!b->IsType(ObjectType::SHIP)) return true;
	if (!(flags & (SceneGraph::CollisionGeometry::DOCKING | SceneGraph::CollisionGeometry::ENTRANCE))) return true;

	Ship *s = static_cast<Ship *>(b);
	if(s->ManualDocking() && (flags & SceneGraph::CollisionGeometry::ENTRANCE)) return false;

	bool touchOrbitalPad = (flags & SceneGraph::CollisionGeometry::DOCKING) && !IsGroundStation();

	int bay = -1;
	for (Uint32 i = 0; i < m_shipDocking.size(); i++) {
		if (m_shipDocking[i].ship == s) {
			bay = i;
			break;
		}
	}

	bool bayUnavailable = bay == -1 || IsPortLocked(bay) || m_shipDocking[bay].stage == DockStage::NONE;
	if (bayUnavailable) {
		return DoShipDamage(s, flags, relVel);
	}

	if (IsGroundStation() || touchOrbitalPad) {
		// must be oriented sensibly and have wheels down
		matrix4x4d bayTrans = GetBayTransform(bay);

		vector3d dockingNormal = bayTrans.Up();
		const double dot = s->GetOrient().VectorY().Dot(dockingNormal);
		if ((dot < 0.99) || (s->GetWheelState() < 1.0))
		{
			return DoShipDamage(s, flags, relVel); // <0.99 harsh?
		}
		// check speed
		if (s->GetVelocity().Length() > MAX_LANDING_SPEED)
		{
			return DoShipDamage(s, flags, relVel);
		}
		// check if you're near your pad
		float dist = (s->GetPosition() - bayTrans.GetTranslate()).LengthSqr();
		// docking allowed only if inside a circle 70% greater than pad itself (*1.7)
		float maxDist = static_cast<float>(m_type->FindPortByBay(bay)->maxShipSize / 2) * 1.7;
		if (dist > (maxDist * maxDist))
		{
			return DoShipDamage(s, flags, relVel);
		}
	}

	// docking is in progress
	if (s->GetFlightState() == Ship::DOCKING) return false;

	// launch docking
	// set up a control structure
	// from now on, the location of the ship will be set by the station using this data
	shipDocking_t &sd = m_shipDocking[bay];
	sd.ship = s;
	sd.stagePos = 0;
	// capture the current location of the ship
	sd.fromPos = (s->GetPosition() - GetPosition()) * GetOrient(); // station space
	sd.fromRot = Quaterniond::FromMatrix3x3(GetOrient().Transpose() * s->GetOrient());
	// prepare the ship for passive existence
	s->SetFlightState(Ship::DOCKING);
	s->SetVelocity(vector3d(0.0));
	s->SetAngVelocity(vector3d(0.0));
	s->ClearThrusterState();

	SwitchToStage(bay, (touchOrbitalPad || m_type->NumUndockStages() == 0) ? DockStage::TOUCHDOWN : DockStage::DOCK_ANIMATION_1);

	return false;
}

bool SpaceStation::DoShipDamage(Ship *s, Uint32 flags, double relVel)
{
	if (s == nullptr) return false;
	s->DynamicBody::OnCollision(this, flags, relVel);
	return true;
}

void SpaceStation::SwitchToStage(Uint32 bay, DockStage stage)
{
	shipDocking_t &dt = m_shipDocking[bay];

	switch (stage) {

	case DockStage::TOUCHDOWN: {
		dt.ship->ClearThrusterState();
		// remember the position of the ship relative to the pad
		matrix4x4d padSpace = GetBayTransform(bay);
		padSpace.Translate(0.0, -dt.ship->GetLandingPosOffset(), 0.0);
		padSpace = padSpace.Inverse();
		matrix3x3d shipToPadOrient = padSpace.GetOrient() * dt.ship->GetOrient();
		// the position of the ship relative to the pad will be saved here
		// all subsequent stages will use this
		dt.fromRot = Quaterniond::FromMatrix3x3(shipToPadOrient);
		dt.fromPos = padSpace * dt.ship->GetPosition();

		SwitchToStage(bay, DockStage::LEVELING);
		break;
	}

	// if we want to generate a docking event
	case DockStage::JUST_DOCK: {
		LuaEvent::Queue("onShipDocked", dt.ship, this);
		m_doorAnimationStep = -0.3; // close door
		dt.ship->SetDockedWith(this, bay);
		break;
	}

	case DockStage::UNDOCK_BEGIN:
		if (m_type->NumUndockStages() > 0) {
			SwitchToStage(bay, DockStage::UNDOCK_ANIMATION_1);
		} else {
			SwitchToStage(bay, DockStage::UNDOCK_END);
		}
		break;

	case DockStage::UNDOCK_END:
		dt.ship->SetAngVelocity(GetAngVelocity());
		if (m_type->IsSurfaceStation()) {
			dt.ship->SetThrusterState(1, 1.0); // up
		} else {
			dt.ship->SetThrusterState(2, -1.0); // forward
		}
		dt.ship->SetFlightState(Ship::FLYING);
		SwitchToStage(bay, DockStage::LEAVE);
		break;

	case DockStage::LEAVE:
		LuaEvent::Queue("onShipUndocked", dt.ship, this);
		dt.ship = nullptr;
		dt.stagePos = 0;
		dt.maxOffset = 0;
		LockPort(bay, false);
		m_doorAnimationStep = -0.3; // close door
		SwitchToStage(bay, DockStage::NONE);
		break;

	case DockStage::CLEARANCE_GRANTED:
		m_doorAnimationStep = 0.3; // open door
		dt.stage = stage;
		break;

	default:
		dt.stage = stage;
		break;
	}
}

void SpaceStation::DockingUpdate(const double timeStep)
{
	for (Uint32 i = 0; i < m_shipDocking.size(); i++) {
		shipDocking_t &dt = m_shipDocking[i];
		if (!dt.ship) continue;

		switch (dt.stage) {

		case DockStage::LEVELING: // aligning the ship with the plane of the pad
			if (!LevelShip(dt.ship, i, timeStep)) continue;
			SwitchToStage(i, DockStage::REPOSITION);
			continue;

		case DockStage::REPOSITION:
			// moving the ship if it is displaced too much
			// until it is not displaced too much
			// the ship should already be correctly aligned with the wheels with the pad
			if (dt.fromPos.LengthSqr() > dt.maxOffset) {
				const float MOVE_PER_FRAME = 0.1; // meter per frame
				// with perfect alignmend fromPos is zero
				dt.fromPos -= dt.fromPos.Normalized() * MOVE_PER_FRAME;
			} else {
				SwitchToStage(i, DockStage::JUST_DOCK);
			}
			continue;

		case DockStage::DOCKED:
			continue;

		case DockStage::CLEARANCE_GRANTED: // waiting for collision
			if (dt.stagePos >= 1.0) {
				LuaEvent::Queue("onDockingClearanceExpired", this, dt.ship);
				dt.ship = nullptr;
				m_doorAnimationStep = -0.3; // close door
				SwitchToStage(i, DockStage::NONE);
			}
			continue;

		default:
			break;
		}

		// by default docking or undocking animation occurs

		// next animation step
		double stageDuration = SpaceStationType::IsDockStage(dt.stage) ?
			GetDockAnimStageDuration(i, dt.stage) :
			GetUndockAnimStageDuration(i, dt.stage);
		dt.stagePos += timeStep / stageDuration;

		// next animation stage
		if (dt.stagePos > 1.0) {

			// overrun due to too much time acceleration?
			if (dt.stagePos > 1.05) {
				dt.stagePos = 1.0;
				PositionDockingShip(dt.ship, i);
			}

			// transition between docking stages
			dt.stagePos = 0;
			if (dt.stage == m_type->LastDockStage()) {
				// end dock animation
				SwitchToStage(i, DockStage::TOUCHDOWN);
				continue;
			} else if (dt.stage == m_type->LastUndockStage()) {
				// end undock animation
				SwitchToStage(i, DockStage::UNDOCK_END);
				continue;
			} else {
				dt.fromPos = (dt.ship->GetPosition() - GetPosition()) * GetOrient();
				dt.fromRot = Quaterniond::FromMatrix3x3(GetOrient().Transpose() * dt.ship->GetOrient());
				// continue dock or undock animation
				SwitchToStage(i, SpaceStationType::NextAnimStage(dt.stage));
				continue;
			}
		}
	}

	m_doorAnimationState = Clamp(m_doorAnimationState + m_doorAnimationStep * timeStep, 0.0, 1.0);
	if (m_doorAnimation)
		m_doorAnimation->SetProgress(m_doorAnimationState);
}

bool SpaceStation::LevelShip(Ship *ship, int bay, const float timeStep)
{
	shipDocking_t &dt = m_shipDocking[bay];

	auto shipOrient = dt.fromRot.ToMatrix3x3<double>();
	vector3d dist = dt.fromPos;
	vector3d dockingNormal(0.0, 1.0, 0.0);

	double cosUp = dockingNormal.Dot(shipOrient.VectorY());
	if (cosUp < 0.999999) {
		// need level ship
		double angle;
		if (cosUp < 0.985)
			angle = -0.8 * timeStep;
		else
			angle = -acos(cosUp);
		vector3d rotAxis = dockingNormal.Cross(shipOrient.VectorY());
		rotAxis = rotAxis.NormalizedSafe();

		Quaterniond rot(angle, rotAxis);
		dt.fromRot = (rot * dt.fromRot).Normalized();
	}
	if (fabs(dist.y) > 0.01) {
		dt.fromPos.y = 0.0;
	}
	return (cosUp >= 0.999999) && (fabs(dist.y) < 0.01);
}

void SpaceStation::PositionDockingShip(Ship *ship, int bay) const
{
	const shipDocking_t &dt = m_shipDocking[bay];
	DockStage pStage = m_type->PivotStage(dt.stage);
	// the position of the ship at this stage is not controlled by the station
	if (pStage == DockStage::NONE) return;
	if (pStage == DockStage::MANUAL) {
		// at these stages the ship has already touched the pad,
		// so it is positioned based on dt.fromPos
		PositionDockedShip(ship, bay);
		return;
	}

	auto stageTrans = matrix4x4d(m_type->GetStageTransform(bay, pStage));

	// the last stage of the docking animation should take into account the
	// displacement of the center of the ship
	if (dt.stage == m_type->LastDockStage()) {
		stageTrans.Translate(0.0, -dt.ship->GetLandingPosOffset(), 0.0);
	}

	float ratio = pStage == DockStage::DOCK_ANIMATION_1 ?
		AnimationCurves::  OutSineEasing(dt.stagePos):
		AnimationCurves::InOutSineEasing(dt.stagePos);

	vector3d interPos = MathUtil::mix<vector3d, double>(dt.fromPos, stageTrans.GetTranslate(), ratio);

	ship->SetPosition(GetPosition() + GetOrient() * interPos);
	// use quaternion spherical linear interpolation to do
	// rotation smoothly
	Quaterniond wantQuat = Quaterniond::FromMatrix3x3(stageTrans.GetOrient());
	// use Slerp so that the turn takes the smallest path
	Quaterniond q = Quaterniond::Slerp(dt.fromRot, wantQuat, AnimationCurves::InOutSineEasing(dt.stagePos));
	ship->SetOrient(GetOrient() * q.ToMatrix3x3<double>());
}

void SpaceStation::PositionDockedShip(Ship *ship, int bay) const
{
	const shipDocking_t &dt = m_shipDocking[bay];
	// pad placement in the world:
	auto padTrans = GetBayTransform(bay);
	// ship center height
	padTrans.Translate(0.0, -ship->GetLandingPosOffset(), 0.0);

	ship->SetPosition(padTrans * dt.fromPos);
	ship->SetOrient(padTrans.GetOrient() * dt.fromRot.ToMatrix3x3<double>());
}

void SpaceStation::StaticUpdate(const float timeStep)
{
	DockingUpdate(timeStep);
	m_navLights->Update(timeStep);
}

void SpaceStation::TimeStepUpdate(const float timeStep)
{
	// rotate the thing
	double len = m_type->AngVel() * timeStep;
	if (!is_zero_exact(len)) {
		matrix3x3d r = matrix3x3d::RotateY(-len); // RotateY is backwards
		SetOrient(r * GetOrient());
	}
	m_oldAngDisplacement = len;

	// reposition the ships that are docked or docking here
	for (unsigned int i = 0; i < m_type->NumDockingPorts(); i++) {
		const shipDocking_t &dt = m_shipDocking[i];
		if (!dt.ship) { //free
			m_navLights->SetColor(i + 1, NavLights::NAVLIGHT_OFF);
			continue;
		}
		if (SpaceStationType::IsDockStage(dt.stage)) {
			m_navLights->SetColor(i + 1, NavLights::NAVLIGHT_GREEN);
			m_navLights->SetMask(i + 1, 0x33); // 00110011 on two off two
		}
		if (SpaceStationType::IsUndockStage(dt.stage)) { // undocking anim
			m_navLights->SetColor(i + 1, NavLights::NAVLIGHT_YELLOW);
		}
		if (dt.stage == DockStage::JUST_DOCK) { // just docked
			m_navLights->SetColor(i + 1, NavLights::NAVLIGHT_BLUE);
			m_navLights->SetMask(i + 1, 0xf6); // 11110110
		}
		if (dt.ship->GetFlightState() == Ship::DOCKED) { //docked
			PositionDockedShip(dt.ship, i);
		} else if (dt.ship->GetFlightState() == Ship::DOCKING || dt.ship->GetFlightState() == Ship::UNDOCKING) {
			PositionDockingShip(dt.ship, i);
		}
	}

	ModelBody::TimeStepUpdate(timeStep);
}

void SpaceStation::UpdateInterpTransform(double alpha)
{
	double len = m_oldAngDisplacement * (1.0 - alpha);
	if (!is_zero_exact(len)) {
		matrix3x3d rot = matrix3x3d::RotateY(len); // RotateY is backwards
		m_interpOrient = rot * GetOrient();
	} else
		m_interpOrient = GetOrient();
	m_interpPos = GetPosition();
}

bool SpaceStation::IsGroundStation() const
{
	return m_type->IsSurfaceStation();
}

// Renders space station and adjacent city if applicable
static const double SQRMAXCITYDIST = 1e5 * 1e5;

void SpaceStation::Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	Body *b = Frame::GetFrame(GetFrame())->GetBody();
	assert(b);

	if (!b->IsType(ObjectType::PLANET)) {
		// orbital spaceport -- don't make city turds or change lighting based on atmosphere
		RenderModel(r, camera, viewCoords, viewTransform);
		m_navLights->Render(r);
		r->GetStats().AddToStatCount(Graphics::Stats::STAT_SPACESTATIONS, 1);
	} else {
		// don't render city if too far away
		if (viewCoords.LengthSqr() >= SQRMAXCITYDIST) {
			return;
		}

		if (!m_adjacentCity) {
			m_adjacentCity = new CityOnPlanet(static_cast<Planet *>(b), this, m_sbody->GetSeed());
			// Update clipping radius
			SetClipRadius(m_adjacentCity->GetClipRadius());
		}

		m_adjacentCity->Render(r, camera->GetContext()->GetFrustum(), this, viewCoords, viewTransform);

		RenderModel(r, camera, viewCoords, viewTransform);
		m_navLights->Render(r);

		r->GetStats().AddToStatCount(Graphics::Stats::STAT_GROUNDSTATIONS, 1);
	}
}

void SpaceStation::SetLabel(const std::string &label)
{
	assert(GetModel());
	GetModel()->SetLabel(label);
	Body::SetLabel(label);
}

// find an empty position for a static ship and mark it as used. these aren't
// saved and are only needed to help modules place bulk ships. this isn't a
// great place for this, but its gotta be tracked somewhere
bool SpaceStation::AllocateStaticSlot(int &slot)
{
	// no slots at ground stations
	if (IsGroundStation())
		return false;

	for (int i = 0; i < NUM_STATIC_SLOTS; i++) {
		if (!m_staticSlot[i]) {
			m_staticSlot[i] = true;
			slot = i;
			return true;
		}
	}

	return false;
}

vector3d SpaceStation::GetTargetIndicatorPosition() const
{
	// return the next waypoint if permission has been granted for player,
	// and the docking point's position once the docking anim starts
	for (Uint32 i = 0; i < m_shipDocking.size(); i++) {
		if ((m_shipDocking[i].ship == Pi::player) &&
				(m_shipDocking[i].stage == DockStage::CLEARANCE_GRANTED)) { // last part is "not currently docked" ????
			return vector3d(m_type->GetStageTransform(i, DockStage::DOCKED).GetTranslate());
		}
	}
	return Body::GetTargetIndicatorPosition();
}

bool SpaceStation::IsPortLocked(const int bay) const
{
	for (auto &bayIter : m_ports) {
		for (auto &idIter : bayIter.bayIDs) {
			if (idIter.first == bay) {
				return bayIter.inUse;
			}
		}
	}
	// is it safer to return that the is loacked?
	return true;
}

void SpaceStation::LockPort(const int bay, const bool lockIt)
{
	for (auto &bayIter : m_ports) {
		for (auto &idIter : bayIter.bayIDs) {
			if (idIter.first == bay) {
				bayIter.inUse = lockIt;
				return;
			}
		}
	}
}

matrix4x4d SpaceStation::GetBayTransform(Uint32 bay) const {
	matrix4x4d bayTrans = matrix4x4d::Translation(GetPosition()) * GetOrient();
	bayTrans = bayTrans * matrix4x4d(m_type->GetStageTransform(bay, DockStage::DOCKED));
	return bayTrans;
}

double SpaceStation::GetDockAnimStageDuration(int bay, DockStage stage) const
{
	if (stage == DockStage::NONE) return 0.0;
	if (m_type->IsSurfaceStation()) return 0.0;
	auto dt = m_shipDocking[bay];
	vector3f p1 = vector3f(dt.fromPos);
	vector3f p2 = m_type->GetStageTransform(bay, stage).GetTranslate();
	float stageLength = (p2 - p1).Length();
	float averageVelocity = stage == m_type->LastDockStage() ? 10 : 30; // m/s
	return stageLength / averageVelocity;
}

double SpaceStation::GetUndockAnimStageDuration(int bay, DockStage stage) const
{
	if (m_type->IsSurfaceStation()) return 0.0;
	auto dt = m_shipDocking[bay];
	vector3f p1 = vector3f(dt.fromPos);
	vector3f p2 = m_type->GetStageTransform(bay, stage).GetTranslate();
	float stageLength = (p2 - p1).Length();
	float averageVelocity = 10; // m/s
	return stageLength / averageVelocity;
}

