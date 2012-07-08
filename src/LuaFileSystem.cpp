#include "LuaFileSystem.h"
#include "LuaObject.h"
#include "FileSystem.h"
#include "Pi.h"

/*
 * Interface: FileSystem
 *
 * A global table that provides access to the filesystem.
 */

void LuaFileSystem::Register()
{
	LuaObjectBase::CreateObject(0, 0, 0);
	lua_setglobal(Pi::luaManager->GetLuaState(), "FileSystem");
}
