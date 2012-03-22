#include "Ship.h"
#include "ShipAICmd.h"
#include "Frame.h"
#include "Pi.h"
#include "WorldView.h"
#include "Space.h"
#include "Serializer.h"
#include "collider/collider.h"
#include "Sfx.h"
#include "CargoBody.h"
#include "Planet.h"
#include "StarSystem.h"
#include "Sector.h"
#include "Projectile.h"
#include "Sound.h"
#include "HyperspaceCloud.h"
#include "ShipCpanel.h"
#include "LmrModel.h"
#include "Polit.h"
#include "CityOnPlanet.h"
#include "Missile.h"
#include "Lang.h"
#include "StringF.h"
#include "Game.h"
#include "graphics/Material.h"
#include "graphics/Drawables.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/Shader.h"
#include "graphics/TextureBuilder.h"

static const std::string ecmTextureFilename("textures/ecm.png");

#define TONS_HULL_PER_SHIELD 10.0f

void SerializableEquipSet::Save(Serializer::Writer &wr)
{
	wr.Int32(Equip::SLOT_MAX);
	for (int i=0; i<Equip::SLOT_MAX; i++) {
		wr.Int32(equip[i].size());
		for (unsigned int j=0; j<equip[i].size(); j++) {
			wr.Int32(static_cast<int>(equip[i][j]));
		}
	}
}

/*
 * Should have initialised with EquipSet(ShipType::Type) first
 */
void SerializableEquipSet::Load(Serializer::Reader &rd)
{
	const int numSlots = rd.Int32();
	assert(numSlots <= Equip::SLOT_MAX);
	for (int i=0; i<numSlots; i++) {
		const int numItems = rd.Int32();
		for (int j=0; j<numItems; j++) {
			if (j < signed(equip[i].size())) {
				equip[i][j] = static_cast<Equip::Type>(rd.Int32());
			} else {
				// equipment slot sizes have changed. just
				// dump the difference
				rd.Int32();
			}
		}
	}
	onChange.emit(Equip::NONE);
}

void Ship::Save(Serializer::Writer &wr, Space *space)
{
	DynamicBody::Save(wr, space);
	wr.Vector3d(m_angThrusters);
	wr.Vector3d(m_thrusters);
	wr.Int32(m_wheelTransition);
	wr.Float(m_wheelState);
	wr.Float(m_launchLockTimeout);
	wr.Bool(m_testLanded);
	wr.Int32(int(m_flightState));
	wr.Int32(int(m_alertState));
	wr.Double(m_lastFiringAlert);

	// XXX make sure all hyperspace attrs and the cloud get saved
	m_hyperspace.dest.Serialize(wr);
	wr.Float(m_hyperspace.countdown);

	for (int i=0; i<ShipType::GUNMOUNT_MAX; i++) {
		wr.Int32(m_gunState[i]);
		wr.Float(m_gunRecharge[i]);
		wr.Float(m_gunTemperature[i]);
	}
	wr.Float(m_ecmRecharge);
	m_shipFlavour.Save(wr);
	wr.Int32(m_dockedWithPort);
	wr.Int32(space->GetIndexForBody(m_dockedWith));
	m_equipment.Save(wr);
	wr.Float(m_stats.hull_mass_left);
	wr.Float(m_stats.shield_mass_left);
	if(m_curAICmd) { wr.Int32(1); m_curAICmd->Save(wr); }
	else wr.Int32(0);
	wr.Int32(int(m_aiMessage));
	wr.Float(m_thrusterFuel);
}

