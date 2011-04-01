#include "libs.h"
#include "PiLuaAPI.h"
#include "Pi.h"
#include "Space.h"
#include "ShipCpanel.h"
#include "Ship.h"
#include "Player.h"
#include "SpaceStation.h"
#include "StarSystem.h"
#include "Sound.h"
#include "LuaChatForm.h"
#include "NameGenerator.h"
#include "HyperspaceCloud.h"
#include "Polit.h"
#include "LuaObject.h"

void ship_randomly_equip(Ship *ship, double power)
{
	const shipstats_t *stats;
	const ShipType &type = ship->GetShipType();

	// Start with lots of money, so the AI can buy and sell fuel and stuff
	ship->SetMoney(10000000);

	// SLOT_ENGINE
	ship->m_equipment.Set(Equip::SLOT_ENGINE, 0, type.hyperdrive);
	stats = ship->CalcStats();

	// go through equipment in random order, seeing if we want the damn stuff
	int *equip_types = (int*)alloca(sizeof(int)*Equip::TYPE_MAX);
	for (int i=0; i<Equip::TYPE_MAX; i++) equip_types[i] = i;
	for (int i=0; i<Equip::TYPE_MAX; i++) { // randomize order
		int swap_with = Pi::rng.Int32(Equip::TYPE_MAX);
		int temp = equip_types[i];
		equip_types[i] = equip_types[swap_with];
		equip_types[swap_with] = temp;
	}
	
	for (int i=0; i<Equip::TYPE_MAX; i++) {
		const Equip::Type t = (Equip::Type)equip_types[i];
		const EquipType &e = EquipType::types[t];
		// leave space for fuel
		if (e.mass > stats->free_capacity - type.hyperdrive) continue;
		switch (e.slot) {
		case Equip::SLOT_CARGO:
		case Equip::SLOT_ENGINE:
			break;
		case Equip::SLOT_LASER:
			if (e.mass > stats->free_capacity/2) break;
			if (Pi::rng.Double() < power) {
				ship->m_equipment.Set(e.slot, 0, t);
			}
			break;
		case Equip::SLOT_MISSILE:
			if (Pi::rng.Double() < power) {
				ship->m_equipment.Add(t);
			}
			break;
		case Equip::SLOT_ECM:
		case Equip::SLOT_SCANNER:
		case Equip::SLOT_RADARMAPPER:
		case Equip::SLOT_HYPERCLOUD:
		case Equip::SLOT_HULLAUTOREPAIR:
		case Equip::SLOT_ENERGYBOOSTER:
		case Equip::SLOT_ATMOSHIELD:
		case Equip::SLOT_FUELSCOOP:
		case Equip::SLOT_LASERCOOLER:
		case Equip::SLOT_CARGOLIFESUPPORT:
		case Equip::SLOT_AUTOPILOT:
			if (Pi::rng.Double() < power) {
				ship->m_equipment.Add(t);
			}
			break;
		case Equip::SLOT_MAX: break;
			
		}
		stats = ship->CalcStats();
	}
	
	int amount = std::min(EquipType::types[type.hyperdrive].pval, stats->free_capacity);
	while (amount--) ship->m_equipment.Add(Equip::HYDROGEN);
	ship->UpdateMass();
}

/////////////////////////////////////////////////////////////
class Rand: public MTRand {
public:
	// don't add members, only methods (because dirty cast Pi::rng to Rand)
	Rand(): MTRand() {}
	Rand(unsigned long seed): MTRand(seed) {}
	std::string PersonName(bool isfemale) {
		return NameGenerator::FullName(*this, isfemale);
	}
	std::string Surname() {
		return NameGenerator::Surname(*this);
	}
};
OOLUA_CLASS_NO_BASES(Rand)
	OOLUA_NO_TYPEDEFS
	OOLUA_CONSTRUCTORS_BEGIN
		OOLUA_CONSTRUCTOR_1(unsigned long)
	OOLUA_CONSTRUCTORS_END
	OOLUA_MEM_FUNC_2_RENAME(Real, double, Double, double, double)
	OOLUA_MEM_FUNC_2_RENAME(Int, int, Int32, int, int)
	OOLUA_MEM_FUNC_1(std::string, PersonName, bool)
	OOLUA_MEM_FUNC_0(std::string, Surname)
