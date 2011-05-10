#include "Ship.h"
#include "ShipAICmd.h"
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
#include "Render.h"
#include "HyperspaceCloud.h"
#include "ShipCpanel.h"
#include "LmrModel.h"
#include "Polit.h"
#include "CityOnPlanet.h"
#include "Missile.h"

#define TONS_HULL_PER_SHIELD 10.0f

void Ship::Save(Serializer::Writer &wr)
{
	DynamicBody::Save(wr);
	MarketAgent::Save(wr);
	wr.Int32(Serializer::LookupBody(m_combatTarget));
	wr.Int32(Serializer::LookupBody(m_navTarget));
	wr.Vector3d(m_angThrusters);
	wr.Vector3d(m_thrusters);
	wr.Float(m_wheelTransition);
	wr.Float(m_wheelState);
	wr.Float(m_launchLockTimeout);
	wr.Bool(m_testLanded);
	wr.Int32((int)m_flightState);
	wr.Int32((int)m_alertState);
	wr.Float(m_lastFiringAlert);

	m_hyperspace.dest.Serialize(wr);
	wr.Float(m_hyperspace.countdown);
	wr.Int32(m_hyperspace.followHypercloudId);

	for (int i=0; i<ShipType::GUNMOUNT_MAX; i++) {
		wr.Int32(m_gunState[i]);
		wr.Float(m_gunRecharge[i]);
		wr.Float(m_gunTemperature[i]);
	}
	wr.Float(m_ecmRecharge);
	m_shipFlavour.Save(wr);
	wr.Int32(m_dockedWithPort);
	wr.Int32(Serializer::LookupBody(m_dockedWith));
	m_equipment.Save(wr);
	wr.Float(m_stats.hull_mass_left);
	wr.Float(m_stats.shield_mass_left);
	if(m_curAICmd) { wr.Int32(1); m_curAICmd->Save(wr); }
	else wr.Int32(0);
}

void Ship::Load(Serializer::Reader &rd)
{
	DynamicBody::Load(rd);
	MarketAgent::Load(rd);
	// needs fixups
	m_combatTarget = (Body*)rd.Int32();
	m_navTarget = (Body*)rd.Int32();
	m_angThrusters = rd.Vector3d();
	m_thrusters = rd.Vector3d();
	m_wheelTransition = rd.Float();
	m_wheelState = rd.Float();
	m_launchLockTimeout = rd.Float();
	m_testLanded = rd.Bool();
	m_flightState = (FlightState) rd.Int32();
	m_alertState = (AlertState) rd.Int32();
	m_lastFiringAlert = rd.Float();
	
	SBodyPath::Unserialize(rd, &m_hyperspace.dest);
	m_hyperspace.countdown = rd.Float();
	m_hyperspace.followHypercloudId = rd.Int32();

	for (int i=0; i<ShipType::GUNMOUNT_MAX; i++) {
		m_gunState[i] = rd.Int32();
		m_gunRecharge[i] = rd.Float();
		m_gunTemperature[i] = rd.Float();
	}
	m_ecmRecharge = rd.Float();
	m_shipFlavour.Load(rd);
	m_dockedWithPort = rd.Int32();
	m_dockedWith = (SpaceStation*)rd.Int32();
	m_equipment.InitSlotSizes(m_shipFlavour.type);
	m_equipment.Load(rd);
	Init();
	m_stats.hull_mass_left = rd.Float(); // must be after Init()...
	m_stats.shield_mass_left = rd.Float();
	if(rd.Int32()) m_curAICmd = AICommand::Load(rd);
	else m_curAICmd = 0;
}

void Ship::Init()
{
	const ShipType &stype = GetShipType();
	SetModel(stype.lmrModelName.c_str());
	SetMassDistributionFromModel();
	UpdateMass();
	m_stats.hull_mass_left = (float)stype.hullMass;
	m_stats.shield_mass_left = 0;
}

void Ship::PostLoadFixup()
{
	m_combatTarget = Serializer::LookupBody((size_t)m_combatTarget);
	m_navTarget = Serializer::LookupBody((size_t)m_navTarget);
	m_dockedWith = (SpaceStation*)Serializer::LookupBody((size_t)m_dockedWith);
	if (m_curAICmd) m_curAICmd->PostLoadFixup();
}

Ship::Ship(ShipType::Type shipType): DynamicBody()
{
	m_flightState = FLYING;
	m_alertState = ALERT_NONE;
	m_lastFiringAlert = 0.0;
	m_testLanded = false;
	m_launchLockTimeout = 0;
	m_wheelTransition = 0;
	m_wheelState = 0;
	m_dockedWith = 0;
	m_dockedWithPort = 0;
	m_navTarget = 0;
	m_combatTarget = 0;
	m_shipFlavour = ShipFlavour(shipType);
	m_thrusters.x = m_thrusters.y = m_thrusters.z = 0;
	m_angThrusters.x = m_angThrusters.y = m_angThrusters.z = 0;
	m_equipment.InitSlotSizes(shipType);
	m_hyperspace.countdown = 0;
	m_hyperspace.followHypercloudId = 0;
	for (int i=0; i<ShipType::GUNMOUNT_MAX; i++) {
		m_gunState[i] = 0;
		m_gunRecharge[i] = 0;
		m_gunTemperature[i] = 0;
	}
	m_ecmRecharge = 0;
	SetLabel(m_shipFlavour.regid);
	m_curAICmd = 0;

	Init();	
}