void Ship::Load(Serializer::Reader &rd, Space *space)
{
	DynamicBody::Load(rd, space);
	// needs fixups
	m_angThrusters = rd.Vector3d();
	m_thrusters = rd.Vector3d();
	m_wheelTransition = rd.Int32();
	m_wheelState = rd.Float();
	m_launchLockTimeout = rd.Float();
	m_testLanded = rd.Bool();
	m_flightState = FlightState(rd.Int32());
	m_alertState = AlertState(rd.Int32());
	m_lastFiringAlert = rd.Double();
	
	m_hyperspace.dest = SystemPath::Unserialize(rd);
	m_hyperspace.countdown = rd.Float();

	for (int i=0; i<ShipType::GUNMOUNT_MAX; i++) {
		m_gunState[i] = rd.Int32();
		m_gunRecharge[i] = rd.Float();
		m_gunTemperature[i] = rd.Float();
	}
	m_ecmRecharge = rd.Float();
	m_shipFlavour.Load(rd);
	m_dockedWithPort = rd.Int32();
	m_dockedWithIndex = rd.Int32();
	m_equipment.InitSlotSizes(m_shipFlavour.type);
	m_equipment.Load(rd);
	Init();
	m_stats.hull_mass_left = rd.Float(); // must be after Init()...
	m_stats.shield_mass_left = rd.Float();
	if(rd.Int32()) m_curAICmd = AICommand::Load(rd);
	else m_curAICmd = 0;
	m_aiMessage = AIError(rd.Int32());
	SetFuel(rd.Float());
	m_stats.fuel_tank_mass_left = GetShipType().fuelTankMass * GetFuel();

	m_equipment.onChange.connect(sigc::mem_fun(this, &Ship::OnEquipmentChange));
}

void Ship::Init()
{
	// XXX the animation namespace must match that in LuaConstants
	// note: this must be set before generating the collision mesh
	// (which happens in SetModel()) and before rendering
	GetLmrObjParams().animationNamespace = "ShipAnimation";
	GetLmrObjParams().equipment = &m_equipment;

	const ShipType &stype = GetShipType();
	SetModel(stype.lmrModelName.c_str());
	SetMassDistributionFromModel();
	UpdateMass();
	m_stats.hull_mass_left = float(stype.hullMass);
	m_stats.shield_mass_left = 0;
	m_hyperspace.now = false;			// TODO: move this on next savegame change, maybe
	m_hyperspaceCloud = 0;
}

void Ship::PostLoadFixup(Space *space)
{
	m_dockedWith = reinterpret_cast<SpaceStation*>(space->GetBodyByIndex(m_dockedWithIndex));
	if (m_curAICmd) m_curAICmd->PostLoadFixup(space);
}

Ship::Ship(ShipType::Type shipType): DynamicBody(),
	m_thrusterFuel(1.f)
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
	m_shipFlavour = ShipFlavour(shipType);
	m_thrusters.x = m_thrusters.y = m_thrusters.z = 0;
	m_angThrusters.x = m_angThrusters.y = m_angThrusters.z = 0;
	m_equipment.InitSlotSizes(shipType);
	m_hyperspace.countdown = 0;
	m_hyperspace.now = false;
	for (int i=0; i<ShipType::GUNMOUNT_MAX; i++) {
		m_gunState[i] = 0;
		m_gunRecharge[i] = 0;
		m_gunTemperature[i] = 0;
	}
	m_ecmRecharge = 0;
	SetLabel(m_shipFlavour.regid);
	m_curAICmd = 0;
	m_aiMessage = AIERROR_NONE;
	m_equipment.onChange.connect(sigc::mem_fun(this, &Ship::OnEquipmentChange));

	Init();	
}

Ship::~Ship()
{
	if (m_curAICmd) delete m_curAICmd;
}


float Ship::GetPercentHull() const
{
	const ShipType &stype = GetShipType();
	return 100.0f * (m_stats.hull_mass_left / float(stype.hullMass));
}

float Ship::GetPercentShields() const
{
	if (m_stats.shield_mass <= 0) return 100.0f;
	else return 100.0f * (m_stats.shield_mass_left / m_stats.shield_mass);
}

void Ship::SetPercentHull(float p)
{
	const ShipType &stype = GetShipType();
	m_stats.hull_mass_left = 0.01f * Clamp(p, 0.0f, 100.0f) * float(stype.hullMass);
}

