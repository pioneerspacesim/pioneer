// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Ship.h"
#include "CityOnPlanet.h"
#include "Lang.h"
#include "EnumStrings.h"
#include "LuaEvent.h"
#include "Missile.h"
#include "Projectile.h"
#include "ShipAICmd.h"
#include "ShipController.h"
#include "Sound.h"
#include "Sfx.h"
#include "galaxy/Sector.h"
#include "galaxy/SectorCache.h"
#include "Frame.h"
#include "WorldView.h"
#include "HyperspaceCloud.h"
#include "graphics/Drawables.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/TextureBuilder.h"
#include "collider/collider.h"
#include "StringF.h"

static const float TONS_HULL_PER_SHIELD = 10.f;
static const double KINETIC_ENERGY_MULT	= 0.01;
HeatGradientParameters_t Ship::s_heatGradientParams;
const float Ship::DEFAULT_SHIELD_COOLDOWN_TIME = 1.0f;

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
	m_skin.Save(wr);
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
		wr.Int32(m_gun[i].state);
		wr.Float(m_gun[i].recharge);
		wr.Float(m_gun[i].temperature);
	}
	wr.Float(m_ecmRecharge);
	wr.String(m_type->id);
	wr.Int32(m_dockedWithPort);
	wr.Int32(space->GetIndexForBody(m_dockedWith));
	m_equipment.Save(wr);
	wr.Float(m_stats.hull_mass_left);
	wr.Float(m_stats.shield_mass_left);
	wr.Float(m_shieldCooldown);
	if(m_curAICmd) { wr.Int32(1); m_curAICmd->Save(wr); }
	else wr.Int32(0);
	wr.Int32(int(m_aiMessage));
	wr.Double(m_thrusterFuel);
	wr.Double(m_reserveFuel);

	wr.Int32(static_cast<int>(m_controller->GetType()));
	m_controller->Save(wr, space);

	m_navLights->Save(wr);
}

void Ship::Load(Serializer::Reader &rd, Space *space)
{
	DynamicBody::Load(rd, space);
	m_skin.Load(rd);
	m_skin.Apply(GetModel());
	// needs fixups
	m_angThrusters = rd.Vector3d();
	m_thrusters = rd.Vector3d();
	m_wheelTransition = rd.Int32();
	m_wheelState = rd.Float();
	m_launchLockTimeout = rd.Float();
	m_testLanded = rd.Bool();
	m_flightState = static_cast<FlightState>(rd.Int32());
	m_alertState = static_cast<AlertState>(rd.Int32());
	Properties().Set("flightState", EnumStrings::GetString("ShipFlightState", m_flightState));
	Properties().Set("alertStatus", EnumStrings::GetString("ShipAlertStatus", m_alertState));
	m_lastFiringAlert = rd.Double();

	m_hyperspace.dest = SystemPath::Unserialize(rd);
	m_hyperspace.countdown = rd.Float();

	for (int i=0; i<ShipType::GUNMOUNT_MAX; i++) {
		m_gun[i].state = rd.Int32();
		m_gun[i].recharge = rd.Float();
		m_gun[i].temperature = rd.Float();
	}
	m_ecmRecharge = rd.Float();
	SetShipId(rd.String()); // XXX handle missing thirdparty ship
	m_dockedWithPort = rd.Int32();
	m_dockedWithIndex = rd.Int32();
	m_equipment.InitSlotSizes(m_type->id);
	m_equipment.Load(rd);
	Init();
	m_stats.hull_mass_left = rd.Float(); // must be after Init()...
	m_stats.shield_mass_left = rd.Float();
	m_shieldCooldown = rd.Float();
	if(rd.Int32()) m_curAICmd = AICommand::Load(rd);
	else m_curAICmd = 0;
	m_aiMessage = AIError(rd.Int32());
	SetFuel(rd.Double());
	m_stats.fuel_tank_mass_left = GetShipType()->fuelTankMass * GetFuel();
	m_reserveFuel = rd.Double();
	UpdateStats(); // this is necessary, UpdateStats() in Ship::Init has wrong values of m_thrusterFuel after Load

	PropertyMap &p = Properties();
	p.Set("hullMassLeft", m_stats.hull_mass_left);
	p.Set("hullPercent", 100.0f * (m_stats.hull_mass_left / float(m_type->hullMass)));
	p.Set("shieldMassLeft", m_stats.shield_mass_left);
	p.Set("fuelMassLeft", m_stats.fuel_tank_mass_left);

	m_controller = 0;
	const ShipController::Type ctype = static_cast<ShipController::Type>(rd.Int32());
	if (ctype == ShipController::PLAYER)
		SetController(new PlayerShipController());
	else
		SetController(new ShipController());
	m_controller->Load(rd);

	m_navLights->Load(rd);

	m_equipment.onChange.connect(sigc::mem_fun(this, &Ship::OnEquipmentChange));
}

void Ship::InitGun(const char *tag, int num)
{
	const SceneGraph::MatrixTransform *mt = GetModel()->FindTagByName(tag);
	if (mt) {
		const matrix4x4f &trans = mt->GetTransform();
		m_gun[num].pos = trans.GetTranslate();
		m_gun[num].dir = trans.GetOrient().VectorZ();
	}
	else {
		// XXX deprecated
		m_gun[num].pos = m_type->gunMount[num].pos;
		m_gun[num].dir = m_type->gunMount[num].dir;
	}
}

void Ship::InitMaterials()
{
	SceneGraph::Model *pModel = GetModel();
	assert(pModel);
	const Uint32 numMats = pModel->GetNumMaterials();
	for( Uint32 m=0; m<numMats; m++ ) {
		RefCountedPtr<Graphics::Material> mat = pModel->GetMaterialByIndex(m);
		mat->heatGradient = Graphics::TextureBuilder::Decal("textures/heat_gradient.png").GetOrCreateTexture(Pi::renderer, "model");
		mat->specialParameter0 = &s_heatGradientParams;
	}
	s_heatGradientParams.heatingAmount = 0.0f;
	s_heatGradientParams.heatingNormal = vector3f(0.0f, -1.0f, 0.0f);
}

