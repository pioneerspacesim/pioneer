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

struct SpaceStationType {
	const char *sbreModelName;
	enum { ORBITAL, SURFACE } dockMethod;
};

struct SpaceStationType stationTypes[SpaceStation::TYPE_MAX] = {
	{ "nice_spacestation", SpaceStationType::ORBITAL },
	{ "basic_groundstation", SpaceStationType::SURFACE },
};

void SpaceStation::Save()
{
	using namespace Serializer::Write;
	ModelBody::Save();
	MarketAgent::Save();
	wr_int((int)m_type);
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
		wr_vector3d(m_shipDocking[i].from);

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
	m_type = (TYPE)rd_int();
	m_numPorts = 0;
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
		m_shipDocking[i].from = rd_vector3d();

		m_openAnimState[i] = rd_float();
		m_dockAnimState[i] = rd_float();
	}
	m_lastUpdatedShipyard = rd_double();
	m_sbody = Serializer::LookupSystemBody(rd_int());
	Init();
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

static LmrObjParams params = {
	{ 0.5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f },

	{	// pColor[3]
	{ { 1.0f, 0.0f, 1.0f, 1.0f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 },
	{ { 0.8f, 0.6f, 0.5f, 1.0f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 },
	{ { 0.5f, 0.5f, 0.5f, 1.0f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 } },

	// pText[3][256]	
	{ "Hello old bean", "DIET STEAKETTE" },
};

SpaceStation::SpaceStation(const SBody *sbody): ModelBody()
{
	if (sbody->type == SBody::TYPE_STARPORT_ORBITAL) {
		m_type = JJHOOP;
	} else {
		m_type = GROUND_FLAVOURED;
	}
	m_sbody = sbody;
	m_numPorts = 0;
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
		m_openAnimState[i] = 0;
		m_dockAnimState[i] = 0;
	}
	SetMoney(1000000000);
	Init();
}

/*
 * So, positionOrient_t things for docking animations are stored in the model
 * as two triangles with geomflag 0x8000+i (0x8010+i for stage2)
 * tri(position, zerovec, zerovec)
 * tri(zerovec, xaxis, yaxis)
 * Since positionOrient triangles in subobjects will be scaled and translated,
 * the xaxis and yaxis normals must be un-translated (xaxis minus translated
 * zerovec), and re-normalized.
 */
static void SetPositionOrientFromTris(const vector3d v[6], SpaceStation::positionOrient_t &outOrient)
{
	outOrient.pos = v[0];
	outOrient.xaxis = (v[4] - v[1]).Normalized();
	outOrient.normal = (v[5] - v[1]).Normalized();
}

void SpaceStation::Init()
{
	m_adjacentCity = 0;
	SetModel(stationTypes[m_type].sbreModelName, true);
	LmrCollMesh *mesh = GetLmrCollMesh();
	int i=0;
	for (i=0; i<MAX_DOCKING_PORTS; i++) {
		port[i].exists = false;
		port_s2[i].exists = false;
	}
	vector3d v[6];
	for (i=0; i<MAX_DOCKING_PORTS; i++) {
		if (mesh->GetTrisWithGeomflag(0x8000+i, 2, v) != 2) break;
		// 0x8000+i tri gives position, xaxis, yaxis of docking port surface
		port[i].exists = true;
		SetPositionOrientFromTris(v, port[i]);
		/*printf("%p port %d, %f,%f,%f, %f,%f,%f\n", this, i,
				port[i].xaxis.x,
				port[i].xaxis.y,
				port[i].xaxis.z,
				port[i].normal.x,
				port[i].normal.y,
				port[i].normal.z);*/
		if (mesh->GetTrisWithGeomflag(0x8010+i, 2, v) != 2) continue;
		port_s2[i].exists = true;
		SetPositionOrientFromTris(v, port_s2[i]);
		assert(mesh->GetTrisWithGeomflag(0x8020+i, 2, v)==2);
		port_s3[i].exists = true;
		SetPositionOrientFromTris(v, port_s3[i]);
	}
	m_numPorts = i;

	assert(m_numPorts > 0);
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

static void rotateBy(Ship *s, vector3d axis, const float timeStep)
{
	double ang = timeStep;
	if (ang == 0) return;
	vector3d rotAxis = axis.Normalized();
	matrix4x4d rotMatrix = matrix4x4d::RotateMatrix(ang, rotAxis.x, rotAxis.y, rotAxis.z);
	
	matrix4x4d orient;
	s->GetRotMatrix(orient);
	orient = orient * rotMatrix;
	s->SetRotMatrix(orient);
}

static void rotateTo(Ship *s, matrix4x4d &rot, const float timeStep)
{
	matrix4x4d cur;
	s->GetRotMatrix(cur);
	vector3d cx(cur[0], cur[4], cur[8]);
	vector3d rx(rot[0], rot[4], rot[8]);
	vector3d cz(cur[2], cur[6], cur[10]);
	vector3d rz(rot[2], rot[6], rot[10]);
	double dx = vector3d::Dot(cx,rx);
	double dz = vector3d::Dot(cz,rz);
	if (dx < 0.999) {
		vector3d xrot = vector3d::Cross(rx, cx);
		rotateBy(s, xrot, timeStep);
	} else if (dz < 0.999) {
		vector3d zrot = vector3d::Cross(rz, cz);
		rotateBy(s, zrot, timeStep);
	} else {
		s->SetRotMatrix(rot);
	}
}

void SpaceStation::DoDockingAnimation(const float timeStep)
{
	matrix4x4d rot, wantRot;
	vector3d p1, p2, zaxis;
	for (int i=0; i<MAX_DOCKING_PORTS; i++) {
		shipDocking_t &dt = m_shipDocking[i];
		if (!dt.ship) {
			m_openAnimState[i] -= 0.3*timeStep;
			m_dockAnimState[i] -= 0.3*timeStep;
			continue;
		}
		GetRotMatrix(rot);

		switch (dt.stage) {
		/* Launching stages */
		case -1:
			// open inner doors
			dt.stagePos += timeStep*0.3;
			m_openAnimState[i] -= 0.3*timeStep;
			m_dockAnimState[i] += 0.3*timeStep;
			if (dt.stagePos >= 1.0) {
				dt.stagePos = 0;
				dt.stage--;
			}
			break;
		case -2:
			// move into inner region
			dt.stagePos += timeStep*0.3;
			p1 = GetPosition() + rot*port_s3[i].pos;
			p2 = GetPosition() + rot*port_s2[i].pos;
			dt.ship->SetPosition(p1 + (p2-p1)*(double)dt.stagePos);
			///
			zaxis = vector3d::Cross(port_s2[i].xaxis, port_s2[i].normal);
			wantRot = matrix4x4d::MakeRotMatrix(
					port_s2[i].xaxis, port_s2[i].normal, zaxis) * rot;
			rotateTo(dt.ship, wantRot, timeStep);
			if (dt.stagePos >= 1.0) {
				dt.stagePos = 0;
				dt.stage--;
			}
			break;
		case -3:
			// close inner doors and pretend an elevator is
			// moving...
			dt.stagePos += timeStep*0.2;
			m_openAnimState[i] -= 0.3*timeStep;
			m_dockAnimState[i] -= 0.3*timeStep;
			if (dt.stagePos >= 1.0) {
				dt.stagePos = 0;
				dt.stage--;
			}
			break;
		case -4:
			// open inner doors
			dt.stagePos += timeStep*0.3;
			m_openAnimState[i] -= 0.3*timeStep;
			m_dockAnimState[i] += 0.3*timeStep;
			dt.ship->SetPosition(GetPosition() + rot*port_s2[i].pos);
			///
			zaxis = vector3d::Cross(port[i].xaxis, port[i].normal);
			wantRot = matrix4x4d::MakeRotMatrix(
					port[i].xaxis, port[i].normal, zaxis) * rot;
			rotateTo(dt.ship, wantRot, timeStep);
			if (dt.stagePos >= 1.0) {
				dt.stagePos = 0;
				dt.stage--;
			}
			break;
		case -5:
			// move ship to outer section
			p1 = GetPosition() + rot*port_s2[i].pos;
			p2 = GetPosition() + rot*port[i].pos;
			dt.ship->SetPosition(p1 + (p2-p1)*(double)dt.stagePos);
			dt.stagePos += 0.2*timeStep;
			if (dt.stagePos >= 1.0) {
				dt.stagePos = 0;
				dt.stage--;
			}
			break;
		case -6:
			// close inner door
			m_openAnimState[i] -= 0.3*timeStep;
			m_dockAnimState[i] -= 0.3*timeStep;
			dt.stagePos += 0.3*timeStep;
			if (dt.stagePos >= 1.0) {
				dt.stagePos = 0;
				dt.stage--;
			}
			break;
		case -7:
			// open outer door
			m_openAnimState[i] += 0.3*timeStep;
			m_dockAnimState[i] -= 0.3*timeStep;
			dt.stagePos += 0.3*timeStep;
			if (dt.stagePos >= 1.0) {
				dt.ship->Enable();
				dt.ship->SetFlightState(Ship::FLYING);
				dt.ship->SetVelocity(GetFrame()->GetStasisVelocityAtPosition(dt.ship->GetPosition()));
				dt.ship->SetAngVelocity(GetFrame()->GetAngVelocity());
				dt.ship->SetForce(vector3d(0,0,0));
				dt.ship->SetTorque(vector3d(0,0,0));
				dt.ship->SetThrusterState(ShipType::THRUSTER_REAR, 1.0);
				dt.stagePos = 0;
				dt.stage--;
			}
		case -8:
			// give the fucker some time to leave before closing doors
			m_openAnimState[i] += 0.3*timeStep;
			dt.stagePos += timeStep/30.0;
			if (dt.stagePos >= 1.0) {
				dt.ship = 0;
			}
			break;

		/* Docking stages */
		case 0:
			// docking has been granted but ship hasn't docked yet
			dt.stagePos += timeStep/DOCKING_TIMEOUT_SECONDS;
			m_openAnimState[i] += 0.3*timeStep;
			m_dockAnimState[i] -= 0.3*timeStep;

			if (dt.stagePos >= 1.0) {
				if (dt.ship == (Ship*)Pi::player) Pi::onDockingClearanceExpired.emit(this);
				dt.ship = 0;
			}
			break;
		case 1:
			// close outer door, center ship
			dt.stagePos += 0.3*timeStep;
			p2 = GetPosition() + rot*port[i].pos;
			dt.ship->SetPosition(dt.from + (p2-dt.from)*(double)dt.stagePos);
			m_openAnimState[i] -= 0.3*timeStep;
			m_dockAnimState[i] -= 0.3*timeStep;
			///
			zaxis = vector3d::Cross(port[i].xaxis, port[i].normal);
			wantRot = matrix4x4d::MakeRotMatrix(
					port[i].xaxis, port[i].normal, zaxis) * matrix4x4d::RotateYMatrix(M_PI) * rot;
			rotateTo(dt.ship, wantRot, timeStep);
			///
			if (dt.stagePos >= 1.0) {
				dt.stagePos = 0;
				dt.stage++;
			}
			break;
		case 2:
			// open inner door
			dt.stagePos += 0.3*timeStep;
			m_openAnimState[i] -= 0.3*timeStep;
			m_dockAnimState[i] += 0.3*timeStep;
			/// XXX identical to code in step 1... XXX
			zaxis = vector3d::Cross(port[i].xaxis, port[i].normal);
			wantRot = matrix4x4d::MakeRotMatrix(
					port[i].xaxis, port[i].normal, zaxis) * matrix4x4d::RotateYMatrix(M_PI) * rot;
			rotateTo(dt.ship, wantRot, timeStep);
			///
			if (dt.stagePos >= 1.0) {
				dt.stagePos = 0;
				dt.stage++;
			}
			break;
		case 3:
			// move into inner region
			p1 = GetPosition() + rot*port[i].pos;
			p2 = GetPosition() + rot*port_s2[i].pos;
			dt.ship->SetPosition(p1 + (p2-p1)*(double)dt.stagePos);
			dt.stagePos += 0.2*timeStep;

			///
			zaxis = vector3d::Cross(port_s2[i].xaxis, port_s2[i].normal);
			wantRot = matrix4x4d::MakeRotMatrix(
					port_s2[i].xaxis, port_s2[i].normal, zaxis) * matrix4x4d::RotateYMatrix(M_PI) * rot;
			rotateTo(dt.ship, wantRot, timeStep);
			///

			if (dt.stagePos >= 1.0) {
				dt.stagePos = 0;
				dt.stage++;
			}
			break;
		case 4:
			// close inner door, rotate 180
			dt.stagePos += 0.2*timeStep;
			m_dockAnimState[i] -= 0.3*timeStep;
			/// XXX same as codein stage3 XXX
			zaxis = vector3d::Cross(port_s2[i].xaxis, port_s2[i].normal);
			wantRot = matrix4x4d::MakeRotMatrix(
					port_s2[i].xaxis, port_s2[i].normal, zaxis) * rot;
			rotateTo(dt.ship, wantRot, timeStep);
			///
			if (dt.stagePos >= 1.0) {
				dt.stagePos = 0;
				dt.stage++;
			}
			break;
		case 5:
			// open inner doors, showing final resting place
			dt.stagePos += 0.3*timeStep;
			m_dockAnimState[i] += 0.3*timeStep;
			if (dt.stagePos >= 1.0) {
				dt.stagePos = 0;
				dt.stage++;
			}
			break;
		case 6:
			// enter 
			dt.stagePos += 0.3*timeStep;
			// move into inner region
			p1 = GetPosition() + rot*port_s2[i].pos;
			p2 = GetPosition() + rot*port_s3[i].pos;
			dt.ship->SetPosition(p1 + (p2-p1)*(double)dt.stagePos);
			///
			zaxis = vector3d::Cross(port_s3[i].xaxis, port_s3[i].normal);
			wantRot = matrix4x4d::MakeRotMatrix(
					port_s3[i].xaxis, port_s3[i].normal, zaxis) * rot;
			rotateTo(dt.ship, wantRot, timeStep);
			///
			if (dt.stagePos >= 1.0) {
				dt.stagePos = 0;
				dt.stage++;
			}
			break;
		case 7:
			// close door and dock
			dt.stagePos += 0.3*timeStep;
			m_dockAnimState[i] -= 0.3*timeStep;
			if (dt.stagePos >= 1.0) {
				dt.ship->SetDockedWith(this, i);
				dt.ship = 0;
			}
			break;
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
	return (stationTypes[m_type].dockMethod ==
	        SpaceStationType::SURFACE);
}

void SpaceStation::OrientDockedShip(Ship *ship, int port) const
{
	const positionOrient_t *dport = &this->port[port];
	const int dockMethod = stationTypes[m_type].dockMethod;
	if (dockMethod == SpaceStationType::SURFACE) {
		matrix4x4d stationRot;
		GetRotMatrix(stationRot);
		vector3d port_z = vector3d::Cross(dport->xaxis, dport->normal);
		matrix4x4d rot = stationRot * matrix4x4d::MakeRotMatrix(dport->xaxis, dport->normal, port_z);
		vector3d pos = GetPosition() + stationRot*dport->pos;

		// position with wheels perfectly on ground :D
		Aabb aabb;
		ship->GetAabb(aabb);
		pos += stationRot*vector3d(0,-aabb.min.y,0);

		ship->SetPosition(pos);
		ship->SetRotMatrix(rot);
	}
}

void SpaceStation::PositionDockedShip(Ship *ship, int port)
{
	const positionOrient_t *dport = &this->port[port];
	const int dockMethod = stationTypes[m_type].dockMethod;
	if (dockMethod == SpaceStationType::ORBITAL) {
		matrix4x4d rot;
		GetRotMatrix(rot);
		vector3d p = GetPosition() + rot*port_s3[port].pos;
		
		ship->SetFrame(GetFrame());
		ship->SetPosition(p);
		// duplicated from DoDockingAnimation()
		vector3d zaxis = vector3d::Cross(port_s3[port].xaxis, port_s3[port].normal);
		ship->SetRotMatrix(matrix4x4d::MakeRotMatrix(port_s3[port].xaxis,
					port_s3[port].normal, zaxis) * rot);
	} else {
		Aabb aabb;
		ship->GetAabb(aabb);

	 	matrix4x4d stationRot;
		GetRotMatrix(stationRot);
		vector3d port_z = vector3d::Cross(dport->xaxis, dport->normal);
		matrix4x4d rot = stationRot * matrix4x4d::MakeRotMatrix(dport->xaxis, dport->normal, port_z);
		// position slightly (1m) off landing surface
		vector3d pos = GetPosition() + stationRot*(dport->pos +
				dport->normal - 
				dport->normal*aabb.min.y);
		ship->SetPosition(pos);
		ship->SetRotMatrix(rot);
	}
}

void SpaceStation::LaunchShip(Ship *ship, int port)
{
	const int dockMethod = stationTypes[m_type].dockMethod;
	
	if (dockMethod == SpaceStationType::ORBITAL) {
		shipDocking_t &sd = m_shipDocking[port];
		sd.ship = ship;
		sd.stage = -1;
		sd.stagePos = 0;
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
		if (port[i].exists == false) continue;
		if (m_shipDocking[i].ship == s) {
			outMsg = stringf(256, "Clearance already granted. Proceed to docking bay %d.", i+1);
			return true;
		}
	}
	for (int i=0; i<MAX_DOCKING_PORTS; i++) {
		if (port[i].exists == false) continue;
		if (m_shipDocking[i].ship != 0) continue;
		shipDocking_t &sd = m_shipDocking[i];
		sd.ship = s;
		sd.stage = 0;
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
		positionOrient_t *dport = &port[flags & 0xf];
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
				vector3d dockingNormal = stationRot*dport->normal;

				// check player is sortof sensibly oriented for landing
				const double dot = vector3d::Dot(vector3d(invRot[1], invRot[5], invRot[9]), dockingNormal);
				if ((dot < 0.99) || (s->GetWheelState() != 1.0)) return true;
			}
			
			if ((speed < MAX_LANDING_SPEED) &&
			    (!s->GetDockedWith()) &&
			    (m_shipDocking[flags&0xf].ship == s) &&
			    (m_shipDocking[flags&0xf].stage == 0)) {
				// if there is more docking port anim to do,
				// don't set docked yet
				if (port_s2[flags & 0xf].exists) {
					shipDocking_t &sd = m_shipDocking[flags&0xf];
					sd.ship = s;
					sd.stage = 1;
					sd.stagePos = 0;
					sd.from = s->GetPosition();
					s->Disable();
					s->SetFlightState(Ship::DOCKING);
				} else {
					m_shipDocking[flags&0xf].ship = 0;
					s->SetDockedWith(this, flags & 0xf);
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

static std::vector<int> s_advertModels;

#define ARG_STATION_BAY1_DOOR1 6
#define ARG_STATION_BAY1_DOOR2 10
#define ARG_STATION_BAY1_STAGE1 14
#define ARG_STATION_BAY1_STAGE2 18

void SpaceStation::Render(const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	/* Well this is nice... */
	static int poo=0;
	if (!poo) {
		poo = 1;
//		sbreGetModelsWithTag("advert", s_advertModels);
	}
	// it is silly to do this every render call
	//
	// random advert models in pFlag[16 .. 19]
	// station name in pText[0]
	// docking port in pText[1]
	MTRand rand;
	rand.seed(m_sbody->seed);
	/* XXX make adverts work again */
	/*
	params.pFlag[16] = s_advertModels[rand.Int32(s_advertModels.size())];
	params.pFlag[17] = s_advertModels[rand.Int32(s_advertModels.size())];
	params.pFlag[18] = s_advertModels[rand.Int32(s_advertModels.size())];
	params.pFlag[19] = s_advertModels[rand.Int32(s_advertModels.size())];*/
	strncpy(params.argStrings[4], "diet_steakette", 256);
	strncpy(params.argStrings[5], "diet_steakette", 256);
	strncpy(params.argStrings[6], "diet_steakette", 256);
	strncpy(params.argStrings[7], "diet_steakette", 256);
	strncpy(params.argStrings[0], GetLabel().c_str(), 256);
	snprintf(params.argStrings[1], 256, "DOCKING BAY %d", 1+Pi::player->GetDockingPort());

	for (int i=0; i<MAX_DOCKING_PORTS; i++) {
		params.argFloats[ARG_STATION_BAY1_DOOR1 + i] = m_openAnimState[i];
		params.argFloats[ARG_STATION_BAY1_STAGE1 + i] = 1.0;
		params.argFloats[ARG_STATION_BAY1_DOOR2 + i] = m_dockAnimState[i];
		params.argFloats[ARG_STATION_BAY1_STAGE2 + i] = 1.0;
		const int stage = m_shipDocking[i].stage;

		/* nice */
		if (
			// if the player is in this docking bay draw its inner
			// bits
				((Pi::player->GetDockedWith() == this) &&
				(Pi::player->GetDockingPort() == i))
			||
			// or if the player is at the bits of the docking
			// sequence when they should be visible
				((m_shipDocking[i].ship == (Ship*)Pi::player) && 
				// during docking
				( (stage >= 5) ||
				// during launch
				  ((stage >= -3) && (stage <= -1)) ))
		) {
			params.argFloats[ARG_STATION_BAY1_STAGE1 + i] = 0;
			params.argFloats[ARG_STATION_BAY1_STAGE2 + i] = 1;
		} else {
			params.argFloats[ARG_STATION_BAY1_STAGE1 + i] = 1;
			params.argFloats[ARG_STATION_BAY1_STAGE2 + i] = 0;
		}
	}
	/*
	for (int i=0; i<MAX_DOCKING_PORTS; i++) {
		if (m_shipDocking[i].ship && (m_shipDocking[i].stage == 1)) {
			params.pAnim[ASRC_STATION_DOCK_PORT1] = m_shipDocking[i].stagePos;
			break;
		}
		if (m_shipDocking[i].ship && (m_shipDocking[i].stage == 2)) {
			params.pAnim[ASRC_STATION_DOCK_PORT1] = 1.0;
			break;
		}
		if (m_shipDocking[i].ship && (m_shipDocking[i].stage == 3)) {
			params.pAnim[ASRC_STATION_DOCK_PORT1] = 1.0 - m_shipDocking[i].stagePos;
			break;
		}
	}*/
	RenderLmrModel(viewCoords, viewTransform, &params);
	
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
