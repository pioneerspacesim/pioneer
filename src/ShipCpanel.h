// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
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
private:
	void InitObject();

	// Handler for scanner view / equipment view toggle button
	void OnClickScannerEquip(Gui::MultiStateImageButton *b);

	void OnUserChangeMultiFunctionDisplay(multifuncfunc_t f);
	void ChangeMultiFunctionDisplay(multifuncfunc_t selected);
	void OnMultiFuncGrabFocus(multifuncfunc_t);
	void OnMultiFuncUngrabFocus(multifuncfunc_t);

	Game* m_game;

	multifuncfunc_t m_userSelectedMfuncWidget;

	ScannerWidget *m_scanner;
	UseEquipWidget *m_useEquipWidget;
	Gui::MultiStateImageButton *m_scannerEquipButton;

	Gui::Label *m_overlay[OVERLAY_MAX];
};

#endif /* _SHIP_CPANEL_H */
