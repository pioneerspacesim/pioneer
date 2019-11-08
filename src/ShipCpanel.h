// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SHIPCPANEL_H
#define _SHIPCPANEL_H

#include "JsonFwd.h"
#include "gui/GuiFixed.h"

class Body;
class Game;
class SpaceStation;
class RadarWidget;

namespace Graphics {
	class Renderer;
}

class ShipCpanel : public Gui::Fixed {
public:
	ShipCpanel(Graphics::Renderer *r);
	ShipCpanel(const Json &jsonObj, Graphics::Renderer *r);
	virtual ~ShipCpanel();
	virtual void Draw();
	void Update();

	void TimeStepUpdate(float step);

	void SaveToJson(Json &jsonObj);

	void SetRadarVisible(bool visible);

private:
	void InitObject();

	enum MapView { MAP_SECTOR,
		MAP_SYSTEM,
		MAP_INFO,
		MAP_GALACTIC };

	RadarWidget *m_radar;
};

#endif /* _SHIP_CPANEL_H */
