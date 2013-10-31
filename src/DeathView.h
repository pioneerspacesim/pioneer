// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef DEATH_VIEW_H
#define DEATH_VIEW_H

#include "libs.h"
#include "View.h"
#include "Camera.h"

class DeathView : public View {
public:
	DeathView();
	virtual ~DeathView();

	void Init();

	virtual void Update();
	virtual void Draw3D();

protected:
	virtual void OnSwitchTo();

private:
	std::unique_ptr<Camera> m_cam;
	float m_cameraDist;
};

#endif
