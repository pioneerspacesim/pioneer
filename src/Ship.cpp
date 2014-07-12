// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Ship.h"
#include "CityOnPlanet.h"
#include "Lang.h"
#include "EnumStrings.h"
#include "LuaEvent.h"
#include "LuaUtils.h"
#include "Missile.h"
#include "Player.h"
#include "Projectile.h"
#include "ShipAICmd.h"
#include "ShipController.h"
#include "Sound.h"
#include "Sfx.h"
#include "galaxy/Galaxy.h"
#include "galaxy/Sector.h"
#include "galaxy/GalaxyCache.h"
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

#include <algorithm>

static const float TONS_HULL_PER_SHIELD = 10.f;
static const double KINETIC_ENERGY_MULT	= 0.01;
HeatGradientParameters_t Ship::s_heatGradientParams;
const float Ship::DEFAULT_SHIELD_COOLDOWN_TIME = 1.0f;

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
	m_hyperspace.duration = 0;

	for (int i=0; i<ShipType::GUNMOUNT_MAX; i++) {
		m_gun[i].state = rd.Int32();
		m_gun[i].recharge = rd.Float();
		m_gun[i].temperature = rd.Float();
	}
	m_ecmRecharge = rd.Float();
	SetShipId(rd.String()); // XXX handle missing thirdparty ship
	m_dockedWithPort = rd.Int32();
	m_dockedWithIndex = rd.Int32();
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
	p.Set("mass_cap", 0);
	p.PushLuaTable();
	lua_State *l = Lua::manager->GetLuaState();
	lua_getfield(l, -1, "equipSet");
	m_equipSet = LuaRef(l, -1);
	lua_pop(l, 2);

	m_controller = 0;
	const ShipController::Type ctype = static_cast<ShipController::Type>(rd.Int32());
	if (ctype == ShipController::PLAYER)
		SetController(new PlayerShipController());
	else
		SetController(new ShipController());
	m_controller->Load(rd);

	m_navLights->Load(rd);

}

