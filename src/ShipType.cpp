// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#define ALLOW_LUA_SHIP_DEF 0

#include "ShipType.h"
#if ALLOW_LUA_SHIP_DEF
#include "LuaVector.h"
#include "LuaUtils.h"
#include "LuaTable.h"
#include "LuaConstants.h"
#endif
#include "FileSystem.h"
#include "utils.h"
#include "Lang.h"
#include "json/json.h"
#include <algorithm>


std::map<ShipType::Id, const ShipType> ShipType::types;
std::vector<ShipType::Id> ShipType::player_ships;
std::vector<ShipType::Id> ShipType::static_ships;
std::vector<ShipType::Id> ShipType::missile_ships;

const std::string ShipType::POLICE				= "kanara";
const std::string ShipType::MISSILE_GUIDED		= "missile_guided";
const std::string ShipType::MISSILE_NAVAL		= "missile_naval";
const std::string ShipType::MISSILE_SMART		= "missile_smart";
const std::string ShipType::MISSILE_UNGUIDED	= "missile_unguided";

float ShipType::GetFuelUseRate() const
{
	const float denominator = fuelTankMass * effectiveExhaustVelocity * 10;
	return denominator > 0 ? -linThrust[THRUSTER_FORWARD]/denominator : 1e9;
}

// returns velocity of engine exhausts in m/s
static double GetEffectiveExhaustVelocity(double fuelTankMass, double thrusterFuelUse, double forwardThrust) {
	double denominator = fuelTankMass * thrusterFuelUse * 10;
	return fabs(denominator > 0 ? forwardThrust/denominator : 1e9);
}

static bool ShipIsUnbuyable(const std::string &id)
{
	const ShipType &t = ShipType::types[id];
	return is_zero_exact(t.baseprice);
}

