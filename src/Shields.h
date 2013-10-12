// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SHIELDS_H_
#define _SHIELDS_H_
/*
 * Blinking navigation lights for ships and stations
 */
#include "libs.h"
#include "Serializer.h"

namespace Graphics { class Renderer; }
namespace SceneGraph { class Model; class StaticGeometry; class Group; }

class Shields
{
public:
	struct Shield
	{
		Shield(const Color3ub color, SceneGraph::StaticGeometry *sg);
		Color3ub m_colour; // I'm English, so it's "colour" ;)
		RefCountedPtr<SceneGraph::StaticGeometry> m_mesh;
	};

	Shields(SceneGraph::Model*, float period = 2.f);
	virtual ~Shields();
	virtual void Save(Serializer::Writer &wr);
	virtual void Load(Serializer::Reader &rd);

	void SetEnabled(const bool on) { m_enabled = on; }
	void Update(const float shieldStrength /* 0.0f to 1.0f */);
	void SetColor(const Color3ub);

	static void Init(Graphics::Renderer*);
	static void Uninit();

protected:
	std::vector<Shield> m_shields;
	bool m_enabled;

	static bool s_initialised;
};

#endif
