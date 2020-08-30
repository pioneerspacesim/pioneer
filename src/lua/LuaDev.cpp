// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaDev.h"
#include "Game.h"
#include "LuaObject.h"
#include "Pi.h"
#include "WorldView.h"
#include <sstream>

/*
 * Lua commands used in development & debugging
 * Everything here is subject to rapid changes
 */

/*
 * Method: GalaxyStats
 *
 * Output to stdout and lua console statictics in the given cube. Start the game before using!
 *
 * using:
 * > GalaxyStats(centerX, centerY, centerZ, radius of cube, processors)
 *
 * example using from console:
 * > require 'Dev'.GalaxyStats(0,0,0,5, "CountSystemNames CountSystems CountPopulation")
 *
 * Parameters:
 *   centerX, centerY, centerZ - integer, coordinates of center of the cube, 0, 0, 0 = Sol
 *   radius - integer - distance in sectors from center to edge of the cube
 *   processors - string - Processor names separated by spaces
 *     available processor names:
 *       CountSystems
 *       CountSystemNames
 *       CountPopulation
 *
 * Availability:
 *
 *   2020
 *
 * Status:
 *
 *   experimental
 */

static int l_dev_galaxy_stats(lua_State *l)
{
	// base class to process data from star system
	class Processor {
	public:
		virtual void ProcessSystem(const Sector::System &system) = 0;
		virtual std::string Report() = 0;
	};

	// specific processors

	class : public Processor {
		uint32_t systems = 0;

	public:
		void ProcessSystem(const Sector::System &system) override
		{
			systems++;
		}
		std::string Report() override
		{
			return std::to_string(systems) + " systems total.\n";
		}
	} CountSystems;

	class : public Processor {
		std::map<std::string, int> names;

	public:
		void ProcessSystem(const Sector::System &system) override
		{
			// counting repeats of each name
			names[system.GetName()]++;
		}
		std::string Report() override
		{
			// sorting
			std::vector<std::pair<std::string, int>> sorted(names.begin(), names.end());
			std::sort(sorted.begin(), sorted.end(), [](std::pair<std::string, int> &n1, std::pair<std::string, int> &n2) {
				if (n1.second == n2.second)
					return n1.first < n2.first;
				else
					return n1.second > n2.second;
			});
			std::string s;
			s += "Top 10 system names:\n";
			for (unsigned i = 0; i < 10; i++)
				s += std::to_string(i + 1) + ". " + sorted[i].first + " : " + std::to_string(sorted[i].second) + "\n";
			s += "Bottom 10 system names:\n";
			for (unsigned i = sorted.size() - 10; i < sorted.size(); i++)
				s += std::to_string(i + 1) + ". " + sorted[i].first + " : " + std::to_string(sorted[i].second) + "\n";
			return s + "Total names: " + std::to_string(sorted.size()) + "\n";
		}
	} CountSystemNames;

	class : public Processor {
		uint32_t explored = 0;
		uint32_t inhabited = 0;
		double population;
		RefCountedPtr<Galaxy> galaxy = Pi::game->GetGalaxy();

	public:
		void ProcessSystem(const Sector::System &system) override
		{
			if (system.IsExplored()) explored++;
			double current = galaxy->GetStarSystem(SystemPath(system.sx, system.sy, system.sz, system.idx))->GetTotalPop().ToDouble();
			if (current > 0) {
				inhabited++;
				population += current;
			}
		}
		std::string Report() override
		{
			return std::to_string(explored) + " explored.\n" +
				std::to_string(inhabited) + " inhabited.\n" +
				"population: " + std::to_string(population) + " billion.\n";
		}
	} CountPopulation;

	// lua args
	int centerX = LuaPull<int>(l, 1);
	int centerY = LuaPull<int>(l, 2);
	int centerZ = LuaPull<int>(l, 3);
	int radius = LuaPull<int>(l, 4);
	std::string options_string = LuaPull<std::string>(l, 5);

	// parse options string to separate strings
	std::istringstream iss(options_string);
	std::vector<std::string> options((std::istream_iterator<std::string>(iss)),
		std::istream_iterator<std::string>());

	// map strings into processors
	std::map<const std::string, Processor *> processors = { { "CountSystemNames", &CountSystemNames },
		{ "CountPopulation", &CountPopulation },
		{ "CountSystems", &CountSystems } };
	std::vector<Processor *> P;
	for (auto &option : options)
		if (processors.find(option) != processors.end())
			P.push_back(processors[option]);

	// iterating all processors on all systems in given sectors
	RefCountedPtr<Galaxy> galaxy = Pi::game->GetGalaxy();
	for (int sx = centerX - radius; sx <= centerX + radius; ++sx) {
		for (int sy = centerY - radius; sy <= centerY + radius; ++sy) {
			for (int sz = centerZ - radius; sz <= centerZ + radius; ++sz) {
				SystemPath sp(sx, sy, sz);
				auto sec = galaxy->GetSector(sp);
				for (auto &system : sec->m_systems)
					for (auto &p : P)
						p->ProcessSystem(system);
			}
		}
	}

	// reporting from all processors
	std::string result = "";
	for (auto &p : P)
		result += p->Report();
	LuaPush<std::string>(l, result);
	return 1;
}

/*
 * Set current camera offset to vector,
 * (the offset will reset when switching cameras)
 *
 * Dev.SetCameraOffset(x, y, z)
 */
static int l_dev_set_camera_offset(lua_State *l)
{
	if (!Pi::game || !Pi::game->GetWorldView())
		return luaL_error(l, "Dev.SetCameraOffset only works when there is a game running");
	CameraController *cam = Pi::game->GetWorldView()->shipView->GetCameraController();
	const float x = luaL_checknumber(l, 1);
	const float y = luaL_checknumber(l, 2);
	const float z = luaL_checknumber(l, 3);
	cam->SetPosition(vector3d(x, y, z));
	return 0;
}

void LuaDev::Register()
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_Reg methods[] = {
		{ "GalaxyStats", l_dev_galaxy_stats },
		{ "SetCameraOffset", l_dev_set_camera_offset },
		{ 0, 0 }
	};

	lua_getfield(l, LUA_REGISTRYINDEX, "CoreImports");
	luaL_newlib(l, methods);
	lua_setfield(l, -2, "Dev");
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}