ShipType::ShipType(const Id &_id, const std::string &path)
{
	Json::Reader reader;
	Json::Value data;

	isGlobalColorDefined = false;

	auto fd = FileSystem::gameDataFiles.ReadFile(path);
	if (!fd) {
		Output("couldn't open ship def '%s'\n", path.c_str());
		return;
	}

	if (!reader.parse(fd->GetData(), fd->GetData()+fd->GetSize(), data)) {
		Output("couldn't read ship def '%s': %s\n", path.c_str(), reader.getFormattedErrorMessages().c_str());
		return;
	}

	// determine what kind (tag) of ship this is.
	const std::string tagStr = data.get("tag", "").asString();
	if( tagStr.empty() || strcasecmp(tagStr.c_str(), "ship")==0 ) {
		tag = TAG_SHIP;
	} else if( strcasecmp(tagStr.c_str(), "static")==0 ) {
		tag = TAG_STATIC_SHIP;
	} else if( strcasecmp(tagStr.c_str(), "missile")==0 ) {
		tag = TAG_MISSILE;
	}

	id = _id;
	name = data.get("name", "").asString();
	shipClass = data.get("ship_class", "").asString();
	manufacturer = data.get("manufacturer", "").asString();
	modelName = data.get("model", "").asString();
	cockpitName = data.get("cockpit", "").asString();

	linThrust[THRUSTER_REVERSE] = data.get("reverse_thrust", 0.0f).asFloat();
	linThrust[THRUSTER_FORWARD] = data.get("forward_thrust", 0.0f).asFloat();
	linThrust[THRUSTER_UP] = data.get("up_thrust", 0.0f).asFloat();
	linThrust[THRUSTER_DOWN] = data.get("down_thrust", 0.0f).asFloat();
	linThrust[THRUSTER_LEFT] = data.get("left_thrust", 0.0f).asFloat();
	linThrust[THRUSTER_RIGHT] = data.get("right_thrust", 0.0f).asFloat();
	angThrust = data.get("angular_thrust", 0.0f).asFloat();

	// Parse global thrusters color
	bool error = false;
	int parse = 0;
	for( Json::Value::iterator thruster_color = data["thruster_global_color"].begin() ; thruster_color != data["thruster_global_color"].end() ; ++thruster_color ) {
		const std::string colorchannel = thruster_color.key().asString();
		if (colorchannel.length()!=1) {
			error = true;
			break;
		}
		if (colorchannel.at(0) == 'r') {
			globalThrusterColor.r = data["thruster_global_color"].get(colorchannel, 0).asInt();
			parse++;
			continue;
		} else if (colorchannel.at(0) == 'g') {
			globalThrusterColor.g = data["thruster_global_color"].get(colorchannel, 0).asInt();
			parse++;
			continue;
		} else if (colorchannel.at(0) == 'b') {
			globalThrusterColor.b = data["thruster_global_color"].get(colorchannel, 0).asInt();
			parse++;
			continue;
		} else {
			// No 'r', no 'g', no 'b', no good :/
			error = true;
			break;
		}
	}
	if (error==true) {
		Output("In file \"%s.json\" global thrusters custom color must be \"r\",\"g\" and \"b\"\n", modelName.c_str());
	} else if (parse>0 && parse<3) {
		Output("In file \"%s.json\" global thrusters custom color is malformed\n", modelName.c_str());
	} else if (parse==3) {
		globalThrusterColor.a = 255;
		isGlobalColorDefined = true;
	}
	// Parse direction thrusters color
	for (int i=0; i<THRUSTER_MAX; i++) isDirectionColorDefined[i]=false;
	error = false;
	for( Json::Value::iterator thruster_color = data["thruster_direction_color"].begin() ; thruster_color != data["thruster_direction_color"].end() ; ++thruster_color ) {
		const std::string th_color_dir = thruster_color.key().asString();
		Json::Value dir_color = data["thruster_direction_color"].get(th_color_dir, 0);
		Color color;
		if (!dir_color.isMember("r")||!dir_color.isMember("g")||!dir_color.isMember("b")) {
			error = true;
			continue /* for */;
		} else {
			color.r = dir_color["r"].asInt();
			color.g = dir_color["g"].asInt();
			color.b = dir_color["b"].asInt();
			color.a = 255;
		}
		if (th_color_dir.find("forward")!=std::string::npos) {
			isDirectionColorDefined[THRUSTER_FORWARD]=true;
			directionThrusterColor[THRUSTER_FORWARD]= color;
		}
		if (th_color_dir.find("retro")!=std::string::npos) {
			isDirectionColorDefined[THRUSTER_REVERSE]=true;
			directionThrusterColor[THRUSTER_REVERSE]= color;
		}
		if (th_color_dir.find("left")!=std::string::npos) {
			isDirectionColorDefined[THRUSTER_LEFT]=true;
			directionThrusterColor[THRUSTER_LEFT]= color;
		}
		if (th_color_dir.find("right")!=std::string::npos) {
			isDirectionColorDefined[THRUSTER_RIGHT]=true;
			directionThrusterColor[THRUSTER_RIGHT]= color;
		}
		if (th_color_dir.find("up")!=std::string::npos) {
			isDirectionColorDefined[THRUSTER_UP]=true;
			directionThrusterColor[THRUSTER_UP]= color;
		}
		if (th_color_dir.find("down")!=std::string::npos) {
			isDirectionColorDefined[THRUSTER_DOWN]=true;
			directionThrusterColor[THRUSTER_DOWN]= color;
		}
	}
	if (error==true) {
		for (int i=0; i<THRUSTER_MAX; i++) isDirectionColorDefined[i]=false;
		Output("In file \"%s.json\" directional thrusters custom color must be \"r\",\"g\" and \"b\"\n", modelName.c_str());
	}
	// invert values where necessary
	linThrust[THRUSTER_FORWARD] *= -1.f;
	linThrust[THRUSTER_LEFT] *= -1.f;
	linThrust[THRUSTER_DOWN] *= -1.f;
	// angthrust fudge (XXX: why?)
	angThrust = angThrust * 0.5f;

	hullMass = data.get("hull_mass", 100).asInt();
	capacity = data.get("capacity", 0).asInt();
	fuelTankMass = data.get("fuel_tank_mass", 5).asInt();

	for( Json::Value::iterator slot = data["slots"].begin() ; slot != data["slots"].end() ; ++slot ) {
		const std::string slotname = slot.key().asString();
		slots[slotname] = data["slots"].get(slotname, 0).asInt();
	}

	for( Json::Value::iterator role = data["roles"].begin(); role != data["roles"].end(); ++role ) {
		const std::string rolename = role.key().asString();
		roles[rolename] = data["roles"].get(rolename, 0).asBool();
	}

	for(int it=0;it<4;it++) thrusterUpgrades[it] = 1.0 + (double(it)/10.0);
	for( Json::Value::iterator slot = data["thrust_upgrades"].begin() ; slot != data["thrust_upgrades"].end() ; ++slot ) {
		const std::string slotname = slot.key().asString();
		const int index = Clamp(atoi(&slotname.c_str()[9]), 1, 3);
		thrusterUpgrades[index] = data["thrust_upgrades"].get(slotname, 0).asDouble();
	}

	atmosphericPressureLimit = data.get("atmospheric_pressure_limit", 10.0).asDouble();	// 10 atmosphere is about 90 metres underwater (on Earth)

	{
		const auto it = slots.find("engine");
		if (it != slots.end())
		{
			it->second = Clamp(it->second, 0, 1);
		}
	}

	effectiveExhaustVelocity = data.get("effective_exhaust_velocity", -1.0f).asFloat();
	const float thruster_fuel_use = data.get("thruster_fuel_use", -1.0f).asFloat();

	if(effectiveExhaustVelocity < 0 && thruster_fuel_use < 0) {
		// default value of v_c is used
		effectiveExhaustVelocity = 55000000;
	} else if(effectiveExhaustVelocity < 0 && thruster_fuel_use >= 0) {
		// v_c undefined and thruster fuel use defined -- use it!
		effectiveExhaustVelocity = GetEffectiveExhaustVelocity(fuelTankMass, thruster_fuel_use, linThrust[Thruster::THRUSTER_FORWARD]);
	} else {
		if(thruster_fuel_use >= 0) {
			Output("Warning: Both thruster_fuel_use and effective_exhaust_velocity defined for %s, using effective_exhaust_velocity.\n", modelName.c_str());
		}
	}

	baseprice = data.get("price", 0.0).asDouble();
	minCrew = data.get("min_crew", 1).asInt();
	maxCrew = data.get("max_crew", 1).asInt();
	hyperdriveClass = data.get("hyperdrive_class", 1).asInt();
}