OOLUA_CLASS_END

EXPORT_OOLUA_FUNCTIONS_0_CONST(Rand)
EXPORT_OOLUA_FUNCTIONS_4_NON_CONST(Rand,
		Real, Int, PersonName, Surname)

/////////////////////////////////////////////////////////////

// oolua doesn't like namespaces
class SoundEvent: public Sound::Event {};

OOLUA_CLASS_NO_BASES(SoundEvent)
	OOLUA_NO_TYPEDEFS
	OOLUA_ONLY_DEFAULT_CONSTRUCTOR
	OOLUA_MEM_FUNC_4(void, Play, const char *, float, float, Uint32)
	OOLUA_MEM_FUNC_0(bool, Stop)
OOLUA_CLASS_END

EXPORT_OOLUA_FUNCTIONS_2_NON_CONST(SoundEvent,
		Play, Stop)
EXPORT_OOLUA_FUNCTIONS_0_CONST(SoundEvent)

///////////////////////////////////////////////////////////////


struct UnknownShipType {};
/**
 * power 0 = unarmed, power 1 = armed to the teeth
 */
static std::string get_random_ship_type(double power, int minMass, int maxMass)
{
	// find a ship that fits in the mass range
	std::vector<ShipType::Type> candidates;

	for (std::vector<ShipType::Type>::iterator i = ShipType::player_ships.begin();
			i != ShipType::player_ships.end(); ++i) {

        ShipType type = ShipType::types[*i];

		int hullMass = type.hullMass;
		if (hullMass >= minMass && hullMass <= maxMass)
			candidates.push_back(*i);
	}
	//printf("%d candidates\n", candidates.size());
	if (candidates.size() == 0) throw UnknownShipType();

	return candidates[ Pi::rng.Int32(candidates.size()) ];
}

