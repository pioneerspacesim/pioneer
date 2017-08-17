// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
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
#include "Serializer.h"
#include "Ship.h"
#include "Space.h"
#include "StringF.h"
#include "ShipCpanel.h"
#include "galaxy/StarSystem.h"
#include "graphics/Graphics.h"
#include "scenegraph/ModelSkin.h"
#include "json/JsonUtils.h"
#include <algorithm>

void SpaceStation::Init()
{
	SpaceStationType::Init();
}

void SpaceStation::SaveToJson(Json::Value &jsonObj, Space *space)
{
	ModelBody::SaveToJson(jsonObj, space);

	Json::Value spaceStationObj(Json::objectValue); // Create JSON object to contain space station data.

	Json::Value shipDockingArray(Json::arrayValue); // Create JSON array to contain ship docking data.
	for (Uint32 i = 0; i<m_shipDocking.size(); i++)
	{
		Json::Value shipDockingArrayEl(Json::objectValue); // Create JSON object to contain ship docking.
		shipDockingArrayEl["index_for_body"] = space->GetIndexForBody(m_shipDocking[i].ship);
		shipDockingArrayEl["stage"] = m_shipDocking[i].stage;
		shipDockingArrayEl["stage_pos"] = DoubleToStr(m_shipDocking[i].stagePos); // stagePos is a double but was saved as a float in pre-JSON system for some reason (saved as double here).
		VectorToJson(shipDockingArrayEl, m_shipDocking[i].fromPos, "from_pos");
		QuaternionToJson(shipDockingArrayEl, m_shipDocking[i].fromRot, "from_rot");
		shipDockingArray.append(shipDockingArrayEl); // Append ship docking object to array.
	}
	spaceStationObj["ship_docking"] = shipDockingArray; // Add ship docking array to space station object.

	// store each of the port details and bay IDs
	Json::Value portArray(Json::arrayValue); // Create JSON array to contain port data.
	for (Uint32 i = 0; i < m_ports.size(); i++)
	{
		Json::Value portArrayEl(Json::objectValue); // Create JSON object to contain port.

		portArrayEl["min_ship_size"] = m_ports[i].minShipSize;
		portArrayEl["max_ship_size"] = m_ports[i].maxShipSize;
		portArrayEl["in_use"] = m_ports[i].inUse;

		Json::Value bayArray(Json::arrayValue); // Create JSON array to contain bay data.
		for (Uint32 j = 0; j<m_ports[i].bayIDs.size(); j++)
		{
			Json::Value bayArrayEl(Json::objectValue); // Create JSON object to contain bay.
			bayArrayEl["bay_id"] = m_ports[i].bayIDs[j].first;
			bayArrayEl["name"] = m_ports[i].bayIDs[j].second;
			bayArray.append(bayArrayEl); // Append bay object to array.
		}
		portArrayEl["bays"] = bayArray; // Add bay array to port object.

		portArray.append(portArrayEl); // Append port object to array.
	}
	spaceStationObj["ports"] = portArray; // Add port array to space station object.

	spaceStationObj["index_for_system_body"] = space->GetIndexForSystemBody(m_sbody);

	spaceStationObj["door_animation_step"] = DoubleToStr(m_doorAnimationStep);
	spaceStationObj["door_animation_state"] = DoubleToStr(m_doorAnimationState);

	m_navLights->SaveToJson(spaceStationObj);

	jsonObj["space_station"] = spaceStationObj; // Add space station object to supplied object.
}

