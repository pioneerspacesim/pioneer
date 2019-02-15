
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
