#ifndef _LUATIMER_H
#define _LUATIMER_H

#include "LuaManager.h"
#include "DeleteEmitter.h"

class LuaTimer : public DeleteEmitter {
public:
	void Tick();
};

#endif
