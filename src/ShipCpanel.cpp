// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
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
	m_scanner = new ScannerWidget(r);

	InitObject();
}

ShipCpanel::ShipCpanel(const Json::Value &jsonObj, Graphics::Renderer *r, Game* game) : Gui::Fixed(float(Gui::Screen::GetWidth()), 80),
m_game(game)
{
	if (!jsonObj.isMember("ship_c_panel")) throw SavedGameCorruptException();
	Json::Value shipCPanelObj = jsonObj["ship_c_panel"];

	m_scanner = new ScannerWidget(r, shipCPanelObj);

	InitObject();

	if (!shipCPanelObj.isMember("cam_button_state")) throw SavedGameCorruptException();
}

void ShipCpanel::InitObject()
{
	SetTransparency(true);

	Gui::Image *img = new Gui::Image("icons/cpanel.png");
	img->SetRenderDimensions(800, 80);
	Add(img, 0, 0);

	m_currentMapView = MAP_SECTOR;
	m_useEquipWidget = new UseEquipWidget();

	m_userSelectedMfuncWidget = MFUNC_SCANNER;

	m_scanner->onGrabFocus.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnMultiFuncGrabFocus), MFUNC_SCANNER));
	m_useEquipWidget->onGrabFocus.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnMultiFuncGrabFocus), MFUNC_EQUIPMENT));

	m_scanner->onUngrabFocus.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnMultiFuncUngrabFocus), MFUNC_SCANNER));
	m_useEquipWidget->onUngrabFocus.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnMultiFuncUngrabFocus), MFUNC_EQUIPMENT));

	// Toggle Scanner / Equipment View
	m_scannerEquipButton = new Gui::MultiStateImageButton();
	m_scannerEquipButton->SetShortcut(SDLK_F9, KMOD_NONE);
	m_scannerEquipButton->AddState(0, "icons/multifunc_scanner.png", Lang::TOGGLE_SCANNER_VIEW);
	m_scannerEquipButton->AddState(1, "icons/multifunc_equip.png", Lang::TOGGLE_EQUIPMENT_VIEW);
	m_scannerEquipButton->onClick.connect(sigc::mem_fun(this, &ShipCpanel::OnClickScannerEquip));
	m_scannerEquipButton->SetRenderDimensions(34, 17);
	Add(m_scannerEquipButton, 675, 35);
	ChangeMultiFunctionDisplay(MFUNC_SCANNER);

//	Gui::RadioGroup *g = new Gui::RadioGroup();
	Gui::ImageRadioButton *b = new Gui::ImageRadioButton(0, "icons/timeaccel0.png", "icons/timeaccel0_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), Game::TIMEACCEL_PAUSED));
	b->SetShortcut(SDLK_ESCAPE, KMOD_LSHIFT);
	b->SetRenderDimensions(22, 18);
	Add(b, 0, 60);
	m_timeAccelButtons[0] = b;

	b = new Gui::ImageRadioButton(0, "icons/timeaccel1.png", "icons/timeaccel1_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), Game::TIMEACCEL_1X));
	b->SetShortcut(SDLK_F1, KMOD_LSHIFT);
	b->SetSelected(true);
	b->SetRenderDimensions(22, 18);
	Add(b, 22, 60);
	m_timeAccelButtons[1] = b;

	b = new Gui::ImageRadioButton(0, "icons/timeaccel2.png", "icons/timeaccel2_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), Game::TIMEACCEL_10X));
	b->SetShortcut(SDLK_F2, KMOD_LSHIFT);
	b->SetRenderDimensions(22, 18);
	Add(b, 44, 60);
	m_timeAccelButtons[2] = b;

	b = new Gui::ImageRadioButton(0, "icons/timeaccel3.png", "icons/timeaccel3_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), Game::TIMEACCEL_100X));
	b->SetShortcut(SDLK_F3, KMOD_LSHIFT);
	b->SetRenderDimensions(22, 18);
	Add(b, 66, 60);
	m_timeAccelButtons[3] = b;

	b = new Gui::ImageRadioButton(0, "icons/timeaccel4.png", "icons/timeaccel4_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), Game::TIMEACCEL_1000X));
	b->SetShortcut(SDLK_F4, KMOD_LSHIFT);
	b->SetRenderDimensions(22, 18);
	Add(b, 88, 60);
	m_timeAccelButtons[4] = b;

	b = new Gui::ImageRadioButton(0, "icons/timeaccel5.png", "icons/timeaccel5_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), Game::TIMEACCEL_10000X));
	b->SetShortcut(SDLK_F5, KMOD_LSHIFT);
	b->SetRenderDimensions(22, 18);
	Add(b, 110, 60);
	m_timeAccelButtons[5] = b;

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
	delete m_rightButtonGroup;
	Remove(m_scanner);
	Remove(m_useEquipWidget);
	Remove(m_scannerEquipButton);
	delete m_scanner;
	delete m_useEquipWidget;
	delete m_scannerEquipButton;
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
	if (f == MFUNC_SCANNER) selected = m_scanner;
	if (f == MFUNC_EQUIPMENT) selected = m_useEquipWidget;

	Remove(m_scanner);
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

	m_scanner->Update();
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

void ShipCpanel::OnClickScannerEquip(Gui::MultiStateImageButton *b)
{
	int state = m_scannerEquipButton->GetState();
	ChangeMultiFunctionDisplay((0==state) ? MFUNC_SCANNER : MFUNC_EQUIPMENT);
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
	m_scanner->TimeStepUpdate(step);
}

void ShipCpanel::SaveToJson(Json::Value &jsonObj)
{
	Json::Value shipCPanelObj(Json::objectValue); // Create JSON object to contain ship control panel data.
	m_scanner->SaveToJson(shipCPanelObj);
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
	Gui::RadioGroup* group = m_rightButtonGroup;
	group->SetSelected(idx);
}
