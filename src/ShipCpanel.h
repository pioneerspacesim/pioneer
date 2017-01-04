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
	void SetAlertState(Ship::AlertState as);

	void TimeStepUpdate(float step);

	void SaveToJson(Json::Value &jsonObj);

	enum OverlayTextPos {
		OVERLAY_TOP_LEFT,
		OVERLAY_TOP_RIGHT,
		OVERLAY_BOTTOM_LEFT,
		OVERLAY_BOTTOM_RIGHT,
		OVERLAY_OVER_PANEL_RIGHT_1,
		OVERLAY_OVER_PANEL_RIGHT_2,
		OVERLAY_OVER_PANEL_RIGHT_3,
		OVERLAY_OVER_PANEL_RIGHT_4,
		OVERLAY_MAX
	};
	void SetOverlayText(OverlayTextPos pos, const std::string &text);
	void SetOverlayToolTip(OverlayTextPos pos, const std::string &text);
	void ClearOverlay();
	// Selects the specified button
	// @param int gid the buttons group (0 = left, 1 = right)
	// @param int idx the 0-based button index within the specified group
	void SelectGroupButton(int gid, int idx);

private:
	void InitObject();
	void OnRotationDampingChanged();

	enum MapView { MAP_SECTOR, MAP_SYSTEM, MAP_INFO, MAP_GALACTIC };

	void OnChangeCamView(Gui::MultiStateImageButton *b);
	void OnChangeToMapView(Gui::MultiStateImageButton *b);
	void OnChangeMapView(enum MapView);
	void OnChangeInfoView(Gui::MultiStateImageButton *b);
	void OnClickTimeaccel(Game::TimeAccel val);
	void OnClickComms(Gui::MultiStateImageButton *b);
	void OnClickRotationDamping(Gui::MultiStateImageButton *b);
	// Handler for radar view / equipment view toggle button
	void OnClickRadarEquip(Gui::MultiStateImageButton *b);

	void OnUserChangeMultiFunctionDisplay(multifuncfunc_t f);
	void ChangeMultiFunctionDisplay(multifuncfunc_t selected);
	void OnMultiFuncGrabFocus(multifuncfunc_t);
	void OnMultiFuncUngrabFocus(multifuncfunc_t);
	void HideMapviewButtons();

	Game* m_game;

	enum MapView m_currentMapView;
	multifuncfunc_t m_userSelectedMfuncWidget;
	Gui::Label *m_clock;

	sigc::connection m_connOnRotationDampingChanged;

	RadarWidget *m_radar;
	UseEquipWidget *m_useEquipWidget;
	Gui::MultiStateImageButton *m_camButton;
	Gui::MultiStateImageButton *m_radarEquipButton;
	Gui::RadioGroup *m_leftButtonGroup, *m_rightButtonGroup;
	Gui::ImageRadioButton *m_timeAccelButtons[6];
	Gui::Widget *m_mapViewButtons[4];
	Gui::MultiStateImageButton *m_rotationDampingButton;
	Gui::Image *m_alertLights[3];

	Gui::Label *m_overlay[OVERLAY_MAX];
};

#endif /* _SHIP_CPANEL_H */
