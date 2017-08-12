// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "Pi.h"
#include "ShipCpanel.h"
#include "Player.h"
#include "WorldView.h"
#include "SpaceStation.h"
#include "ShipCpanelMultiFuncDisplays.h"
#include "SectorView.h"
#include "SystemView.h"
#include "SystemInfoView.h"
#include "UIView.h"
#include "Lang.h"
#include "Game.h"

// XXX duplicated in WorldView. should probably be a theme variable
static const Color s_hudTextColor(0,255,0,204);

ShipCpanel::ShipCpanel(Graphics::Renderer *r, Game* game): Gui::Fixed(float(Gui::Screen::GetWidth()), 80), m_game(game)
{
	m_radar = new RadarWidget(r);

	InitObject();
}

ShipCpanel::ShipCpanel(const Json::Value &jsonObj, Graphics::Renderer *r, Game* game) : Gui::Fixed(float(Gui::Screen::GetWidth()), 80),
m_game(game)
{
	if (!jsonObj.isMember("ship_c_panel")) throw SavedGameCorruptException();
	Json::Value shipCPanelObj = jsonObj["ship_c_panel"];

	m_radar = new RadarWidget(r, shipCPanelObj);

	InitObject();
}

void ShipCpanel::InitObject()
{
	SetTransparency(true);

	Gui::Image *img = new Gui::Image("icons/cpanel.png");
	img->SetRenderDimensions(800, 80);
	Add(img, 0, 0);

	m_currentMapView = MAP_SECTOR;
	m_useEquipWidget = new UseEquipWidget();

	m_userSelectedMfuncWidget = MFUNC_RADAR;

	m_radar->onGrabFocus.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnMultiFuncGrabFocus), MFUNC_RADAR));
	m_useEquipWidget->onGrabFocus.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnMultiFuncGrabFocus), MFUNC_EQUIPMENT));

	m_radar->onUngrabFocus.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnMultiFuncUngrabFocus), MFUNC_RADAR));
	m_useEquipWidget->onUngrabFocus.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnMultiFuncUngrabFocus), MFUNC_EQUIPMENT));

	ChangeMultiFunctionDisplay(MFUNC_RADAR);

	m_overlay[OVERLAY_TOP_LEFT]     = (new Gui::Label(""))->Color(s_hudTextColor);
	m_overlay[OVERLAY_TOP_RIGHT]    = (new Gui::Label(""))->Color(s_hudTextColor);
	m_overlay[OVERLAY_BOTTOM_LEFT]  = (new Gui::Label(""))->Color(s_hudTextColor);
	m_overlay[OVERLAY_OVER_PANEL_RIGHT_1] = (new Gui::Label(""))->Color(s_hudTextColor);
	m_overlay[OVERLAY_OVER_PANEL_RIGHT_2] = (new Gui::Label(""))->Color(s_hudTextColor);
	m_overlay[OVERLAY_OVER_PANEL_RIGHT_3] = (new Gui::Label(""))->Color(s_hudTextColor);
	m_overlay[OVERLAY_OVER_PANEL_RIGHT_4] = (new Gui::Label(""))->Color(s_hudTextColor);
	Add(m_overlay[OVERLAY_TOP_LEFT],     170.0f, 2.0f);
	Add(m_overlay[OVERLAY_TOP_RIGHT],    500.0f, 2.0f);
	Add(m_overlay[OVERLAY_BOTTOM_LEFT],  150.0f, 62.0f);
	Add(m_overlay[OVERLAY_OVER_PANEL_RIGHT_1], 691.0f, -17.0f);
	Add(m_overlay[OVERLAY_OVER_PANEL_RIGHT_2], 723.0f, -17.0f);
	Add(m_overlay[OVERLAY_OVER_PANEL_RIGHT_3], 691.0f, -4.0f);
	Add(m_overlay[OVERLAY_OVER_PANEL_RIGHT_4], 723.0f, -4.0f);

	View::SetCpanel(this);
}