void Ship::SetHyperspaceTarget(const SBodyPath *path)
{
	if (path == 0) {
		// need to properly handle unsetting target
		SBodyPath p(0,0,0);
		SetHyperspaceTarget(&p);
	} else {
		m_hyperspace.followHypercloudId = 0;
		m_hyperspace.dest = *path;
		if (this == (Ship*)Pi::player) Pi::onPlayerChangeHyperspaceTarget.emit();
	}
}

void Ship::SetHyperspaceTarget(HyperspaceCloud *cloud)
{
	m_hyperspace.followHypercloudId = cloud->GetId();
	m_hyperspace.dest = *cloud->GetShip()->GetHyperspaceTarget();
	if (this == (Ship*)Pi::player) Pi::onPlayerChangeHyperspaceTarget.emit();
}

void Ship::ClearHyperspaceTarget()
{
	m_hyperspace.followHypercloudId = 0;
	m_hyperspace.dest = SBodyPath();
	m_hyperspace.countdown = 0;
}

float Ship::GetPercentHull() const
{
	const ShipType &stype = GetShipType();
	return 100.0f * (m_stats.hull_mass_left / (float)stype.hullMass);
}

float Ship::GetPercentShields() const
{
	if (m_stats.shield_mass == 0) return 100.0f;
	else return 100.0f * (m_stats.shield_mass_left / m_stats.shield_mass);
}

void Ship::SetPercentHull(float p)
{
	const ShipType &stype = GetShipType();
	m_stats.hull_mass_left = 0.01f * Clamp(p, 0.0f, 100.0f) * (float)stype.hullMass;
}

void Ship::UpdateMass()
{
	CalcStats();
	SetMass(m_stats.total_mass*1000);
}

bool Ship::OnDamage(Object *attacker, float kgDamage)
{
	if (!IsDead()) {
		float dam = kgDamage*0.001f;
		if (m_stats.shield_mass_left > 0.0f) {
			if (m_stats.shield_mass_left > dam) {
				m_stats.shield_mass_left -= dam;
				dam = 0;
			} else {
				dam -= m_stats.shield_mass_left;
				m_stats.shield_mass_left = 0;
			}
		}

		m_stats.hull_mass_left -= dam;
		if (m_stats.hull_mass_left < 0) {
			if (attacker) {
				if (attacker->IsType(Object::BODY)) {
					// XXX remove this call. kill stuff (including elite rating) should be in a script
					static_cast<Body*>(attacker)->OnHaveKilled(this);
					Pi::luaOnShipDestroyed.Queue(this, dynamic_cast<Body*>(attacker));
				}

				if (attacker->IsType(Object::SHIP))
					Polit::NotifyOfCrime((Ship*)attacker, Polit::CRIME_MURDER);
			}

			Space::KillBody(this);
			Sfx::Add(this, Sfx::TYPE_EXPLOSION);
			Sound::BodyMakeNoise(this, "Explosion_1", 1.0f);
		}
		
		else {
			if (attacker && attacker->IsType(Object::SHIP))
				Polit::NotifyOfCrime((Ship*)attacker, Polit::CRIME_PIRACY);

			if (Pi::rng.Double() < kgDamage)
				Sfx::Add(this, Sfx::TYPE_DAMAGE);
			
			if (dam < 0.01 * (float)GetShipType().hullMass)
				Sound::BodyMakeNoise(this, "Hull_hit_Small", 1.0f);
			else
				Sound::BodyMakeNoise(this, "Hull_Hit_Medium", 1.0f);
		}
	}

	//printf("Ouch! %s took %.1f kilos of damage from %s! (%.1f t hull left)\n", GetLabel().c_str(), kgDamage, attacker->GetLabel().c_str(), m_stats.hull_mass_left);
	return true;
}

#define KINETIC_ENERGY_MULT	0.01
bool Ship::OnCollision(Object *b, Uint32 flags, double relVel)
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

	if (
		b->IsType(Object::CITYONPLANET) ||
		b->IsType(Object::SHIP) ||
		b->IsType(Object::PLAYER) ||
		b->IsType(Object::SPACESTATION) ||
		b->IsType(Object::PLANET) ||
		b->IsType(Object::STAR) ||
		b->IsType(Object::CARGOBODY))
	{
		Pi::luaOnShipCollided.Queue(this,
			b->IsType(Object::CITYONPLANET) ? dynamic_cast<CityOnPlanet*>(b)->GetPlanet() : dynamic_cast<Body*>(b));
	}

	return DynamicBody::OnCollision(b, flags, relVel);
}

