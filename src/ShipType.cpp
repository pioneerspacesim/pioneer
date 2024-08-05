// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ShipType.h"
#include "FileSystem.h"
#include "JsonUtils.h"
#include "Lang.h"
#include "utils.h"
#include <algorithm>

// TODO: Fix the horrible control flow that makes this exception type necessary.
struct ShipTypeLoadError {};

std::map<ShipType::Id, const ShipType> ShipType::types;
std::vector<ShipType::Id> ShipType::player_ships;
std::vector<ShipType::Id> ShipType::static_ships;
std::vector<ShipType::Id> ShipType::missile_ships;

const std::string ShipType::POLICE = "kanara";
const std::string ShipType::MISSILE_GUIDED = "missile_guided";
const std::string ShipType::MISSILE_NAVAL = "missile_naval";
const std::string ShipType::MISSILE_SMART = "missile_smart";
const std::string ShipType::MISSILE_UNGUIDED = "missile_unguided";

float ShipType::GetFuelUseRate() const
{
	const float denominator = fuelTankMass * effectiveExhaustVelocity * 10;
	return denominator > 0 ? linThrust[THRUSTER_FORWARD] / denominator : 1e9;
}

// returns velocity of engine exhausts in m/s
static double GetEffectiveExhaustVelocity(double fuelTankMass, double thrusterFuelUse, double forwardThrust)
{
	double denominator = fuelTankMass * thrusterFuelUse * 10;
	return fabs(denominator > 0 ? forwardThrust / denominator : 1e9);
}

static bool ShipIsUnbuyable(const std::string &id)
{
	const ShipType &t = ShipType::types[id];
	return is_zero_exact(t.baseprice);
}

