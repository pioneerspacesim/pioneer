// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SHIPCPANEL_H
#define _SHIPCPANEL_H

#include "libs.h"
#include "gui/Gui.h"
#include "ShipCpanelMultiFuncDisplays.h"
#include "Ship.h"
#include "Serializer.h"
#include "Game.h"
#include "WorldView.h"

class Body;
class SpaceStation;
namespace Graphics { class Renderer; }

class ShipCpanel: public Gui::Fixed {
public:
	ShipCpanel(Graphics::Renderer *r, Game* game);
	ShipCpanel(const Json::Value &jsonObj, Graphics::Renderer *r, Game* game);
	virtual ~ShipCpanel();
	virtual void Draw();
	void Update();

	void TimeStepUpdate(float step);

	void SaveToJson(Json::Value &jsonObj);

	void SetRadarVisible(bool visible) { if(visible) m_radar->Show(); else m_radar->Hide(); }

private:
	void InitObject();

	enum MapView { MAP_SECTOR, MAP_SYSTEM, MAP_INFO, MAP_GALACTIC };

	// Handler for radar view / equipment view toggle button
	void OnClickRadarEquip(Gui::MultiStateImageButton *b);

	Game* m_game;

	RadarWidget *m_radar;
};

#endif /* _SHIP_CPANEL_H */
