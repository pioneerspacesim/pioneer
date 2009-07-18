#include "SpaceStation.h"
#include "Ship.h"
#include "Planet.h"
#include "ModelCollMeshData.h"
#include "gameconsts.h"
#include "StarSystem.h"
#include "Serializer.h"
#include "Frame.h"
#include "Pi.h"
#include "Mission.h"

struct SpaceStationType {
	const char *sbreModelName;
	enum { ORBITAL, SURFACE } dockMethod;
};

struct SpaceStationType stationTypes[SpaceStation::TYPE_MAX] = {
	{ "65", SpaceStationType::ORBITAL },
	{ "90", SpaceStationType::SURFACE },
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
	m_lastUpdatedShipyard = rd_double();
	m_sbody = Serializer::LookupSystemBody(rd_int());
	Init();
}

static ObjParams params = {
	{ 0.5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f },

	{	// pColor[3]
	{ { 1.0f, 0.0f, 1.0f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 },
	{ { 0.8f, 0.6f, 0.5f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 },
	{ { 0.5f, 0.5f, 0.5f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 } },

	// pText[3][256]	
	{ "Hello old bean", "DIET STEAKETTE" },
};

void SpaceStation::GetDockingSurface(const CollMeshSet *mset, int midx)
{
	meshinfo_t * const minfo = &mset->meshInfo[midx];
	dockingport_t * const dport = &port[minfo->flags & 0xf];
	m_numPorts++;
	
	assert(m_numPorts <= MAX_DOCKING_PORTS);
	assert((minfo->flags & 0xf) < MAX_DOCKING_PORTS);
	assert(minfo->numTris);
	
	dport->center = vector3d(0.0);
	const int t = minfo->triStart;
	float *const vts = mset->sbreCollMesh->pVertex;
	for (int pos=0; pos<minfo->numTris; pos++) {
		vector3d v1(vts + 3*mset->triIndices[t+pos].v1);
		vector3d v2(vts + 3*mset->triIndices[t+pos].v2);
		vector3d v3(vts + 3*mset->triIndices[t+pos].v3);
		// use first tri to get docking port normal (which points out of the
		// docking port)
		if (pos == 0) {
			dport->normal = vector3d::Cross(v2-v1,v2-v3).Normalized();
			dport->horiz = (v1-v2).Normalized();
		}
		dport->center += v1+v2+v3;
	}
	dport->center *= 1.0/(3.0*minfo->numTris);
/*	printf("Docking port center %f,%f,%f, normal %f,%f,%f, horiz %f,%f,%f\n",
		dport->center.x,
		dport->center.y,
		dport->center.z,
		dport->normal.x,
		dport->normal.y,
		dport->normal.z,
		dport->horiz.x,
		dport->horiz.y,
		dport->horiz.z); */
}

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
		m_equipmentStock[i] = Pi::rng.Int32(0,100);
	}
	SetMoney(1000000000);
	Init();
}

void SpaceStation::Init()
{
	SetModel(stationTypes[m_type].sbreModelName);
	const CollMeshSet *mset = GetModelCollMeshSet(GetSbreModel());
	for (int i=0; i<mset->numMeshParts; i++) {
		if (mset->meshInfo[i].flags & 0x10) GetDockingSurface(mset, i);
	}
}

SpaceStation::~SpaceStation()
{
	for (std::vector<Mission*>::iterator i = m_bbmissions.begin();
			i != m_bbmissions.end(); ++i) {
		delete *i;
	}
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

void SpaceStation::TimeStepUpdate(const float timeStep)
{
	if (Pi::GetGameTime() > m_lastUpdatedShipyard) {
		UpdateBB();
		UpdateShipyard();
		// update again in an hour or two
		m_lastUpdatedShipyard = Pi::GetGameTime() + 3600.0 + 3600.0*Pi::rng.Double();
	}
}

bool SpaceStation::IsGroundStation() const
{
	return (stationTypes[m_type].dockMethod ==
	        SpaceStationType::SURFACE);
}

void SpaceStation::OrientDockedShip(Ship *ship, int port) const
{
	const dockingport_t *dport = &this->port[port];
	const int dockMethod = stationTypes[m_type].dockMethod;
	if (dockMethod == SpaceStationType::SURFACE) {
		matrix4x4d stationRot;
		GetRotMatrix(stationRot);
		vector3d port_y = vector3d::Cross(-dport->horiz, dport->normal);
		matrix4x4d rot = stationRot * matrix4x4d::MakeRotMatrix(-dport->horiz, -dport->normal, -port_y);
		vector3d pos = GetPosition() + stationRot*dport->center;

		// position with wheels perfectly on ground :D
		Aabb aabb;
		ship->GetAabb(aabb);
		pos += stationRot*vector3d(0,-aabb.min.y,0);

		ship->SetPosition(pos);
		ship->SetRotMatrix(rot);
	}
}

void SpaceStation::OrientLaunchingShip(Ship *ship, int port) const
{
	const dockingport_t *dport = &this->port[port];
	const int dockMethod = stationTypes[m_type].dockMethod;
	if (dockMethod == SpaceStationType::ORBITAL) {
		// position ship in middle of docking bay, pointing out of it
		// XXX need to do forced thrusting thingy...
		// XXX ang vel not zeroed for some reason...
		matrix4x4d stationRot;
		GetRotMatrix(stationRot);
		vector3d port_y = vector3d::Cross(dport->horiz, dport->normal);

		vector3d port_x = vector3d::Cross(port_y, dport->normal).Normalized();
		port_y = vector3d::Cross(dport->normal, port_x).Normalized();

		matrix4x4d rot = stationRot * matrix4x4d::MakeRotMatrix(port_x, port_y, dport->normal);
		vector3d pos = GetPosition() + stationRot*dport->center;
		ship->SetPosition(pos);
		ship->SetRotMatrix(rot);
		ship->SetVelocity(GetFrame()->GetStasisVelocityAtPosition(pos));
		ship->SetAngVelocity(GetFrame()->GetAngVelocity());
		ship->SetForce(vector3d(0,0,0));
		ship->SetTorque(vector3d(0,0,0));
	}
	else if (dockMethod == SpaceStationType::SURFACE) {
		ship->Blastoff();

	/*	not necessary, since for the time being 'SURFACE' starports are on planets
	 *	so the positioning Blastoff does is fine
	 *
	 *	matrix4x4d stationRot;
		GetRotMatrix(stationRot);
		vector3d port_y = vector3d::Cross(-dport->horiz, dport->normal);
		matrix4x4d rot = stationRot * matrix4x4d::MakeRotMatrix(-dport->horiz, -dport->normal, -port_y);
		vector3d pos = GetPosition() + stationRot*dport->center;
		ship->SetPosition(pos - stationRot*(10*dport->normal));
		ship->SetRotMatrix(rot);
		ship->SetVelocity(vector3d(0,0,0));
		ship->SetAngVelocity(vector3d(0,0,0)); */
	} else {
		assert(0);
	}
}

bool SpaceStation::GetDockingClearance(Ship *s)
{
	s->SetDockingTimer(60*10);
	return true;
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
int SpaceStation::GetPrice(Equip::Type t) const {
	int mul = 100;
	SBody *sbody = GetFrame()->GetSBodyFor();
	if (sbody) {
		mul += sbody->tradeLevel[t];
	}
	return (mul * EquipType::types[t].basePrice) / 100;
}

bool SpaceStation::OnCollision(Body *b, Uint32 flags)
{
	if (flags & 0x10) {
		dockingport_t *dport = &port[flags & 0xf];
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
				const double dot = vector3d::Dot(vector3d(-invRot[1], -invRot[5], -invRot[9]), dockingNormal);
				if ((dot < 0.99) || (s->GetWheelState() != 1.0)) return true;
			}
			
			if ((speed < MAX_LANDING_SPEED) &&
			    (!s->GetDockedWith()) &&
			    (s->GetDockingTimer()!=0.0f)) {
				s->SetDockedWith(this, flags & 0xf);
			}
		}
		if (!IsGroundStation()) return false;
		return true;
	} else {
		return true;
	}
}

static Plane planes[6];
ObjParams cityobj_params;

static void drawModel(const matrix4x4d &rot, vector3d pos, int modelNum)
{
	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();

//	glPushAttrib(GL_ALL_ATTRIB_BITS);
	sbreRenderModel(&pos.x, &rot[0], modelNum, &cityobj_params);
//	glPopAttrib();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

#define START_SEG_SIZE 500.0
#define DIVIDE_SEG_SIZE 100.0
#define MAX_CITY_BUILDING 5

const char *city_buildings[MAX_CITY_BUILDING] = {
	"building1",
	"building2",
	"building3",
	"factory1",
	"42", // a house
};

static void putCityBit(const Planet *planet, MTRand &rand, const matrix4x4d &frameTrans, const matrix4x4d &rot, vector3d p1, vector3d p2, vector3d p3, vector3d p4)
{
	double rad = (p1-p2).Length()*0.5;
	int modelNum;
	double modelRad;
       
	for (int tries=10; tries--; ) {
       		modelNum = sbreLookupModelByName(city_buildings[rand.Int32(MAX_CITY_BUILDING)]);
		modelRad = sbreGetModelRadius(modelNum);
		if (modelRad < rad) break;
	}


	if ((rad > DIVIDE_SEG_SIZE) || (rad > modelRad*2.0)) {
		vector3d a = (p1+p2)*0.5;
		vector3d b = (p2+p3)*0.5;
		vector3d c = (p3+p4)*0.5;
		vector3d d = (p4+p1)*0.5;
		vector3d e = (p1+p2+p3+p4)*0.25;
		putCityBit(planet, rand, frameTrans, rot, p1, a, e, d);
		putCityBit(planet, rand, frameTrans, rot, a, p2, b, e);
		putCityBit(planet, rand, frameTrans, rot, e, b, p3, c);
		putCityBit(planet, rand, frameTrans, rot, d, e, c, p4);
	} else {
		vector3d cent = (p1+p2+p3+p4)*0.25;

		
		cent = cent.Normalized();
		cent = cent * planet->GetTerrainHeight(cent);
		cent = frameTrans * cent;

		/* frustum cull */
		for (int i=0; i<6; i++) {
			if (planes[i].DistanceToPoint(cent)+sbreGetModelRadius(modelNum) < 0) {
				return;
			}
		}

		drawModel(rot, cent, modelNum);
	/*	
		p1 = frameTrans*p1;
		p2 = frameTrans*p2;
		p3 = frameTrans*p3;
		p4 = frameTrans*p4;
		
		glDisable(GL_LIGHTING);
		glColor3f(0.5,.5,.5);
		glBegin(GL_LINE_LOOP);
			glVertex3dv(&p1.x);
			glVertex3dv(&p2.x);
			glVertex3dv(&p3.x);
			glVertex3dv(&p4.x);
		glEnd();
		glEnable(GL_LIGHTING);
	*/
	}
}

void SpaceStation::Render(const Frame *camFrame)
{
	RenderSbreModel(camFrame, &params);

	// find planet Body*
	Planet *planet;
	{
		Body *_planet = GetFrame()->m_astroBody;
		if ((!_planet) || !_planet->IsType(Object::PLANET)) {
			// orbital spaceport -- don't make city turds
			return;
		}
		planet = static_cast<Planet*>(_planet);
	}

	Aabb aabb;
	GetAabb(aabb);
	
	matrix4x4d m;
	GetRotMatrix(m);

	vector3d mx = m*vector3d(1,0,0);
	vector3d mz = m*vector3d(0,0,1);
		
	matrix4x4d frameTrans;
	Frame::GetFrameTransform(GetFrame(), camFrame, frameTrans);
	
	if ((frameTrans*GetPosition()).Length() > WORLDVIEW_ZFAR) {
		return;
	}
	
	m = frameTrans * m;
	
	GetFrustum(planes);
	
	memset(&cityobj_params, 0, sizeof(ObjParams));
	MTRand rand;
	rand.seed(m_sbody->seed);

	for (int i=0; i<4; i++)
	{
		vector3d p = GetPosition();
		vector3d p1, p2, p3, p4;
		const double sizex = START_SEG_SIZE + rand.Int32((int)START_SEG_SIZE);
		const double sizez = START_SEG_SIZE + rand.Int32((int)START_SEG_SIZE);
		
		switch(i) {
			case 3:
				p1 = p + mx*(aabb.min.x) + mz*aabb.min.z;
				p2 = p + mx*(aabb.min.x) + mz*(aabb.min.z-sizez);
				p3 = p + mx*(aabb.min.x+sizex) + mz*(aabb.min.z-sizez);
				p4 = p + mx*(aabb.min.x+sizex) + mz*(aabb.min.z);
				break;
			case 2:
				p1 = p + mx*(aabb.min.x-sizex) + mz*aabb.max.z;
				p2 = p + mx*(aabb.min.x-sizex) + mz*(aabb.max.z-sizez);
				p3 = p + mx*(aabb.min.x) + mz*(aabb.max.z-sizez);
				p4 = p + mx*(aabb.min.x) + mz*(aabb.max.z);
				break;
			case 1:
				p1 = p + mx*(aabb.max.x-sizex) + mz*aabb.max.z;
				p2 = p + mx*(aabb.max.x) + mz*aabb.max.z;
				p3 = p + mx*(aabb.max.x) + mz*(aabb.max.z+sizez);
				p4 = p + mx*(aabb.max.x-sizex) + mz*(aabb.max.z+sizez);
				break;
			default:
			case 0:
				p1 = p + mx*aabb.max.x + mz*aabb.min.z;
				p2 = p + mx*(aabb.max.x+sizex) + mz*aabb.min.z;
				p3 = p + mx*(aabb.max.x+sizex) + mz*(aabb.min.z+sizez);
				p4 = p + mx*aabb.max.x + mz*(aabb.min.z+sizez);
				break;
		}

		putCityBit(planet, rand, frameTrans, m, p1, p2, p3, p4);
		/*
		glPushMatrix();
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		
		sbreSetDepthRange(Pi::GetScrWidth()*0.5, 0.0f, 1.0f);

		matrix4x4d frameTrans;
		Frame::GetFrameTransform(GetFrame(), camFrame, frameTrans);

		vector3d pos = frameTrans * (GetPosition() + p);

			ObjParams params;
			memset(&params, 0, sizeof(ObjParams));
			glPushAttrib(GL_ALL_ATTRIB_BITS);
			matrix4x4d rot;
			GetRotMatrix(rot);
			frameTrans.ClearToRotOnly();
			rot = frameTrans * rot;

			sbreRenderModel(&pos.x, &rot[0], sbreLookupModelByName("building1"), &params);
			glPopAttrib();

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		*/
	}
}