ShipType::ShipType(const Id &_id, const std::string &path)
{
	PROFILE_SCOPED()
	Json data = JsonUtils::LoadJsonDataFile(path);
	if (data.is_null()) {
		Output("couldn't read ship def '%s'\n", path.c_str());
		throw ShipTypeLoadError();
	}

	isGlobalColorDefined = false;

	// determine what kind (tag) of ship this is.
	const std::string tagStr = data.value("tag", "");
	if (tagStr.empty() || strcasecmp(tagStr.c_str(), "ship") == 0) {
		tag = TAG_SHIP;
	} else if (strcasecmp(tagStr.c_str(), "static") == 0) {
		tag = TAG_STATIC_SHIP;
	} else if (strcasecmp(tagStr.c_str(), "missile") == 0) {
		tag = TAG_MISSILE;
	}

	id = _id;
	name = data.value("name", "");
	shipClass = data.value("ship_class", "");
	manufacturer = data.value("manufacturer", "");
	modelName = data.value("model", "");
	cockpitName = data.value("cockpit", "");
	shieldName = data.value("shield_model", modelName + "_shield");

	linThrust[THRUSTER_REVERSE] = data.value("reverse_thrust", 0.0f);
	linThrust[THRUSTER_FORWARD] = data.value("forward_thrust", 0.0f);
	linThrust[THRUSTER_UP] = data.value("up_thrust", 0.0f);
	linThrust[THRUSTER_DOWN] = data.value("down_thrust", 0.0f);
	linThrust[THRUSTER_LEFT] = data.value("left_thrust", 0.0f);
	linThrust[THRUSTER_RIGHT] = data.value("right_thrust", 0.0f);
	angThrust = data.value("angular_thrust", 0.0f);

	linAccelerationCap[THRUSTER_REVERSE] = data.value("reverse_acceleration_cap", INFINITY);
	linAccelerationCap[THRUSTER_FORWARD] = data.value("forward_acceleration_cap", INFINITY);
	linAccelerationCap[THRUSTER_UP] = data.value("up_acceleration_cap", INFINITY);
	linAccelerationCap[THRUSTER_DOWN] = data.value("down_acceleration_cap", INFINITY);
	linAccelerationCap[THRUSTER_LEFT] = data.value("left_acceleration_cap", INFINITY);
	linAccelerationCap[THRUSTER_RIGHT] = data.value("right_acceleration_cap", INFINITY);

	//get atmospheric flight values
	topCrossSection = data.value("top_cross_section", 1.0f);
	sideCrossSection = data.value("side_cross_section", 1.0f);
	frontCrossSection = data.value("front_cross_section", 1.0f);

	topDragCoeff = data.value("top_drag_coeff", 0.1f);
	sideDragCoeff = data.value("side_drag_coeff", 0.1f);
	frontDragCoeff = data.value("front_drag_coeff", 0.1f);

	shipLiftCoefficient = data.value("lift_coeff", 0.0f);
	atmoStability = data.value("aero_stability", 0.0f);

	// Parse global thrusters color
	bool error = false;
	int parse = 0;
	for (Json::iterator thruster_color = data["thruster_global_color"].begin(); thruster_color != data["thruster_global_color"].end(); ++thruster_color) {
		const std::string colorchannel = thruster_color.key();
		if (colorchannel.length() != 1) {
			error = true;
			break;
		}
		if (colorchannel.at(0) == 'r') {
			globalThrusterColor.r = data["thruster_global_color"].value(colorchannel, 0);
			parse++;
			continue;
		} else if (colorchannel.at(0) == 'g') {
			globalThrusterColor.g = data["thruster_global_color"].value(colorchannel, 0);
			parse++;
			continue;
		} else if (colorchannel.at(0) == 'b') {
			globalThrusterColor.b = data["thruster_global_color"].value(colorchannel, 0);
			parse++;
			continue;
		} else {
			// No 'r', no 'g', no 'b', no good :/
			error = true;
			break;
		}
	}
	if (error == true) {
		Output("In file \"%s.json\" global thrusters custom color must be \"r\",\"g\" and \"b\"\n", modelName.c_str());
		throw ShipTypeLoadError();
	} else if (parse > 0 && parse < 3) {
		Output("In file \"%s.json\" global thrusters custom color is malformed\n", modelName.c_str());
		throw ShipTypeLoadError();
	} else if (parse == 3) {
		globalThrusterColor.a = 255;
		isGlobalColorDefined = true;
	}
	// Parse direction thrusters color
	for (int i = 0; i < THRUSTER_MAX; i++)
		isDirectionColorDefined[i] = false;
	error = false;
	for (Json::iterator thruster_color = data["thruster_direction_color"].begin(); thruster_color != data["thruster_direction_color"].end(); ++thruster_color) {
		const std::string th_color_dir = thruster_color.key();
		try {
			Color color = data["thruster_direction_color"];
			if (th_color_dir.find("forward") != std::string::npos) {
				isDirectionColorDefined[THRUSTER_FORWARD] = true;
				directionThrusterColor[THRUSTER_FORWARD] = color;
			}
			if (th_color_dir.find("retro") != std::string::npos) {
				isDirectionColorDefined[THRUSTER_REVERSE] = true;
				directionThrusterColor[THRUSTER_REVERSE] = color;
			}
			if (th_color_dir.find("left") != std::string::npos) {
				isDirectionColorDefined[THRUSTER_LEFT] = true;
				directionThrusterColor[THRUSTER_LEFT] = color;
			}
			if (th_color_dir.find("right") != std::string::npos) {
				isDirectionColorDefined[THRUSTER_RIGHT] = true;
				directionThrusterColor[THRUSTER_RIGHT] = color;
			}
			if (th_color_dir.find("up") != std::string::npos) {
				isDirectionColorDefined[THRUSTER_UP] = true;
				directionThrusterColor[THRUSTER_UP] = color;
			}
			if (th_color_dir.find("down") != std::string::npos) {
				isDirectionColorDefined[THRUSTER_DOWN] = true;
				directionThrusterColor[THRUSTER_DOWN] = color;
			}
		} catch (Json::type_error &) {
			for (int i = 0; i < THRUSTER_MAX; i++)
				isDirectionColorDefined[i] = false;
			Output("In file \"%s.json\" directional thrusters custom color must be \"r\",\"g\" and \"b\"\n", modelName.c_str());
			throw ShipTypeLoadError();
		}
	}
	// angthrust fudge (XXX: why?)
	angThrust = angThrust * 0.5f;

	hullMass = data.value("hull_mass", 100);
	capacity = data.value("capacity", 0);
	fuelTankMass = data.value("fuel_tank_mass", 5);

	for (Json::iterator slot = data["slots"].begin(); slot != data["slots"].end(); ++slot) {
		const std::string slotname = slot.key();
		slots[slotname] = data["slots"].value(slotname, 0);
	}

	for (Json::iterator role = data["roles"].begin(); role != data["roles"].end(); ++role) {
		roles[*role] = true;
	}

	for (int it = 0; it < 4; it++)
		thrusterUpgrades[it] = 1.0 + (double(it) / 10.0);
	for (Json::iterator slot = data["thrust_upgrades"].begin(); slot != data["thrust_upgrades"].end(); ++slot) {
		const std::string slotname = slot.key();
		const int index = Clamp(atoi(&slotname.c_str()[9]), 1, 3);
		thrusterUpgrades[index] = data["thrust_upgrades"].value(slotname, 0);
	}

	atmosphericPressureLimit = data.value("atmospheric_pressure_limit", 10.0); // 10 atmosphere is about 90 metres underwater (on Earth)

	{
		const auto it = slots.find("engine");
		if (it != slots.end()) {
			it->second = Clamp(it->second, 0, 1);
		}
	}

	effectiveExhaustVelocity = data.value("effective_exhaust_velocity", -1.0f);
	const float thruster_fuel_use = data.value("thruster_fuel_use", -1.0f);

	if (effectiveExhaustVelocity < 0 && thruster_fuel_use < 0) {
		// default value of v_c is used
		effectiveExhaustVelocity = 55000000;
	} else if (effectiveExhaustVelocity < 0 && thruster_fuel_use >= 0) {
		// v_c undefined and thruster fuel use defined -- use it!
		effectiveExhaustVelocity = GetEffectiveExhaustVelocity(fuelTankMass, thruster_fuel_use, linThrust[Thruster::THRUSTER_FORWARD]);
	} else {
		if (thruster_fuel_use >= 0) {
			Output("Warning: Both thruster_fuel_use and effective_exhaust_velocity defined for %s, using effective_exhaust_velocity.\n", modelName.c_str());
		}
	}

	baseprice = data.value("price", 0.0);
	minCrew = data.value("min_crew", 1);
	maxCrew = data.value("max_crew", 1);
	hyperdriveClass = data.value("hyperdrive_class", 1);
}