void Ship::UpdateMass()
{
	CalcStats();
	SetMass((m_stats.total_mass + GetFuel()*GetShipType().fuelTankMass)*1000);
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
					Pi::luaOnShipDestroyed->Queue(this, dynamic_cast<Body*>(attacker));
				}

				if (attacker->IsType(Object::SHIP))
					Polit::NotifyOfCrime(static_cast<Ship*>(attacker), Polit::CRIME_MURDER);
			}

			Pi::game->GetSpace()->KillBody(this);
			Sfx::Add(this, Sfx::TYPE_EXPLOSION);
			Sound::BodyMakeNoise(this, "Explosion_1", 1.0f);
		}
		
		else {
			if (attacker && attacker->IsType(Object::SHIP))
				Polit::NotifyOfCrime(static_cast<Ship*>(attacker), Polit::CRIME_PIRACY);

			if (Pi::rng.Double() < kgDamage)
				Sfx::Add(this, Sfx::TYPE_DAMAGE);
			
			if (dam < 0.01 * float(GetShipType().hullMass))
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

	// hitting cargo scoop surface shouldn't do damage
	if ((m_equipment.Get(Equip::SLOT_CARGOSCOOP) != Equip::NONE) && b->IsType(Object::CARGOBODY) && (flags & 0x100) && m_stats.free_capacity) {
		Equip::Type item = dynamic_cast<CargoBody*>(b)->GetCargoType();
		m_equipment.Add(item);
		Pi::game->GetSpace()->KillBody(dynamic_cast<Body*>(b));
		if (this->IsType(Object::PLAYER))
			Pi::Message(stringf(Lang::CARGO_SCOOP_ACTIVE_1_TONNE_X_COLLECTED, formatarg("item", Equip::types[item].name)));
		// XXX Sfx::Add(this, Sfx::TYPE_SCOOP);
		UpdateMass();
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
		Pi::luaOnShipCollided->Queue(this,
			b->IsType(Object::CITYONPLANET) ? dynamic_cast<CityOnPlanet*>(b)->GetPlanet() : dynamic_cast<Body*>(b));
	}

	return DynamicBody::OnCollision(b, flags, relVel);
}

void Ship::SetThrusterState(const vector3d &levels)
{
	if (m_thrusterFuel <= 0.f) {
		m_thrusters = vector3d(0.0);
	} else {
		m_thrusters.x = Clamp(levels.x, -1.0, 1.0);
		m_thrusters.y = Clamp(levels.y, -1.0, 1.0);
		m_thrusters.z = Clamp(levels.z, -1.0, 1.0);
	}
}

void Ship::SetAngThrusterState(const vector3d &levels)
{
	m_angThrusters.x = Clamp(levels.x, -1.0, 1.0);
	m_angThrusters.y = Clamp(levels.y, -1.0, 1.0);
	m_angThrusters.z = Clamp(levels.z, -1.0, 1.0);
}

vector3d Ship::GetMaxThrust(const vector3d &dir) const
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

double Ship::GetAccelMin() const
{
	const ShipType &stype = GetShipType();
	float val = stype.linThrust[ShipType::THRUSTER_UP];
	val = std::min(val, stype.linThrust[ShipType::THRUSTER_RIGHT]);
	val = std::min(val, -stype.linThrust[ShipType::THRUSTER_LEFT]);
	return val / GetMass();
}

void Ship::ClearThrusterState()
{
	m_angThrusters = vector3d(0,0,0);
	if (m_launchLockTimeout <= 0.0f) m_thrusters = vector3d(0,0,0);
}

