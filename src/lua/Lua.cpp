// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Lua.h"
#include "Pi.h"

#include "LuaColor.h"
#include "LuaConsole.h"
#include "LuaConstants.h"
#include "LuaDev.h"
#include "LuaEconomy.h"
#include "LuaEngine.h"
#include "LuaEvent.h"
#include "LuaFileSystem.h"
#include "LuaFormat.h"
#include "LuaGame.h"
#include "LuaInput.h"
#include "LuaJson.h"
#include "LuaLang.h"
#include "LuaManager.h"
#include "LuaMusic.h"
#include "LuaNameGen.h"
#include "LuaSerializer.h"
#include "LuaShipDef.h"
#include "LuaSpace.h"
#include "LuaTimer.h"
#include "LuaVector.h"
#include "LuaVector2.h"

#include "Body.h"
#include "SectorView.h"
#include "Ship.h"
#include "SpaceStation.h"
#include "Star.h"
#include "SystemView.h"

#include "galaxy/StarSystem.h"
#include "pigui/LuaPiGui.h"
#include "scenegraph/Lua.h"

namespace Lua {

	LuaManager *manager = 0;

	void InitMath();

	void Init()
	{
		manager = new LuaManager();
		InitMath();
	}

	void Uninit()
	{
		delete manager;
		manager = 0;
	}

	// initialize standalone math types as the "extended standard library" for all lua instances
	void InitMath()
	{
		LuaObject<Random>::RegisterClass();

		LuaVector::Register(manager->GetLuaState());
		LuaVector2::Register(manager->GetLuaState());
		LuaColor::Register(manager->GetLuaState());

		LuaEngine::Register();

		pi_lua_dofile(manager->GetLuaState(), "libs/autoload.lua");
	}

	void InitModules()
	{
		PROFILE_SCOPED()
		lua_State *l = Lua::manager->GetLuaState();

		LuaObject<PropertyMap>::RegisterClass();

		LuaObject<Body>::RegisterClass();
		LuaObject<Ship>::RegisterClass();
		LuaObject<SpaceStation>::RegisterClass();
		LuaObject<Planet>::RegisterClass();
		LuaObject<Star>::RegisterClass();
		LuaObject<Player>::RegisterClass();
		LuaObject<Missile>::RegisterClass();
		LuaObject<CargoBody>::RegisterClass();
		LuaObject<ModelBody>::RegisterClass();
		LuaObject<HyperspaceCloud>::RegisterClass();

		LuaObject<StarSystem>::RegisterClass();
		LuaObject<SystemPath>::RegisterClass();
		LuaObject<SystemView>::RegisterClass();
		LuaObject<SectorView>::RegisterClass();
		LuaObject<SectorMap>::RegisterClass();
		LuaObject<SystemBody>::RegisterClass();
		LuaObject<Faction>::RegisterClass();
		LuaObject<Galaxy>::RegisterClass();

		Pi::luaSerializer = new LuaSerializer();
		Pi::luaTimer = new LuaTimer();

		LuaObject<LuaSerializer>::RegisterClass();
		LuaObject<LuaTimer>::RegisterClass();

		LuaEvent::Init();

		LuaConstants::Register(Lua::manager->GetLuaState());
		LuaLang::Register();
		LuaEconomy::Register();
		LuaInput::Register();
		LuaFileSystem::Register();
		LuaJson::Register();
#ifdef ENABLE_SERVER_AGENT
		LuaServerAgent::Register();
#endif
		LuaGame::Register();
		LuaFormat::Register();
		LuaSpace::Register();
		LuaShipDef::Register();
		LuaMusic::Register();
		LuaDev::Register();
		// LuaConsole::Register();

		// XXX sigh
		SceneGraph::Lua::Init();

		// XXX load everything. for now, just modules
		// pi_lua_dofile(l, "libs/autoload.lua");
		lua_pop(l, 1);

		pi_lua_import(l, "pigui", true);

		pi_lua_import_recursive(l, "pigui.modules");
		pi_lua_import_recursive(l, "pigui.views");

		pi_lua_import_recursive(l, "modules");

		Pi::luaNameGen = new LuaNameGen(Lua::manager);
	}

	void UninitModules()
	{
		LuaEvent::Uninit();

		delete Pi::luaNameGen;

		delete Pi::luaSerializer;
		delete Pi::luaTimer;
	}

} // namespace Lua
