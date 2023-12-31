// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _DELETEEMITTER_H
#define _DELETEEMITTER_H

// inherit from this class to be able to get notifications when an object is
// destroyed. the LuaObject tracking layer uses this to properly "forget"
// about objects that are currently exposed to lua

// including sigc++ directly so we don't get circular dependencies
#include <sigc++/sigc++.h>

#include "lua/LuaWrappable.h"

class DeleteEmitter : public LuaWrappable {
public:
	DeleteEmitter() {}
	virtual ~DeleteEmitter()
	{
		onDelete.emit();
	}

	// onDelete is mutable since its not unusual to want to know when a const
	// object is deleted, and attaching this signal does not conceptually
	// affect the object's state
	mutable sigc::signal<void> onDelete;

private:
	// sigc++ signals cannot be copied, but long-standing design flaw means
	// they don't have a private copy constructor. we protect them ourselves
	DeleteEmitter(const DeleteEmitter &) {}
};

#endif