void Ship::SetThrusterState(const vector3d &levels)
{
	m_thrusters.x = Clamp(levels.x, -1.0, 1.0);
	m_thrusters.y = Clamp(levels.y, -1.0, 1.0);
	m_thrusters.z = Clamp(levels.z, -1.0, 1.0);
}

void Ship::SetAngThrusterState(const vector3d &levels)
{
	m_angThrusters.x = Clamp(levels.x, -1.0, 1.0);
	m_angThrusters.y = Clamp(levels.y, -1.0, 1.0);
	m_angThrusters.z = Clamp(levels.z, -1.0, 1.0);
}

vector3d Ship::GetMaxThrust(const vector3d &dir)
{
	const ShipType &stype = GetShipType();
	vector3d maxThrust;
	maxThrust.x = (dir.x > 0) ? stype.linThrust[ShipType::THRUSTER_RIGHT]
		: -stype.linThrust[ShipType::THRUSTER_LEFT];
	maxThrust.y = (dir.y > 0) ? stype.linThrust[ShipType::THRUSTER_UP]
		: -stype.linThrust[ShipType::THRUSTER_DOWN];
	maxThrust.z = (dir.z > 0) ? stype.linThrust[ShipType::THRUSTER_REVERSE]
		: -stype.linThrust[ShipType::THRUSTER_FORWARD];
	return maxThrust;
}

void Ship::ClearThrusterState()
{
	m_angThrusters = vector3d(0,0,0);
	if (m_launchLockTimeout == 0) m_thrusters = vector3d(0,0,0);
}

Equip::Type Ship::GetHyperdriveFuelType() const
{
	Equip::Type t = m_equipment.Get(Equip::SLOT_ENGINE);
	return EquipType::types[t].inputs[0];
}

const shipstats_t *Ship::CalcStats()
{
	const ShipType &stype = GetShipType();
	m_stats.max_capacity = stype.capacity;
	m_stats.used_capacity = 0;
	m_stats.used_cargo = 0;
	Equip::Type fuelType = GetHyperdriveFuelType();

	for (int i=0; i<Equip::SLOT_MAX; i++) {
		for (int j=0; j<stype.equipSlotCapacity[i]; j++) {
			Equip::Type t = m_equipment.Get((Equip::Slot)i, j);
			if (t) m_stats.used_capacity += EquipType::types[t].mass;
			if ((Equip::Slot)i == Equip::SLOT_CARGO) m_stats.used_cargo += EquipType::types[t].mass;
		}
	}
	m_stats.free_capacity = m_stats.max_capacity - m_stats.used_capacity;
	m_stats.total_mass = m_stats.used_capacity + stype.hullMass;

	m_stats.shield_mass = TONS_HULL_PER_SHIELD * (float)m_equipment.Count(Equip::SLOT_CARGO, Equip::SHIELD_GENERATOR);

	if (stype.equipSlotCapacity[Equip::SLOT_ENGINE]) {
		Equip::Type t = m_equipment.Get(Equip::SLOT_ENGINE);
		int hyperclass = EquipType::types[t].pval;
		if (!hyperclass) { // no drive
			m_stats.hyperspace_range = m_stats.hyperspace_range_max = 0;
		} else {
			m_stats.hyperspace_range_max = Pi::CalcHyperspaceRange(hyperclass, (m_stats.total_mass*0.6));
			m_stats.hyperspace_range = std::min(m_stats.hyperspace_range_max, m_stats.hyperspace_range_max * m_equipment.Count(Equip::SLOT_CARGO, fuelType) /
				(hyperclass * hyperclass));
		}
	} else {
		m_stats.hyperspace_range = m_stats.hyperspace_range_max = 0;
	}
	return &m_stats;
}

static float distance_to_system(const SBodyPath *dest)
{
	int locSecX, locSecY, locSysIdx;
	Pi::currentSystem->GetPos(&locSecX, &locSecY, &locSysIdx);
	
	Sector from_sec(locSecX, locSecY);
	Sector to_sec(dest->sectorX, dest->sectorY);
	return Sector::DistanceBetween(&from_sec, locSysIdx, &to_sec, dest->systemNum);
}

void Ship::UseHyperspaceFuel(const SBodyPath *dest)
{
	int fuel_cost;
	Equip::Type fuelType = GetHyperdriveFuelType();
	double dur;
	bool hscheck = CanHyperspaceTo(dest, fuel_cost, dur);
	assert(hscheck);
	m_equipment.Remove(fuelType, fuel_cost);
	if (fuelType == Equip::MILITARY_FUEL) {
		m_equipment.Add(Equip::RADIOACTIVES, fuel_cost);
	}
}