ShipCpanel::~ShipCpanel()
{
	View::SetCpanel(nullptr);
	Remove(m_radar);
	Remove(m_useEquipWidget);
	delete m_radar;
	delete m_useEquipWidget;
}

void ShipCpanel::ChangeMultiFunctionDisplay(multifuncfunc_t f)
{
	Gui::Widget *selected = 0;
	if (f == MFUNC_RADAR) selected = m_radar;
	if (f == MFUNC_EQUIPMENT) selected = m_useEquipWidget;

	Remove(m_radar);
	Remove(m_useEquipWidget);
	if (selected) {
		Add(selected, 200, 18);
		selected->ShowAll();
	}
	m_userSelectedMfuncWidget = f;
}

void ShipCpanel::OnMultiFuncGrabFocus(multifuncfunc_t f)
{
	ChangeMultiFunctionDisplay(f);
}

void ShipCpanel::OnMultiFuncUngrabFocus(multifuncfunc_t f)
{
	ChangeMultiFunctionDisplay(m_userSelectedMfuncWidget);
}

void ShipCpanel::Update()
{
	PROFILE_SCOPED()
	m_radar->Update();
	m_useEquipWidget->Update();

}

void ShipCpanel::Draw()
{
	Gui::Fixed::Draw();
}

void ShipCpanel::OnClickTimeaccel(Game::TimeAccel val)
{
	Pi::BoinkNoise();
	if ((m_game->GetTimeAccel() == val) && (val == Game::TIMEACCEL_PAUSED)) {
		if (Pi::GetView() != m_game->GetSettingsView())
			Pi::SetView(m_game->GetSettingsView());
		else
			Pi::SetView(m_game->GetWorldView());
	}
	else {
		if (Pi::GetView() == m_game->GetSettingsView())
			Pi::SetView(m_game->GetWorldView());
		m_game->RequestTimeAccel(val, Pi::KeyState(SDLK_LCTRL) || Pi::KeyState(SDLK_RCTRL));
	}
}

void ShipCpanel::OnClickComms(Gui::MultiStateImageButton *b)
{
	Pi::BoinkNoise();
	if (Pi::player->GetFlightState() == Ship::DOCKED)
		if (Pi::GetView() == m_game->GetSpaceStationView())
			Pi::SetView(m_game->GetWorldView());
		else
			Pi::SetView(m_game->GetSpaceStationView());
	else {
		Pi::SetView(m_game->GetWorldView());
		m_game->GetWorldView()->ToggleTargetActions();
	}
}

void ShipCpanel::TimeStepUpdate(float step)
{
	PROFILE_SCOPED()
	m_radar->TimeStepUpdate(step);
}

void ShipCpanel::SaveToJson(Json::Value &jsonObj)
{
	Json::Value shipCPanelObj(Json::objectValue); // Create JSON object to contain ship control panel data.
	m_radar->SaveToJson(shipCPanelObj);
	jsonObj["ship_c_panel"] = shipCPanelObj; // Add ship control panel object to supplied object.
}

void ShipCpanel::SetOverlayText(OverlayTextPos pos, const std::string &text)
{
	m_overlay[pos]->SetText(text);
	if (text.length() == 0)
		m_overlay[pos]->Hide();
	else
		m_overlay[pos]->Show();
}

void ShipCpanel::SetOverlayToolTip(OverlayTextPos pos, const std::string &text)
{
	m_overlay[pos]->SetToolTip(text);
}

void ShipCpanel::SetOverlayTextColour(OverlayTextPos pos, const Color &colour)
{
	m_overlay[pos]->Color(colour);
}

void ShipCpanel::ClearOverlay()
{
	for (int i = 0; i < OVERLAY_MAX; i++) {
		m_overlay[i]->SetText("");
		m_overlay[i]->SetToolTip("");
	}
}