void SpaceStation::LoadFromJson(const Json::Value &jsonObj, Space *space)
{
	ModelBody::LoadFromJson(jsonObj, space);
	GetModel()->SetLabel(GetLabel());

	if (!jsonObj.isMember("space_station")) throw SavedGameCorruptException();
	Json::Value spaceStationObj = jsonObj["space_station"];

	if (!spaceStationObj.isMember("ship_docking")) throw SavedGameCorruptException();
	if (!spaceStationObj.isMember("ports")) throw SavedGameCorruptException();
	if (!spaceStationObj.isMember("index_for_system_body")) throw SavedGameCorruptException();
	if (!spaceStationObj.isMember("door_animation_step")) throw SavedGameCorruptException();
	if (!spaceStationObj.isMember("door_animation_state")) throw SavedGameCorruptException();

	m_oldAngDisplacement = 0.0;

	Json::Value shipDockingArray = spaceStationObj["ship_docking"];
	if (!shipDockingArray.isArray()) throw SavedGameCorruptException();
	m_shipDocking.reserve(shipDockingArray.size());
	for (Uint32 i = 0; i < shipDockingArray.size(); i++)
	{
		m_shipDocking.push_back(shipDocking_t());
		shipDocking_t &sd = m_shipDocking.back();

		Json::Value shipDockingArrayEl = shipDockingArray[i];
		if (!shipDockingArrayEl.isMember("index_for_body")) throw SavedGameCorruptException();
		if (!shipDockingArrayEl.isMember("stage")) throw SavedGameCorruptException();
		if (!shipDockingArrayEl.isMember("stage_pos")) throw SavedGameCorruptException();
		if (!shipDockingArrayEl.isMember("from_pos")) throw SavedGameCorruptException();
		if (!shipDockingArrayEl.isMember("from_rot")) throw SavedGameCorruptException();

		sd.shipIndex = shipDockingArrayEl["index_for_body"].asInt();
		sd.stage = shipDockingArrayEl["stage"].asInt();
		sd.stagePos = StrToDouble(shipDockingArrayEl["stage_pos"].asString()); // For some reason stagePos was saved as a float in pre-JSON system (saved & loaded as double here).
		JsonToVector(&(sd.fromPos), shipDockingArrayEl, "from_pos");
		JsonToQuaternion(&(sd.fromRot), shipDockingArrayEl, "from_rot");
	}

	// retrieve each of the port details and bay IDs
	Json::Value portArray = spaceStationObj["ports"];
	if (!portArray.isArray()) throw SavedGameCorruptException();
	m_ports.reserve(portArray.size());
	for (Uint32 i = 0; i < portArray.size(); i++)
	{
		m_ports.push_back(SpaceStationType::SPort());
		SpaceStationType::SPort &port = m_ports.back();

		Json::Value portArrayEl = portArray[i];
		if (!portArrayEl.isMember("min_ship_size")) throw SavedGameCorruptException();
		if (!portArrayEl.isMember("max_ship_size")) throw SavedGameCorruptException();
		if (!portArrayEl.isMember("in_use")) throw SavedGameCorruptException();
		if (!portArrayEl.isMember("bays")) throw SavedGameCorruptException();

		port.minShipSize = portArrayEl["min_ship_size"].asInt();
		port.maxShipSize = portArrayEl["max_ship_size"].asInt();
		port.inUse = portArrayEl["in_use"].asBool();

		Json::Value bayArray = portArrayEl["bays"];
		if (!bayArray.isArray()) throw SavedGameCorruptException();
		port.bayIDs.reserve(bayArray.size());
		for (Uint32 j = 0; j < bayArray.size(); j++)
		{
			Json::Value bayArrayEl = bayArray[j];
			if (!bayArrayEl.isMember("bay_id")) throw SavedGameCorruptException();
			if (!bayArrayEl.isMember("name")) throw SavedGameCorruptException();

			port.bayIDs.push_back(std::make_pair(bayArrayEl["bay_id"].asInt(), bayArrayEl["name"].asString()));
		}
	}

	m_sbody = space->GetSystemBodyByIndex(spaceStationObj["index_for_system_body"].asUInt());

	m_doorAnimationStep = StrToDouble(spaceStationObj["door_animation_step"].asString());
	m_doorAnimationState = StrToDouble(spaceStationObj["door_animation_state"].asString());

	InitStation();

	m_navLights->LoadFromJson(spaceStationObj);
}