void Ship::Init()
{
	m_invulnerable = false;

	m_sensors.reset(new Sensors(this));

	m_navLights.reset(new NavLights(GetModel()));
	m_navLights->SetEnabled(true);

	SetMassDistributionFromModel();
	UpdateStats();
	m_stats.hull_mass_left = float(m_type->hullMass);
	m_stats.shield_mass_left = 0;

	PropertyMap &p = Properties();
	p.Set("hullMassLeft", m_stats.hull_mass_left);
	p.Set("hullPercent", 100.0f * (m_stats.hull_mass_left / float(m_type->hullMass)));
	p.Set("shieldMassLeft", m_stats.shield_mass_left);
	p.Set("fuelMassLeft", m_stats.fuel_tank_mass_left);

	m_hyperspace.now = false;			// TODO: move this on next savegame change, maybe
	m_hyperspaceCloud = 0;

	m_landingGearAnimation = GetModel()->FindAnimation("gear_down");

	InitGun("tag_gunmount_0", 0);
	InitGun("tag_gunmount_1", 1);

	// If we've got the tag_landing set then use it for an offset otherwise grab the AABB
	const SceneGraph::MatrixTransform *mt = GetModel()->FindTagByName("tag_landing");
	if( mt ) {
		m_landingMinOffset = mt->GetTransform().GetTranslate().y;
	} else {
		m_landingMinOffset = GetAabb().min.y;
	}

	InitMaterials();
}

void Ship::PostLoadFixup(Space *space)
{
	DynamicBody::PostLoadFixup(space);
	m_dockedWith = static_cast<SpaceStation*>(space->GetBodyByIndex(m_dockedWithIndex));
	if (m_curAICmd) m_curAICmd->PostLoadFixup(space);
	m_controller->PostLoadFixup(space);
}

Ship::Ship(ShipType::Id shipId): DynamicBody(),
	m_controller(0),
	m_thrusterFuel(1.0),
	m_reserveFuel(0.0),
	m_landingGearAnimation(nullptr)
{
	m_flightState = FLYING;
	m_alertState = ALERT_NONE;
	Properties().Set("flightState", EnumStrings::GetString("ShipFlightState", m_flightState));
	Properties().Set("alertStatus", EnumStrings::GetString("ShipAlertStatus", m_alertState));

	m_lastFiringAlert = 0.0;
	m_testLanded = false;
	m_launchLockTimeout = 0;
	m_wheelTransition = 0;
	m_wheelState = 0;
	m_dockedWith = 0;
	m_dockedWithPort = 0;
	SetShipId(shipId);
	m_thrusters.x = m_thrusters.y = m_thrusters.z = 0;
	m_angThrusters.x = m_angThrusters.y = m_angThrusters.z = 0;
	m_equipment.InitSlotSizes(shipId);
	m_hyperspace.countdown = 0;
	m_hyperspace.now = false;
	m_hyperspace.ignoreFuel = false;
	for (int i=0; i<ShipType::GUNMOUNT_MAX; i++) {
		m_gun[i].state = 0;
		m_gun[i].recharge = 0;
		m_gun[i].temperature = 0;
	}
	m_ecmRecharge = 0;
	m_shieldCooldown = 0.0f;
	m_curAICmd = 0;
	m_aiMessage = AIERROR_NONE;
	m_decelerating = false;
	m_equipment.onChange.connect(sigc::mem_fun(this, &Ship::OnEquipmentChange));

	SetModel(m_type->modelName.c_str());
	SetLabel("UNLABELED_SHIP");
	m_skin.SetRandomColors(Pi::rng);
	m_skin.SetDecal(m_type->manufacturer);
	m_skin.Apply(GetModel());
	GetModel()->SetPattern(Pi::rng.Int32(0, GetModel()->GetNumPatterns()));

	Init();
	SetController(new ShipController());
}

Ship::~Ship()
{
	if (m_curAICmd) delete m_curAICmd;
	delete m_controller;
}

void Ship::SetController(ShipController *c)
{
	assert(c != 0);
	if (m_controller) delete m_controller;
	m_controller = c;
	m_controller->m_ship = this;
}

float Ship::GetPercentHull() const
{
	return 100.0f * (m_stats.hull_mass_left / float(m_type->hullMass));
}

float Ship::GetPercentShields() const
{
	if (m_stats.shield_mass <= 0) return 100.0f;
	else return 100.0f * (m_stats.shield_mass_left / m_stats.shield_mass);
}

void Ship::SetPercentHull(float p)
{
	m_stats.hull_mass_left = 0.01f * Clamp(p, 0.0f, 100.0f) * float(m_type->hullMass);
	Properties().Set("hullMassLeft", m_stats.hull_mass_left);
	Properties().Set("hullPercent", 100.0f * (m_stats.hull_mass_left / float(m_type->hullMass)));
}

void Ship::UpdateMass()
{
	SetMass((m_stats.total_mass + GetFuel()*GetShipType()->fuelTankMass)*1000);
}

void Ship::SetFuel(const double f)
{
	m_thrusterFuel = Clamp(f, 0.0, 1.0);
	Properties().Set("fuel", m_thrusterFuel*100); // XXX to match SetFuelPercent
}

// returns speed that can be reached using fuel minus reserve according to the Tsiolkovsky equation
double Ship::GetSpeedReachedWithFuel() const
{
	const double fuelmass = 1000*GetShipType()->fuelTankMass * (m_thrusterFuel - m_reserveFuel);
	if (fuelmass < 0) return 0.0;
	return GetShipType()->effectiveExhaustVelocity * log(GetMass()/(GetMass()-fuelmass));
}

bool Ship::OnDamage(Object *attacker, float kgDamage, const CollisionContact& contactData)
{
	if (m_invulnerable) {
		Sound::BodyMakeNoise(this, "Hull_hit_Small", 0.5f);
		return true;
	}

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
			Properties().Set("shieldMassLeft", m_stats.shield_mass_left);
		}

		m_shieldCooldown = DEFAULT_SHIELD_COOLDOWN_TIME;
		// transform the collision location into the models local space (from world space) and add it as a hit.
		matrix4x4d mtx = GetOrient();
		mtx.SetTranslate( GetPosition() );
		const matrix4x4d invmtx = mtx.InverseOf();
		const vector3d localPos = invmtx * contactData.pos;
		GetShields()->AddHit(localPos);

		m_stats.hull_mass_left -= dam;
		Properties().Set("hullMassLeft", m_stats.hull_mass_left);
		Properties().Set("hullPercent", 100.0f * (m_stats.hull_mass_left / float(m_type->hullMass)));
		if (m_stats.hull_mass_left < 0) {
			if (attacker) {
				if (attacker->IsType(Object::BODY))
					LuaEvent::Queue("onShipDestroyed", this, dynamic_cast<Body*>(attacker));

				if (attacker->IsType(Object::SHIP))
					Polit::NotifyOfCrime(static_cast<Ship*>(attacker), Polit::CRIME_MURDER);
			}

			Explode();
		} else {
			if (attacker && attacker->IsType(Object::SHIP))
				Polit::NotifyOfCrime(static_cast<Ship*>(attacker), Polit::CRIME_PIRACY);

			if (Pi::rng.Double() < kgDamage)
				Sfx::Add(this, Sfx::TYPE_DAMAGE);

			if (dam < 0.01 * float(GetShipType()->hullMass))
				Sound::BodyMakeNoise(this, "Hull_hit_Small", 1.0f);
			else
				Sound::BodyMakeNoise(this, "Hull_Hit_Medium", 1.0f);
		}
	}

	//Output("Ouch! %s took %.1f kilos of damage from %s! (%.1f t hull left)\n", GetLabel().c_str(), kgDamage, attacker->GetLabel().c_str(), m_stats.hull_mass_left);
	return true;
}

