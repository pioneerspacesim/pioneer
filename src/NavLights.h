// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _NAVLIGHTS_H
#define _NAVLIGHTS_H
/*
 * Blinking navigation lights for ships and stations
 */
#include "libs.h"

namespace SceneGraph { class Model; class Billboard; }

class NavLights
{
public:
	struct LightBulb
	{
		LightBulb(Uint8 mask, SceneGraph::Billboard *bb);
		Uint8 mask;
		SceneGraph::Billboard *billboard;
	};
	NavLights(SceneGraph::Model*);
	~NavLights();
	void SetEnabled(bool on) { m_enabled = on; }
	void Update(float time);

private:
	std::vector<LightBulb> m_allLights;
	float m_time;
	float m_period;
	bool m_enabled;
};

#endif

