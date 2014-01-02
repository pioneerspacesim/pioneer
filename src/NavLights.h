// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _NAVLIGHTS_H
#define _NAVLIGHTS_H
/*
 * Blinking navigation lights for ships and stations
 */
#include "libs.h"
#include "Serializer.h"

namespace Graphics { class Renderer; }
namespace SceneGraph { class Model; class Billboard; }

class NavLights
{
public:
	enum LightColor {
		NAVLIGHT_RED    = 0,
		NAVLIGHT_GREEN  = 1,
		NAVLIGHT_BLUE   = 2,
		NAVLIGHT_YELLOW = 3
	};

	struct LightBulb
	{
		LightBulb(Uint8 group, Uint8 mask, Uint8 color, SceneGraph::Billboard *bb);
		Uint8 group;
		Uint8 mask; //bitmask: 00001111 light on half the period, 11111111 light on the entire period etc...
		Uint8 color;
		SceneGraph::Billboard *billboard;
	};

	NavLights(SceneGraph::Model*, float period = 2.f);
	virtual ~NavLights();
	virtual void Save(Serializer::Writer &wr);
	virtual void Load(Serializer::Reader &rd);

	void SetEnabled(bool on) { m_enabled = on; }
	void Update(float time);
	void SetColor(unsigned int group, LightColor);

	static void Init(Graphics::Renderer*);
	static void Uninit();

protected:
	std::vector<LightBulb> m_lights;
	float m_time;
	float m_period;
	bool m_enabled;
};

#endif
