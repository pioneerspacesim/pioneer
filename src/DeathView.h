// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef DEATH_VIEW_H
#define DEATH_VIEW_H

#include "libs.h"
#include "View.h"
#include "Camera.h"
#include "RefCounted.h"

class Game;

class DeathView : public View {
public:
	DeathView(Game* game);
	virtual ~DeathView();

	void Init();

	virtual void Update();
	virtual void Draw3D();

protected:
	virtual void OnSwitchTo();

private:
	RefCountedPtr<CameraContext> m_cameraContext;
	std::unique_ptr<Camera> m_camera;
	float m_cameraDist;
	Game* m_game;
};

#endif