void SpaceStation::PostLoadFixup(Space *space)
{
	ModelBody::PostLoadFixup(space);
	for (Uint32 i=0; i<m_shipDocking.size(); i++) {
		m_shipDocking[i].ship = static_cast<Ship*>(space->GetBodyByIndex(m_shipDocking[i].shipIndex));
	}
}

SpaceStation::SpaceStation(const SystemBody *sbody): ModelBody(), m_type(nullptr)
{
	m_sbody = sbody;

	m_oldAngDisplacement = 0.0;

	m_doorAnimationStep = m_doorAnimationState = 0.0;

	InitStation();
}

void SpaceStation::InitStation()
{
	m_adjacentCity = 0;
	for(int i=0; i<NUM_STATIC_SLOTS; i++) m_staticSlot[i] = false;
	Random rand(m_sbody->GetSeed());
	const bool ground = m_sbody->GetType() == SystemBody::TYPE_STARPORT_ORBITAL ? false : true;
	const std::string &space_station_type = m_sbody->GetSpaceStationType();
	if(space_station_type != "") {
		m_type = SpaceStationType::FindByName(space_station_type);
		if(m_type == nullptr)
			Output("WARNING: SpaceStation::InitStation wants to initialize a custom station of type %s, but no station type with that id has been found.\n", space_station_type.c_str());
	}
	if(m_type == nullptr)
		m_type = SpaceStationType::RandomStationType(rand, ground);

	if(m_shipDocking.empty()) {
		m_shipDocking.reserve(m_type->NumDockingPorts());
		for (unsigned int i=0; i<m_type->NumDockingPorts(); i++) {
			m_shipDocking.push_back(shipDocking_t());
		}
		// only (re)set these if we've not come from the ::Load method
		m_doorAnimationStep = m_doorAnimationState = 0.0;
	}
	assert(m_shipDocking.size() == m_type->NumDockingPorts());

	// This SpaceStation's bay ports are an instance of...
	m_ports = m_type->Ports();

	SetStatic(ground);			// orbital stations are dynamic now

	// XXX hack. if we loaded a game then ModelBody::Load already restored the
	// model and we shouldn't overwrite it
	if (!GetModel())
		SetModel(m_type->ModelName().c_str());

	SceneGraph::Model *model = GetModel();

	m_navLights.reset(new NavLights(model, 2.2f));
	m_navLights->SetEnabled(true);

	if (ground) SetClipRadius(CITY_ON_PLANET_RADIUS);		// overrides setmodel

	m_doorAnimation = model->FindAnimation("doors");

	SceneGraph::ModelSkin skin;
	skin.SetDecal("pioneer");

	skin.SetRandomColors(rand);
	skin.Apply(model);
	if (model->SupportsPatterns()) {
		model->SetPattern(rand.Int32(0, model->GetNumPatterns()-1));
	}
}

SpaceStation::~SpaceStation()
{
	if (m_adjacentCity) delete m_adjacentCity;
}

void SpaceStation::NotifyRemoved(const Body* const removedBody)
{
	for (Uint32 i=0; i<m_shipDocking.size(); i++) {
		if (m_shipDocking[i].ship == removedBody) {
			m_shipDocking[i].ship = 0;
		}
	}
}

int SpaceStation::GetMyDockingPort(const Ship *s) const
{
	for (Uint32 i=0; i<m_shipDocking.size(); i++) {
		if (s == m_shipDocking[i].ship) return i;
	}
	return -1;
}

int SpaceStation::NumShipsDocked() const
{
	Sint32 numShipsDocked = 0;
	for (Uint32 i=0; i<m_shipDocking.size(); i++) {
		if (NULL != m_shipDocking[i].ship)
			++numShipsDocked;
	}
	return numShipsDocked;
}

