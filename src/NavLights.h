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
	NavLights(SceneGraph::Model*);
	~NavLights();
	void SetEnabled(bool on) { m_enabled = on; }
	void Update(float time);

private:
	std::vector<SceneGraph::Billboard*> m_groupRed;
	std::vector<SceneGraph::Billboard*> m_groupGreen;
	std::vector<SceneGraph::Billboard*> m_groupBlue;
	std::vector<SceneGraph::Billboard*> m_allLights;
	float m_time;
	bool m_enabled;
};

#endif