bool Ship::CanHyperspaceTo(const SBodyPath *dest, int &outFuelRequired, double &outDurationSecs, enum HyperjumpStatus *outStatus) 
{
	Equip::Type t = m_equipment.Get(Equip::SLOT_ENGINE);
	Equip::Type fuelType = GetHyperdriveFuelType();
	int hyperclass = EquipType::types[t].pval;
	int fuel = m_equipment.Count(Equip::SLOT_CARGO, fuelType);
	outFuelRequired = 0;
	if (hyperclass == 0) {
		if (outStatus) *outStatus = HYPERJUMP_NO_DRIVE;
		return false;
	}

	if (Pi::currentSystem && Pi::currentSystem->IsSystem(dest->sectorX, dest->sectorY, dest->systemNum)) {
		if (outStatus) *outStatus = HYPERJUMP_CURRENT_SYSTEM;
		return false;
	}

	float dist = distance_to_system(dest);

	this->CalcStats();
	outFuelRequired = (int)ceil(hyperclass*hyperclass*dist / m_stats.hyperspace_range_max);
	double m_totalmass = m_stats.total_mass;
	if (outFuelRequired > hyperclass*hyperclass) outFuelRequired = hyperclass*hyperclass;
	if (outFuelRequired < 1) outFuelRequired = 1;
	if (dist > m_stats.hyperspace_range_max) {
		outFuelRequired = 0;
		if (outStatus) *outStatus = HYPERJUMP_OUT_OF_RANGE;
		return false;
	} else if (fuel < outFuelRequired) {
		if (outStatus) *outStatus = HYPERJUMP_INSUFFICIENT_FUEL;
		return false;
	} else {
		// Old comments:
		// take at most a week. why a week? because a week is a
		// fundamental physical unit in the same sense that the planck length
		// is, and so it is very probable that future hyperspace
		// technologies will involve travelling a week through time.

		// Now mass has more of an effect on the time taken, this is mainly
		// for gameplay considerations for courier missions and the like.
		outDurationSecs = ((dist * dist * 0.5) / (m_stats.hyperspace_range_max *
			hyperclass)) * 
			(60.0 * 60.0 * 24.0 * sqrt(m_totalmass));
		//float hours = outDurationSecs * 0.0002778;
		//printf("%f LY in %f hours OR %d seconds \n", dist, hours, outDurationSecs);
		//printf("%d seconds\n", outDurationSecs);
		if (outFuelRequired <= fuel) {
			if (outStatus) *outStatus = HYPERJUMP_OK;
			return true;
		} else {
			if (outStatus) *outStatus = HYPERJUMP_INSUFFICIENT_FUEL;
			return false;
		}
	}
}

void Ship::TryHyperspaceTo(const SBodyPath *dest)
{
	if (GetFlightState() != Ship::FLYING) return;

	int fuelUsage;
	double dur;
	Equip::Type t = m_equipment.Get(Equip::SLOT_ENGINE);
	int hyperclass = EquipType::types[t].pval;
	if (m_hyperspace.countdown) return;
	if (!CanHyperspaceTo(dest, fuelUsage, dur)) return;
	m_hyperspace.countdown = 1 + hyperclass;
	m_hyperspace.dest = *dest;
}

float Ship::GetECMRechargeTime()
{
	const Equip::Type t = m_equipment.Get(Equip::SLOT_ECM);
	if (t != Equip::NONE) {
		return EquipType::types[t].rechargeTime;
	} else {
		return 0;
	}
}

void Ship::UseECM()
{
	const Equip::Type t = m_equipment.Get(Equip::SLOT_ECM);
	if (m_ecmRecharge) return;
	if (t != Equip::NONE) {
		Sound::BodyMakeNoise(this, "ECM", 1.0f);
		m_ecmRecharge = GetECMRechargeTime();
		Space::DoECM(GetFrame(), GetPosition(), EquipType::types[t].pval);
	}
}

bool Ship::FireMissile(int idx, Ship *target)
{
	assert(target);

	const Equip::Type t = m_equipment.Get(Equip::SLOT_MISSILE, idx);
	if (t == Equip::NONE) {
		return false;
	}

	m_equipment.Set(Equip::SLOT_MISSILE, idx, Equip::NONE);
	CalcStats();

	matrix4x4d m;
	GetRotMatrix(m);
	vector3d dir = m*vector3d(0,0,-1);
	
	ShipType::Type mtype;
	switch (t) {
		case Equip::MISSILE_SMART: mtype = ShipType::MISSILE_SMART; break;
		case Equip::MISSILE_NAVAL: mtype = ShipType::MISSILE_NAVAL; break;
		case Equip::MISSILE_UNGUIDED: mtype = ShipType::MISSILE_UNGUIDED; break;
		default:
		case Equip::MISSILE_GUIDED: mtype = ShipType::MISSILE_GUIDED; break;
	}
	Missile *missile = new Missile(mtype, this, target);
	missile->SetRotMatrix(m);
	missile->SetFrame(GetFrame());
	// XXX DODGY! need to put it in a sensible location
	missile->SetPosition(GetPosition()+50.0*dir);
	missile->SetVelocity(GetVelocity());
	Space::AddBody(missile);
	return true;
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
	SetThrusterState(1, 1.0);		// thrust upwards
}