namespace LuaPi {
	static int GetPlayer(lua_State *l) {
		LuaPlayer::PushToLua(Pi::player);
		return 1;
	}
	static int GetGameTime(lua_State *l) {
		LuaFloat::PushToLua(Pi::GetGameTime());
		return 1;
	}
	static int Message(lua_State *l) {
		std::string from = LuaString::GetFromLua(1);
		std::string msg = LuaString::GetFromLua(2);
		Pi::cpan->MsgLog()->Message(from, msg);
		return 0;
	}
	static int ImportantMessage(lua_State *l) {
		std::string from = LuaString::GetFromLua(1);
		std::string msg = LuaString::GetFromLua(2);
		Pi::cpan->MsgLog()->ImportantMessage(from, msg);
		return 0;
	}
	static int GetCurrentSystem(lua_State *l) {
		LuaStarSystem::PushToLua(Pi::currentSystem);
		return 1;
	}
	static int FormatDate(lua_State *l) {
		double t = LuaFloat::GetFromLua(1);
		std::string s = format_date(t);
		LuaString::PushToLua(s.c_str());
		return 1;
	}
	static int _spawn_ship_docked(lua_State *l, std::string type, double power, SpaceStation *station) {
		if (ShipType::Get(type.c_str()) == 0) {
			throw UnknownShipType();
		} else {
			if (!Space::IsSystemBeingBuilt()) {
				lua_pushnil(l);
				lua_pushstring(l, "Cannot spawn ships in starports except from onEnterSystem");
				return 2;
			}
			int port = station->GetFreeDockingPort();
			if (port == -1) {
				lua_pushnil(l);
				lua_pushstring(l, "No free docking port");
				return 2;
			} else {
				Ship *ship = new Ship(type.c_str());
				ship_randomly_equip(ship, power);
				ship->SetFrame(station->GetFrame());
				Space::AddBody(ship);
				ship->SetDockedWith(station, port);
				LuaShip::PushToLua(ship);
				return 1;
			}
		}
	}
	static int _spawn_ship(lua_State *l, std::string type, double due, double power) {
		if (ShipType::Get(type.c_str()) == 0) {
			throw UnknownShipType();
		} else {

			float longitude = Pi::rng.Double(-M_PI,M_PI);
			float latitude = Pi::rng.Double(-M_PI,M_PI);
			float dist = (1.0 + Pi::rng.Double(9.0)) * AU;
			const vector3d pos(sin(longitude)*cos(latitude)*dist, sin(latitude)*dist, cos(longitude)*cos(latitude)*dist);

			if (due <= Pi::GetGameTime()) {
				// already entered
				if (!Space::IsSystemBeingBuilt()) {
					lua_pushnil(l);
					lua_pushstring(l, "Insufficient time to generate ship entry");
					return 2;
				}
				if ((due <= 0) || (due < Pi::GetGameTime()-HYPERCLOUD_DURATION)) {
					// ship is supposed to have entered some time
					// ago and the hyperspace cloud is gone
					Ship *ship = new Ship(type.c_str());
					ship_randomly_equip(ship, power);
					ship->SetFrame(Space::rootFrame);
					ship->SetPosition(pos);
					ship->SetVelocity(vector3d(0,0,0));
					Space::AddBody(ship);
					LuaShip::PushToLua(ship);
					return 1;
				} else {
					// hypercloud still present
					Ship *ship = new Ship(type.c_str());
					ship_randomly_equip(ship, power);
					HyperspaceCloud *cloud = new HyperspaceCloud(ship, due, true);
					cloud->SetFrame(Space::rootFrame);
					cloud->SetPosition(pos);
					cloud->SetVelocity(vector3d(0,0,0));
					Space::AddBody(cloud);
					LuaShip::PushToLua(ship);
					return 1;
				}
			} else {
				// to hyperspace in shortly
				Ship *ship = new Ship(type.c_str());
				ship_randomly_equip(ship, power);
				HyperspaceCloud *cloud = new HyperspaceCloud(ship, due, true);
				cloud->SetFrame(Space::rootFrame);
				cloud->SetPosition(pos);
				cloud->SetVelocity(vector3d(0,0,0));
				Space::AddBody(cloud);
				LuaShip::PushToLua(ship);
				return 1;
			}
		}
	}
	static int SpawnShip(lua_State *l) {
		double due = LuaFloat::GetFromLua(1);
		std::string type = LuaString::GetFromLua(2);
		int ret;
		try {
			ret = _spawn_ship(l, type, due, 0.0);
		} catch (UnknownShipType) {
			lua_pushnil(l);
			lua_pushstring(l, "Unknown ship type");
			return 2;
		}
		return ret;
	}
	static int SpawnRandomShip(lua_State *l) {
		double due = LuaFloat::GetFromLua(1);
		double power = LuaFloat::GetFromLua(2);
		int minMass = LuaInt::GetFromLua(3);
		int maxMass = LuaInt::GetFromLua(4);
		//printf("power %f, mass %d to %d\n", power, minMass, maxMass);
		std::string type;
		int ret;
		try {
			type = get_random_ship_type(power, minMass, maxMass);
			ret = _spawn_ship(l, type, due, power);
		} catch (UnknownShipType) {
			lua_pushnil(l);
			lua_pushstring(l, "Unknown ship type");
			return 2;
		}
		return ret;
	}
	static int SpawnRandomDockedShip(lua_State *l) {
		SpaceStation *station = LuaSpaceStation::GetFromLua(1);
		double power = LuaFloat::GetFromLua(2);
		int minMass = LuaInt::GetFromLua(3);
		int maxMass = LuaInt::GetFromLua(4);
		//printf("power %f, mass %d to %d, docked with %s\n", power, minMass, maxMass, station->GetLabel().c_str());
		std::string type;
		int ret;
		try {
			type = get_random_ship_type(power, minMass, maxMass);
			//printf("Spawning a %s\n", type.c_str());
			ret = _spawn_ship_docked(l, type, power, station);
		} catch (UnknownShipType) {
			lua_pushnil(l);
			lua_pushstring(l, "Unknown ship type");
			return 2;
		}
		return ret;
	}
	static int SpawnRandomStaticShip(lua_State *l) {
		SpaceStation *station = LuaSpaceStation::GetFromLua(1);

		int slot;
		if (!station->AllocateStaticSlot(slot)) {
			lua_pushnil(l);
			lua_pushstring(l, "no space near station to spawn static ship");
			return 2;
		}

		Ship *ship = new Ship(ShipType::GetRandomStaticType().c_str());

		vector3d pos, vel;
		matrix4x4d rot = matrix4x4d::Identity();

		const SBody *body = station->GetSBody();

		if ( body->type == SBody::TYPE_STARPORT_SURFACE ) {
			vel = vector3d(0.0);

			pos = station->GetPosition() * 1.1;
			station->GetRotMatrix(rot);

			vector3d axis1, axis2;

			axis1 = pos.Cross(vector3d(0.0,1.0,0.0));
			axis2 = pos.Cross(axis1);

			double ang = atan((140 + ship->GetLmrCollMesh()->GetBoundingRadius()) / pos.Length());
			if (slot<2) ang = -ang;

			vector3d axis = (slot == 0 || slot == 3) ? axis1 : axis2;

			pos.ArbRotate(axis, ang);
		}

		else {
			double dist = 100 + ship->GetLmrCollMesh()->GetBoundingRadius();
			double xpos = (slot == 0 || slot == 3) ? -dist : dist;
			double zpos = (slot == 0 || slot == 1) ? -dist : dist;

			pos = vector3d(xpos,5000,zpos);
			vel = vector3d(0.0);
			rot.RotateX(M_PI/2);
		}

		ship->SetFrame(station->GetFrame());

		ship->SetVelocity(vel);
		ship->SetPosition(pos);
		ship->SetRotMatrix(rot);

		Space::AddBody(ship);

		ship->AIHoldPosition(station);

		LuaShip::PushToLua(ship);
		return 1;
	}
	static int AddPlayerCrime(lua_State *l) {
		Sint64 crimeBitset = LuaInt::GetFromLua(1);
		double fine = LuaFloat::GetFromLua(2);
		Sint64 _fine = (Sint64)(100.0*fine);
		Polit::AddCrime(crimeBitset, _fine);
		return 0;
	}
}

