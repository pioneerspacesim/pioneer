// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "PiGuiLua.h"
#include "Face.h"
#include "Image.h"
#include "ModelSpinner.h"

namespace PiGUI {
	namespace Lua {

		void Init()
		{
			LuaObject<PiGUI::Image>::RegisterClass();
			LuaObject<PiGUI::Face>::RegisterClass();
			LuaObject<PiGUI::ModelSpinner>::RegisterClass();
			RegisterSandbox();
		}

	} // namespace Lua
} // namespace PiGUI
