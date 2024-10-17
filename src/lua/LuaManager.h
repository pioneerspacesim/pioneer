// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUAMANAGER_H
#define _LUAMANAGER_H

#include "JobQueue.h"
#include "LuaUtils.h"

class LuaManager {
public:
	// Create a new LuaManager
	// @param[in] asyncJobQueue A job queue to run Lua jobs on. This should be
	//            an asynchronous job queue to allow jobs to run in the
	//            background via the \p ScheduleJob method.
	LuaManager(JobQueue *asyncJobQueue);
	~LuaManager();

	lua_State *GetLuaState() { return m_lua; }
	size_t GetMemoryUsage() const;
	void CollectGarbage();

	// Schedule a job to be run on the LuaManager job queue
	void ScheduleJob(Job *job);

private:
	LuaManager(const LuaManager &);
	LuaManager &operator=(const LuaManager &) = delete;

	lua_State *m_lua;
	JobSet m_jobs;
};

#endif