bool Ship::OnCollision(Object *b, Uint32 flags, double relVel)
{
	// hitting space station docking surfaces shouldn't do damage
	if (b->IsType(Object::SPACESTATION) && (flags & 0x10)) {
		return true;
	}

	// hitting cargo scoop surface shouldn't do damage
	if ((m_equipment.Get(Equip::SLOT_CARGOSCOOP) != Equip::NONE) && b->IsType(Object::CARGOBODY) && m_stats.free_capacity) {
		Equip::Type item = dynamic_cast<CargoBody*>(b)->GetCargoType();
		Pi::game->GetSpace()->KillBody(dynamic_cast<Body*>(b));
		m_equipment.Add(item);
		UpdateEquipStats();
		if (this->IsType(Object::PLAYER))
			Pi::Message(stringf(Lang::CARGO_SCOOP_ACTIVE_1_TONNE_X_COLLECTED, formatarg("item", Equip::types[item].name)));
		// XXX Sfx::Add(this, Sfx::TYPE_SCOOP);
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
		LuaEvent::Queue("onShipCollided", this,
			b->IsType(Object::CITYONPLANET) ? dynamic_cast<CityOnPlanet*>(b)->GetPlanet() : dynamic_cast<Body*>(b));
	}

	return DynamicBody::OnCollision(b, flags, relVel);
}

//destroy ship in an explosion
void Ship::Explode()
{
	if (m_invulnerable) return;

	Pi::game->GetSpace()->KillBody(this);
	Sfx::Add(this, Sfx::TYPE_EXPLOSION);
	Sound::BodyMakeNoise(this, "Explosion_1", 1.0f);
	ClearThrusterState();
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
	vector3d maxThrust;
	maxThrust.x = (dir.x > 0) ? m_type->linThrust[ShipType::THRUSTER_RIGHT]
		: -m_type->linThrust[ShipType::THRUSTER_LEFT];
	maxThrust.y = (dir.y > 0) ? m_type->linThrust[ShipType::THRUSTER_UP]
		: -m_type->linThrust[ShipType::THRUSTER_DOWN];
	maxThrust.z = (dir.z > 0) ? m_type->linThrust[ShipType::THRUSTER_REVERSE]
		: -m_type->linThrust[ShipType::THRUSTER_FORWARD];
	return maxThrust;
}

