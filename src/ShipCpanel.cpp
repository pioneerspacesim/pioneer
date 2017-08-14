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

	m_useEquipWidget = new UseEquipWidget();

	m_userSelectedMfuncWidget = MFUNC_RADAR;

	m_radar->onGrabFocus.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnMultiFuncGrabFocus), MFUNC_RADAR));
	m_useEquipWidget->onGrabFocus.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnMultiFuncGrabFocus), MFUNC_EQUIPMENT));

	m_radar->onUngrabFocus.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnMultiFuncUngrabFocus), MFUNC_RADAR));
	m_useEquipWidget->onUngrabFocus.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnMultiFuncUngrabFocus), MFUNC_EQUIPMENT));

	ChangeMultiFunctionDisplay(MFUNC_RADAR);

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
