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

	enum OverlayTextPos {
		OVERLAY_TOP_LEFT,
		OVERLAY_TOP_RIGHT,
		OVERLAY_BOTTOM_LEFT,
		OVERLAY_OVER_PANEL_RIGHT_1,
		OVERLAY_OVER_PANEL_RIGHT_2,
		OVERLAY_OVER_PANEL_RIGHT_3,
		OVERLAY_OVER_PANEL_RIGHT_4,
		OVERLAY_MAX
	};
	void SetOverlayText(OverlayTextPos pos, const std::string &text);
	void SetOverlayToolTip(OverlayTextPos pos, const std::string &text);
	void SetOverlayTextColour(OverlayTextPos pos, const Color &colour);
	void ClearOverlay();
	void SetRadarVisible(bool visible) { if(visible) m_radar->Show(); else m_radar->Hide(); }

	void ChangeMultiFunctionDisplay(multifuncfunc_t selected);

private:
	void InitObject();

	enum MapView { MAP_SECTOR, MAP_SYSTEM, MAP_INFO, MAP_GALACTIC };

	void OnClickTimeaccel(Game::TimeAccel val);
	void OnClickComms(Gui::MultiStateImageButton *b);
	// Handler for radar view / equipment view toggle button
	void OnClickRadarEquip(Gui::MultiStateImageButton *b);

	void OnMultiFuncGrabFocus(multifuncfunc_t);
	void OnMultiFuncUngrabFocus(multifuncfunc_t);

	Game* m_game;

	enum MapView m_currentMapView;
	multifuncfunc_t m_userSelectedMfuncWidget;

	RadarWidget *m_radar;
	UseEquipWidget *m_useEquipWidget;
	Gui::Widget *m_mapViewButtons[4];

	Gui::Label *m_overlay[OVERLAY_MAX];
};

#endif /* _SHIP_CPANEL_H */
