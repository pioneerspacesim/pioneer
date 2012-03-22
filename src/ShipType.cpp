#include "ShipType.h"
#include "LmrModel.h"
#include "MyLuaMathTypes.h"
#include "LuaUtils.h"
#include "utils.h"
#include "Lang.h"

const char *ShipType::gunmountNames[GUNMOUNT_MAX] = {
	Lang::FRONT, Lang::REAR };

std::map<ShipType::Type, ShipType> ShipType::types;

std::vector<ShipType::Type> ShipType::player_ships;
std::vector<ShipType::Type> ShipType::static_ships;
std::vector<ShipType::Type> ShipType::missile_ships;

std::string ShipType::LADYBIRD				= "Ladybird Starfighter";
std::string ShipType::SIRIUS_INTERDICTOR	= "Sirius Interdictor";
std::string ShipType::EAGLE_LRF				= "Eagle Long Range Fighter";
std::string ShipType::EAGLE_MK3				= "Eagle MK-III";
std::string ShipType::MISSILE_GUIDED		= "MISSILE_GUIDED";
std::string ShipType::MISSILE_NAVAL			= "MISSILE_NAVAL";
std::string ShipType::MISSILE_SMART			= "MISSILE_SMART";
std::string ShipType::MISSILE_UNGUIDED		= "MISSILE_UNGUIDED";

static void _get_string_attrib(lua_State *L, const char *key, std::string &output,
		const char *default_output)
{
	LUA_DEBUG_START(L);
	lua_pushstring(L, key);
	lua_gettable(L, -2);
	if (lua_isnil(L, -1)) {
		output = default_output;
	} else {
		output = lua_tostring(L,-1);
	}
	lua_pop(L, 1);
	LUA_DEBUG_END(L, 0);
}

static void _get_float_attrib(lua_State *L, const char *key, float &output,
		const float default_output)
{
	LUA_DEBUG_START(L);
	lua_pushstring(L, key);
	lua_gettable(L, -2);
	if (lua_isnil(L, -1)) {
		output = default_output;
	} else {
		output = lua_tonumber(L,-1);
	}
	lua_pop(L, 1);
	LUA_DEBUG_END(L, 0);
}

static void _get_int_attrib(lua_State *L, const char *key, int &output,
		const int default_output)
{
	LUA_DEBUG_START(L);
	lua_pushstring(L, key);
	lua_gettable(L, -2);
	if (lua_isnil(L, -1)) {
		output = default_output;
	} else {
		output = lua_tointeger(L,-1);
	}
	lua_pop(L, 1);
	LUA_DEBUG_END(L, 0);
}

