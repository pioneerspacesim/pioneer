// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

class WorldView;

class InteractionController {
public:
	InteractionController(WorldView *v) :
		parentView(v) {}

	virtual void Activated() = 0;
	virtual void Deactivated() = 0;
	virtual void Update() = 0;

	WorldView *parentView;
};