Equip::Type Ship::GetHyperdriveFuelType() const
{
	Equip::Type t = m_equipment.Get(Equip::SLOT_ENGINE);
	return Equip::types[t].inputs[0];
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
			Equip::Type t = m_equipment.Get(Equip::Slot(i), j);
			if (t) m_stats.used_capacity += Equip::types[t].mass;
			if (Equip::Slot(i) == Equip::SLOT_CARGO) m_stats.used_cargo += Equip::types[t].mass;
		}
	}
	m_stats.free_capacity = m_stats.max_capacity - m_stats.used_capacity;
	m_stats.total_mass = m_stats.used_capacity + stype.hullMass;

	m_stats.shield_mass = TONS_HULL_PER_SHIELD * float(m_equipment.Count(Equip::SLOT_SHIELD, Equip::SHIELD_GENERATOR));

	if (stype.equipSlotCapacity[Equip::SLOT_ENGINE]) {
		Equip::Type t = m_equipment.Get(Equip::SLOT_ENGINE);
		int hyperclass = Equip::types[t].pval;
		if (!hyperclass) { // no drive
			m_stats.hyperspace_range = m_stats.hyperspace_range_max = 0;
		} else {
			// for the sake of hyperspace range, we count ships mass as 60% of original.
			m_stats.hyperspace_range_max = Pi::CalcHyperspaceRange(hyperclass, m_stats.total_mass); 
			m_stats.hyperspace_range = std::min(m_stats.hyperspace_range_max, m_stats.hyperspace_range_max * m_equipment.Count(Equip::SLOT_CARGO, fuelType) /
				(hyperclass * hyperclass));
		}
	} else {
		m_stats.hyperspace_range = m_stats.hyperspace_range_max = 0;
	}

	m_stats.fuel_tank_mass = stype.fuelTankMass;
	m_stats.fuel_use = stype.thrusterFuelUse;
	m_stats.fuel_tank_mass_left = m_stats.fuel_tank_mass * GetFuel();
	//calculate thruster fuel usage weights
	const float fwd = fabs(stype.linThrust[ShipType::THRUSTER_FORWARD]);
	const float rev = fabs(stype.linThrust[ShipType::THRUSTER_REVERSE]);
	//left/right assumed to match
	const float side = fabs(stype.linThrust[ShipType::THRUSTER_LEFT]);
	//up/down don't always match, but meh. Take average.
	const float up = (fabs(stype.linThrust[ShipType::THRUSTER_UP]) +
		fabs(stype.linThrust[ShipType::THRUSTER_DOWN])) / 2.0;
	const float max = std::max(fwd, rev);
	m_fuelUseWeights[0] = fwd / max;
	m_fuelUseWeights[1] = rev / max;
	m_fuelUseWeights[2] = side / max;
	m_fuelUseWeights[3] = up / max;
	return &m_stats;
}

static float distance_to_system(const SystemPath *dest)
{
	SystemPath here = Pi::game->GetSpace()->GetStarSystem()->GetPath();
	
	Sector sec1(here.sectorX, here.sectorY, here.sectorZ);
	Sector sec2(dest->sectorX, dest->sectorY, dest->sectorZ);

	return Sector::DistanceBetween(&sec1, here.systemIndex, &sec2, dest->systemIndex);
}

void Ship::UseHyperspaceFuel(const SystemPath *dest)
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