double Ship::GetAccelMin() const
{
	float val = m_type->linThrust[ShipType::THRUSTER_UP];
	val = std::min(val, m_type->linThrust[ShipType::THRUSTER_RIGHT]);
	val = std::min(val, -m_type->linThrust[ShipType::THRUSTER_LEFT]);
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

void Ship::UpdateEquipStats()
{
	PropertyMap &p = Properties();

	m_stats.used_capacity = 0;
	m_stats.used_cargo = 0;

	for (int i=0; i<Equip::SLOT_MAX; i++) {
		for (int j=0; j<m_type->equipSlotCapacity[i]; j++) {
			Equip::Type t = m_equipment.Get(Equip::Slot(i), j);
			if (t) m_stats.used_capacity += Equip::types[t].mass;
			if (Equip::Slot(i) == Equip::SLOT_CARGO) m_stats.used_cargo += Equip::types[t].mass;
		}
	}
	m_stats.free_capacity = m_type->capacity - m_stats.used_capacity;
	m_stats.total_mass = m_stats.used_capacity + m_type->hullMass;

	p.Set("usedCapacity", m_stats.used_capacity);

	p.Set("usedCargo", m_stats.used_cargo);
	p.Set("totalCargo", m_stats.free_capacity+m_stats.used_cargo);

	const int usedCabins = m_equipment.Count(Equip::SLOT_CABIN, Equip::PASSENGER_CABIN);
	const int unusedCabins = m_equipment.Count(Equip::SLOT_CABIN, Equip::UNOCCUPIED_CABIN);
	p.Set("usedCabins", usedCabins);
	p.Set("totalCabins", usedCabins+unusedCabins);

	p.Set("freeCapacity", m_stats.free_capacity);
	p.Set("totalMass", m_stats.total_mass);

	m_stats.shield_mass = TONS_HULL_PER_SHIELD * float(m_equipment.Count(Equip::SLOT_SHIELD, Equip::SHIELD_GENERATOR));
	p.Set("shieldMass", m_stats.shield_mass);

	UpdateFuelStats();

	Equip::Type fuelType = GetHyperdriveFuelType();

	m_stats.hyperspace_range = m_stats.hyperspace_range_max = 0;
	if (m_type->equipSlotCapacity[Equip::SLOT_ENGINE]) {
		Equip::Type t = m_equipment.Get(Equip::SLOT_ENGINE);
		int hyperclass = Equip::types[t].pval;
		if (hyperclass) {
			m_stats.hyperspace_range_max = Pi::CalcHyperspaceRangeMax(hyperclass, GetMass()/1000);
			m_stats.hyperspace_range = Pi::CalcHyperspaceRange(hyperclass, GetMass()/1000, m_equipment.Count(Equip::SLOT_CARGO, fuelType));
		}
	}

	p.Set("hyperspaceRange", m_stats.hyperspace_range);
	p.Set("maxHyperspaceRange", m_stats.hyperspace_range_max);
}

void Ship::UpdateFuelStats()
{
	m_stats.fuel_tank_mass_left = m_type->fuelTankMass * GetFuel();
	Properties().Set("fuelMassLeft", m_stats.fuel_tank_mass_left);

	UpdateMass();
}

void Ship::UpdateStats()
{
	UpdateEquipStats();
}

static float distance_to_system(const SystemPath &src, const SystemPath &dest)
{
	assert(src.HasValidSystem());
	assert(dest.HasValidSystem());

	RefCountedPtr<const Sector> sec1 = Sector::cache.GetCached(src);
	RefCountedPtr<const Sector> sec2 = Sector::cache.GetCached(dest);

	return Sector::DistanceBetween(sec1, src.systemIndex, sec2, dest.systemIndex);
}

Ship::HyperjumpStatus Ship::GetHyperspaceDetails(const SystemPath &src, const SystemPath &dest, int &outFuelRequired, double &outDurationSecs)
{
	assert(dest.HasValidSystem());

	outFuelRequired = 0;
	outDurationSecs = 0.0;

	if (src.IsSameSystem(dest))
		return HYPERJUMP_CURRENT_SYSTEM;

	UpdateStats();

	if (!m_hyperspace.ignoreFuel) {
		Equip::Type t = m_equipment.Get(Equip::SLOT_ENGINE);
		Equip::Type fuelType = GetHyperdriveFuelType();
		int hyperclass = Equip::types[t].pval;
		int fuel = m_equipment.Count(Equip::SLOT_CARGO, fuelType);
		if (hyperclass == 0)
			return HYPERJUMP_NO_DRIVE;

		float dist = distance_to_system(src, dest);

		outFuelRequired = Pi::CalcHyperspaceFuelOut(hyperclass, dist, m_stats.hyperspace_range_max);
		double m_totalmass = GetMass()/1000;
		if (dist > m_stats.hyperspace_range_max) {
			outFuelRequired = 0;
			return HYPERJUMP_OUT_OF_RANGE;
		} else if (fuel < outFuelRequired) {
			return HYPERJUMP_INSUFFICIENT_FUEL;
		} else {
			outDurationSecs = Pi::CalcHyperspaceDuration(hyperclass, m_totalmass, dist);

			if (outFuelRequired <= fuel) {
				return GetFlightState() == JUMPING ? HYPERJUMP_INITIATED : HYPERJUMP_OK;
			} else {
				return HYPERJUMP_INSUFFICIENT_FUEL;
			}
		}
	}
	return GetFlightState() == JUMPING ? HYPERJUMP_INITIATED : HYPERJUMP_OK;
}

Ship::HyperjumpStatus Ship::GetHyperspaceDetails(const SystemPath &dest, int &outFuelRequired, double &outDurationSecs)
{
	if (GetFlightState() == HYPERSPACE) {
		outFuelRequired = 0;
		outDurationSecs = 0.0;
		return HYPERJUMP_DRIVE_ACTIVE;
	}
	return GetHyperspaceDetails(Pi::game->GetSpace()->GetStarSystem()->GetPath(), dest, outFuelRequired, outDurationSecs);
}

Ship::HyperjumpStatus Ship::CheckHyperjumpCapability() const {
	if (GetFlightState() == HYPERSPACE)
		return HYPERJUMP_DRIVE_ACTIVE;

	if (GetFlightState() != FLYING && GetFlightState() != JUMPING)
		return HYPERJUMP_SAFETY_LOCKOUT;

	return HYPERJUMP_OK;
}

Ship::HyperjumpStatus Ship::CheckHyperspaceTo(const SystemPath &dest, int &outFuelRequired, double &outDurationSecs)
{
	assert(dest.HasValidSystem());

	outFuelRequired = 0;
	outDurationSecs = 0.0;

	if (GetFlightState() != FLYING && GetFlightState() != JUMPING)
		return HYPERJUMP_SAFETY_LOCKOUT;

	return GetHyperspaceDetails(dest, outFuelRequired, outDurationSecs);
}

Ship::HyperjumpStatus Ship::InitiateHyperjumpTo(const SystemPath &dest, int warmup_time, double duration, LuaRef checks) {
	if (!dest.HasValidSystem() || GetFlightState() != FLYING || warmup_time < 1)
		return HYPERJUMP_SAFETY_LOCKOUT;
	StarSystem *s = Pi::game->GetSpace()->GetStarSystem().Get();
	if (s && s->GetPath().IsSameSystem(dest))
		return HYPERJUMP_CURRENT_SYSTEM;

	m_hyperspace.dest = dest;
	m_hyperspace.countdown = warmup_time;
	m_hyperspace.now = false;
	m_hyperspace.ignoreFuel = true;
	m_hyperspace.duration = duration;
	m_hyperspace.checks = checks;

	return Ship::HYPERJUMP_OK;
}

void Ship::AbortHyperjump() {
	m_hyperspace.countdown = 0;
	m_hyperspace.now = false;
	m_hyperspace.ignoreFuel = false;
	m_hyperspace.duration = 0;
	m_hyperspace.checks = LuaRef();
}

Ship::HyperjumpStatus Ship::StartHyperspaceCountdown(const SystemPath &dest)
{
	HyperjumpStatus status = CheckHyperspaceTo(dest);
	if (status != HYPERJUMP_OK)
		return status;

	m_hyperspace.dest = dest;

	Equip::Type t = m_equipment.Get(Equip::SLOT_ENGINE);
	m_hyperspace.countdown = 1.0f + Equip::types[t].pval;
	m_hyperspace.now = false;
	m_hyperspace.ignoreFuel = false;

	return Ship::HYPERJUMP_OK;
}

void Ship::ResetHyperspaceCountdown()
{
	m_hyperspace.countdown = 0;
	m_hyperspace.now = false;
	m_hyperspace.ignoreFuel = false;
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

		Space::BodyNearList nearby;
		Pi::game->GetSpace()->GetBodiesMaybeNear(this, ECM_RADIUS, nearby);
		for (Space::BodyNearIterator i = nearby.begin(); i != nearby.end(); ++i) {
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

Missile * Ship::SpawnMissile(ShipType::Id missile_type, int power) {
	if (GetFlightState() != FLYING)
		return 0;

	Missile *missile = new Missile(missile_type, this, power);
	missile->SetOrient(GetOrient());
	missile->SetFrame(GetFrame());
	const vector3d pos = GetOrient() * vector3d(0, GetAabb().min.y - 10, GetAabb().min.z);
	const vector3d vel = -40.0 * GetOrient().VectorZ();
	missile->SetPosition(GetPosition()+pos);
	missile->SetVelocity(GetVelocity()+vel);
	Pi::game->GetSpace()->AddBody(missile);
	return missile;
}

void Ship::SetFlightState(Ship::FlightState newState)
{
	if (m_flightState == newState) return;
	if (IsHyperspaceActive() && (newState != FLYING))
		ResetHyperspaceCountdown();

	if (newState == FLYING) {
		m_testLanded = false;
		if (m_flightState == DOCKING || m_flightState == DOCKED) onUndock.emit();
		m_dockedWith = 0;
		// lock thrusters for two seconds to push us out of station
		m_launchLockTimeout = 2.0;
	}

	m_flightState = newState;
	Properties().Set("flightState", EnumStrings::GetString("ShipFlightState", m_flightState));

	switch (m_flightState)
	{
		case FLYING:		SetMoving(true);	SetColliding(true);		SetStatic(false);	break;
		case DOCKING:		SetMoving(false);	SetColliding(false);	SetStatic(false);	break;
// TODO: set collision index? dynamic stations... use landed for open-air?
		case DOCKED:		SetMoving(false);	SetColliding(false);	SetStatic(false);	break;
		case LANDED:		SetMoving(false);	SetColliding(true);		SetStatic(true);	break;
		case JUMPING:		SetMoving(true);	SetColliding(false);	SetStatic(false);	break;
		case HYPERSPACE:	SetMoving(false);	SetColliding(false);	SetStatic(false);	break;
	}
}

void Ship::Blastoff()
{
	if (m_flightState != LANDED) return;

	vector3d up = GetPosition().Normalized();
	assert(GetFrame()->GetBody()->IsType(Object::PLANET));
	const double planetRadius = 2.0 + static_cast<Planet*>(GetFrame()->GetBody())->GetTerrainHeight(up);
	SetVelocity(vector3d(0, 0, 0));
	SetAngVelocity(vector3d(0, 0, 0));
	SetFlightState(FLYING);

	SetPosition(up*planetRadius - GetAabb().min.y*up);
	SetThrusterState(1, 1.0);		// thrust upwards

	LuaEvent::Queue("onShipTakeOff", this, GetFrame()->GetBody());
}

void Ship::TestLanded()
{
	m_testLanded = false;
	if (m_launchLockTimeout > 0.0f) return;
	if (m_wheelState < 1.0f) return;
	if (GetFrame()->GetBody()->IsType(Object::PLANET)) {
		double speed = GetVelocity().Length();
		vector3d up = GetPosition().Normalized();
		const double planetRadius = static_cast<Planet*>(GetFrame()->GetBody())->GetTerrainHeight(up);

		if (speed < MAX_LANDING_SPEED) {
			// check player is sortof sensibly oriented for landing
			if (GetOrient().VectorY().Dot(up) > 0.99) {
				// position at zero altitude
				SetPosition(up * (planetRadius - GetAabb().min.y));

				// position facing in roughly the same direction
				vector3d right = up.Cross(GetOrient().VectorZ()).Normalized();
				SetOrient(matrix3x3d::FromVectors(right, up));

				SetVelocity(vector3d(0, 0, 0));
				SetAngVelocity(vector3d(0, 0, 0));
				ClearThrusterState();
				SetFlightState(LANDED);
				Sound::BodyMakeNoise(this, "Rough_Landing", 1.0f);
				LuaEvent::Queue("onShipLanded", this, GetFrame()->GetBody());
			}
		}
	}
}

void Ship::SetLandedOn(Planet *p, float latitude, float longitude)
{
	m_wheelTransition = 0;
	m_wheelState = 1.0f;
	Frame* f = p->GetFrame()->GetRotFrame();
	SetFrame(f);
	vector3d up = vector3d(cos(latitude)*sin(longitude), sin(latitude), cos(latitude)*cos(longitude));
	const double planetRadius = p->GetTerrainHeight(up);
	SetPosition(up * (planetRadius - GetAabb().min.y));
	vector3d right = up.Cross(vector3d(0,0,1)).Normalized();
	SetOrient(matrix3x3d::FromVectors(right, up));
	SetVelocity(vector3d(0, 0, 0));
	SetAngVelocity(vector3d(0, 0, 0));
	ClearThrusterState();
	SetFlightState(LANDED);
	LuaEvent::Queue("onShipLanded", this, p);
}

void Ship::SetFrame(Frame *f)
{
	DynamicBody::SetFrame(f);
	m_sensors->ResetTrails();
}

void Ship::TimeStepUpdate(const float timeStep)
{
	// If docked, station is responsible for updating position/orient of ship
	// but we call this crap anyway and hope it doesn't do anything bad

	vector3d maxThrust = GetMaxThrust(m_thrusters);
	vector3d thrust = vector3d(maxThrust.x*m_thrusters.x, maxThrust.y*m_thrusters.y,
		maxThrust.z*m_thrusters.z);
	AddRelForce(thrust);
	AddRelTorque(GetShipType()->angThrust * m_angThrusters);

	if (m_landingGearAnimation)
		m_landingGearAnimation->SetProgress(m_wheelState);

	DynamicBody::TimeStepUpdate(timeStep);

	// fuel use decreases mass, so do this as the last thing in the frame
	UpdateFuel(timeStep, thrust);

	m_navLights->SetEnabled(m_wheelState > 0.01f);
	m_navLights->Update(timeStep);
	if (m_sensors.get()) m_sensors->Update(timeStep);
}

void Ship::DoThrusterSounds() const
{
	// XXX any ship being the current camera body should emit sounds
	// also, ship sounds could be split to internal and external sounds

	// XXX sound logic could be part of a bigger class (ship internal sounds)
	/* Ship engine noise. less loud inside */
	float v_env = (Pi::worldView->GetCameraController()->IsExternal() ? 1.0f : 0.5f) * Sound::GetSfxVolume();
	static Sound::Event sndev;
	float volBoth = 0.0f;
	volBoth += 0.5f*fabs(GetThrusterState().y);
	volBoth += 0.5f*fabs(GetThrusterState().z);

	float targetVol[2] = { volBoth, volBoth };
	if (GetThrusterState().x > 0.0)
		targetVol[0] += 0.5f*float(GetThrusterState().x);
	else targetVol[1] += -0.5f*float(GetThrusterState().x);

	targetVol[0] = v_env * Clamp(targetVol[0], 0.0f, 1.0f);
	targetVol[1] = v_env * Clamp(targetVol[1], 0.0f, 1.0f);
	float dv_dt[2] = { 4.0f, 4.0f };
	if (!sndev.VolumeAnimate(targetVol, dv_dt)) {
		sndev.Play("Thruster_large", 0.0f, 0.0f, Sound::OP_REPEAT);
		sndev.VolumeAnimate(targetVol, dv_dt);
	}
	float angthrust = 0.1f * v_env * float(GetAngThrusterState().Length());

	static Sound::Event angThrustSnd;
	if (!angThrustSnd.VolumeAnimate(angthrust, angthrust, 5.0f, 5.0f)) {
		angThrustSnd.Play("Thruster_Small", 0.0f, 0.0f, Sound::OP_REPEAT);
		angThrustSnd.VolumeAnimate(angthrust, angthrust, 5.0f, 5.0f);
	}
}

// for timestep changes, to stop autopilot overshoot
// either adds half of current accel if decelerating
void Ship::TimeAccelAdjust(const float timeStep)
{
	if (!AIIsActive()) return;
#ifdef DEBUG_AUTOPILOT
	if (this->IsType(Object::PLAYER))
		Output("Time accel adjustment, step = %.1f, decel = %s\n", double(timeStep),
			m_decelerating ? "true" : "false");
#endif
	vector3d vdiff = double(timeStep) * GetLastForce() * (1.0 / GetMass());
	if (!m_decelerating) vdiff = -2.0 * vdiff;
	SetVelocity(GetVelocity() + vdiff);
}

void Ship::FireWeapon(int num)
{
	if (m_flightState != FLYING) return;

	const matrix3x3d &m = GetOrient();
	const vector3d dir = m * vector3d(m_gun[num].dir);
	const vector3d pos = m * vector3d(m_gun[num].pos) + GetPosition();

	m_gun[num].temperature += 0.01f;

	Equip::Type t = m_equipment.Get(Equip::SLOT_LASER, num);
	const LaserType &lt = Equip::lasers[Equip::types[t].tableIndex];
	m_gun[num].recharge = lt.rechargeTime;
	vector3d baseVel = GetVelocity();
	vector3d dirVel = lt.speed * dir.Normalized();

	if (lt.flags & Equip::LASER_DUAL)
	{
		const ShipType::DualLaserOrientation orient = m_type->gunMount[num].orient;
		const vector3d orient_norm =
				(orient == ShipType::DUAL_LASERS_VERTICAL) ? m.VectorX() : m.VectorY();
		const vector3d sep = m_type->gunMount[num].sep * dir.Cross(orient_norm).NormalizedSafe();

		Projectile::Add(this, t, pos + sep, baseVel, dirVel);
		Projectile::Add(this, t, pos - sep, baseVel, dirVel);
	}
	else
		Projectile::Add(this, t, pos, baseVel, dirVel);

	Polit::NotifyOfCrime(this, Polit::CRIME_WEAPON_DISCHARGE);
	Sound::BodyMakeNoise(this, "Pulse_Laser", 1.0f);
}

double Ship::GetHullTemperature() const
{
	double dragGs = GetAtmosForce().Length() / (GetMass() * 9.81);
	if (m_equipment.Get(Equip::SLOT_ATMOSHIELD) == Equip::NONE) {
		return dragGs / 5.0;
	} else {
		return dragGs / 300.0;
	}
}

void Ship::SetAlertState(AlertState as)
{
	m_alertState = as;
	Properties().Set("alertStatus", EnumStrings::GetString("ShipAlertStatus", as));
}

void Ship::UpdateAlertState()
{
	// no alerts if no scanner
	if (m_equipment.Get(Equip::SLOT_SCANNER) == Equip::NONE) {
		// clear existing alert state if there was one
		if (GetAlertState() != ALERT_NONE) {
			SetAlertState(ALERT_NONE);
			LuaEvent::Queue("onShipAlertChanged", this, EnumStrings::GetString("ShipAlertStatus", ALERT_NONE));
		}
		return;
	}

	static const double ALERT_DISTANCE = 100000.0; // 100km

	Space::BodyNearList nearby;
	Pi::game->GetSpace()->GetBodiesMaybeNear(this, ALERT_DISTANCE, nearby);

	bool ship_is_near = false, ship_is_firing = false;
	for (Space::BodyNearIterator i = nearby.begin(); i != nearby.end(); ++i)
	{
		if ((*i) == this) continue;
		if (!(*i)->IsType(Object::SHIP) || (*i)->IsType(Object::MISSILE)) continue;

		const Ship *ship = static_cast<const Ship*>(*i);

		if (ship->GetShipType()->tag == ShipType::TAG_STATIC_SHIP) continue;
		if (ship->GetFlightState() == LANDED || ship->GetFlightState() == DOCKED) continue;

		if (GetPositionRelTo(ship).LengthSqr() < ALERT_DISTANCE*ALERT_DISTANCE) {
			ship_is_near = true;

			Uint32 gunstate = 0;
			for (int j = 0; j < ShipType::GUNMOUNT_MAX; j++)
				gunstate |= ship->m_gun[j].state;

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
		LuaEvent::Queue("onShipAlertChanged", this, EnumStrings::GetString("ShipAlertStatus", GetAlertState()));
}

void Ship::UpdateFuel(const float timeStep, const vector3d &thrust)
{
	const double fuelUseRate = GetShipType()->GetFuelUseRate() * 0.01;
	double totalThrust = (fabs(thrust.x) + fabs(thrust.y) + fabs(thrust.z))
		/ -GetShipType()->linThrust[ShipType::THRUSTER_FORWARD];

	FuelState lastState = GetFuelState();
	SetFuel(GetFuel() - timeStep * (totalThrust * fuelUseRate));
	FuelState currentState = GetFuelState();

	UpdateFuelStats();

	if (currentState != lastState)
		LuaEvent::Queue("onShipFuelChanged", this, EnumStrings::GetString("ShipFuelStatus", currentState));
}

void Ship::StaticUpdate(const float timeStep)
{
	// do player sounds before dead check, so they also turn off
	if (IsType(Object::PLAYER)) DoThrusterSounds();

	if (IsDead()) return;

	if (m_controller) m_controller->StaticUpdate(timeStep);

	if (GetHullTemperature() > 1.0)
		Explode();

	UpdateAlertState();

	/* FUEL SCOOPING!!!!!!!!! */
	if ((m_flightState == FLYING) && (m_equipment.Get(Equip::SLOT_FUELSCOOP) != Equip::NONE)) {
		Body *astro = GetFrame()->GetBody();
		if (astro && astro->IsType(Object::PLANET)) {
			Planet *p = static_cast<Planet*>(astro);
			if (p->GetSystemBody()->IsScoopable()) {
				double dist = GetPosition().Length();
				double pressure, density;
				p->GetAtmosphericState(dist, &pressure, &density);

				double speed = GetVelocity().Length();
				vector3d vdir = GetVelocity().Normalized();
				vector3d pdir = -GetOrient().VectorZ();
				double dot = vdir.Dot(pdir);
				if ((m_stats.free_capacity) && (dot > 0.95) && (speed > 2000.0) && (density > 1.0)) {
					double rate = speed*density*0.00001f;
					if (Pi::rng.Double() < rate) {
						m_equipment.Add(Equip::HYDROGEN);
						UpdateEquipStats();
						if (this->IsType(Object::PLAYER)) {
							Pi::Message(stringf(Lang::FUEL_SCOOP_ACTIVE_N_TONNES_H_COLLECTED,
									formatarg("quantity", m_equipment.Count(Equip::SLOT_CARGO, Equip::HYDROGEN))));
						}
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
	if (m_flightState == JUMPING || m_flightState == HYPERSPACE)
		m_launchLockTimeout = 0;

	// lasers
	for (int i=0; i<ShipType::GUNMOUNT_MAX; i++) {
		m_gun[i].recharge -= timeStep;
		float rateCooling = 0.01f;
		if (m_equipment.Get(Equip::SLOT_LASERCOOLER) != Equip::NONE)  {
			rateCooling *= float(Equip::types[ m_equipment.Get(Equip::SLOT_LASERCOOLER) ].pval);
		}
		m_gun[i].temperature -= rateCooling*timeStep;
		if (m_gun[i].temperature < 0.0f) m_gun[i].temperature = 0;
		if (m_gun[i].recharge < 0.0f) m_gun[i].recharge = 0;

		if (!m_gun[i].state) continue;
		if (m_gun[i].recharge > 0.0f) continue;
		if (m_gun[i].temperature > 1.0) continue;

		FireWeapon(i);
	}

	if (m_ecmRecharge > 0.0f) {
		m_ecmRecharge = std::max(0.0f, m_ecmRecharge - timeStep);
	}

	if (m_shieldCooldown > 0.0f) {
		m_shieldCooldown = std::max(0.0f, m_shieldCooldown - timeStep);
	}

	if (m_stats.shield_mass_left < m_stats.shield_mass) {
		// 250 second recharge
		float recharge_rate = 0.004f;
		if (m_equipment.Get(Equip::SLOT_ENERGYBOOSTER) != Equip::NONE) {
			recharge_rate *= float(Equip::types[ m_equipment.Get(Equip::SLOT_ENERGYBOOSTER) ].pval);
		}
		m_stats.shield_mass_left = Clamp(m_stats.shield_mass_left + m_stats.shield_mass * recharge_rate * timeStep, 0.0f, m_stats.shield_mass);
		Properties().Set("shieldMassLeft", m_stats.shield_mass_left);
	}

	if (m_wheelTransition) {
		m_wheelState += m_wheelTransition*0.3f*timeStep;
		m_wheelState = Clamp(m_wheelState, 0.0f, 1.0f);
		if (is_equal_exact(m_wheelState, 0.0f) || is_equal_exact(m_wheelState, 1.0f))
			m_wheelTransition = 0;
	}

	if (m_testLanded) TestLanded();

	if (m_equipment.Get(Equip::SLOT_HULLAUTOREPAIR) == Equip::HULL_AUTOREPAIR) {
		m_stats.hull_mass_left = std::min(m_stats.hull_mass_left + 0.1f*timeStep, float(m_type->hullMass));
		Properties().Set("hullMassLeft", m_stats.hull_mass_left);
		Properties().Set("hullPercent", 100.0f * (m_stats.hull_mass_left / float(m_type->hullMass)));
	}

	// After calling StartHyperspaceTo this Ship must not spawn objects
	// holding references to it (eg missiles), as StartHyperspaceTo
	// removes the ship from Space::bodies and so the missile will not
	// have references to this cleared by NotifyRemoved()
	if (m_hyperspace.now) {
		m_hyperspace.now = false;
		EnterHyperspace();
	}

	if (m_hyperspace.countdown > 0.0f) {
		// Check the Lua function
		bool abort = false;
		lua_State * l = m_hyperspace.checks.GetLua();
		if (l) {
			m_hyperspace.checks.PushCopyToStack();
			if (lua_isfunction(l, -1)) {
				lua_call(l, 0, 1);
				abort = !lua_toboolean(l, -1);
				lua_pop(l, 1);
			}
		}
		if (abort) {
			AbortHyperjump();
		} else {
			m_hyperspace.countdown = m_hyperspace.countdown - timeStep;
			if (!abort && m_hyperspace.countdown <= 0.0f) {
				m_hyperspace.countdown = 0;
				m_hyperspace.now = true;
				SetFlightState(JUMPING);
			}
		}
	}

	//Add smoke trails for missiles on thruster state
	if (m_type->tag == ShipType::TAG_MISSILE && m_thrusters.z < 0.0 && 0.1*Pi::rng.Double() < timeStep) {
		vector3d pos = GetOrient() * vector3d(0, 0 , 5);
		Sfx::AddThrustSmoke(this, Sfx::TYPE_SMOKE, std::min(10.0*GetVelocity().Length()*abs(m_thrusters.z),100.0),pos);
	}
}

void Ship::NotifyRemoved(const Body* const removedBody)
{
	if (m_curAICmd) m_curAICmd->OnDeleted(removedBody);
}

bool Ship::Undock()
{
	return (m_dockedWith && m_dockedWith->LaunchShip(this, m_dockedWithPort));
}

void Ship::SetDockedWith(SpaceStation *s, int port)
{
	if (s) {
		m_dockedWith = s;
		m_dockedWithPort = port;
		m_wheelTransition = 0;
		m_wheelState = 1.0f;
		// hand position/state responsibility over to station
		m_dockedWith->SetDocked(this, port);
		onDock.emit();
	} else {
		Undock();
	}
}

void Ship::SetGunState(int idx, int state)
{
	if (m_equipment.Get(Equip::SLOT_LASER, idx) != Equip::NONE) {
		m_gun[idx].state = state;
	}
}

bool Ship::SetWheelState(bool down)
{
	if (m_flightState != FLYING) return false;
	if (is_equal_exact(m_wheelState, down ? 1.0f : 0.0f)) return false;
	int newWheelTransition = (down ? 1 : -1);
	if (newWheelTransition == m_wheelTransition) return false;
	m_wheelTransition = newWheelTransition;
	return true;
}

void Ship::Render(Graphics::Renderer *renderer, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	if (IsDead()) return;

	//angthrust negated, for some reason
	GetModel()->SetThrust(vector3f(m_thrusters), -vector3f(m_angThrusters));

	matrix3x3f mt;
	matrix3x3dtof(viewTransform.InverseOf().GetOrient(), mt);
	s_heatGradientParams.heatingMatrix = mt;
	s_heatGradientParams.heatingNormal = vector3f(GetVelocity().Normalized());
	s_heatGradientParams.heatingAmount = Clamp(GetHullTemperature(),0.0,1.0);

	// This has to be done per-model with a shield and just before it's rendered
	const bool shieldsVisible = m_shieldCooldown > 0.01f && m_stats.shield_mass_left > (m_stats.shield_mass / 100.0f);
	GetShields()->SetEnabled(shieldsVisible);
	GetShields()->Update(m_shieldCooldown, 0.01f*GetPercentShields());

	//strncpy(params.pText[0], GetLabel().c_str(), sizeof(params.pText));
	RenderModel(renderer, camera, viewCoords, viewTransform);

	if (m_ecmRecharge > 0.0f) {
		// ECM effect: a cloud of particles for a sparkly effect
		vector3f v[100];
		for (int i=0; i<100; i++) {
			const double r1 = Pi::rng.Double()-0.5;
			const double r2 = Pi::rng.Double()-0.5;
			const double r3 = Pi::rng.Double()-0.5;
			v[i] = vector3f(viewTransform * (
				GetPosition() + GetPhysRadius() *
				vector3d(r1, r2, r3).Normalized()
			));
		}
		Color c(128,128,255,255);
		float totalRechargeTime = GetECMRechargeTime();
		if (totalRechargeTime >= 0.0f) {
			c.a = (m_ecmRecharge / totalRechargeTime) * 255;
		}

		Sfx::ecmParticle->diffuse = c;
		renderer->DrawPointSprites(100, v, Sfx::additiveAlphaState, Sfx::ecmParticle, 50.f);
	}
}

bool Ship::SpawnCargo(CargoBody * c_body) const
{
	if (m_flightState != FLYING) return false;
	vector3d pos = GetOrient() * vector3d(0, GetAabb().min.y - 5, 0);
	c_body->SetFrame(GetFrame());
	c_body->SetPosition(GetPosition() + pos);
	c_body->SetVelocity(GetVelocity() + GetOrient()*vector3d(0, -10, 0));
	Pi::game->GetSpace()->AddBody(c_body);
	return true;
}

void Ship::OnEquipmentChange(Equip::Type e)
{
	LuaEvent::Queue("onShipEquipmentChanged", this, EnumStrings::GetString("EquipType", e));
}

void Ship::EnterHyperspace() {
	assert(GetFlightState() != Ship::HYPERSPACE);

	// Two code paths:
	// The first one is the traditional "check fuel and destination and whatnut
	// The second one only checks the bare minimum to insure consistency.
	if (!m_hyperspace.ignoreFuel) {

		const SystemPath dest = GetHyperspaceDest();
		int fuel_cost;

		Ship::HyperjumpStatus status = CheckHyperspaceTo(dest, fuel_cost, m_hyperspace.duration);
		if (status != HYPERJUMP_OK && status != HYPERJUMP_INITIATED) {
			// XXX something has changed (fuel loss, mass change, whatever).
			// could report it to the player but better would be to cancel the
			// countdown before this is reached. either way do something
			if (m_flightState == JUMPING)
				SetFlightState(FLYING);
			return;
		}

		Equip::Type fuelType = GetHyperdriveFuelType();
		m_equipment.Remove(fuelType, fuel_cost);
		if (fuelType == Equip::MILITARY_FUEL) {
			m_equipment.Add(Equip::RADIOACTIVES, fuel_cost);
		}
		UpdateEquipStats();
	} else {
		Ship::HyperjumpStatus status = CheckHyperjumpCapability();
		if (status != HYPERJUMP_OK && status != HYPERJUMP_INITIATED) {
			if (m_flightState == JUMPING)
				SetFlightState(FLYING);
			return;
		}
	}
	m_hyperspace.ignoreFuel = false;

	LuaEvent::Queue("onLeaveSystem", this);

	SetFlightState(Ship::HYPERSPACE);

	// virtual call, do class-specific things
	OnEnterHyperspace();
}

void Ship::OnEnterHyperspace() {
	Sound::BodyMakeNoise(this, "Hyperdrive_Jump", 1.f);
	m_hyperspaceCloud = new HyperspaceCloud(this, Pi::game->GetTime() + m_hyperspace.duration, false);
	m_hyperspaceCloud->SetFrame(GetFrame());
	m_hyperspaceCloud->SetPosition(GetPosition());

	Space *space = Pi::game->GetSpace();

	space->RemoveBody(this);
	space->AddBody(m_hyperspaceCloud);
}

void Ship::EnterSystem() {
	PROFILE_SCOPED()
	assert(GetFlightState() == Ship::HYPERSPACE);

	// virtual call, do class-specific things
	OnEnterSystem();

	SetFlightState(Ship::FLYING);

	LuaEvent::Queue("onEnterSystem", this);
}

void Ship::OnEnterSystem() {
	m_hyperspaceCloud = 0;
}

void Ship::SetShipId(const ShipType::Id &shipId)
{
	m_type = &ShipType::types[shipId];
	Properties().Set("shipId", shipId);
}

void Ship::SetShipType(const ShipType::Id &shipId)
{
	SetShipId(shipId);
	m_equipment.InitSlotSizes(shipId);
	SetModel(m_type->modelName.c_str());
	m_skin.SetDecal(m_type->manufacturer);
	m_skin.Apply(GetModel());
	Init();
	onFlavourChanged.emit();
	if (IsType(Object::PLAYER))
		Pi::worldView->SetCamType(Pi::worldView->GetCamType());
	LuaEvent::Queue("onShipTypeChanged", this);
}

void Ship::SetLabel(const std::string &label)
{
	DynamicBody::SetLabel(label);
	m_skin.SetLabel(label);
	m_skin.Apply(GetModel());
}

void Ship::SetSkin(const SceneGraph::ModelSkin &skin)
{
	m_skin = skin;
	m_skin.Apply(GetModel());
}

Uint8 Ship::GetRelations(Body *other) const
{
	auto it = m_relationsMap.find(other);
	if (it != m_relationsMap.end())
		return it->second;

	return 50;
}

void Ship::SetRelations(Body *other, Uint8 percent)
{
	m_relationsMap[other] = percent;
	if (m_sensors.get()) m_sensors->UpdateIFF(other);
}