int _define_ship(lua_State *L, ShipType::Tag tag, std::vector<ShipType::Type> *list)
{
	ShipType s;
	s.tag = tag;

	LUA_DEBUG_START(L);
	_get_string_attrib(L, "name", s.name, "");
	_get_string_attrib(L, "model", s.lmrModelName, "");
	_get_float_attrib(L, "reverse_thrust", s.linThrust[ShipType::THRUSTER_REVERSE], 0.0f);
	_get_float_attrib(L, "forward_thrust", s.linThrust[ShipType::THRUSTER_FORWARD], 0.0f);
	_get_float_attrib(L, "up_thrust", s.linThrust[ShipType::THRUSTER_UP], 0.0f);
	_get_float_attrib(L, "down_thrust", s.linThrust[ShipType::THRUSTER_DOWN], 0.0f);
	_get_float_attrib(L, "left_thrust", s.linThrust[ShipType::THRUSTER_LEFT], 0.0f);
	_get_float_attrib(L, "right_thrust", s.linThrust[ShipType::THRUSTER_RIGHT], 0.0f);
	_get_float_attrib(L, "angular_thrust", s.angThrust, 0.0f);
	s.angThrust = s.angThrust / 2;		// fudge

	for (int i=0; i<Equip::SLOT_MAX; i++) s.equipSlotCapacity[i] = 0;
	_get_int_attrib(L, "max_cargo", s.equipSlotCapacity[Equip::SLOT_CARGO], 0);
	_get_int_attrib(L, "max_engine", s.equipSlotCapacity[Equip::SLOT_ENGINE], 1);
	_get_int_attrib(L, "max_laser", s.equipSlotCapacity[Equip::SLOT_LASER], 1);
	_get_int_attrib(L, "max_missile", s.equipSlotCapacity[Equip::SLOT_MISSILE], 0);
	_get_int_attrib(L, "max_ecm", s.equipSlotCapacity[Equip::SLOT_ECM], 1);
	_get_int_attrib(L, "max_scanner", s.equipSlotCapacity[Equip::SLOT_SCANNER], 1);
	_get_int_attrib(L, "max_radarmapper", s.equipSlotCapacity[Equip::SLOT_RADARMAPPER], 1);
	_get_int_attrib(L, "max_hypercloud", s.equipSlotCapacity[Equip::SLOT_HYPERCLOUD], 1);
	_get_int_attrib(L, "max_hullautorepair", s.equipSlotCapacity[Equip::SLOT_HULLAUTOREPAIR], 1);
	_get_int_attrib(L, "max_energybooster", s.equipSlotCapacity[Equip::SLOT_ENERGYBOOSTER], 1);
	_get_int_attrib(L, "max_atmoshield", s.equipSlotCapacity[Equip::SLOT_ATMOSHIELD], 1);
	_get_int_attrib(L, "max_cabin", s.equipSlotCapacity[Equip::SLOT_CABIN], 50);
	_get_int_attrib(L, "max_shield", s.equipSlotCapacity[Equip::SLOT_SHIELD], 9999);
	_get_int_attrib(L, "max_fuelscoop", s.equipSlotCapacity[Equip::SLOT_FUELSCOOP], 1);
	_get_int_attrib(L, "max_cargoscoop", s.equipSlotCapacity[Equip::SLOT_CARGOSCOOP], 1);
	_get_int_attrib(L, "max_lasercooler", s.equipSlotCapacity[Equip::SLOT_LASERCOOLER], 1);
	_get_int_attrib(L, "max_cargolifesupport", s.equipSlotCapacity[Equip::SLOT_CARGOLIFESUPPORT], 1);
	_get_int_attrib(L, "max_autopilot", s.equipSlotCapacity[Equip::SLOT_AUTOPILOT], 1);

	_get_int_attrib(L, "capacity", s.capacity, 0);
	_get_int_attrib(L, "hull_mass", s.hullMass, 100);
	_get_int_attrib(L, "fuel_tank_mass", s.fuelTankMass, 5);
	_get_float_attrib(L, "thruster_fuel_use", s.thrusterFuelUse, 1.f);
	_get_int_attrib(L, "price", s.baseprice, 0);
	s.baseprice *= 100; // in hundredths of credits

	{
		int hyperclass;
		_get_int_attrib(L, "hyperdrive_class", hyperclass, 1);
		if (!hyperclass) {
			s.hyperdrive = Equip::NONE;
		} else {
			s.hyperdrive = Equip::Type(Equip::DRIVE_CLASS1+hyperclass-1);
		}
	}
	
	lua_pushstring(L, "gun_mounts");
	lua_gettable(L, -2);
	if (lua_istable(L, -1)) {
		for (unsigned int i=0; i<lua_objlen(L,-1); i++) {
			lua_pushinteger(L, i+1);
			lua_gettable(L, -2);
			if (lua_istable(L, -1) && lua_objlen(L,-2) == 2)	{
				lua_pushinteger(L, 1);
				lua_gettable(L, -2);
				s.gunMount[i].pos = *MyLuaVec::checkVec(L, -1);
				lua_pop(L, 1);
				lua_pushinteger(L, 2);
				lua_gettable(L, -2);
				s.gunMount[i].dir = *MyLuaVec::checkVec(L, -1);
				lua_pop(L, 1);
			}
			lua_pop(L, 1);
		}
	}
	lua_pop(L, 1);
	LUA_DEBUG_END(L, 0);

	//sanity check
	if (s.name.empty())
		return luaL_error(L, "Ship has no name");

	if (s.lmrModelName.empty())
		return luaL_error(L, "Missing model name in ship");

	//this shouldn't necessarily be a fatal problem, could just warn+mark ship unusable
	//or replace with proxy geometry
	try {
		LmrLookupModelByName(s.lmrModelName.c_str());
	} catch (LmrModelNotFoundException &) {
		return luaL_error(L, "Model %s is not defined", s.lmrModelName.c_str());
	}

	ShipType::types[s.name] = s;
	list->push_back(s.name);
	return 0;
}

int define_ship(lua_State *L)
{
	return _define_ship(L, ShipType::TAG_SHIP, &ShipType::player_ships);
}

int define_static_ship(lua_State *L)
{
	return _define_ship(L, ShipType::TAG_STATIC_SHIP, &ShipType::static_ships);
}

int define_missile(lua_State *L)
{
	return _define_ship(L, ShipType::TAG_MISSILE, &ShipType::missile_ships);
}

void ShipType::Init()
{
	static bool isInitted = false;
	if (isInitted) return;
	isInitted = true;

	lua_State *l = lua_open();
	luaL_openlibs(l);

	LUA_DEBUG_START(l);

	MyLuaVec::Vec_register(l);
	lua_pop(l, 1);
	lua_register(l, "v", MyLuaVec::Vec_new);
	lua_register(l, "define_ship", define_ship);
	lua_register(l, "define_static_ship", define_static_ship);
	lua_register(l, "define_missile", define_missile);

	lua_pushstring(l, PIONEER_DATA_DIR);
	lua_setglobal(l, "CurrentDirectory");
	pi_lua_dofile_recursive(l, "ships");

	LUA_DEBUG_END(l, 0);

	lua_close(l);

	if (ShipType::player_ships.empty())
		Warning("No playable ships have been defined! The game cannot run.");
}

