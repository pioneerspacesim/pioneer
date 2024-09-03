// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef LUAWRAPPABLE_H
#define LUAWRAPPABLE_H

// all classes that can be passed through to Lua must inherit from
// LuaWrappable. this is mostly to ensure that dynamic_cast will always work
//
// this is in a separate file so that it can be included without including all
// of LuaObject

class LuaWrappable {
public:
	virtual ~LuaWrappable() {}
};

#endif
