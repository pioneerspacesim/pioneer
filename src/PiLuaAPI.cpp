#include "libs.h"
#include "PiLuaAPI.h"
#include "PiLuaModules.h"
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

////////////////////////////////////////////////////////////

std::map<Object *, int> ObjectWrapper::objWrapLookup;

EXPORT_OOLUA_NO_FUNCTIONS(Object)
EXPORT_OOLUA_FUNCTIONS_0_NON_CONST(ObjectWrapper)
EXPORT_OOLUA_FUNCTIONS_3_CONST(ObjectWrapper,
		IsBody,
		IsValid,
		GetLabel)

ObjectWrapper::ObjectWrapper(Object *o): m_obj(o) {
	if (o) {
		m_delCon = o->onDelete.connect(sigc::mem_fun(this, &ObjectWrapper::OnDelete));
	}
}
bool ObjectWrapper::IsBody() const {
	return Is(Object::BODY);
}
void ObjectWrapper::SetLabel(const char *label) {
	if (Is(Object::BODY)) {
		static_cast<Body*>(m_obj)->SetLabel(label);
	}
}
const char *ObjectWrapper::GetLabel() const {
	if (Is(Object::BODY)) {
		return static_cast<Body*>(m_obj)->GetLabel().c_str();
	} else {
		return "";
	}
}

void ObjectWrapper::Push(lua_State * const s, Object * const & o)
{
	if (!o) {
		lua_pushnil(s);
		return;
	}
	std::map<Object *, int>::iterator i = objWrapLookup.find(o);
	if (i == objWrapLookup.end()) {
		ObjectWrapper *p = new ObjectWrapper(o);
		OOLUA::push2lua(s, p, OOLUA::Lua);
		lua_pushvalue(s, -1);
		const int ref = luaL_ref(s, LUA_REGISTRYINDEX);
		objWrapLookup[o] = ref;
	} else {
		lua_rawgeti(s, LUA_REGISTRYINDEX, (*i).second);
	}
}

ObjectWrapper::~ObjectWrapper() {
//	printf("ObjWrapper for %s is being deleted\n", GetLabel());
	OnDelete();
}
bool ObjectWrapper::Is(Object::Type t) const {
	return m_obj && m_obj->IsType(t);
}
void ObjectWrapper::OnDelete() {
	// object got deleted out from under us
//	printf("%p->OnDelete %p\n", this, m_obj);
	if (m_obj) {
		std::map<Object *, int>::iterator i = objWrapLookup.find(m_obj);
		if (i != objWrapLookup.end()) {
			lua_unref(PiLuaModules::GetLuaState(), (*i).second);
			objWrapLookup.erase(i);
		}
	}
	m_obj = 0;
	m_delCon.disconnect();
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

#if 0
static int UserDataSerialize(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	if (mylua_checkudata(L, 1, "ObjectWrapper")) {
		char buf[256];
		ObjectWrapper *o;
		OOLUA::pull2cpp(L, o);
		snprintf(buf, sizeof(buf), "ObjectWrapper\n%d\n", Serializer::LookupBody((Body*)o->m_obj));
		lua_pushstring(L, buf);
		return 1;
	} else if (mylua_checkudata(L, 1, "SBodyPath")) {
		Serializer::Writer wr;
		SBodyPath *path;
		OOLUA::pull2cpp(L, path);
		path->Serialize(wr);
		std::string out = "SBodyPath\n";
		out += wr.GetData();
		OOLUA::push2lua(L, out);
		return 1;
	} else if (mylua_checkudata(L, 1, "SysLoc")) {
		Serializer::Writer wr;
		SysLoc *systemid;
		OOLUA::pull2cpp(L, systemid);
		systemid->Serialize(wr);
		std::string out = "SysLoc\n";
		out += wr.GetData();
		OOLUA::push2lua(L, out);
		return 1;
	} else {
		Error("Tried to serialize unknown userdata type.");
		return 0;
	}
}

static int UserDataUnserialize(lua_State *L)
{
	std::string str;
	OOLUA::pull2cpp(L, str);
	if (str.substr(0, 14) == "ObjectWrapper\n") {
		int idx = atoi(str.substr(14).c_str());
		Body *b = Serializer::LookupBody(idx);
		OOLUA::push2lua(L, static_cast<Object*>(b));
		return 1;
	} else if (str.substr(0, 10) == "SBodyPath\n") {
		Serializer::Reader r(str.substr(10));
		SBodyPath *p = new SBodyPath;
		SBodyPath::Unserialize(r, p);
		push2luaWithGc(L, p);
		return 1;
	} else if (str.substr(0, 7) == "SysLoc\n") {
		Serializer::Reader r(str.substr(7));
		SysLoc *p = new SysLoc;
		SysLoc::Unserialize(r, p);
		push2luaWithGc(L, p);
		return 1;
	}
	return 0;
}
#endif


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
		LuaShip::PushToLua(dynamic_cast<Ship*>(Pi::player));
		return 1;
	}
	static int GetGameTime(lua_State *l) {
		LuaFloat::PushToLua(Pi::GetGameTime());
		return 1;
	}
	static int Message(lua_State *l) {
		const char *from = LuaString::PullFromLua();
		const char *msg = LuaString::PullFromLua();
		Pi::cpan->MsgLog()->Message(from, msg);
		return 0;
	}
	static int ImportantMessage(lua_State *l) {
		const char *from = LuaString::PullFromLua();
		const char *msg = LuaString::PullFromLua();
		Pi::cpan->MsgLog()->ImportantMessage(from, msg);
		return 0;
	}
	static int GetCurrentSystem(lua_State *l) {
		LuaStarSystem::PushToLua(Pi::currentSystem);
		return 1;
	}
	static int FormatDate(lua_State *l) {
		double t = LuaFloat::PullFromLua();
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
		double due = LuaFloat::PullFromLua();
		const char *type = LuaString::PullFromLua();
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
		double due = LuaFloat::PullFromLua();
		double power = LuaFloat::PullFromLua();
		int minMass = LuaInt::PullFromLua();
		int maxMass = LuaInt::PullFromLua();
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
		SpaceStation *station = LuaSpaceStation::PullFromLua();
		double power = LuaFloat::PullFromLua();
		int minMass = LuaInt::PullFromLua();
		int maxMass = LuaInt::PullFromLua();
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
		SpaceStation *station = LuaSpaceStation::PullFromLua();

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
		Sint64 crimeBitset = LuaInt::PullFromLua();
		double fine = LuaFloat::PullFromLua();
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
	OOLUA::register_class<ObjectWrapper>(l);
	OOLUA::register_class<Object>(l);
	OOLUA::register_class<LuaChatForm>(l);
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