#define REG_FUNC(fnname, fnptr) \
	lua_pushcfunction(l, fnptr);\
	lua_setfield(l, -2, fnname)

/**
 * Register functions and stuff used by pioneer module/mission scripts
 */
void RegisterPiLuaAPI(lua_State *l)
{
	LUA_DEBUG_START(l);
	OOLUA::register_class<SoundEvent>(l);
	OOLUA::register_class<Rand>(l);
	
#if 0
	lua_register(l, "UserDataSerialize", UserDataSerialize);
	lua_register(l, "UserDataUnserialize", UserDataUnserialize);
#endif

	lua_newtable(l);
	REG_FUNC("AddPlayerCrime", &LuaPi::AddPlayerCrime);
	REG_FUNC("GetCurrentSystem", &LuaPi::GetCurrentSystem);
	REG_FUNC("GetPlayer", &LuaPi::GetPlayer);
	REG_FUNC("GetGameTime", &LuaPi::GetGameTime);
	REG_FUNC("Message", &LuaPi::Message);
	REG_FUNC("ImportantMessage", &LuaPi::ImportantMessage);
	REG_FUNC("SpawnShip", &LuaPi::SpawnShip);
	REG_FUNC("SpawnRandomShip", &LuaPi::SpawnRandomShip);
	REG_FUNC("SpawnRandomDockedShip", &LuaPi::SpawnRandomDockedShip);
	REG_FUNC("SpawnRandomStaticShip", &LuaPi::SpawnRandomStaticShip);
	lua_setglobal(l, "Pi");
	
	lua_newtable(l);
	REG_FUNC("Format", &LuaPi::FormatDate);
	lua_setglobal(l, "Date");
	LUA_DEBUG_END(l, 0);
}