void Ship::TestLanded()
{
	m_testLanded = false;
	if (m_launchLockTimeout != 0) return;
	if (m_wheelState != 1.0) return;
	if (GetFrame()->GetBodyFor()->IsType(Object::PLANET)) {
		double speed = GetVelocity().Length();
		vector3d up = GetPosition().Normalized();
		const double planetRadius = static_cast<Planet*>(GetFrame()->GetBodyFor())->GetTerrainHeight(up);

		if (speed < MAX_LANDING_SPEED) {
			// orient the damn thing right
			// Q: i'm totally lost. why is the inverse of the body rot matrix being used?
			// A: NFI. it just works this way
			matrix4x4d rot;
			GetRotMatrix(rot);
			matrix4x4d invRot = rot.InverseOf();

			// check player is sortof sensibly oriented for landing
			const double dot = vector3d(invRot[1], invRot[5], invRot[9]).Normalized().Dot(up);
			if (dot > 0.99) {

				Aabb aabb;
				GetAabb(aabb);

				// position at zero altitude
				SetPosition(up * (planetRadius - aabb.min.y));

				vector3d forward = rot * vector3d(0,0,1);
				vector3d other = up.Cross(forward).Normalized();
				forward = other.Cross(up);

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
				Sound::PlaySfx("Rough_Landing", 1.0f, 1.0f, 0);
			}
		}
	}
}

void Ship::TimeStepUpdate(const float timeStep)
{
	// XXX why not just fucking do this rather than track down the
	// reason that ship geoms are being collision tested during launch
	if (m_flightState == FLYING) Enable();
	else Disable();

	vector3d maxThrust = GetMaxThrust(m_thrusters);
	AddRelForce(vector3d(maxThrust.x*m_thrusters.x, maxThrust.y*m_thrusters.y,
		maxThrust.z*m_thrusters.z));
	AddRelTorque(GetShipType().angThrust * m_angThrusters);

	DynamicBody::TimeStepUpdate(timeStep);
}

void Ship::FireWeapon(int num)
{
	const ShipType &stype = GetShipType();
	if (m_flightState != FLYING) return;
	matrix4x4d m;
	GetRotMatrix(m);

	vector3d dir = vector3d(stype.gunMount[num].dir);
	vector3d pos = vector3d(stype.gunMount[num].pos);
	m_gunTemperature[num] += 0.01f;

	dir = m.ApplyRotationOnly(dir);
	pos = m.ApplyRotationOnly(pos);
	pos += GetPosition();
	
	Equip::Type t = m_equipment.Get(Equip::SLOT_LASER, num);
	const LaserType &lt = Equip::lasers[Equip::types[t].tableIndex];
	m_gunRecharge[num] = lt.rechargeTime;
	vector3d baseVel = GetVelocity();
	vector3d dirVel = lt.speed * dir.Normalized();
	
	if (lt.flags & Equip::LASER_DUAL)
	{
		vector3d sep = dir.Cross(vector3d(m[4],m[5],m[6])).Normalized();
		Projectile::Add(this, t, pos+5.0*sep, baseVel, dirVel);
		Projectile::Add(this, t, pos-5.0*sep, baseVel, dirVel);
	}
	else Projectile::Add(this, t, pos, baseVel, dirVel);

	/*
			// trace laser beam through frame to see who it hits
			CollisionContact c;
			GetFrame()->GetCollisionSpace()->TraceRay(pos, dir, 10000.0, &c, this->GetGeom());
			if (c.userData1) {
				Body *hit = static_cast<Body*>(c.userData1);
				hit->OnDamage(this, damage);
			}
	*/

	Polit::NotifyOfCrime(this, Polit::CRIME_WEAPON_DISCHARGE);
	Sound::BodyMakeNoise(this, "Pulse_Laser", 1.0f);
}

double Ship::GetHullTemperature() const
{
	double dragGs = GetAtmosForce().Length() / (GetMass() * 9.81);
	if (m_equipment.Get(Equip::SLOT_ATMOSHIELD) == Equip::NONE) {
		return dragGs / 30.0;
	} else {
		return dragGs / 300.0;
	}
}

