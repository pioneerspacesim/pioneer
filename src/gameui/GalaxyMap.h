// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef GAMEUI_GALAXYMAP_H
#define GAMEUI_GALAXYMAP_H

#include "ui/Context.h"
#include "ui/Single.h"

namespace GameUI {

class GalaxyMap : public UI::Single {
public:
	GalaxyMap(UI::Context *context);

	virtual UI::Point PreferredSize() override;
	virtual void Update() override;

	float GetZoom() const { return m_zoom; }
	GalaxyMap *SetZoom(float v);
	GalaxyMap *SetCentreSector(const vector2f &at);

private:
	UI::Image *m_baseImage;
	float m_zoom;
	vector2f m_centreSector;
};

}

#endif
