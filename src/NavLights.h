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
	enum LightColor {
		NAVLIGHT_RED,
		NAVLIGHT_GREEN,
		NAVLIGHT_BLUE,
		NAVLIGHT_YELLOW
	};

	struct LightBulb
	{
		LightBulb(Uint8 group, Uint8 mask, SceneGraph::Billboard *bb);
		Uint8 group;
		Uint8 mask;
		LightColor color;
		SceneGraph::Billboard *billboard;
	};

	NavLights(SceneGraph::Model*, float period = 2.f);
	virtual ~NavLights();
	void SetEnabled(bool on) { m_enabled = on; }
	void Update(float time);
	void SetColor(unsigned int group, LightColor);

	static void NavLights::Init(Graphics::Renderer*);
	static void NavLights::Uninit();

protected:
	std::vector<LightBulb> m_lights;
	float m_time;
	float m_period;
	bool m_enabled;
};

#endif
