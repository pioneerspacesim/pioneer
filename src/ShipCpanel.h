// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SHIPCPANEL_H
#define _SHIPCPANEL_H

#include "Game.h"
#include "Ship.h"
#include "ShipCpanelMultiFuncDisplays.h"
#include "WorldView.h"
#include "gui/Gui.h"
#include "libs.h"

class Body;
class SpaceStation;
namespace Graphics {
	class Renderer;
}

class ShipCpanel : public Gui::Fixed {
public:
	ShipCpanel(Graphics::Renderer *r, Game *game);
	ShipCpanel(const Json &jsonObj, Graphics::Renderer *r, Game *game);
	virtual ~ShipCpanel();
	virtual void Draw();
	void Update();

	void TimeStepUpdate(float step);

	void SaveToJson(Json &jsonObj);

	void SetRadarVisible(bool visible)
	{
		if (visible)
			m_radar->Show();
		else
			m_radar->Hide();
	}

private:
	void InitObject();

	enum MapView { MAP_SECTOR,
		MAP_SYSTEM,
		MAP_INFO,
		MAP_GALACTIC };

	// Handler for radar view / equipment view toggle button
	void OnClickRadarEquip(Gui::MultiStateImageButton *b);

	Game *m_game;

	RadarWidget *m_radar;
};

#endif /* _SHIP_CPANEL_H */
