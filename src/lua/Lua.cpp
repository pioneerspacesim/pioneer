// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Lua.h"
#include "Pi.h"

#include "LuaColor.h"
#include "LuaComms.h"
#include "LuaConsole.h"
#include "LuaConstants.h"
#include "LuaDev.h"
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
#include "Ship.h"
#include "SpaceStation.h"
#include "Star.h"
#include "SystemView.h"

#include "galaxy/StarSystem.h"
#include "gameui/Lua.h"
#include "pigui/PiGuiLua.h"
#include "scenegraph/Lua.h"
#include "ui/Lua.h"

namespace Lua {

	LuaManager *manager = 0;

	void Init()
	{
		manager = new LuaManager();
	}

	void Uninit()
	{
		delete manager;
		manager = 0;
	}

	void InitModules()
	{
		PROFILE_SCOPED()
		lua_State *l = Lua::manager->GetLuaState();

		LuaObject<PropertiedObject>::RegisterClass();

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
		LuaObject<SystemBody>::RegisterClass();
		LuaObject<Random>::RegisterClass();
		LuaObject<Faction>::RegisterClass();

		Pi::luaSerializer = new LuaSerializer();
		Pi::luaTimer = new LuaTimer();

		LuaObject<LuaSerializer>::RegisterClass();
		LuaObject<LuaTimer>::RegisterClass();

		LuaConstants::Register(Lua::manager->GetLuaState());
		LuaLang::Register();
		LuaEngine::Register();
		LuaInput::Register();
		LuaFileSystem::Register();
		LuaJson::Register();
#ifdef ENABLE_SERVER_AGENT
		LuaServerAgent::Register();
#endif
		LuaGame::Register();
		LuaComms::Register();
		LuaFormat::Register();
		LuaSpace::Register();
		LuaShipDef::Register();
		LuaMusic::Register();
		LuaDev::Register();
		LuaConsole::Register();
		LuaVector::Register(Lua::manager->GetLuaState());
		LuaVector2::Register(Lua::manager->GetLuaState());
		LuaColor::Register(Lua::manager->GetLuaState());

		// XXX sigh
		UI::Lua::Init();
		GameUI::Lua::Init();
		SceneGraph::Lua::Init();

		// XXX load everything. for now, just modules
		pi_lua_dofile(l, "libs/autoload.lua");
		lua_pop(l, 1);

		pi_lua_import_recursive(l, "ui");

		pi_lua_import(l, "pigui", true);

		pi_lua_import_recursive(l, "pigui.modules");
		pi_lua_import_recursive(l, "pigui.views");

		pi_lua_import_recursive(l, "modules");

		Pi::luaNameGen = new LuaNameGen(Lua::manager);
	}

	void UninitModules()
	{
		delete Pi::luaNameGen;

		delete Pi::luaSerializer;
		delete Pi::luaTimer;
	}

} // namespace Lua
