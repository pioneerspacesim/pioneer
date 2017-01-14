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

	if (!shipCPanelObj.isMember("cam_button_state")) throw SavedGameCorruptException();
	m_camButton->SetActiveState(shipCPanelObj["cam_button_state"].asInt());
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

	// Toggle Radar / Equipment View
	m_radarEquipButton = new Gui::MultiStateImageButton();
	m_radarEquipButton->SetShortcut(SDLK_F9, KMOD_NONE);
	m_radarEquipButton->AddState(0, "icons/multifunc_scanner.png", Lang::TOGGLE_RADAR_VIEW);
	m_radarEquipButton->AddState(1, "icons/multifunc_equip.png", Lang::TOGGLE_EQUIPMENT_VIEW);
	m_radarEquipButton->onClick.connect(sigc::mem_fun(this, &ShipCpanel::OnClickRadarEquip));
	m_radarEquipButton->SetRenderDimensions(34, 17);
	Add(m_radarEquipButton, 675, 35);
	ChangeMultiFunctionDisplay(MFUNC_RADAR);

//	Gui::RadioGroup *g = new Gui::RadioGroup();
	Gui::ImageRadioButton *b = new Gui::ImageRadioButton(0, "icons/timeaccel0.png", "icons/timeaccel0_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), Game::TIMEACCEL_PAUSED));
	b->SetShortcut(SDLK_ESCAPE, KMOD_LSHIFT);
	b->SetRenderDimensions(22, 18);
	Add(b, 0, 34);
	m_timeAccelButtons[0] = b;

	b = new Gui::ImageRadioButton(0, "icons/timeaccel1.png", "icons/timeaccel1_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), Game::TIMEACCEL_1X));
	b->SetShortcut(SDLK_F1, KMOD_LSHIFT);
	b->SetSelected(true);
	b->SetRenderDimensions(22, 18);
	Add(b, 22, 34);
	m_timeAccelButtons[1] = b;

	b = new Gui::ImageRadioButton(0, "icons/timeaccel2.png", "icons/timeaccel2_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), Game::TIMEACCEL_10X));
	b->SetShortcut(SDLK_F2, KMOD_LSHIFT);
	b->SetRenderDimensions(22, 18);
	Add(b, 44, 34);
	m_timeAccelButtons[2] = b;

	b = new Gui::ImageRadioButton(0, "icons/timeaccel3.png", "icons/timeaccel3_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), Game::TIMEACCEL_100X));
	b->SetShortcut(SDLK_F3, KMOD_LSHIFT);
	b->SetRenderDimensions(22, 18);
	Add(b, 66, 34);
	m_timeAccelButtons[3] = b;

	b = new Gui::ImageRadioButton(0, "icons/timeaccel4.png", "icons/timeaccel4_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), Game::TIMEACCEL_1000X));
	b->SetShortcut(SDLK_F4, KMOD_LSHIFT);
	b->SetRenderDimensions(22, 18);
	Add(b, 88, 34);
	m_timeAccelButtons[4] = b;

	b = new Gui::ImageRadioButton(0, "icons/timeaccel5.png", "icons/timeaccel5_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), Game::TIMEACCEL_10000X));
	b->SetShortcut(SDLK_F5, KMOD_LSHIFT);
	b->SetRenderDimensions(22, 18);
	Add(b, 110, 34);
	m_timeAccelButtons[5] = b;

	m_leftButtonGroup = new Gui::RadioGroup();
	m_camButton = new Gui::MultiStateImageButton();
	m_leftButtonGroup->Add(m_camButton);
	m_camButton->SetSelected(true);
	m_camButton->AddState(WorldView::CAM_INTERNAL, "icons/cam_internal.png", "icons/cam_internal_on.png", Lang::INTERNAL_VIEW);
	m_camButton->AddState(WorldView::CAM_EXTERNAL, "icons/cam_external.png", "icons/cam_external_on.png", Lang::EXTERNAL_VIEW);
	m_camButton->AddState(WorldView::CAM_SIDEREAL, "icons/cam_sidereal.png", "icons/cam_sidereal_on.png", Lang::SIDEREAL_VIEW);
	m_camButton->SetShortcut(SDLK_F1, KMOD_NONE);
	m_camButton->onClick.connect(sigc::mem_fun(this, &ShipCpanel::OnChangeCamView));
	m_camButton->SetRenderDimensions(30, 22);
	Add(m_camButton, 2, 56);

	Gui::MultiStateImageButton *map_button = new Gui::MultiStateImageButton();
	m_leftButtonGroup->Add(map_button);
	map_button->SetSelected(false);
	map_button->SetShortcut(SDLK_F2, KMOD_NONE);
	map_button->AddState(0, "icons/cpan_f2_map.png", "icons/cpan_f2_map_on.png", Lang::NAVIGATION_STAR_MAPS);
	map_button->onClick.connect(sigc::mem_fun(this, &ShipCpanel::OnChangeToMapView));
	map_button->SetRenderDimensions(30, 22);
	Add(map_button, 34, 56);

	Gui::MultiStateImageButton *info_button = new Gui::MultiStateImageButton();
	m_leftButtonGroup->Add(info_button);
	info_button->SetSelected(false);
	info_button->SetShortcut(SDLK_F3, KMOD_NONE);
	info_button->AddState(0, "icons/cpan_f3_shipinfo.png", "icons/cpan_f3_shipinfo_on.png", Lang::SHIP_INFORMATION);
	info_button->onClick.connect(sigc::mem_fun(this, &ShipCpanel::OnChangeInfoView));
	info_button->SetRenderDimensions(30, 22);
	Add(info_button, 66, 56);

	Gui::MultiStateImageButton *comms_button = new Gui::MultiStateImageButton();
	m_leftButtonGroup->Add(comms_button);
	comms_button->SetSelected(false);
	comms_button->SetShortcut(SDLK_F4, KMOD_NONE);
	comms_button->AddState(0, "icons/comms_f4.png", "icons/comms_f4_on.png", Lang::COMMS);
	comms_button->onClick.connect(sigc::mem_fun(this, &ShipCpanel::OnClickComms));
	comms_button->SetRenderDimensions(30, 22);
	Add(comms_button, 98, 56);

	m_clock = (new Gui::Label(""))->Color(255,178,0);
	Add(m_clock, 3, 15);

	m_rightButtonGroup = new Gui::RadioGroup();
	b = new Gui::ImageRadioButton(m_rightButtonGroup, "icons/map_sector_view.png", "icons/map_sector_view_on.png");
	m_rightButtonGroup->SetSelected(0);
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnChangeMapView), MAP_SECTOR));
	b->SetShortcut(SDLK_F5, KMOD_NONE);
	b->SetToolTip(Lang::GALAXY_SECTOR_VIEW);
	b->SetRenderDimensions(30, 22);
	Add(b, 674, 56);
	m_mapViewButtons[0] = b;
	b = new Gui::ImageRadioButton(m_rightButtonGroup, "icons/map_system_view.png", "icons/map_system_view_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnChangeMapView), MAP_SYSTEM));
	b->SetShortcut(SDLK_F6, KMOD_NONE);
	b->SetToolTip(Lang::SYSTEM_ORBIT_VIEW);
	b->SetRenderDimensions(30, 22);
	Add(b, 706, 56);
	m_mapViewButtons[1] = b;
	b = new Gui::ImageRadioButton(m_rightButtonGroup, "icons/map_sysinfo_view.png", "icons/map_sysinfo_view_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnChangeMapView), MAP_INFO));
	b->SetShortcut(SDLK_F7, KMOD_NONE);
	b->SetToolTip(Lang::STAR_SYSTEM_INFORMATION);
	b->SetRenderDimensions(30, 22);
	Add(b, 738, 56);
	m_mapViewButtons[2] = b;
	b = new Gui::ImageRadioButton(m_rightButtonGroup, "icons/map_galactic_view.png", "icons/map_galactic_view_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnChangeMapView), MAP_GALACTIC));
	b->SetShortcut(SDLK_F8, KMOD_NONE);
	b->SetToolTip(Lang::GALACTIC_VIEW);
	b->SetRenderDimensions(30, 22);
	Add(b, 770, 56);
	m_mapViewButtons[3] = b;

	m_rotationDampingButton = new Gui::MultiStateImageButton();
	m_rotationDampingButton->SetSelected(false);
	m_rotationDampingButton->AddState(0, "icons/rotation_damping_off.png", Lang::ROTATION_DAMPING_OFF);
	m_rotationDampingButton->AddState(1, "icons/rotation_damping_on.png", Lang::ROTATION_DAMPING_ON);
	m_rotationDampingButton->onClick.connect(sigc::mem_fun(this, &ShipCpanel::OnClickRotationDamping));
	m_rotationDampingButton->SetRenderDimensions(20, 13);
	m_rotationDampingButton->SetActiveState(Pi::player->GetPlayerController()->GetRotationDamping());
	Add(m_rotationDampingButton, 760, 39);
	m_connOnRotationDampingChanged = Pi::player->GetPlayerController()->onRotationDampingChanged.connect(
			sigc::mem_fun(this, &ShipCpanel::OnRotationDampingChanged));

	img = new Gui::Image("icons/alert_green.png");
	img->SetToolTip(Lang::NO_ALERT);
	img->SetRenderDimensions(20, 13);
	Add(img, 780, 39);
	m_alertLights[0] = img;
	img = new Gui::Image("icons/alert_yellow.png");
	img->SetToolTip(Lang::SHIP_NEARBY);
	img->SetRenderDimensions(20, 13);
	Add(img, 780, 39);
	m_alertLights[1] = img;
	img = new Gui::Image("icons/alert_red.png");
	img->SetToolTip(Lang::LASER_FIRE_DETECTED);
	img->SetRenderDimensions(20, 13);
	Add(img, 780, 39);
	m_alertLights[2] = img;

	m_overlay[OVERLAY_TOP_LEFT]     = (new Gui::Label(""))->Color(s_hudTextColor);
	m_overlay[OVERLAY_TOP_RIGHT]    = (new Gui::Label(""))->Color(s_hudTextColor);
	m_overlay[OVERLAY_BOTTOM_LEFT]  = (new Gui::Label(""))->Color(s_hudTextColor);
	m_overlay[OVERLAY_BOTTOM_RIGHT] = (new Gui::Label(""))->Color(s_hudTextColor);
	m_overlay[OVERLAY_OVER_PANEL_RIGHT_1] = (new Gui::Label(""))->Color(s_hudTextColor);
	m_overlay[OVERLAY_OVER_PANEL_RIGHT_2] = (new Gui::Label(""))->Color(s_hudTextColor);
	m_overlay[OVERLAY_OVER_PANEL_RIGHT_3] = (new Gui::Label(""))->Color(s_hudTextColor);
	m_overlay[OVERLAY_OVER_PANEL_RIGHT_4] = (new Gui::Label(""))->Color(s_hudTextColor);
	Add(m_overlay[OVERLAY_TOP_LEFT],     170.0f, 2.0f);
	Add(m_overlay[OVERLAY_TOP_RIGHT],    500.0f, 2.0f);
	Add(m_overlay[OVERLAY_BOTTOM_LEFT],  150.0f, 62.0f);
	Add(m_overlay[OVERLAY_BOTTOM_RIGHT], 520.0f, 62.0f);
	Add(m_overlay[OVERLAY_OVER_PANEL_RIGHT_1], 691.0f, -17.0f);
	Add(m_overlay[OVERLAY_OVER_PANEL_RIGHT_2], 723.0f, -17.0f);
	Add(m_overlay[OVERLAY_OVER_PANEL_RIGHT_3], 691.0f, -4.0f);
	Add(m_overlay[OVERLAY_OVER_PANEL_RIGHT_4], 723.0f, -4.0f);

	View::SetCpanel(this);
}

ShipCpanel::~ShipCpanel()
{
	View::SetCpanel(nullptr);
	delete m_leftButtonGroup;
	delete m_rightButtonGroup;
	Remove(m_radar);
	Remove(m_useEquipWidget);
	Remove(m_radarEquipButton);
	delete m_radar;
	delete m_useEquipWidget;
	delete m_radarEquipButton;
	m_connOnRotationDampingChanged.disconnect();
}

void ShipCpanel::OnUserChangeMultiFunctionDisplay(multifuncfunc_t f)
{
	m_userSelectedMfuncWidget = f;
	ChangeMultiFunctionDisplay(f);
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
	int timeAccel = m_game->GetTimeAccel();
	int requested = m_game->GetRequestedTimeAccel();

	for (int i=0; i<Game::TimeAccel::TIMEACCEL_HYPERSPACE; i++) {
		m_timeAccelButtons[i]->SetSelected(timeAccel == i);
	}
	// make requested but not selected icon blink
	if (timeAccel != requested) {
		m_timeAccelButtons[Clamp(requested,0,5)]->SetSelected((SDL_GetTicks() & 0x200) != 0);
	}

	m_radar->Update();
	m_useEquipWidget->Update();

	View *cur = Pi::GetView();
	if ((cur != m_game->GetSectorView()) && (cur != m_game->GetSystemView()) &&
		(cur != m_game->GetSystemInfoView()) && (cur != m_game->GetGalacticView())) {
		HideMapviewButtons();
	}
}

void ShipCpanel::Draw()
{
	static double prevTime = -1.0;
	const double currTime = m_game->GetTime();
	if(!is_equal_exact(prevTime, currTime)) {
		prevTime = currTime;
		const std::string time = format_date(currTime);
		m_clock->SetText(time);
	}

	Gui::Fixed::Draw();
}

void ShipCpanel::OnChangeCamView(Gui::MultiStateImageButton *b)
{
	Pi::BoinkNoise();
	const int newState = b->GetState();
	b->SetActiveState(newState);
	m_game->GetWorldView()->SetCamType(WorldView::CamType(newState));
	Pi::SetView(m_game->GetWorldView());
}

void ShipCpanel::OnChangeInfoView(Gui::MultiStateImageButton *b)
{
	Pi::BoinkNoise();
	if (Pi::GetView() != m_game->GetInfoView())
		Pi::SetView(m_game->GetInfoView());
	else
		Pi::SetView(m_game->GetWorldView());
}

void ShipCpanel::OnChangeToMapView(Gui::MultiStateImageButton *b)
{
	Pi::BoinkNoise();
	OnChangeMapView(m_currentMapView);
}

void ShipCpanel::OnChangeMapView(enum MapView view)
{
	m_currentMapView = view;
	switch (m_currentMapView) {
		case MAP_SECTOR: Pi::SetView(m_game->GetSectorView()); break;
		case MAP_SYSTEM: Pi::SetView(m_game->GetSystemView()); break;
		case MAP_INFO:
			if (Pi::GetView() == m_game->GetSystemInfoView()) {
				m_game->GetSystemInfoView()->NextPage();
			} else {
				Pi::SetView(m_game->GetSystemInfoView());
			}
			break;
		case MAP_GALACTIC: Pi::SetView(m_game->GetGalacticView()); break;
	}
	for (int i=0; i<4; i++) m_mapViewButtons[i]->Show();
}

void ShipCpanel::HideMapviewButtons()
{
	for (int i=0; i<4; i++) m_mapViewButtons[i]->Hide();
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

void ShipCpanel::OnClickRotationDamping(Gui::MultiStateImageButton *b)
{
	Pi::player->GetPlayerController()->ToggleRotationDamping();
}

void ShipCpanel::OnClickRadarEquip(Gui::MultiStateImageButton *b)
{
	int state = m_radarEquipButton->GetState();
	ChangeMultiFunctionDisplay((0==state) ? MFUNC_RADAR : MFUNC_EQUIPMENT);
}

void ShipCpanel::OnRotationDampingChanged()
{
	m_rotationDampingButton->SetActiveState(Pi::player->GetPlayerController()->GetRotationDamping());
}

void ShipCpanel::SetAlertState(Ship::AlertState as)
{
	switch (as) {
		case Ship::ALERT_NONE:
			m_alertLights[0]->Show();
			m_alertLights[1]->Hide();
			m_alertLights[2]->Hide();
			break;
		case Ship::ALERT_SHIP_NEARBY:
			m_alertLights[0]->Hide();
			m_alertLights[1]->Show();
			m_alertLights[2]->Hide();
			break;
		case Ship::ALERT_SHIP_FIRING:
			m_alertLights[0]->Hide();
			m_alertLights[1]->Hide();
			m_alertLights[2]->Show();
			break;
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
	shipCPanelObj["cam_button_state"] = m_camButton->GetState();
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

void ShipCpanel::ClearOverlay()
{
	for (int i = 0; i < OVERLAY_MAX; i++) {
		m_overlay[i]->SetText("");
		m_overlay[i]->SetToolTip("");
	}
}

void ShipCpanel::SelectGroupButton(int gid, int idx)
{
	Pi::BoinkNoise();
	Gui::RadioGroup* group = (gid==1) ? m_rightButtonGroup : m_leftButtonGroup;
	group->SetSelected(idx);
}
