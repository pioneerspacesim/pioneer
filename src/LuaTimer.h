// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUATIMER_H
#define _LUATIMER_H

#include "LuaManager.h"
#include "DeleteEmitter.h"

class LuaTimer : public DeleteEmitter {
public:
	void Tick();
	void RemoveAll();
};

#endif
