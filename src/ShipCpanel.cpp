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

	Add(m_radar, 200, 18);
	m_radar->ShowAll();
}

ShipCpanel::ShipCpanel(const Json::Value &jsonObj, Graphics::Renderer *r, Game* game) : Gui::Fixed(float(Gui::Screen::GetWidth()), 80),
m_game(game)
{
	if (!jsonObj.isMember("ship_c_panel")) throw SavedGameCorruptException();
	Json::Value shipCPanelObj = jsonObj["ship_c_panel"];

	m_radar = new RadarWidget(r, shipCPanelObj);

	InitObject();

	Add(m_radar, 200, 18);
	m_radar->ShowAll();
}

void ShipCpanel::InitObject()
{
	SetTransparency(true);

	Gui::Image *img = new Gui::Image("icons/cpanel.png");
	img->SetRenderDimensions(800, 80);
	Add(img, 0, 0);

	View::SetCpanel(this);
}

ShipCpanel::~ShipCpanel()
{
	View::SetCpanel(nullptr);
	Remove(m_radar);
	delete m_radar;
}

void ShipCpanel::Update()
{
	PROFILE_SCOPED()
	m_radar->Update();
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