int SpaceStation::GetFreeDockingPort(const Ship *s) const
{
	assert(s);
	for (unsigned int i=0; i<m_type->NumDockingPorts(); i++) {
		if (m_shipDocking[i].ship == nullptr) {
			// size-of-ship vs size-of-bay check
			const SpaceStationType::SPort *const pPort = m_type->FindPortByBay(i);
			if( !pPort ) continue;

			const Aabb &bbox = s->GetAabb();
			const double bboxRad = bbox.GetRadius();

			if( pPort->minShipSize < bboxRad && bboxRad < pPort->maxShipSize ) {
				return i;
			}
		}
	}
	return -1;
}

void SpaceStation::SetDocked(Ship *ship, const int port)
{
	assert(m_shipDocking.size() > Uint32(port));
	m_shipDocking[port].ship = ship;
	m_shipDocking[port].stage = m_type->NumDockingStages()+3;

	// have to do this crap again in case it was called directly (Ship::SetDockWith())
	ship->SetFlightState(Ship::DOCKED);
	ship->SetVelocity(vector3d(0.0));
	ship->SetAngVelocity(vector3d(0.0));
	PositionDockedShip(ship, port);
}

void SpaceStation::SwapDockedShipsPort(const int oldPort, const int newPort)
{
	if( oldPort == newPort )
		return;

	// set new location
	Ship *ship = m_shipDocking[oldPort].ship;
	assert(ship);
	ship->SetDockedWith(this, newPort);

	m_shipDocking[oldPort].ship = 0;
	m_shipDocking[oldPort].stage = 0;
}

bool SpaceStation::LaunchShip(Ship *ship, const int port)
{
	shipDocking_t &sd = m_shipDocking[port];
	if (sd.stage < 0) return true;			// already launching
	if (IsPortLocked(port)) return false;	// another ship docking
	LockPort(port, true);

	sd.ship = ship;
	sd.stage = -1;
	sd.stagePos = 0.0;

	m_doorAnimationStep = 0.3; // open door

	const vector3d up = ship->GetOrient().VectorY().Normalized() * ship->GetLandingPosOffset();

	sd.fromPos = (ship->GetPosition() - GetPosition() + up) * GetOrient();	// station space
	sd.fromRot = Quaterniond::FromMatrix3x3(GetOrient().Transpose() * ship->GetOrient());

	ship->SetFlightState(Ship::UNDOCKING);

	return true;
}

bool SpaceStation::GetDockingClearance(Ship *s, std::string &outMsg)
{
	assert(m_shipDocking.size() == m_type->NumDockingPorts());
	for (Uint32 i=0; i<m_shipDocking.size(); i++) {
		if (m_shipDocking[i].ship == s) {
			outMsg = stringf(Lang::CLEARANCE_ALREADY_GRANTED_BAY_N, formatarg("bay", i+1));
			return (m_shipDocking[i].stage > 0); // grant docking only if the ship is not already docked/undocking
		}
	}

	const Aabb &bbox = s->GetAabb();
	const float bboxRad = vector2f( float(bbox.max.x), float(bbox.max.z)).Length();

	for (Uint32 i=0; i<m_shipDocking.size(); i++) {
		// initial unoccupied check
		if (m_shipDocking[i].ship != 0) continue;

		// size-of-ship vs size-of-bay check
		const SpaceStationType::SPort *const pPort = m_type->FindPortByBay(i);
		if( !pPort ) continue;

		if( pPort->minShipSize < bboxRad && bboxRad < pPort->maxShipSize ) {
			shipDocking_t &sd = m_shipDocking[i];
			sd.ship = s;
			sd.stage = 1;
			sd.stagePos = 0;
			// Note: maxOffset is squared
			sd.maxOffset = std::max((pPort->maxShipSize/2 - bboxRad), float(pPort->maxShipSize/5.0) );
			sd.maxOffset *= sd.maxOffset;
			outMsg = stringf(Lang::CLEARANCE_GRANTED_BAY_N, formatarg("bay", i+1));
			return true;
		}
	}
	outMsg = Lang::CLEARANCE_DENIED_NO_BAYS;
	return false;
}

