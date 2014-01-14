// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUAFILESYSTEM_H
#define _LUAFILESYSTEM_H

namespace LuaFileSystem {
	void Register();

	enum Root { // <enum scope='LuaFileSystem' name=FileSystemRoot prefix=ROOT_ public>
		ROOT_USER,
		ROOT_DATA
	};
}

#endif
