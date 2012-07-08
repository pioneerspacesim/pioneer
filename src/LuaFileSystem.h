#ifndef _LUAFILESYSTEM_H
#define _LUAFILESYSTEM_H

namespace LuaFileSystem {
	void Register();

	enum Root { // <enum scope='LuaFileSystem' name=FileSystemRoot prefix=ROOT_>
		ROOT_USER,
		ROOT_DATA
	};
}

#endif