void Ship::UpdateAlertState()
{
	// no alerts if no scanner
	if (m_equipment.Get(Equip::SLOT_SCANNER) == Equip::NONE) {
		// clear existing alert state if there was one
		if (GetAlertState() != ALERT_NONE) {
			SetAlertState(ALERT_NONE);
			Pi::luaOnShipAlertChanged.Queue(this);
		}
		return;
	}

	bool ship_is_near = false, ship_is_firing = false;
	for (Space::bodiesIter_t i = Space::bodies.begin(); i != Space::bodies.end(); i++)
	{
		if ((*i) == this) continue;
		if (!(*i)->IsType(Object::SHIP) || (*i)->IsType(Object::MISSILE)) continue;

		Ship *ship = static_cast<Ship*>(*i);

		if (GetPositionRelTo(ship).LengthSqr() < 100000.0*100000.0) {
			ship_is_near = true;

			Uint32 gunstate = 0;
			for (int j = 0; j < ShipType::GUNMOUNT_MAX; j++)
				gunstate |= ship->m_gunState[j];

			if (gunstate) {
				ship_is_firing = true;
				break;
			}
		}
	}

	bool changed = false;
	switch (m_alertState) {
		case ALERT_NONE:
			if (ship_is_near) {
				SetAlertState(ALERT_SHIP_NEARBY);
				changed = true;
            }
			if (ship_is_firing) {
				m_lastFiringAlert = Pi::GetGameTime();
				SetAlertState(ALERT_SHIP_FIRING);
				changed = true;
			}
			break;

		case ALERT_SHIP_NEARBY:
			if (!ship_is_near) {
				SetAlertState(ALERT_NONE);
				changed = true;
			}
			else if (ship_is_firing) {
				m_lastFiringAlert = Pi::GetGameTime();
				SetAlertState(ALERT_SHIP_FIRING);
				changed = true;
			}
			break;

		case ALERT_SHIP_FIRING:
			if (!ship_is_near) {
				SetAlertState(ALERT_NONE);
				changed = true;
			}
			else if (ship_is_firing) {
				m_lastFiringAlert = Pi::GetGameTime();
			}
			else if (m_lastFiringAlert + 60.0 <= Pi::GetGameTime()) {
				SetAlertState(ALERT_SHIP_NEARBY);
				changed = true;
			}
			break;
	}

	if (changed)
		Pi::luaOnShipAlertChanged.Queue(this);
}

void Ship::StaticUpdate(const float timeStep)
{
	AITimeStep(timeStep);		// moved to correct place, maybe

	if (GetHullTemperature() > 1.0) {
		Space::KillBody(this);
	}

	UpdateAlertState();

	/* FUEL SCOOPING!!!!!!!!! */
	if (m_equipment.Get(Equip::SLOT_FUELSCOOP) != Equip::NONE) {
		Body *astro = GetFrame()->m_astroBody;
		if (astro && astro->IsType(Object::PLANET)) {
			Planet *p = static_cast<Planet*>(astro);
			if (p->IsSuperType(SBody::SUPERTYPE_GAS_GIANT)) {
				double dist = GetPosition().Length();
				double pressure, density;
				p->GetAtmosphericState(dist, &pressure, &density);
			
				double speed = GetVelocity().Length();
				vector3d vdir = GetVelocity().Normalized();
				matrix4x4d rot;
				GetRotMatrix(rot);
				vector3d pdir = -vector3d(rot[8], rot[9], rot[10]).Normalized();
				double dot = vdir.Dot(pdir);
				if ((m_stats.free_capacity) && (dot > 0.95) && (speed > 2000.0) && (density > 1.0)) {
					double rate = speed*density*0.00001f;
					if (Pi::rng.Double() < rate) {
						m_equipment.Add(Equip::HYDROGEN);
						Pi::Message(stringf(512, "Fuel scoop active. You now have %d tonnes of hydrogen.",
									m_equipment.Count(Equip::SLOT_CARGO, Equip::HYDROGEN)));
						UpdateMass();
					}
				}
			}
		}
	}

	// Cargo bay life support
	if (m_equipment.Get(Equip::SLOT_CARGOLIFESUPPORT) != Equip::CARGO_LIFE_SUPPORT) {
		// Hull is pressure-sealed, it just doesn't provide
		// temperature regulation and breathable atmosphere
		
		// kill stuff roughly every 5 seconds
		if ((!m_dockedWith) && (5.0*Pi::rng.Double() < timeStep)) {
			Equip::Type t = (Pi::rng.Int32(2) ? Equip::LIVE_ANIMALS : Equip::SLAVES);
			
			if (m_equipment.Remove(t, 1)) {
				m_equipment.Add(Equip::FERTILIZER);
				Pi::Message("Sensors report critical cargo bay life-support conditions.");
			}
		}
	}
	
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
		float rateCooling = 0.01f;
		if (m_equipment.Get(Equip::SLOT_LASERCOOLER) != Equip::NONE)  {
			rateCooling *= (float)EquipType::types[ m_equipment.Get(Equip::SLOT_LASERCOOLER) ].pval;
		}
		m_gunTemperature[i] -= rateCooling*timeStep;
		if (m_gunTemperature[i] < 0) m_gunTemperature[i] = 0;
		if (m_gunRecharge[i] < 0) m_gunRecharge[i] = 0;

		if (!m_gunState[i]) continue;
		if (m_gunRecharge[i] != 0) continue;
		if (m_gunTemperature[i] > 1.0) continue;

		FireWeapon(i);
	}

	if (m_ecmRecharge) {
		m_ecmRecharge = std::max(0.0f, m_ecmRecharge - timeStep);
	}

	if (m_stats.shield_mass_left < m_stats.shield_mass) {
		// 250 second recharge
		float recharge_rate = 0.004f;
		if (m_equipment.Get(Equip::SLOT_ENERGYBOOSTER) != Equip::NONE) {
			recharge_rate *= (float)EquipType::types[ m_equipment.Get(Equip::SLOT_ENERGYBOOSTER) ].pval;
		}
		m_stats.shield_mass_left += m_stats.shield_mass * recharge_rate * timeStep;
	}
	m_stats.shield_mass_left = Clamp(m_stats.shield_mass_left, 0.0f, m_stats.shield_mass);

	if (m_wheelTransition != 0.0f) {
		m_wheelState += m_wheelTransition*0.3f*timeStep;
		m_wheelState = Clamp(m_wheelState, 0.0f, 1.0f);
		if ((m_wheelState == 0) || (m_wheelState == 1)) m_wheelTransition = 0;
	}

	if (m_testLanded) TestLanded();

	if (m_equipment.Get(Equip::SLOT_HULLAUTOREPAIR) == Equip::HULL_AUTOREPAIR) {
		const ShipType &stype = GetShipType();
		m_stats.hull_mass_left = std::min(m_stats.hull_mass_left + 0.1f*timeStep, (float)stype.hullMass);
	}

	// After calling StartHyperspaceTo this Ship must not spawn objects
	// holding references to it (eg missiles), as StartHyperspaceTo
	// removes the ship from Space::bodies and so the missile will not
	// have references to this cleared by NotifyDeleted()
	if (m_hyperspace.countdown) {
		m_hyperspace.countdown = std::max(m_hyperspace.countdown - timeStep, 0.0f);
		if (m_hyperspace.countdown == 0) {
			Space::StartHyperspaceTo(this, &m_hyperspace.dest);
		}
	}
}

