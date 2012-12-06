// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef GAMEUI_LUA_H
#define GAMEUI_LUA_H

#include "LuaObject.h"
#include "GameUI.h"

namespace GameUI {
namespace Lua {

	void Init();

}
}

template <> class LuaAcquirer<GameUI::Face> : public LuaAcquirerRefCounted {};
template <> class LuaAcquirer<GameUI::ShipSpinner> : public LuaAcquirerRefCounted {};

#endif
