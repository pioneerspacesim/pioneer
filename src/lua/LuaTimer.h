// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUATIMER_H
#define _LUATIMER_H

#include "DeleteEmitter.h"
#include "LuaManager.h"
#include "JsonFwd.h"

#include <deque>

class LuaTimer : public DeleteEmitter {
public:
	LuaTimer();
	~LuaTimer();

	void Init();

	void Tick();
	void RemoveAll();

	// For internal use only
	void Insert(double at, int callbackId, bool repeats);

private:
	/*
	 * Minimal tracking struct for the next time a callback should be triggered
	 * Avoids the overhead of constant round-trips into Lua
	 */
	struct CallInfo {
		double at;
		int callbackId;
		bool repeats;
	};

	// Queue of tracked 'timeout' entries
	std::deque<CallInfo> m_timeouts;
	// Scratch buffer for timeouts that elapsed this update
	std::vector<CallInfo> m_called;
};

#endif