void Ship::InitEquipSet() {
	lua_State * l = Lua::manager->GetLuaState();
	PropertyMap & p = Properties();
	LUA_DEBUG_START(l);
	pi_lua_import(l, "EquipSet");
	LuaTable es_class(l, -1);
	LuaTable slots = LuaTable(l).LoadMap(GetShipType()->slots.begin(), GetShipType()->slots.end());
	m_equipSet =  es_class.Call<LuaRef>("New", slots);
	p.Set("equipSet", ScopedTable(m_equipSet));
	UpdateEquipStats();
	{
		ScopedTable es(m_equipSet);
		int usedCargo = es.CallMethod<int>("OccupiedSpace", "cargo");
		int totalCargo = std::min(m_stats.free_capacity + usedCargo, es.CallMethod<int>("SlotSize", "cargo"));
		p.Set("usedCargo", usedCargo);
		p.Set("totalCargo", totalCargo);
	}
	lua_pop(l, 2);
	LUA_DEBUG_END(l, 0);
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
	p.Set("mass_cap", 0);

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

	InitEquipSet();

	m_hyperspace.countdown = 0;
	m_hyperspace.now = false;
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
	int cargoscoop_cap;
	Properties().Get("cargo_scoop_cap", cargoscoop_cap);
	if (b->IsType(Object::CARGOBODY) && !dynamic_cast<Body*>(b)->IsDead()) {
		LuaRef item = dynamic_cast<CargoBody*>(b)->GetCargoType();
		if (LuaObject<Ship>::CallMethod<int>(this, "AddEquip", item) > 0) { // try to add it to the ship cargo.
			Pi::game->GetSpace()->KillBody(dynamic_cast<Body*>(b));
			if (this->IsType(Object::PLAYER))
				Pi::Message(stringf(Lang::CARGO_SCOOP_ACTIVE_1_TONNE_X_COLLECTED, formatarg("item", ScopedTable(item).CallMethod<std::string>("GetName"))));
			// XXX Sfx::Add(this, Sfx::TYPE_SCOOP);
			UpdateEquipStats();
			return true;
		}
        if (this->IsType(Object::PLAYER))
            Pi::Message("Cargo scoop attempted !\n");
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
	if (this->GetFrame() == Pi::player->GetFrame()) {
		Sfx::AddExplosion(this, Sfx::TYPE_EXPLOSION);
		Sound::BodyMakeNoise(this, "Explosion_1", 1.0f);
	}
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

void Ship::UpdateEquipStats()
{
	PropertyMap &p = Properties();

	m_stats.used_capacity = 0;
	p.Get("mass_cap", m_stats.used_capacity);
	m_stats.used_cargo = 0;

	m_stats.free_capacity = m_type->capacity - m_stats.used_capacity;
	m_stats.total_mass = m_stats.used_capacity + m_type->hullMass;

	p.Set("usedCapacity", m_stats.used_capacity);

	p.Set("freeCapacity", m_stats.free_capacity);
	p.Set("totalMass", m_stats.total_mass);

	int shield_cap = 0;
	Properties().Get("shield_cap", shield_cap);
	m_stats.shield_mass = TONS_HULL_PER_SHIELD * float(shield_cap);
	p.Set("shieldMass", m_stats.shield_mass);

	UpdateFuelStats();

	m_stats.hyperspace_range = m_stats.hyperspace_range_max = 0;
	p.Set("hyperspaceRange", m_stats.hyperspace_range);
	p.Set("maxHyperspaceRange", m_stats.hyperspace_range_max);
}

void Ship::UpdateLuaStats() {
	// This code cannot be in UpdateEquipStats itself because *Equip* needs to be
	// called in Init(), which is itself called in the constructor, but we absolutely
	// cannot use LuaObject<Ship>::* in a constructor, or else we'd fix the type of the
	// object to Ship forever, even though it could very well be a Player.
	UpdateEquipStats();
	PropertyMap& p = Properties();
	m_stats.hyperspace_range = m_stats.hyperspace_range_max = 0;
	int hyperclass;
	p.Get<int>("hyperclass_cap", hyperclass);
	if (hyperclass) {
		auto ranges = LuaObject<Ship>::CallMethod<double, double>(this, "GetHyperspaceRange");
		m_stats.hyperspace_range_max = std::get<1>(ranges);
		m_stats.hyperspace_range = std::get<0>(ranges);
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

Ship::HyperjumpStatus Ship::CheckHyperjumpCapability() const {
	if (GetFlightState() == HYPERSPACE)
		return HYPERJUMP_DRIVE_ACTIVE;

	if (GetFlightState() != FLYING && GetFlightState() != JUMPING)
		return HYPERJUMP_SAFETY_LOCKOUT;

	return HYPERJUMP_OK;
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
	m_hyperspace.duration = duration;
	m_hyperspace.checks = checks;

	return Ship::HYPERJUMP_OK;
}

void Ship::AbortHyperjump() {
	m_hyperspace.countdown = 0;
	m_hyperspace.now = false;
	m_hyperspace.duration = 0;
	m_hyperspace.checks = LuaRef();
}

float Ship::GetECMRechargeTime()
{
	float ecm_recharge_cap = 0.f;
	Properties().Get("ecm_recharge_cap", ecm_recharge_cap);
	return ecm_recharge_cap;
}

void Ship::UseECM()
{
	int ecm_power_cap = 0;
	Properties().Get("ecm_power_cap", ecm_power_cap);
	if (m_ecmRecharge > 0.0f) return;

	if (ecm_power_cap > 0) {
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
					static_cast<Missile*>(*i)->ECMAttack(ecm_power_cap);
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
		AbortHyperjump();

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
	std::string prefix(num?"laser_rear_":"laser_front_");
	int damage = 0;
	Properties().Get(prefix+"damage", damage);
	if (!damage)
		return;
	Properties().PushLuaTable();
	LuaTable prop(Lua::manager->GetLuaState(), -1);

	const matrix3x3d &m = GetOrient();
	const vector3d dir = m * vector3d(m_gun[num].dir);
	const vector3d pos = m * vector3d(m_gun[num].pos) + GetPosition();

	m_gun[num].temperature += 0.01f;

	m_gun[num].recharge = prop.Get<float>(prefix+"rechargeTime");
	vector3d baseVel = GetVelocity();
	vector3d dirVel = prop.Get<float>(prefix+"speed") * dir.Normalized();

	Color c(prop.Get<float>(prefix+"rgba_r"), prop.Get<float>(prefix+"rgba_g"),
			prop.Get<float>(prefix+"rgba_b"), prop.Get<float>(prefix+"rgba_a"));
	float lifespan = prop.Get<float>(prefix+"lifespan");
	float width = prop.Get<float>(prefix+"width");
	float length = prop.Get<float>(prefix+"length");
	bool mining = prop.Get<int>(prefix+"mining");
	if (prop.Get<int>(prefix+"dual"))
	{
		const ShipType::DualLaserOrientation orient = m_type->gunMount[num].orient;
		const vector3d orient_norm =
				(orient == ShipType::DUAL_LASERS_VERTICAL) ? m.VectorX() : m.VectorY();
		const vector3d sep = m_type->gunMount[num].sep * dir.Cross(orient_norm).NormalizedSafe();

		Projectile::Add(this, lifespan, damage, length, width, mining, c, pos + sep, baseVel, dirVel);
		Projectile::Add(this, lifespan, damage, length, width, mining, c, pos - sep, baseVel, dirVel);
	}
	else
		Projectile::Add(this, lifespan, damage, length, width, mining, c, pos, baseVel, dirVel);

	Polit::NotifyOfCrime(this, Polit::CRIME_WEAPON_DISCHARGE);
	Sound::BodyMakeNoise(this, "Pulse_Laser", 1.0f);
	lua_pop(prop.GetLua(), 1);
	LuaEvent::Queue("onShipFiring", this);
}

double Ship::GetHullTemperature() const
{
	double dragGs = GetAtmosForce().Length() / (GetMass() * 9.81);
	int atmo_shield_cap = 0;
	const_cast<Ship *>(this)->Properties().Get("atmo_shield_cap", atmo_shield_cap);
	if (atmo_shield_cap) {
		return dragGs / 300.0;
	} else {
		return dragGs / 5.0;
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
	int scanner_cap = 0;
	Properties().Get("scanner_cap", scanner_cap);
	if (scanner_cap <= 0) {
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
	int capacity = 0;
	Properties().Get("fuel_scoop_cap", capacity);
	if (m_flightState == FLYING && capacity > 0) {
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
						lua_State *l = Lua::manager->GetLuaState();
						pi_lua_import(l, "Equipment");
						LuaTable hydrogen = LuaTable(l, -1).Sub("cargo").Sub("hydrogen");
						LuaObject<Ship>::CallMethod(this, "AddEquip", hydrogen);
						UpdateEquipStats();
						if (this->IsType(Object::PLAYER)) {
							Pi::Message(stringf(Lang::FUEL_SCOOP_ACTIVE_N_TONNES_H_COLLECTED,
									formatarg("quantity", LuaObject<Ship>::CallMethod<int>(this, "CountEquip", hydrogen))));
						}
						lua_pop(l, 3);
					}
				}
			}
		}
	}

	// Cargo bay life support
	capacity = 0;
	Properties().Get("cargo_life_support_cap", capacity);
	if (!capacity) {
		// Hull is pressure-sealed, it just doesn't provide
		// temperature regulation and breathable atmosphere

		// kill stuff roughly every 5 seconds
		if ((!m_dockedWith) && (5.0*Pi::rng.Double() < timeStep)) {
			std::string t(Pi::rng.Int32(2) ? "live_animals" : "slaves");

			lua_State *l = Lua::manager->GetLuaState();
			pi_lua_import(l, "Equipment");
			LuaTable cargo = LuaTable(l, -1).Sub("cargo");
			if (LuaObject<Ship>::CallMethod<int>(this, "RemoveEquip", cargo.Sub(t))) {
				LuaObject<Ship>::CallMethod<int>(this, "AddEquip", cargo.Sub("fertilizer"));
				if (this->IsType(Object::PLAYER)) {
					Pi::Message(Lang::CARGO_BAY_LIFE_SUPPORT_LOST);
				}
				lua_pop(l, 4);
			}
			else
				lua_pop(l, 3);
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
		float cooler = 1.0f;
		Properties().Get("laser_cooler_cap", cooler);
		rateCooling *= cooler;
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
		float booster = 1.0f;
		Properties().Get("shield_energy_booster_cap", booster);
		recharge_rate *= booster;
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

	capacity = 0;
	Properties().Get("hull_autorepair_cap", capacity);
	if (capacity) {
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
		const vector3d pos = GetOrient() * vector3d(0, 0 , 5);
		const float speed = std::min(10.0*GetVelocity().Length()*abs(m_thrusters.z),100.0);
		Sfx::AddThrustSmoke(this, Sfx::TYPE_SMOKE, speed, pos);
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
	std::string slot(idx?"laser_rear":"laser_front");
	if (ScopedTable(m_equipSet).CallMethod<int>("OccupiedSpace", slot)) {
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
		renderer->DrawPointSprites(100, v, Sfx::additiveAlphaState, Sfx::ecmParticle.get(), 50.f);
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

void Ship::EnterHyperspace() {
	assert(GetFlightState() != Ship::HYPERSPACE);

	Ship::HyperjumpStatus status = CheckHyperjumpCapability();
	if (status != HYPERJUMP_OK && status != HYPERJUMP_INITIATED) {
		if (m_flightState == JUMPING)
			SetFlightState(FLYING);
		return;
	}

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
	SetModel(m_type->modelName.c_str());
	m_skin.SetDecal(m_type->manufacturer);
	m_skin.Apply(GetModel());
	Init();
	onFlavourChanged.emit();
	if (IsType(Object::PLAYER))
		Pi::worldView->SetCamType(Pi::worldView->GetCamType());
	// We cannot export it to Init() since it gets reloaded on its own
	InitEquipSet();

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