void Ship::NotifyDeleted(const Body* const deletedBody)
{
	if(GetNavTarget() == deletedBody)
		SetNavTarget(0);
	if(GetCombatTarget() == deletedBody)
		SetCombatTarget(0);
	if (m_curAICmd) m_curAICmd->OnDeleted(deletedBody);
}

const ShipType &Ship::GetShipType() const
{
	return ShipType::types[m_shipFlavour.type];
}

bool Ship::Undock()
{
	if (m_dockedWith && m_dockedWith->LaunchShip(this, m_dockedWithPort)) {
		m_testLanded = false;
		onUndock.emit();
		m_dockedWith = 0;
		// lock thrusters for two seconds to push us out of station
		m_launchLockTimeout = 2.0;
		return true;
	} else {
		return false;
	}
}

void Ship::SetDockedWith(SpaceStation *s, int port)
{
	if (s) {
		m_dockedWith = s;
		m_dockedWithPort = port;
		m_wheelState = 1.0f;
		m_flightState = LANDED;
		SetVelocity(vector3d(0,0,0));
		SetAngVelocity(vector3d(0,0,0));
		Disable();
		m_dockedWith->SetDocked(this, port);
		onDock.emit();
	} else {
		Undock();
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
	if (m_wheelState == (down ? 1.0f : 0.0f)) return false;
	if (down) m_wheelTransition = 1;
	else m_wheelTransition = -1;
	return true;
}

void Ship::SetNavTarget(Body* const target)
{
	m_navTarget = target;
	Pi::onPlayerChangeTarget.emit();
	Sound::PlaySfx("OK");
}

void Ship::SetCombatTarget(Body* const target)
{
	m_combatTarget = target;
	Pi::onPlayerChangeTarget.emit();
	Sound::PlaySfx("OK");
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

void Ship::Render(const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	if ((!IsEnabled()) && !m_flightState) return;
	LmrObjParams &params = GetLmrObjParams();
	
	if ( (this != reinterpret_cast<Ship*>(Pi::player)) ||
	     (Pi::worldView->GetCamType() == WorldView::CAM_EXTERNAL) ) {
		m_shipFlavour.ApplyTo(&params);
		SetLmrTimeParams();
		params.angthrust[0] = (float)-m_angThrusters.x;
		params.angthrust[1] = (float)-m_angThrusters.y;
		params.angthrust[2] = (float)-m_angThrusters.z;
		params.linthrust[0] = (float)m_thrusters.x;
		params.linthrust[1] = (float)m_thrusters.y;
		params.linthrust[2] = (float)m_thrusters.z;
		params.argFloats[0] = m_wheelState;
		params.argFloats[5] = (float)m_equipment.Get(Equip::SLOT_FUELSCOOP);
		params.argFloats[6] = (float)m_equipment.Get(Equip::SLOT_ENGINE);
		params.argFloats[7] = (float)m_equipment.Get(Equip::SLOT_ECM);
		params.argFloats[8] = (float)m_equipment.Get(Equip::SLOT_SCANNER);
		params.argFloats[9] = (float)m_equipment.Get(Equip::SLOT_ATMOSHIELD);
		params.argFloats[10] = (float)m_equipment.Get(Equip::SLOT_LASER, 0);
		params.argFloats[11] = (float)m_equipment.Get(Equip::SLOT_LASER, 1);
		for (int i=0; i<8; i++) {
			params.argFloats[12+i] = (float)m_equipment.Get(Equip::SLOT_MISSILE, i);
		}
		//strncpy(params.pText[0], GetLabel().c_str(), sizeof(params.pText));
		RenderLmrModel(viewCoords, viewTransform);

		// draw shield recharge bubble
		if (m_stats.shield_mass_left < m_stats.shield_mass) {
			float shield = 0.01f*GetPercentShields();
			glDisable(GL_LIGHTING);
			glEnable(GL_BLEND);
			glColor4f((1.0f-shield),shield,0.0,0.33f*(1.0f-shield));
			glPushMatrix();
			glTranslatef((GLfloat)viewCoords.x, (GLfloat)viewCoords.y, (GLfloat)viewCoords.z);
			Render::State::UseProgram(Render::simpleShader);
			gluSphere(Pi::gluQuadric, GetLmrCollMesh()->GetBoundingRadius(), 20, 20);
			Render::State::UseProgram(0);
			glPopMatrix();
			glEnable(GL_LIGHTING);
			glDisable(GL_BLEND);
		}
	}
	if (m_ecmRecharge) {
		// pish effect
		vector3f v[100];
		for (int i=0; i<100; i++) {
			const double r1 = Pi::rng.Double()-0.5;
			const double r2 = Pi::rng.Double()-0.5;
			const double r3 = Pi::rng.Double()-0.5;
			v[i] = viewTransform * (
				GetPosition() +
				GetLmrCollMesh()->GetBoundingRadius() *
				vector3d(r1, r2, r3).Normalized()
			);
		}
		Color c(0.5,0.5,1.0,1.0);
		float totalRechargeTime = GetECMRechargeTime();
		if (totalRechargeTime) {
			c.a = m_ecmRecharge / totalRechargeTime;
		}
		GLuint tex = util_load_tex_rgba(PIONEER_DATA_DIR"/textures/ecm.png");

		Render::PutPointSprites(100, v, 50.0f, c, tex);
	}

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
		m_equipment.Remove(t, 1);
		UpdateMass();

		Aabb aabb;
		GetAabb(aabb);
		matrix4x4d rot;
		GetRotMatrix(rot);
		vector3d pos = rot * vector3d(0, aabb.min.y-5, 0);
		CargoBody *cargo = new CargoBody(t);
		cargo->SetFrame(GetFrame());
		cargo->SetPosition(GetPosition()+pos);
		cargo->SetVelocity(GetVelocity()+rot*vector3d(0,-10,0));
		Space::AddBody(cargo);

		Pi::luaOnJettison.Queue(this, cargo);

		return true;
	} else {
		return false;
	}
}

void Ship::UpdateFlavour(const ShipFlavour *f)
{
	assert(f->type == m_shipFlavour.type);
	m_shipFlavour = *f;
}

/*
 * Used when player buys a new ship.
 */
void Ship::ResetFlavour(const ShipFlavour *f)
{
	m_shipFlavour = *f;
	m_equipment.InitSlotSizes(f->type);
	SetLabel(f->regid);
	Init();
}

float Ship::GetWeakestThrustersForce() const
{
	const ShipType &type = GetShipType();
	float val = FLT_MAX;
	for (int i=0; i<ShipType::THRUSTER_MAX; i++) {
		val = std::min(val, fabsf(type.linThrust[i]));
	}
	return val;
}

/* MarketAgent shite */
void Ship::Bought(Equip::Type t) {
	m_equipment.Add(t);
	UpdateMass();
}
void Ship::Sold(Equip::Type t) {
	m_equipment.Remove(t, 1);
	UpdateMass();
}
bool Ship::CanBuy(Equip::Type t, bool verbose) const {
	Equip::Slot slot = EquipType::types[(int)t].slot;
	bool freespace = (m_equipment.FreeSpace(slot)!=0);
	bool freecapacity = (m_stats.free_capacity >= EquipType::types[(int)t].mass);
	if (verbose && (this == (Ship*)Pi::player)) {
		if (!freespace) {
			Pi::Message("You have no free space for this item.");
		}
		else if (!freecapacity) {
			Pi::Message("Your ship is fully laden.");
		}
	}
	return (freespace && freecapacity);
}
bool Ship::CanSell(Equip::Type t, bool verbose) const {
	Equip::Slot slot = EquipType::types[(int)t].slot;
	bool cansell = (m_equipment.Count(slot, t) > 0);
	if (verbose && (this == (Ship*)Pi::player)) {
		if (!cansell) {
			Pi::Message(stringf(512, "You do not have any %s.", EquipType::types[(int)t].name));
		}
	}
	return cansell;
}
Sint64 Ship::GetPrice(Equip::Type t) const {
	if (m_dockedWith) {
		return m_dockedWith->GetPrice(t);
	} else {
		assert(0);
		return 0;
	}
}