bool Ship::CanHyperspaceTo(const SystemPath *dest, int &outFuelRequired, double &outDurationSecs, enum HyperjumpStatus *outStatus) 
{
	Equip::Type t = m_equipment.Get(Equip::SLOT_ENGINE);
	Equip::Type fuelType = GetHyperdriveFuelType();
	int hyperclass = Equip::types[t].pval;
	int fuel = m_equipment.Count(Equip::SLOT_CARGO, fuelType);
	outFuelRequired = 0;
	if (hyperclass == 0) {
		if (outStatus) *outStatus = HYPERJUMP_NO_DRIVE;
		return false;
	}

	StarSystem *s = Pi::game->GetSpace()->GetStarSystem().Get();
	if (s && s->GetPath().IsSameSystem(*dest)) {
		if (outStatus) *outStatus = HYPERJUMP_CURRENT_SYSTEM;
		return false;
	}

	float dist = distance_to_system(dest);

	this->CalcStats();
	outFuelRequired = int(ceil(hyperclass*hyperclass*dist / m_stats.hyperspace_range_max));
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

Ship::HyperjumpStatus Ship::StartHyperspaceCountdown(const SystemPath &dest)
{
	int fuelUsage;
	double duration;

	HyperjumpStatus status;
	if (!CanHyperspaceTo(&dest, fuelUsage, duration, &status))
		return status;

	m_hyperspace.dest = dest;

	Equip::Type t = m_equipment.Get(Equip::SLOT_ENGINE);
	m_hyperspace.countdown = 1.0f + Equip::types[t].pval;
	m_hyperspace.now = false;

	return Ship::HYPERJUMP_OK;
}

void Ship::ResetHyperspaceCountdown()
{
    m_hyperspace.countdown = 0;
	m_hyperspace.now = false;
}

float Ship::GetECMRechargeTime()
{
	const Equip::Type t = m_equipment.Get(Equip::SLOT_ECM);
	if (t != Equip::NONE) {
		return Equip::types[t].rechargeTime;
	} else {
		return 0;
	}
}

void Ship::UseECM()
{
	const Equip::Type t = m_equipment.Get(Equip::SLOT_ECM);
	if (m_ecmRecharge > 0.0f) return;

	if (t != Equip::NONE) {
		Sound::BodyMakeNoise(this, "ECM", 1.0f);
		m_ecmRecharge = GetECMRechargeTime();

		// damage neaby missiles
		const float ECM_RADIUS = 4000.0f;

		for (Space::BodyIterator i = Pi::game->GetSpace()->BodiesBegin(); i != Pi::game->GetSpace()->BodiesEnd(); ++i) {
			if ((*i)->GetFrame() != GetFrame()) continue;
			if (!(*i)->IsType(Object::MISSILE)) continue;

			double dist = ((*i)->GetPosition() - GetPosition()).Length();
			if (dist < ECM_RADIUS) {
				// increasing chance of destroying it with proximity
				if (Pi::rng.Double() > (dist / ECM_RADIUS)) {
					static_cast<Missile*>(*i)->ECMAttack(Equip::types[t].pval);
				}
			}
		}
	}
}

bool Ship::FireMissile(int idx, Ship *target)
{
	assert(target);

	if (GetFlightState() != FLYING) return false;

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
	Pi::game->GetSpace()->AddBody(missile);
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

	Pi::luaOnShipTakeOff->Queue(this, GetFrame()->m_astroBody);
}

void Ship::TestLanded()
{
	m_testLanded = false;
	if (m_launchLockTimeout > 0.0f) return;
	if (m_wheelState < 1.0f) return;
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
				Sound::BodyMakeNoise(this, "Rough_Landing", 1.0f);
				Pi::luaOnShipLanded->Queue(this, GetFrame()->GetBodyFor());
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

	if (IsDead()) Disable();

	vector3d maxThrust = GetMaxThrust(m_thrusters);
	AddRelForce(vector3d(maxThrust.x*m_thrusters.x, maxThrust.y*m_thrusters.y,
		maxThrust.z*m_thrusters.z));
	AddRelTorque(GetShipType().angThrust * m_angThrusters);

	DynamicBody::TimeStepUpdate(timeStep);

	// fuel use decreases mass, so do this as the last thing in the frame
	UpdateFuel(timeStep);
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
			Pi::luaOnShipAlertChanged->Queue(this, LuaConstants::GetConstantString(Pi::luaManager->GetLuaState(), "ShipAlertStatus", ALERT_NONE));
		}
		return;
	}

	bool ship_is_near = false, ship_is_firing = false;
	for (Space::BodyIterator i = Pi::game->GetSpace()->BodiesBegin(); i != Pi::game->GetSpace()->BodiesEnd(); ++i)
	{
		if ((*i) == this) continue;
		if (!(*i)->IsType(Object::SHIP) || (*i)->IsType(Object::MISSILE)) continue;

		Ship *ship = static_cast<Ship*>(*i);

		if (ship->GetFlightState() == LANDED || ship->GetFlightState() == DOCKED) continue;

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
				m_lastFiringAlert = Pi::game->GetTime();
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
				m_lastFiringAlert = Pi::game->GetTime();
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
				m_lastFiringAlert = Pi::game->GetTime();
			}
			else if (m_lastFiringAlert + 60.0 <= Pi::game->GetTime()) {
				SetAlertState(ALERT_SHIP_NEARBY);
				changed = true;
			}
			break;
	}

	if (changed)
		Pi::luaOnShipAlertChanged->Queue(this, LuaConstants::GetConstantString(Pi::luaManager->GetLuaState(), "ShipAlertStatus", GetAlertState()));
}

