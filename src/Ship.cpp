#include "Ship.h"
#include "Frame.h"
#include "Pi.h"
#include "WorldView.h"
#include "Space.h"
#include "SpaceStation.h"
#include "Serializer.h"
#include "collider/collider.h"
#include "Sfx.h"
#include "CargoBody.h"
#include "Planet.h"
#include "StarSystem.h"
#include "Sector.h"
#include "Projectile.h"
#include "Sound.h"

static ObjParams params = {
	{ 0.5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f },

	{	// pColor[3]
	{ { .2f, .2f, .5f }, { 1, 1, 1 }, { 0, 0, 0 }, 100.0 },
	{ { 0.5f, 0.5f, 0.5f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 },
	{ { 0.8f, 0.8f, 0.8f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 } },

	// pText[3][256]	
	{ "IR-L33T", "ME TOO" },
};

void Ship::Save()
{
	using namespace Serializer::Write;
	DynamicBody::Save();
	MarketAgent::Save();
	wr_int(Serializer::LookupBody(m_combatTarget));
	wr_int(Serializer::LookupBody(m_navTarget));
	wr_float(m_angThrusters[0]);
	wr_float(m_angThrusters[1]);
	wr_float(m_angThrusters[2]);
	for (int i=0; i<ShipType::THRUSTER_MAX; i++) wr_float(m_thrusters[i]);
	wr_float(m_wheelTransition);
	wr_float(m_wheelState);
	wr_float(m_launchLockTimeout);
	wr_bool(m_testLanded);
	wr_int((int)m_flightState);
	for (int i=0; i<ShipType::GUNMOUNT_MAX; i++) {
		wr_int(m_gunState[i]);
		wr_float(m_gunRecharge[i]);
	}
	m_shipFlavour.Save();
	wr_int(m_dockedWithPort);
	wr_int(Serializer::LookupBody(m_dockedWith));
	m_equipment.Save();
	wr_float(m_stats.hull_mass_left);
	wr_int(m_todo.size());
	for (std::list<AIInstruction>::iterator i = m_todo.begin(); i != m_todo.end(); ++i) {
		wr_int((int)(*i).cmd);
		switch ((*i).cmd) {
			case DO_KILL:
			case DO_FLY_TO:
				wr_int(Serializer::LookupBody(static_cast<Ship*>((*i).arg)));
				break;
			case DO_NOTHING: wr_int(0); break;
		}
	}
}

void Ship::Load()
{
	using namespace Serializer::Read;
	DynamicBody::Load();
	MarketAgent::Load();
	// needs fixups
	m_combatTarget = (Body*)rd_int();
	m_navTarget = (Body*)rd_int();
	m_angThrusters[0] = rd_float();
	m_angThrusters[1] = rd_float();
	m_angThrusters[2] = rd_float();
	for (int i=0; i<ShipType::THRUSTER_MAX; i++) m_thrusters[i] = rd_float();
	m_wheelTransition = rd_float();
	m_wheelState = rd_float();
	m_launchLockTimeout = rd_float();
	m_testLanded = rd_bool();
	m_flightState = (FlightState) rd_int();
	for (int i=0; i<ShipType::GUNMOUNT_MAX; i++) {
		m_gunState[i] = rd_int();
		if (IsOlderThan(3)) m_gunRecharge[i] = 0;
		else m_gunRecharge[i] = rd_float();
	}
	m_shipFlavour.Load();
	m_dockedWithPort = rd_int();
	m_dockedWith = (SpaceStation*)rd_int();
	m_equipment = EquipSet(m_shipFlavour.type);
	m_equipment.Load();
	Init();
	m_stats.hull_mass_left = rd_float(); // must be after Init()...
	int num = rd_int();
	while (num-- > 0) {
		AICommand c = (AICommand)rd_int();
		void *arg = (void*)rd_int();
		m_todo.push_back(AIInstruction(c, arg));
	}
}

void Ship::Init()
{
	const ShipType &stype = GetShipType();
	SetModel(stype.sbreModelName);
	SetMassDistributionFromModel();
	UpdateMass();
	m_stats.hull_mass_left = (float)stype.hullMass;
}

void Ship::PostLoadFixup()
{
	m_combatTarget = Serializer::LookupBody((size_t)m_combatTarget);
	m_navTarget = Serializer::LookupBody((size_t)m_navTarget);

	{
		SpaceStation *s = (SpaceStation*)Serializer::LookupBody((size_t)m_dockedWith);
		m_dockedWith = 0;
		SetDockedWith(s, m_dockedWithPort);
	}
	for (std::list<AIInstruction>::iterator i = m_todo.begin(); i != m_todo.end(); ++i) {
		switch ((*i).cmd) {
			case DO_KILL:
			case DO_FLY_TO:
				(*i).arg = Serializer::LookupBody((size_t)(*i).arg);
				break;
			case DO_NOTHING: break;
		}
	}
}

Ship::Ship(ShipType::Type shipType): DynamicBody()
{
	m_flightState = FLYING;
	m_testLanded = false;
	m_launchLockTimeout = 0;
	m_wheelTransition = 0;
	m_wheelState = 0;
	m_dockedWith = 0;
	m_dockedWithPort = 0;
	m_navTarget = 0;
	m_combatTarget = 0;
	m_shipFlavour = ShipFlavour(shipType);
	m_angThrusters[0] = m_angThrusters[1] = m_angThrusters[2] = 0;
	m_equipment = EquipSet(shipType);
	for (int i=0; i<ShipType::GUNMOUNT_MAX; i++) {
		m_gunState[i] = 0;
		m_gunRecharge[i] = 0;
	}
	memset(m_thrusters, 0, sizeof(m_thrusters));
	SetLabel(m_shipFlavour.regid);

	Init();	
}

float Ship::GetPercentHull() const
{
	const ShipType &stype = GetShipType();
	return 100.0f * (m_stats.hull_mass_left / (float)stype.hullMass);
}

void Ship::SetPercentHull(float p)
{
	const ShipType &stype = GetShipType();
	m_stats.hull_mass_left = 0.01f * CLAMP(p, 0.0f, 100.0f) * (float)stype.hullMass;
}

void Ship::UpdateMass()
{
	CalcStats();
	SetMass(m_stats.total_mass*1000);
}

bool Ship::OnDamage(Body *attacker, float kgDamage)
{
	if (!IsDead()) {
		m_stats.hull_mass_left -= kgDamage*0.001f;
		if (m_stats.hull_mass_left < 0) {
			attacker->OnHaveKilled(this);
			Space::KillBody(this);
			Sfx::Add(this, Sfx::TYPE_EXPLOSION);
		} else {
			Sfx::Add(this, Sfx::TYPE_DAMAGE);
		}
		Sound::BodyMakeNoise(this, Sound::SFX_COLLISION, 1.0f);
	}
	//printf("Ouch! %s took %.1f kilos of damage from %s! (%.1f t hull left)\n", GetLabel().c_str(), kgDamage, attacker->GetLabel().c_str(), m_stats.hull_mass_left);
	return true;
}

#define KINETIC_ENERGY_MULT	0.01
bool Ship::OnCollision(Body *b, Uint32 flags, double relVel)
{
	// hitting space station docking surfaces shouldn't do damage
	if (b->IsType(Object::SPACESTATION) && (flags & 0x10)) {
		return true;
	}

	if (b->IsType(Object::PLANET)) {
		// geoms still enabled when landed
		if (m_flightState != FLYING) return false;
		else {
			if (GetVelocity().Length() < MAX_LANDING_SPEED) {
				m_testLanded = true;
				return true;
			}
		}
	}
	return DynamicBody::OnCollision(b, flags, relVel);
}

void Ship::SetThrusterState(enum ShipType::Thruster t, float level)
{
	m_thrusters[t] = CLAMP(level, 0.0f, 1.0f);
}

void Ship::ClearThrusterState()
{
	SetAngThrusterState(0, 0.0f);
	SetAngThrusterState(1, 0.0f);
	SetAngThrusterState(2, 0.0f);

	if (m_launchLockTimeout == 0) {
		for (int i=0; i<ShipType::THRUSTER_MAX; i++) m_thrusters[i] = 0;
	}
}

// hyperspace range is:
// (200 * hyperspace_class^2) / total mass (in tonnes)

const shipstats_t *Ship::CalcStats()
{
	const ShipType &stype = GetShipType();
	m_stats.max_capacity = stype.capacity;
	m_stats.used_capacity = 0;
	m_stats.used_cargo = 0;

	for (int i=0; i<Equip::SLOT_MAX; i++) {
		for (int j=0; j<stype.equipSlotCapacity[i]; j++) {
			Equip::Type t = m_equipment.Get((Equip::Slot)i, j);
			if (t) m_stats.used_capacity += EquipType::types[t].mass;
			if ((Equip::Slot)i == Equip::SLOT_CARGO) m_stats.used_cargo += EquipType::types[t].mass;
		}
	}
	m_stats.free_capacity = m_stats.max_capacity - m_stats.used_capacity;
	m_stats.total_mass = m_stats.used_capacity + stype.hullMass;

	Equip::Type t = m_equipment.Get(Equip::SLOT_ENGINE);
	float hyperclass = (float)EquipType::types[t].pval;
	m_stats.hyperspace_range_max = 200 * hyperclass * hyperclass / m_stats.total_mass;
	m_stats.hyperspace_range = MIN(m_stats.hyperspace_range_max, m_stats.hyperspace_range_max * m_equipment.Count(Equip::SLOT_CARGO, Equip::HYDROGEN) /
		(hyperclass * hyperclass));
	return &m_stats;
}

static float distance_to_system(const SBodyPath *dest)
{
	int locSecX, locSecY, locSysIdx;
	Pi::currentSystem->GetPos(&locSecX, &locSecY, &locSysIdx);
	
	Sector from_sec(locSecX, locSecY);
	Sector to_sec(dest->sectorX, dest->sectorY);
	return Sector::DistanceBetween(&from_sec, locSysIdx, &to_sec, dest->systemIdx);
}

void Ship::UseHyperspaceFuel(const SBodyPath *dest)
{
	int fuel_cost;
	bool hscheck = CanHyperspaceTo(dest, fuel_cost);
	assert(hscheck);
	m_equipment.Remove(Equip::SLOT_CARGO, Equip::HYDROGEN, fuel_cost);
}

bool Ship::CanHyperspaceTo(const SBodyPath *dest, int &fuelRequired) 
{
	Equip::Type t = m_equipment.Get(Equip::SLOT_ENGINE);
	float hyperclass = (float)EquipType::types[t].pval;
	int fuel = m_equipment.Count(Equip::SLOT_CARGO, Equip::HYDROGEN);
	fuelRequired = 0;
	if (hyperclass == 0) return false;

	float dist;
	if (Pi::currentSystem && Pi::currentSystem->IsSystem(dest->sectorX, dest->sectorY, dest->systemIdx)) {
		dist = 0;
	} else {
		dist = distance_to_system(dest);
	}

	this->CalcStats();
	fuelRequired = (int)ceil(hyperclass*hyperclass*dist / m_stats.hyperspace_range_max);
	if (fuelRequired > hyperclass*hyperclass) fuelRequired = hyperclass*hyperclass;
	if (fuelRequired < 1) fuelRequired = 1;
	if (dist > m_stats.hyperspace_range) {
		fuelRequired = 0;
		return false;
	} else {
		return fuelRequired <= fuel;
	}
}

void Ship::Blastoff()
{
	if (m_flightState != LANDED) return;

	ClearThrusterState();
	m_flightState = FLYING;
	m_testLanded = false;
	m_dockedWith = 0;
	m_launchLockTimeout = 2.0; // two second of applying thrusters
	
	vector3d up = GetPosition().Normalized();
	Enable();
	assert(GetFrame()->m_astroBody->IsType(Object::PLANET));
	const double planetRadius = 2.0 + static_cast<Planet*>(GetFrame()->m_astroBody)->GetTerrainHeight(up);
	SetVelocity(vector3d(0, 0, 0));
	SetAngVelocity(vector3d(0, 0, 0));
	SetForce(vector3d(0, 0, 0));
	SetTorque(vector3d(0, 0, 0));
	
	Aabb aabb;
	GetAabb(aabb);
	// XXX hm. we need to be able to get sbre aabb
	SetPosition(up*planetRadius - aabb.min.y*up);
	SetThrusterState(ShipType::THRUSTER_TOP, 1.0f);
}

void Ship::TestLanded()
{
	m_testLanded = false;
	if (m_launchLockTimeout != 0) return;
	if (m_wheelState != 1.0) return;
	if (GetFrame()->m_astroBody) {
		double speed = GetVelocity().Length();
		vector3d up = GetPosition().Normalized();
		assert(GetFrame()->m_astroBody->IsType(Object::PLANET));
		const double planetRadius = static_cast<Planet*>(GetFrame()->m_astroBody)->GetTerrainHeight(up);

		if (speed < MAX_LANDING_SPEED) {
			// orient the damn thing right
			// Q: i'm totally lost. why is the inverse of the body rot matrix being used?
			// A: NFI. it just works this way
			matrix4x4d rot;
			GetRotMatrix(rot);
			matrix4x4d invRot = rot.InverseOf();

			// check player is sortof sensibly oriented for landing
			const double dot = vector3d::Dot( vector3d(invRot[1], invRot[5], invRot[9]).Normalized(), up);
			if (dot > 0.99) {

				Aabb aabb;
				GetAabb(aabb);

				// position at zero altitude
				SetPosition(up * (planetRadius - aabb.min.y));

				vector3d forward = rot * vector3d(0,0,1);
				vector3d other = vector3d::Cross(up, forward).Normalized();
				forward = vector3d::Cross(other, up);

				rot = matrix4x4d::MakeRotMatrix(other, up, forward);
				rot = rot.InverseOf();
				SetRotMatrix(rot);

				SetVelocity(vector3d(0, 0, 0));
				SetAngVelocity(vector3d(0, 0, 0));
				SetForce(vector3d(0, 0, 0));
				SetTorque(vector3d(0, 0, 0));
				// we don't use DynamicBody::Disable because that also disables the geom, and that must still get collisions
				DisableBodyOnly();
				ClearThrusterState();
				m_flightState = LANDED;
			}
		}
	}
}
#include "Player.h"

void Ship::TimeStepUpdate(const float timeStep)
{
	DynamicBody::TimeStepUpdate(timeStep);

	const ShipType &stype = GetShipType();
	for (int i=0; i<ShipType::THRUSTER_MAX; i++) {
		float force = stype.linThrust[i] * m_thrusters[i];
		switch (i) {
		case ShipType::THRUSTER_REAR: 
		case ShipType::THRUSTER_FRONT:
			AddRelForce(vector3d(0, 0, force)); break;
		case ShipType::THRUSTER_TOP:
		case ShipType::THRUSTER_BOTTOM:
			AddRelForce(vector3d(0, force, 0)); break;
		case ShipType::THRUSTER_LEFT:
		case ShipType::THRUSTER_RIGHT:
			AddRelForce(vector3d(force, 0, 0)); break;
		}
	}
	AddRelTorque(vector3d(stype.angThrust*m_angThrusters[0],
				  stype.angThrust*m_angThrusters[1],
				  stype.angThrust*m_angThrusters[2]));
}

void Ship::FireWeapon(int num)
{
	const ShipType &stype = GetShipType();
	matrix4x4d m;
	GetRotMatrix(m);

	vector3d dir = vector3d(stype.gunMount[num].dir);
	vector3d pos = vector3d(stype.gunMount[num].pos);

	dir = m.ApplyRotationOnly(dir);
	pos = m.ApplyRotationOnly(pos);
	pos += GetPosition();
	const vector3f sep = vector3f::Cross(dir, vector3f(m[4],m[5],m[6])).Normalized();
	
	Equip::Type t = m_equipment.Get(Equip::SLOT_LASER, num);
	m_gunRecharge[num] = EquipType::types[t].rechargeTime;
//	const float damage = 1000.0f * (float)EquipType::types[t].pval;
	vector3d baseVel = GetVelocity();
	vector3d dirVel = 1000.0*dir.Normalized();
	
	CollisionContact c;
	switch (t) {
		case Equip::PULSECANNON_1MW:
			Projectile::Add(this, Projectile::TYPE_1MW_PULSE, pos, baseVel, dirVel);
			break;
		case Equip::PULSECANNON_DUAL_1MW:
			Projectile::Add(this, Projectile::TYPE_1MW_PULSE, pos+5.0*sep, baseVel, dirVel);
			Projectile::Add(this, Projectile::TYPE_1MW_PULSE, pos-5.0*sep, baseVel, dirVel);
			break;
		case Equip::PULSECANNON_2MW:
			Projectile::Add(this, Projectile::TYPE_2MW_PULSE, pos, baseVel, dirVel);
			break;
		case Equip::PULSECANNON_4MW:
			Projectile::Add(this, Projectile::TYPE_4MW_PULSE, pos, baseVel, dirVel);
			break;
		case Equip::PULSECANNON_10MW:
			Projectile::Add(this, Projectile::TYPE_10MW_PULSE, pos, baseVel, dirVel);
			break;
		case Equip::PULSECANNON_20MW:
			Projectile::Add(this, Projectile::TYPE_20MW_PULSE, pos, baseVel, dirVel);
			break;
			// trace laser beam through frame to see who it hits
	/*		GetFrame()->GetCollisionSpace()->TraceRay(pos, dir, 10000.0, &c, this->GetGeom());
			if (c.userData1) {
				Body *hit = static_cast<Body*>(c.userData1);
				hit->OnDamage(this, damage);
			}
			*/
			break;
		default:
			fprintf(stderr, "Unknown weapon %d\n", t);
			assert(0);
			break;
	}
	Sound::BodyMakeNoise(this, Sound::SFX_PULSECANNON, 1.0f);
}

void Ship::StaticUpdate(const float timeStep)
{
	AITimeStep(timeStep);
	
	if (m_flightState == FLYING)
		m_launchLockTimeout -= timeStep;
	if (m_launchLockTimeout < 0) m_launchLockTimeout = 0;
	/* can't orient ships in SetDockedWith() because it gets
	 * called from collision handler, and collision system gets a bit
	 * weirded out if bodies are moved in the middle of collision detection
	 */
	if (m_dockedWith) m_dockedWith->OrientDockedShip(this, m_dockedWithPort);

	// lasers
	for (int i=0; i<ShipType::GUNMOUNT_MAX; i++) {
		m_gunRecharge[i] -= timeStep;
		if (m_gunRecharge[i] < 0) m_gunRecharge[i] = 0;

		if (!m_gunState[i]) continue;
		if (m_gunRecharge[i] != 0) continue;

		FireWeapon(i);
	}

	if (m_wheelTransition != 0.0f) {
		m_wheelState += m_wheelTransition*timeStep;
		m_wheelState = CLAMP(m_wheelState, 0, 1);
		if ((m_wheelState == 0) || (m_wheelState == 1)) m_wheelTransition = 0;
	}

	if (m_testLanded) TestLanded();
}

void Ship::NotifyDeath(const Body* const dyingBody)
{
	if(GetNavTarget() == dyingBody)
		SetNavTarget(0);
	if(GetCombatTarget() == dyingBody)
		SetCombatTarget(0);
	AIBodyHasDied(dyingBody);
}

const ShipType &Ship::GetShipType() const
{
	return ShipType::types[m_shipFlavour.type];
}

void Ship::SetDockedWith(SpaceStation *s, int port)
{
	if (m_dockedWith && !s) {
		m_dockedWith->LaunchShip(this, m_dockedWithPort);
		m_testLanded = false;
		onUndock.emit();
		m_dockedWith = 0;
		// lock thrusters for two seconds to push us out of station
		m_launchLockTimeout = 2.0;
	} else if (!s) {
	
	} else {
		m_dockedWith = s;
		m_dockedWithPort = port;
		m_wheelState = 1.0f;
		if (s->IsGroundStation()) m_flightState = LANDED;
		else m_flightState = DOCKING;
		SetVelocity(vector3d(0,0,0));
		SetAngVelocity(vector3d(0,0,0));
		Disable();
		m_dockedWith->PositionDockedShip(this, port);
		onDock.emit();
	}
}

void Ship::SetGunState(int idx, int state)
{
	if (m_equipment.Get(Equip::SLOT_LASER, idx) != Equip::NONE) {
		m_gunState[idx] = state;
	}
}

bool Ship::SetWheelState(bool down)
{
	if (m_flightState != FLYING) return false;
	if (down) m_wheelTransition = 1;
	else m_wheelTransition = -1;
	return true;
}

void Ship::SetNavTarget(Body* const target)
{
	m_navTarget = target;
	Pi::onPlayerChangeTarget.emit();
}

void Ship::SetCombatTarget(Body* const target)
{
	m_combatTarget = target;
	Pi::onPlayerChangeTarget.emit();
}

#if 0
bool Ship::IsFiringLasers()
{
	for (int i=0; i<ShipType::GUNMOUNT_MAX; i++) {
		if (m_gunState[i]) return true;
	}
	return false;
}

/* Assumed to be at model coords */
void Ship::RenderLaserfire()
{
	const ShipType &stype = GetShipType();
	glDisable(GL_LIGHTING);
	
	for (int i=0; i<ShipType::GUNMOUNT_MAX; i++) {
		if (!m_gunState[i]) continue;
		glPushAttrib(GL_CURRENT_BIT | GL_LINE_BIT);
		switch (m_equipment.Get(Equip::SLOT_LASER, i)) {
			case Equip::LASER_2MW_BEAM:
				glColor3f(1,.5,0); break;
			case Equip::LASER_4MW_BEAM:
				glColor3f(1,1,0); break;
			default:
			case Equip::LASER_1MW_BEAM:
				glColor3f(1,0,0); break;
		}
		glLineWidth(2.0f);
		glBegin(GL_LINES);
		vector3f pos = stype.gunMount[i].pos;
		glVertex3f(pos.x, pos.y, pos.z);
		glVertex3fv(&((10000)*stype.gunMount[i].dir)[0]);
		glEnd();
		glPopAttrib();
	}
	glEnable(GL_LIGHTING);
}
#endif /* 0 */


static void render_coll_mesh(const CollMesh *m)
{
	glDisable(GL_LIGHTING);
	glColor3f(1,0,1);
	glBegin(GL_TRIANGLES);
	for (int i=0; i<m->ni; i+=3) {
		glVertex3fv(&m->pVertex[3*m->pIndex[i]]);
		glVertex3fv(&m->pVertex[3*m->pIndex[i+1]]);
		glVertex3fv(&m->pVertex[3*m->pIndex[i+2]]);
	}
	glEnd();
	glColor3f(1,1,1);
	glDepthRange(0,1.0f-0.0002f);
	for (int i=0; i<m->ni; i+=3) {
		glBegin(GL_LINE_LOOP);
		glVertex3fv(&m->pVertex[3*m->pIndex[i]]);
		glVertex3fv(&m->pVertex[3*m->pIndex[i+1]]);
		glVertex3fv(&m->pVertex[3*m->pIndex[i+2]]);
		glEnd();
	}
	glDepthRange(0.0,1.0);
	glEnable(GL_LIGHTING);
}

void Ship::Render(const Frame *camFrame)
{
	if ((!IsEnabled()) && !m_flightState) return;
	params.angthrust[0] = -m_angThrusters[0];
	params.angthrust[1] = -m_angThrusters[1];
	params.angthrust[2] = -m_angThrusters[2];
	params.linthrust[0] = m_thrusters[ShipType::THRUSTER_RIGHT] - m_thrusters[ShipType::THRUSTER_LEFT];
	params.linthrust[1] = m_thrusters[ShipType::THRUSTER_TOP] - m_thrusters[ShipType::THRUSTER_BOTTOM];
	params.linthrust[2] = m_thrusters[ShipType::THRUSTER_FRONT] - m_thrusters[ShipType::THRUSTER_REAR];
	params.pAnim[ASRC_SECFRAC] = (float)Pi::GetGameTime();
	params.pAnim[ASRC_MINFRAC] = (float)(Pi::GetGameTime() / 60.0);
	params.pAnim[ASRC_HOURFRAC] = (float)(Pi::GetGameTime() / 3600.0);
	params.pAnim[ASRC_DAYFRAC] = (float)(Pi::GetGameTime() / (24*3600.0));
	params.pAnim[ASRC_GEAR] = m_wheelState;
	params.pFlag[AFLAG_GEAR] = m_wheelState != 0.0f;
	strncpy(params.pText[0], GetLabel().c_str(), sizeof(params.pText));
	RenderSbreModel(camFrame, &params);

#if 0
	if (IsFiringLasers()) {
		glPushMatrix();
		TransformToModelCoords(camFrame);
		RenderLaserfire();
		glPopMatrix();
	}
#endif /* 0 */
}

bool Ship::Jettison(Equip::Type t)
{
	if (m_flightState != FLYING) return false;
	Equip::Slot slot = EquipType::types[(int)t].slot;
	if (m_equipment.Count(slot, t) > 0) {
		m_equipment.Remove(slot, t, 1);

		Aabb aabb;
		GetAabb(aabb);
		matrix4x4d rot;
		GetRotMatrix(rot);
		vector3d pos = rot * vector3d(0, aabb.min.y-5, 0);
		CargoBody *cargo = new CargoBody(t);
		cargo->SetFrame(GetFrame());
		cargo->SetPosition(Pi::player->GetPosition()+pos);
		cargo->SetVelocity(Pi::player->GetVelocity() + rot*vector3d(0,-10,0));
		Space::AddBody(cargo);
		return true;
	} else {
		return false;
	}
}

/*
 * Used when player buys a new ship.
 */
void Ship::ChangeFlavour(const ShipFlavour *f)
{
	m_shipFlavour = *f;
	m_equipment = EquipSet(f->type);
	SetLabel(f->regid);
	Init();
}

/* MarketAgent shite */
void Ship::Bought(Equip::Type t) {
	Equip::Slot slot = EquipType::types[(int)t].slot;
	m_equipment.Add(slot, t);
	CalcStats();
}
void Ship::Sold(Equip::Type t) {
	Equip::Slot slot = EquipType::types[(int)t].slot;
	m_equipment.Remove(slot, t, 1);
	CalcStats();
}
bool Ship::CanBuy(Equip::Type t) const {
	Equip::Slot slot = EquipType::types[(int)t].slot;
	return (m_equipment.FreeSpace(slot)!=0) &&
	       (m_stats.free_capacity >= EquipType::types[(int)t].mass);
}
bool Ship::CanSell(Equip::Type t) const {
	Equip::Slot slot = EquipType::types[(int)t].slot;
	return m_equipment.Count(slot, t) > 0;
}
int Ship::GetPrice(Equip::Type t) const {
	if (m_dockedWith) {
		return m_dockedWith->GetPrice(t);
	} else {
		assert(0);
		return 0;
	}
}