#if ALLOW_LUA_SHIP_DEF
static std::string s_currentShipFile;
int _define_ship(lua_State *L, ShipType::Tag tag, std::vector<ShipType::Id> *list)
{
	if (s_currentShipFile.empty())
		return luaL_error(L, "ship file contains multiple ship definitions");

	Json::Value data;

	ShipType s;
	s.tag = tag;
	s.id = s_currentShipFile;

	LUA_DEBUG_START(L);
	LuaTable t(L, -1);

	s.name = t.Get("name", "");
	s.shipClass = t.Get("ship_class", "unknown");
	s.manufacturer = t.Get("manufacturer", "unknown");
	s.modelName = t.Get("model", "");

	data["name"] = s.name;
	data["ship_class"] = s.shipClass;
	data["manufacturer"] = s.manufacturer;
	data["model"] = s.modelName;

	s.cockpitName = t.Get("cockpit", "");
	s.linThrust[ShipType::THRUSTER_REVERSE] = t.Get("reverse_thrust", 0.0f);
	s.linThrust[ShipType::THRUSTER_FORWARD] = t.Get("forward_thrust", 0.0f);
	s.linThrust[ShipType::THRUSTER_UP] = t.Get("up_thrust", 0.0f);
	s.linThrust[ShipType::THRUSTER_DOWN] = t.Get("down_thrust", 0.0f);
	s.linThrust[ShipType::THRUSTER_LEFT] = t.Get("left_thrust", 0.0f);
	s.linThrust[ShipType::THRUSTER_RIGHT] = t.Get("right_thrust", 0.0f);
	s.angThrust = t.Get("angular_thrust", 0.0f);

	data["cockpit"] = s.cockpitName;
	data["reverse_thrust"] = s.linThrust[ShipType::THRUSTER_REVERSE];
	data["forward_thrust"] = s.linThrust[ShipType::THRUSTER_FORWARD];
	data["up_thrust"] = s.linThrust[ShipType::THRUSTER_UP];
	data["down_thrust"] = s.linThrust[ShipType::THRUSTER_DOWN];
	data["left_thrust"] = s.linThrust[ShipType::THRUSTER_LEFT];
	data["right_thrust"] = s.linThrust[ShipType::THRUSTER_RIGHT];
	data["angular_thrust"] = s.angThrust;

	// invert values where necessary
	s.linThrust[ShipType::THRUSTER_FORWARD] *= -1.f;
	s.linThrust[ShipType::THRUSTER_LEFT] *= -1.f;
	s.linThrust[ShipType::THRUSTER_DOWN] *= -1.f;
	// angthrust fudge (XXX: why?)
	s.angThrust = s.angThrust / 2;

	s.capacity = t.Get("capacity", 0);
	s.hullMass = t.Get("hull_mass", 100);
	s.fuelTankMass = t.Get("fuel_tank_mass", 5);

	data["capacity"] = s.capacity;
	data["hull_mass"] = s.hullMass;
	data["fuel_tank_mass"] = s.fuelTankMass;

	LuaTable slot_table = t.Sub("slots");
	if (slot_table.GetLua()) {
		s.slots = slot_table.GetMap<std::string, int>();
	}
	lua_pop(L, 1);

	{
		const auto it = s.slots.find("engine");
		if (it != s.slots.end()) { it->second = Clamp(it->second, 0, 1); }
	}

	for( auto slot : s.slots ) {
		data["slots"][slot.first] = slot.second;
	}

	// fuel_use_rate can be given in two ways
	float thruster_fuel_use = 0;
	s.effectiveExhaustVelocity = t.Get("effective_exhaust_velocity", -1.0f);
	thruster_fuel_use = t.Get("thruster_fuel_use", -1.0f);

	data["effective_exhaust_velocity"] = s.effectiveExhaustVelocity;
	data["thruster_fuel_use"] = thruster_fuel_use;

	if(s.effectiveExhaustVelocity < 0 && thruster_fuel_use < 0) {
		// default value of v_c is used
		s.effectiveExhaustVelocity = 55000000;
	} else if(s.effectiveExhaustVelocity < 0 && thruster_fuel_use >= 0) {
		// v_c undefined and thruster fuel use defined -- use it!
		s.effectiveExhaustVelocity = GetEffectiveExhaustVelocity(s.fuelTankMass, thruster_fuel_use, s.linThrust[ShipType::THRUSTER_FORWARD]);
	} else {
		if(thruster_fuel_use >= 0)
			Output("Warning: Both thruster_fuel_use and effective_exhaust_velocity defined for %s, using effective_exhaust_velocity.\n", s.modelName.c_str());
	}

	s.baseprice = t.Get("price", 0.0);

	s.minCrew = t.Get("min_crew", 1);
	s.maxCrew = t.Get("max_crew", 1);

	s.hyperdriveClass = t.Get("hyperdrive_class", 1);

	data["price"] = s.baseprice;
	data["min_crew"] = s.minCrew;
	data["max_crew"] = s.maxCrew;
	data["hyperdrive_class"] = s.hyperdriveClass;

	Json::StyledWriter writer;
	const std::string saveMe = writer.write( data );

	const std::string path("ships/" + s_currentShipFile + ".json");
	FileSystem::FileSourceFS newFS(FileSystem::GetDataDir());
	FILE *f = newFS.OpenWriteStream(path);
	if (!f) {
		Output("couldn't open file for writing '%s'\n", path.c_str());
		abort();
	}
	fwrite(saveMe.data(), saveMe.length(), 1, f);
	fclose(f);

	lua_pop(L, 1);
	LUA_DEBUG_END(L, 0);

	//sanity check
	if (s.name.empty())
		return luaL_error(L, "Ship has no name");

	if (s.modelName.empty())
		return luaL_error(L, "Missing model name in ship");

	if (s.minCrew < 1 || s.maxCrew < 1 || s.minCrew > s.maxCrew)
		return luaL_error(L, "Invalid values for min_crew and max_crew");

	const std::string& id = s_currentShipFile;
	typedef std::map<std::string, const ShipType>::iterator iter;
	std::pair<iter, bool> result = ShipType::types.insert(std::make_pair(id, s));
	if (result.second)
		list->push_back(s_currentShipFile);
	else
		return luaL_error(L, "Ship '%s' was already defined by a different file", id.c_str());
	s_currentShipFile.clear();

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
#endif

void ShipType::Init()
{
	static bool isInitted = false;
	if (isInitted)
		return;
	isInitted = true;

	// load all ship definitions
	namespace fs = FileSystem;
	for (fs::FileEnumerator files(fs::gameDataFiles, "ships", fs::FileEnumerator::Recurse); !files.Finished(); files.Next()) {
		const fs::FileInfo &info = files.Current();
		if (ends_with_ci(info.GetPath(), ".json")) {
			const std::string id(info.GetName().substr(0, info.GetName().size()-5));
			ShipType st = ShipType(id, info.GetPath());
			types.insert(std::make_pair(st.id, st));

			// assign the names to the various lists
			switch( st.tag ) {
			case TAG_SHIP:				player_ships.push_back(id);				break;
			case TAG_STATIC_SHIP:		static_ships.push_back(id);				break;
			case TAG_MISSILE:			missile_ships.push_back(id);			break;
				break;
			case TAG_NONE:
			default:
				break;
			}
		}
	}

#if ALLOW_LUA_SHIP_DEF
	lua_State *l = luaL_newstate();

	LUA_DEBUG_START(l);

	luaL_requiref(l, "_G", &luaopen_base, 1);
	luaL_requiref(l, LUA_DBLIBNAME, &luaopen_debug, 1);
	luaL_requiref(l, LUA_MATHLIBNAME, &luaopen_math, 1);
	lua_pop(l, 3);

	LuaConstants::Register(l);
	LuaVector::Register(l);
	LUA_DEBUG_CHECK(l, 0);

	// provide shortcut vector constructor: v = vector.new
	lua_getglobal(l, LuaVector::LibName);
	lua_getfield(l, -1, "new");
	assert(lua_iscfunction(l, -1));
	lua_setglobal(l, "v");
	lua_pop(l, 1); // pop the vector library table

	LUA_DEBUG_CHECK(l, 0);

	// register ship definition functions
	lua_register(l, "define_ship", define_ship);
	lua_register(l, "define_static_ship", define_static_ship);
	lua_register(l, "define_missile", define_missile);

	LUA_DEBUG_CHECK(l, 0);

	// load all ship definitions
	namespace fs = FileSystem;
	for (fs::FileEnumerator files(fs::gameDataFiles, "ships", fs::FileEnumerator::Recurse); !files.Finished(); files.Next()) {
		const fs::FileInfo &info = files.Current();
		if (ends_with_ci(info.GetPath(), ".lua")) {
			const std::string name = info.GetName();
			s_currentShipFile = name.substr(0, name.size() - 4);
			if (ShipType::types.find(s_currentShipFile) == ShipType::types.end())
			{
				pi_lua_dofile(l, info.GetPath());
				s_currentShipFile.clear();
			}
		}
	}

	LUA_DEBUG_END(l, 0);

	lua_close(l);
#endif

	//remove unbuyable ships from player ship list
	ShipType::player_ships.erase(
		std::remove_if(ShipType::player_ships.begin(), ShipType::player_ships.end(), ShipIsUnbuyable),
		ShipType::player_ships.end());

	if (ShipType::player_ships.empty())
		Error("No playable ships have been defined! The game cannot run.");
}