void Ship::UpdateFuel(const float timeStep)
{
	const float fuelUseRate = GetShipType().thrusterFuelUse * 0.01f;
	const vector3d &tstate = GetThrusterState();
	//weights calculated from thrust values during calcstats
	float totalThrust = 0.f;
	if (tstate.z > 0.0)
		totalThrust = fabs(tstate.z) * m_fuelUseWeights[1];  //backwards
	else
		totalThrust = fabs(tstate.z) * m_fuelUseWeights[0];  //forwards (usually 1)

	totalThrust += fabs(tstate.x) * m_fuelUseWeights[2]; //left-right
	totalThrust += fabs(tstate.y) * m_fuelUseWeights[3]; //up-down

	FuelState lastState = GetFuelState();
	SetFuel(GetFuel() - timeStep * (totalThrust * fuelUseRate));
	FuelState currentState = GetFuelState();

	UpdateMass();

	if (currentState != lastState)
		Pi::luaOnShipFuelChanged->Queue(this, LuaConstants::GetConstantString(Pi::luaManager->GetLuaState(), "ShipFuelStatus", currentState));
}

void Ship::StaticUpdate(const float timeStep)
{
	if (IsDead()) return;

	AITimeStep(timeStep);		// moved to correct place, maybe

	if (GetHullTemperature() > 1.0) {
		Pi::game->GetSpace()->KillBody(this);
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
						if (this->IsType(Object::PLAYER)) {
							Pi::Message(stringf(Lang::FUEL_SCOOP_ACTIVE_N_TONNES_H_COLLECTED,
									formatarg("quantity", m_equipment.Count(Equip::SLOT_CARGO, Equip::HYDROGEN))));
						}
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
				if (this->IsType(Object::PLAYER)) {
					Pi::Message(Lang::CARGO_BAY_LIFE_SUPPORT_LOST);
				}
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
			rateCooling *= float(Equip::types[ m_equipment.Get(Equip::SLOT_LASERCOOLER) ].pval);
		}
		m_gunTemperature[i] -= rateCooling*timeStep;
		if (m_gunTemperature[i] < 0.0f) m_gunTemperature[i] = 0;
		if (m_gunRecharge[i] < 0.0f) m_gunRecharge[i] = 0;

		if (!m_gunState[i]) continue;
		if (m_gunRecharge[i] > 0.0f) continue;
		if (m_gunTemperature[i] > 1.0) continue;

		FireWeapon(i);
	}

	if (m_ecmRecharge > 0.0f) {
		m_ecmRecharge = std::max(0.0f, m_ecmRecharge - timeStep);
	}

	if (m_stats.shield_mass_left < m_stats.shield_mass) {
		// 250 second recharge
		float recharge_rate = 0.004f;
		if (m_equipment.Get(Equip::SLOT_ENERGYBOOSTER) != Equip::NONE) {
			recharge_rate *= float(Equip::types[ m_equipment.Get(Equip::SLOT_ENERGYBOOSTER) ].pval);
		}
		m_stats.shield_mass_left += m_stats.shield_mass * recharge_rate * timeStep;
	}
	m_stats.shield_mass_left = Clamp(m_stats.shield_mass_left, 0.0f, m_stats.shield_mass);

	if (m_wheelTransition) {
		m_wheelState += m_wheelTransition*0.3f*timeStep;
		m_wheelState = Clamp(m_wheelState, 0.0f, 1.0f);
		if (is_equal_exact(m_wheelState, 0.0f) || is_equal_exact(m_wheelState, 1.0f))
			m_wheelTransition = 0;
	}

	if (m_testLanded) TestLanded();

	if (m_equipment.Get(Equip::SLOT_HULLAUTOREPAIR) == Equip::HULL_AUTOREPAIR) {
		const ShipType &stype = GetShipType();
		m_stats.hull_mass_left = std::min(m_stats.hull_mass_left + 0.1f*timeStep, float(stype.hullMass));
	}

	// After calling StartHyperspaceTo this Ship must not spawn objects
	// holding references to it (eg missiles), as StartHyperspaceTo
	// removes the ship from Space::bodies and so the missile will not
	// have references to this cleared by NotifyRemoved()
	if (m_hyperspace.countdown > 0.0f) {
		m_hyperspace.countdown = m_hyperspace.countdown - timeStep;
		if (m_hyperspace.countdown <= 0.0f) {
			m_hyperspace.countdown = 0;
			m_hyperspace.now = true;
		}
	}

	if (m_hyperspace.now) {
		m_hyperspace.now = false;
		EnterHyperspace();
	}
}