void ShipType::Init()
{
	PROFILE_SCOPED()
	static bool isInitted = false;
	if (isInitted)
		return;
	isInitted = true;

	// load all ship definitions
	namespace fs = FileSystem;
	for (fs::FileEnumerator files(fs::gameDataFiles, "ships", fs::FileEnumerator::Recurse); !files.Finished(); files.Next()) {
		const fs::FileInfo &info = files.Current();
		if (ends_with_ci(info.GetPath(), ".json")) {
			const std::string id(info.GetName().substr(0, info.GetName().size() - 5));
			try {
				ShipType st = ShipType(id, info.GetPath());
				types.insert(std::make_pair(st.id, st));

				// assign the names to the various lists
				switch (st.tag) {
				case TAG_SHIP: player_ships.push_back(id); break;
				case TAG_STATIC_SHIP: static_ships.push_back(id); break;
				case TAG_MISSILE:
					missile_ships.push_back(id);
					break;
					break;
				case TAG_NONE:
				default:
					break;
				}
			} catch (ShipTypeLoadError) {
				// TODO: Actual error handling would be nice.
				Error("Error while loading Ship data (check stdout/output.txt).\n");
			}
		}
	}

	//remove unbuyable ships from player ship list
	ShipType::player_ships.erase(
		std::remove_if(ShipType::player_ships.begin(), ShipType::player_ships.end(), ShipIsUnbuyable),
		ShipType::player_ships.end());

	if (ShipType::player_ships.empty())
		Error("No playable ships have been defined! The game cannot run.");
}