bool SpaceStation::OnCollision(Object *b, Uint32 flags, double relVel)
{
	if ((flags & 0x10) && (b->IsType(Object::SHIP))) {
		Ship *s = static_cast<Ship*>(b);

		int port = -1;
		for (Uint32 i=0; i<m_shipDocking.size(); i++) {
			if (m_shipDocking[i].ship == s) { port = i; break; }
		}
		if (port == -1) {
			if (IsGroundStation()) {
				return DoShipDamage(s, flags, relVel);					// no permission
			} else return false;
		}
		if (IsPortLocked(port)) {
			return DoShipDamage(s, flags, relVel);
		}
		if (m_shipDocking[port].stage != 1) return DoShipDamage(s, flags, relVel);	// already docking?

		SpaceStationType::positionOrient_t dport;

		if (IsGroundStation()) {
			// must be oriented sensibly and have wheels down
			PiVerify(m_type->GetDockAnimPositionOrient(port, m_type->NumDockingStages(), 1.0f, vector3d(0.0), dport, s));
			vector3d dockingNormal = GetOrient()*dport.yaxis;
			const double dot = s->GetOrient().VectorY().Dot(dockingNormal);
			if ((dot < 0.99) || (s->GetWheelState() < 1.0)) return DoShipDamage(s, flags, relVel);	// <0.99 harsh?
			// check speed
			if (s->GetVelocity().Length() > MAX_LANDING_SPEED) return DoShipDamage(s, flags, relVel);
			// check if you're near your pad
			float dist = (s->GetPosition() - GetPosition() - GetOrient()*dport.pos).LengthSqr();
			// docking allowed only if inside a circle 70% greater than pad itself (*1.7)
			float maxDist = static_cast<float>(m_type->FindPortByBay(port)->maxShipSize/2)*1.7;
			if (dist > (maxDist*maxDist)) return DoShipDamage(s, flags, relVel);
		}

		// why stage 2? Because stage 1 is permission to dock
		// granted, stage 2 is start of docking animation.
		PiVerify(m_type->GetDockAnimPositionOrient(port, 2, 0.0, vector3d(0.0), dport, s));

		// if there is more docking port anim to do, don't set docked yet
		if (m_type->NumDockingStages() >= 2) {
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
		// If this is reached, then you have permission
		// to dock and a collision with docking surface
		return false;
	} else {
		return true;
	}
}

bool SpaceStation::DoShipDamage(Ship* s, Uint32 flags, double relVel) {
	if (s==nullptr) return false;
	s->DynamicBody::OnCollision(this, flags, relVel);
	return true;
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
	for (Uint32 i=0; i<m_shipDocking.size(); i++) {
		shipDocking_t &dt = m_shipDocking[i];
		if (!dt.ship) continue;
		if (dt.stage > m_type->NumDockingStages()) {
			int extraStage = dt.stage - m_type->NumDockingStages();
			SpaceStationType::positionOrient_t dport;
			float dist = 0.0;
			switch ( extraStage ) {
				case 1: // Level ship & Reposition eval
					// PS: This is to avoid to float around if dock
					// at high time steps on an orbital
					if (!IsGroundStation()) {
						dt.fromPos = vector3d(0.0); //No offset
						dt.fromRot = Quaterniond(1.0,0.0,0.0,0.0); //Identity (no rotation)
						dt.stage += 2;
						continue;
					}
					if ( !LevelShip(dt.ship, i, timeStep) ) continue;
					PiVerify(m_type->GetDockAnimPositionOrient(i, m_type->NumDockingStages(), 1.0f, dt.fromPos, dport, dt.ship));
					dist = (dt.ship->GetPosition() - GetPosition() - GetOrient()*dport.pos).LengthSqr();
					if (dist > dt.maxOffset) {
						// Reposition needed
						dt.fromPos = dt.ship->GetPosition();
						matrix3x3d padOrient = matrix3x3d::FromVectors(dport.xaxis, dport.yaxis, dport.zaxis);
						dt.fromRot = Quaterniond::FromMatrix3x3((GetOrient()*padOrient).Transpose() * dt.ship->GetOrient());
						dt.stage++;
						dt.stagePos = 0.0;
					} else {
						// Save ship position
						dt.fromPos = (dt.ship->GetPosition() - GetPosition() - GetOrient()*dport.pos)*GetOrient();
						matrix3x3d padOrient = matrix3x3d::FromVectors(dport.xaxis, dport.yaxis, dport.zaxis);
						dt.fromRot = Quaterniond::FromMatrix3x3((GetOrient()*padOrient).Transpose() * dt.ship->GetOrient());
						dt.stage += 2;
					}
					continue;
				case 2: // Reposition stage
					dt.stagePos += timeStep / 2.0;
					if ( dt.stagePos >= 1.0 ) {
							dt.stage++;
							dt.fromPos = vector3d(0.0); //No offset
							dt.fromRot = Quaterniond(1.0,0.0,0.0,0.0); //Identity (no rotation)
					}
					continue;
				case 3: // Just docked
					dt.ship->SetDockedWith(this, i);
					LuaEvent::Queue("onShipDocked", dt.ship, this);
					if (dt.fromPos.LengthSqr()>0.5) LuaEvent::Queue("onShipBadDocked", dt.ship, this);
					LockPort(i, false);
					m_doorAnimationStep = -0.3; // close door
					dt.stage++;
					continue;
				case 4: // Docked
				default: continue;
			}
		}

		double stageDuration = (dt.stage > 0 ?
				m_type->GetDockAnimStageDuration(dt.stage-1) :
				m_type->GetUndockAnimStageDuration(abs(dt.stage)-1));
		dt.stagePos += timeStep / stageDuration;

		if (dt.stage == 1) {
			// SPECIAL stage! Docking granted but waiting for ship to dock

			m_doorAnimationStep = 0.3; // open door

			if (dt.stagePos >= 1.0) {
				if (dt.ship == Pi::player)
					Pi::game->log->Add(GetLabel(), Lang::DOCKING_CLEARANCE_EXPIRED, GameLog::PRIORITY_IMPORTANT);
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

		if (dt.stage < -m_type->ShipLaunchStage() && dt.ship->GetFlightState() != Ship::FLYING) {
			// launch ship
			dt.ship->SetFlightState(Ship::FLYING);
			dt.ship->SetAngVelocity(GetAngVelocity());
			if (m_type->IsSurfaceStation()) {
				dt.ship->SetThrusterState(1, 1.0);	// up
			} else {
				dt.ship->SetThrusterState(2, -1.0);	// forward
			}
			LuaEvent::Queue("onShipUndocked", dt.ship, this);
		}
		if (dt.stage < -m_type->NumUndockStages()) {
			// undock animation finished, clear port
			dt.stage = 0;
			dt.ship = nullptr;
			dt.stagePos = 0;
			dt.maxOffset = 0;
			LockPort(i, false);
			m_doorAnimationStep = -0.3; // close door
		}
	}

	m_doorAnimationState = Clamp(m_doorAnimationState + m_doorAnimationStep*timeStep, 0.0, 1.0);
	if (m_doorAnimation)
		m_doorAnimation->SetProgress(m_doorAnimationState);
}

bool SpaceStation::LevelShip(Ship *ship, int port, const float timeStep) const
{
	const shipDocking_t &dt = m_shipDocking[port];
	SpaceStationType::positionOrient_t dport;
	PiVerify(m_type->GetDockAnimPositionOrient(port, dt.stage, dt.stagePos, dt.fromPos, dport, ship));
	matrix3x3d shipOrient = ship->GetOrient();
	vector3d dockingNormal = GetOrient()*dport.yaxis;

	const vector3d &shipPos = ship->GetPosition();

	vector3d dist = (shipPos - GetPosition())*GetOrient() - dport.pos;

	double cosUp = dockingNormal.Dot( shipOrient.VectorY() );
	ship->ClearThrusterState();
	if (cosUp < 0.999999 ) {
		// need level ship
		double angle;
		if ( cosUp < 0.985 ) angle = -0.8*timeStep;
		else angle = -acos( cosUp );
		vector3d rotAxis = dockingNormal.Cross( shipOrient.VectorY() );
		rotAxis = rotAxis.NormalizedSafe();
		shipOrient = matrix3x3d::Rotate( angle, rotAxis) * shipOrient;
		ship->SetOrient( shipOrient );
	}
	if ( fabs(dist.y) > 0.01) {
		vector3d inc(0.0, -dist.y, 0.0);
		inc = GetOrient()*inc;
		ship->SetPosition( shipPos + inc );
	}
	return (cosUp>=0.999999) && (fabs(dist.y)<0.01);
}

void SpaceStation::PositionDockingShip(Ship *ship, int port) const
{
	const shipDocking_t &dt = m_shipDocking[port];
	SpaceStationType::positionOrient_t dport;
	PiVerify(m_type->GetDockAnimPositionOrient(port, dt.stage, dt.stagePos, dt.fromPos, dport, ship));
	if ( dt.stage > m_type->NumDockingStages() ) {
		if (dt.stage == m_type->NumDockingStages()+1) {
			// Leveling
			return;
		} else if (dt.stage == m_type->NumDockingStages()+2) {
			// Repositioning
			vector3d wantPos = GetPosition() + GetOrient()*dport.pos;
			ship->SetPosition(dt.fromPos - (dt.fromPos - wantPos)*dt.stagePos);
		}
	} else {
		ship->SetPosition(GetPosition() + GetOrient()*dport.pos);
	}
	matrix3x3d wantRot = matrix3x3d::FromVectors(dport.xaxis, dport.yaxis, dport.zaxis);
	// use quaternion spherical linear interpolation to do
	// rotation smoothly
	Quaterniond wantQuat = Quaterniond::FromMatrix3x3(wantRot);
	Quaterniond q = Quaterniond::Nlerp(dt.fromRot, wantQuat, dt.stagePos);
	wantRot = q.ToMatrix3x3<double>();
	ship->SetOrient(GetOrient() * wantRot);
}

void SpaceStation::PositionDockedShip(Ship *ship, int port) const
{
	const shipDocking_t &dt = m_shipDocking[port];
	SpaceStationType::positionOrient_t dport;
	PiVerify(m_type->GetDockAnimPositionOrient(port, dt.stage, dt.stagePos, dt.fromPos, dport, ship));
	assert(dt.ship == ship);

	ship->SetPosition(GetPosition() + GetOrient()*(dport.pos + dt.fromPos));
	// Note: ship bounding box is used to generate dport.pos
	Quaterniond dportQ = Quaterniond::FromMatrix3x3(matrix3x3d::FromVectors(dport.xaxis, dport.yaxis, dport.zaxis));
	dportQ = dportQ*dt.fromRot;
	matrix3x3d shipRot = dportQ.ToMatrix3x3<double>();
	ship->SetOrient(GetOrient()*shipRot);
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
		matrix3x3d r = matrix3x3d::RotateY(-len);		// RotateY is backwards
		SetOrient(r * GetOrient());
	}
	m_oldAngDisplacement = len;

	// reposition the ships that are docked or docking here
	for (unsigned int i=0; i<m_type->NumDockingPorts(); i++) {
		const shipDocking_t &dt = m_shipDocking[i];
		if (!dt.ship) { //free
			m_navLights->SetColor(i+1, NavLights::NAVLIGHT_GREEN);
			continue;
		}
		if (dt.stage == 1) {//reserved
			m_navLights->SetColor(i+1, NavLights::NAVLIGHT_YELLOW);
			continue;
		}
		if (dt.ship->GetFlightState() == Ship::DOCKED) {
			PositionDockedShip(dt.ship, i);
			continue;
		}
		if (dt.ship->GetFlightState() != Ship::DOCKING
			 && dt.ship->GetFlightState() != Ship::UNDOCKING)
			continue;
		PositionDockingShip(dt.ship, i);
		m_navLights->SetColor(i+1, NavLights::NAVLIGHT_RED); //docked
	}

	ModelBody::TimeStepUpdate(timeStep);
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
	return m_type->IsSurfaceStation();
}

// Renders space station and adjacent city if applicable
// For orbital starports: renders as normal
// For surface starports:
//	Lighting: Calculates available light for model and splits light between directly and ambiently lit
//            Lighting is done by manipulating global lights or setting uniforms in atmospheric models shader
static const double SQRMAXCITYDIST = 1e5 * 1e5;

void SpaceStation::Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	Body *b = GetFrame()->GetBody();
	assert(b);

	if (!b->IsType(Object::PLANET)) {
		// orbital spaceport -- don't make city turds or change lighting based on atmosphere
		RenderModel(r, camera, viewCoords, viewTransform);
		m_navLights->Render(r);
		r->GetStats().AddToStatCount(Graphics::Stats::STAT_SPACESTATIONS, 1);
	} else {
		// don't render city if too far away
		if (viewCoords.LengthSqr() >= SQRMAXCITYDIST) {
			return;
		}
		std::vector<Graphics::Light> oldLights;
		Color oldAmbient;
		SetLighting(r, camera, oldLights, oldAmbient);

		if (!m_adjacentCity) {
			m_adjacentCity = new CityOnPlanet(static_cast<Planet*>(b), this, m_sbody->GetSeed());
		}
		m_adjacentCity->Render(r, camera->GetContext()->GetFrustum(), this, viewCoords, viewTransform);

		RenderModel(r, camera, viewCoords, viewTransform, false);
		m_navLights->Render(r);

		ResetLighting(r, oldLights, oldAmbient);

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

vector3d SpaceStation::GetTargetIndicatorPosition(const Frame *relTo) const
{
	// return the next waypoint if permission has been granted for player,
	// and the docking point's position once the docking anim starts
	for (Uint32 i=0; i<m_shipDocking.size(); i++) {
		if (i >= m_type->NumDockingPorts()) break;
		if ((m_shipDocking[i].ship == Pi::player) && (m_shipDocking[i].stage > 0) && (m_shipDocking[i].stage != m_type->NumDockingStages() + 1)) { // last part is "not currently docked"

			SpaceStationType::positionOrient_t dport;
			if (!m_type->GetShipApproachWaypoints(i, m_shipDocking[i].stage+1, dport))
				PiVerify(m_type->GetDockAnimPositionOrient(i, m_type->NumDockingStages(),
				1.0f, vector3d(0.0), dport, m_shipDocking[i].ship));

			vector3d v = GetInterpPositionRelTo(relTo);
			return v + GetInterpOrientRelTo(relTo) * dport.pos;
		}
	}
	return GetInterpPositionRelTo(relTo);
}

bool SpaceStation::IsPortLocked(const int bay) const
{
	for (auto &bayIter : m_ports ) {
		for ( auto &idIter : bayIter.bayIDs ) {
			if (idIter.first==bay) {
				return bayIter.inUse;
			}
		}
	}
	// is it safer to return that the is loacked?
	return true;
}

void SpaceStation::LockPort(const int bay, const bool lockIt)
{
	for (auto &bayIter : m_ports ) {
		for ( auto &idIter : bayIter.bayIDs ) {
			if (idIter.first==bay) {
				bayIter.inUse = lockIt;
				return;
			}
		}
	}
}