void Ship::NotifyRemoved(const Body* const removedBody)
{
	if (m_curAICmd) m_curAICmd->OnDeleted(removedBody);
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
		m_flightState = DOCKED;
		SetVelocity(vector3d(0,0,0));
		SetAngVelocity(vector3d(0,0,0));
		Disable();
		ClearThrusterState();
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
	if (is_equal_exact(m_wheelState, down ? 1.0f : 0.0f)) return false;
	m_wheelTransition = (down ? 1 : -1);
	return true;
}

void Ship::Render(Graphics::Renderer *renderer, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	if ((!IsEnabled()) && !m_flightState) return;
	LmrObjParams &params = GetLmrObjParams();
	
	if ( (!this->IsType(Object::PLAYER)) ||
	     (Pi::worldView->GetCamType() == WorldView::CAM_EXTERNAL) ||
		(Pi::worldView->GetCamType() == WorldView::CAM_SIDEREAL)) {
		m_shipFlavour.ApplyTo(&params);
		SetLmrTimeParams();
		params.angthrust[0] = float(-m_angThrusters.x);
		params.angthrust[1] = float(-m_angThrusters.y);
		params.angthrust[2] = float(-m_angThrusters.z);
		params.linthrust[0] = float(m_thrusters.x);
		params.linthrust[1] = float(m_thrusters.y);
		params.linthrust[2] = float(m_thrusters.z);
		params.animValues[ANIM_WHEEL_STATE] = m_wheelState;
		params.flightState = m_flightState;

		//strncpy(params.pText[0], GetLabel().c_str(), sizeof(params.pText));
		RenderLmrModel(viewCoords, viewTransform);

		// draw shield recharge bubble
		if (m_stats.shield_mass_left < m_stats.shield_mass) {
			const float shield = 0.01f*GetPercentShields();
			renderer->SetBlendMode(Graphics::BLEND_ADDITIVE);
			glPushMatrix();
			matrix4x4f trans = matrix4x4f::Identity();
			trans.Translate(viewCoords.x, viewCoords.y, viewCoords.z);
			trans.Scale(GetLmrCollMesh()->GetBoundingRadius());
			renderer->SetTransform(trans);

			//fade based on strength
			Sfx::shieldEffect->GetMaterial()->diffuse =
				Color((1.0f-shield),shield,0.0,0.33f*(1.0f-shield));
			Sfx::shieldEffect->Draw(renderer);
			glPopMatrix();
			renderer->SetBlendMode(Graphics::BLEND_SOLID);
		}
	}
	if (m_ecmRecharge > 0.0f) {
		// pish effect
		vector3f v[100];
		for (int i=0; i<100; i++) {
			const double r1 = Pi::rng.Double()-0.5;
			const double r2 = Pi::rng.Double()-0.5;
			const double r3 = Pi::rng.Double()-0.5;
			v[i] = vector3f(viewTransform * (
				GetPosition() +
				GetLmrCollMesh()->GetBoundingRadius() *
				vector3d(r1, r2, r3).Normalized()
			));
		}
		Color c(0.5,0.5,1.0,1.0);
		float totalRechargeTime = GetECMRechargeTime();
		if (totalRechargeTime >= 0.0f) {
			c.a = m_ecmRecharge / totalRechargeTime;
		}

		// XXX no need to recreate material every time
		Graphics::Material mat;
		mat.texture0 = Graphics::TextureBuilder::Model(ecmTextureFilename).GetOrCreateTexture(Pi::renderer, "model");
		mat.unlit = true;
		mat.diffuse = c;
		renderer->DrawPointSprites(100, v, &mat, 50.f);
	}
}

