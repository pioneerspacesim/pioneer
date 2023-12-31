// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

class Camera;
class WorldView;

class ViewController {
public:
	ViewController(WorldView *v) :
		m_parentView(v) {}

	// Called when the view owning this controller becomes active
	virtual void Activated() = 0;

	// Called when the view owning this controller becomes inactive
	virtual void Deactivated() = 0;

	// Called every frame to update the controller before the camera is updated
	virtual void Update() = 0;

	// Do view-specific drawing here, called after the camera has drawn the world
	virtual void Draw(Camera *camera) = 0;

protected:
	WorldView *m_parentView;
};
