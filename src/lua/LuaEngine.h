// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUAENGINE_H
#define _LUAENGINE_H

namespace LuaEngine {
	void Register();

	enum DetailLevel { // <enum scope='LuaEngine' name=DetailLevel prefix=DETAIL_ public>
		DETAIL_VERY_LOW,
		DETAIL_LOW,
		DETAIL_MEDIUM,
		DETAIL_HIGH,
		DETAIL_VERY_HIGH,
	};
} // namespace LuaEngine

#endif