bool Ship::Jettison(Equip::Type t)
{
	if (m_flightState != FLYING && m_flightState != DOCKED && m_flightState != LANDED) return false;
	if (t == Equip::NONE) return false;
	Equip::Slot slot = Equip::types[int(t)].slot;
	if (m_equipment.Count(slot, t) > 0) {
		m_equipment.Remove(t, 1);
		UpdateMass();

		if (m_flightState == FLYING) {
			// create a cargo body in space
			Aabb aabb;
			GetAabb(aabb);
			matrix4x4d rot;
			GetRotMatrix(rot);
			vector3d pos = rot * vector3d(0, aabb.min.y-5, 0);
			CargoBody *cargo = new CargoBody(t);
			cargo->SetFrame(GetFrame());
			cargo->SetPosition(GetPosition()+pos);
			cargo->SetVelocity(GetVelocity()+rot*vector3d(0,-10,0));
			Pi::game->GetSpace()->AddBody(cargo);

			Pi::luaOnJettison->Queue(this, cargo);
		} else if (m_flightState == DOCKED) {
			// XXX should move the cargo to the station's temporary storage
			// (can't be recovered at this moment)
			Pi::luaOnCargoUnload->Queue(GetDockedWith(),
				LuaConstants::GetConstantString(Pi::luaManager->GetLuaState(), "EquipType", t));
		} else { // LANDED
			// the cargo is lost
			Pi::luaOnCargoUnload->Queue(GetFrame()->GetBodyFor(),
				LuaConstants::GetConstantString(Pi::luaManager->GetLuaState(), "EquipType", t));			
		}
		return true;
	} else {
		return false;
	}
}

void Ship::OnEquipmentChange(Equip::Type e)
{
	Pi::luaOnShipEquipmentChanged->Queue(this, LuaConstants::GetConstantString(Pi::luaManager->GetLuaState(), "EquipType", e));
}

void Ship::UpdateFlavour(const ShipFlavour *f)
{
	assert(f->type == m_shipFlavour.type);
	m_shipFlavour = *f;
	Pi::luaOnShipFlavourChanged->Queue(this);
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
	Pi::luaOnShipFlavourChanged->Queue(this);
}

void Ship::EnterHyperspace() {
	assert(GetFlightState() != Ship::HYPERSPACE);

	const SystemPath dest = GetHyperspaceDest();

	int fuel;
	Ship::HyperjumpStatus status;
	if (!CanHyperspaceTo(&dest, fuel, m_hyperspace.duration, &status))
		// XXX something has changed (fuel loss, mass change, whatever).
		// could report it to the player but better would be to cancel the
		// countdown before this is reached. either way do something
		return;

	UseHyperspaceFuel(&dest);

	Pi::luaOnLeaveSystem->Queue(this);

	SetFlightState(Ship::HYPERSPACE);

	// virtual call, do class-specific things
	OnEnterHyperspace();
}

void Ship::OnEnterHyperspace() {
	m_hyperspaceCloud = new HyperspaceCloud(this, Pi::game->GetTime() + m_hyperspace.duration, false);
	m_hyperspaceCloud->SetFrame(GetFrame());
	m_hyperspaceCloud->SetPosition(GetPosition());

	Space *space = Pi::game->GetSpace();

	space->RemoveBody(this);
	space->AddBody(m_hyperspaceCloud);
}

void Ship::EnterSystem() {
	assert(GetFlightState() == Ship::HYPERSPACE);

	// virtual call, do class-specific things
	OnEnterSystem();

	SetFlightState(Ship::FLYING);

	Pi::luaOnEnterSystem->Queue(this);
}

void Ship::OnEnterSystem() {
	m_hyperspaceCloud = 0;
}
