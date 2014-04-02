// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
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
#include "GalacticView.h"
#include "UIView.h"
#include "Lang.h"
#include "Game.h"

// XXX duplicated in WorldView. should probably be a theme variable
static const Color s_hudTextColor(0,255,0,204);

ShipCpanel::ShipCpanel(Graphics::Renderer *r): Gui::Fixed(float(Gui::Screen::GetWidth()), 80)
{
	m_scanner = new ScannerWidget(r);

	InitObject();
}

ShipCpanel::ShipCpanel(Serializer::Reader &rd, Graphics::Renderer *r): Gui::Fixed(float(Gui::Screen::GetWidth()), 80)
{
	m_scanner = new ScannerWidget(r, rd);

	InitObject();

	m_camButton->SetActiveState(rd.Int32());
}

void ShipCpanel::InitObject()
{
	SetTransparency(true);

	Gui::Image *img = new Gui::Image("icons/cpanel.png");
	img->SetRenderDimensions(800, 80);
	Add(img, 0, 0);

	m_currentMapView = MAP_SECTOR;
	m_useEquipWidget = new UseEquipWidget();
	m_msglog = new MsgLogWidget();

	m_userSelectedMfuncWidget = MFUNC_SCANNER;

	m_scanner->onGrabFocus.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnMultiFuncGrabFocus), MFUNC_SCANNER));
	m_useEquipWidget->onGrabFocus.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnMultiFuncGrabFocus), MFUNC_EQUIPMENT));
	m_msglog->onGrabFocus.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnMultiFuncGrabFocus), MFUNC_MSGLOG));

	m_scanner->onUngrabFocus.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnMultiFuncUngrabFocus), MFUNC_SCANNER));
	m_useEquipWidget->onUngrabFocus.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnMultiFuncUngrabFocus), MFUNC_EQUIPMENT));
	m_msglog->onUngrabFocus.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnMultiFuncUngrabFocus), MFUNC_MSGLOG));

	// where the scanner is
	m_mfsel = new MultiFuncSelectorWidget();
	m_mfsel->onSelect.connect(sigc::mem_fun(this, &ShipCpanel::OnUserChangeMultiFunctionDisplay));
	Add(m_mfsel, 656, 18);
	ChangeMultiFunctionDisplay(MFUNC_SCANNER);

//	Gui::RadioGroup *g = new Gui::RadioGroup();
	Gui::ImageRadioButton *b = new Gui::ImageRadioButton(0, "icons/timeaccel0.png", "icons/timeaccel0_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), Game::TIMEACCEL_PAUSED));
	b->SetShortcut(SDLK_ESCAPE, KMOD_LSHIFT);
	b->SetRenderDimensions(22, 18);
	Add(b, 0, 36);
	m_timeAccelButtons[0] = b;

	b = new Gui::ImageRadioButton(0, "icons/timeaccel1.png", "icons/timeaccel1_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), Game::TIMEACCEL_1X));
	b->SetShortcut(SDLK_F1, KMOD_LSHIFT);
	b->SetSelected(true);
	b->SetRenderDimensions(22, 18);
	Add(b, 22, 36);
	m_timeAccelButtons[1] = b;

	b = new Gui::ImageRadioButton(0, "icons/timeaccel2.png", "icons/timeaccel2_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), Game::TIMEACCEL_10X));
	b->SetShortcut(SDLK_F2, KMOD_LSHIFT);
	b->SetRenderDimensions(22, 18);
	Add(b, 44, 36);
	m_timeAccelButtons[2] = b;

	b = new Gui::ImageRadioButton(0, "icons/timeaccel3.png", "icons/timeaccel3_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), Game::TIMEACCEL_100X));
	b->SetShortcut(SDLK_F3, KMOD_LSHIFT);
	b->SetRenderDimensions(22, 18);
	Add(b, 66, 36);
	m_timeAccelButtons[3] = b;

	b = new Gui::ImageRadioButton(0, "icons/timeaccel4.png", "icons/timeaccel4_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), Game::TIMEACCEL_1000X));
	b->SetShortcut(SDLK_F4, KMOD_LSHIFT);
	b->SetRenderDimensions(22, 18);
	Add(b, 88, 36);
	m_timeAccelButtons[4] = b;

	b = new Gui::ImageRadioButton(0, "icons/timeaccel5.png", "icons/timeaccel5_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), Game::TIMEACCEL_10000X));
	b->SetShortcut(SDLK_F5, KMOD_LSHIFT);
	b->SetRenderDimensions(22, 18);
	Add(b, 110, 36);
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
	Add(m_clock, 4, 18);

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
	Add(m_rotationDampingButton, 760, 37);
	m_connOnRotationDampingChanged = Pi::player->GetPlayerController()->onRotationDampingChanged.connect(
			sigc::mem_fun(this, &ShipCpanel::OnRotationDampingChanged));

	img = new Gui::Image("icons/alert_green.png");
	img->SetToolTip(Lang::NO_ALERT);
	img->SetRenderDimensions(20, 13);
	Add(img, 780, 37);
	m_alertLights[0] = img;
	img = new Gui::Image("icons/alert_yellow.png");
	img->SetToolTip(Lang::SHIP_NEARBY);
	img->SetRenderDimensions(20, 13);
	Add(img, 780, 37);
	m_alertLights[1] = img;
	img = new Gui::Image("icons/alert_red.png");
	img->SetToolTip(Lang::LASER_FIRE_DETECTED);
	img->SetRenderDimensions(20, 13);
	Add(img, 780, 37);
	m_alertLights[2] = img;

	m_overlay[OVERLAY_TOP_LEFT]     = (new Gui::Label(""))->Color(s_hudTextColor);
	m_overlay[OVERLAY_TOP_RIGHT]    = (new Gui::Label(""))->Color(s_hudTextColor);
	m_overlay[OVERLAY_BOTTOM_LEFT]  = (new Gui::Label(""))->Color(s_hudTextColor);
	m_overlay[OVERLAY_BOTTOM_RIGHT] = (new Gui::Label(""))->Color(s_hudTextColor);
	Add(m_overlay[OVERLAY_TOP_LEFT],     170.0f, 2.0f);
	Add(m_overlay[OVERLAY_TOP_RIGHT],    500.0f, 2.0f);
	Add(m_overlay[OVERLAY_BOTTOM_LEFT],  150.0f, 62.0f);
	Add(m_overlay[OVERLAY_BOTTOM_RIGHT], 520.0f, 62.0f);
}

ShipCpanel::~ShipCpanel()
{
	delete m_leftButtonGroup;
	delete m_rightButtonGroup;
	Remove(m_scanner);
	Remove(m_useEquipWidget);
	Remove(m_msglog);
	Remove(m_mfsel);
	delete m_scanner;
	delete m_useEquipWidget;
	delete m_msglog;
	delete m_mfsel;
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
	if (f == MFUNC_MSGLOG) selected = m_msglog;

	Remove(m_scanner);
	Remove(m_useEquipWidget);
	Remove(m_msglog);
	if (selected) {
		m_mfsel->SetSelected(f);
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
	int timeAccel = Pi::game->GetTimeAccel();
	int requested = Pi::game->GetRequestedTimeAccel();

	for (int i=0; i<6; i++) {
		m_timeAccelButtons[i]->SetSelected(timeAccel == i);
	}
	// make requested but not selected icon blink
	if (timeAccel != requested) {
		m_timeAccelButtons[Clamp(requested,0,5)]->SetSelected((SDL_GetTicks() & 0x200) != 0);
	}

	m_scanner->Update();
	m_useEquipWidget->Update();
	m_msglog->Update();
}

void ShipCpanel::Draw()
{
	std::string time = format_date(Pi::game->GetTime());
	m_clock->SetText(time);

	View *cur = Pi::GetView();
	if ((cur != Pi::sectorView) && (cur != Pi::systemView) &&
	    (cur != Pi::systemInfoView) && (cur != Pi::galacticView)) {
		HideMapviewButtons();
	}

	Gui::Fixed::Draw();
}

void ShipCpanel::OnChangeCamView(Gui::MultiStateImageButton *b)
{
	Pi::BoinkNoise();
	const int newState = b->GetState();
	b->SetActiveState(newState);
	Pi::worldView->SetCamType(WorldView::CamType(newState));
	Pi::SetView(Pi::worldView);
}

void ShipCpanel::OnChangeInfoView(Gui::MultiStateImageButton *b)
{
	Pi::BoinkNoise();
	if (Pi::GetView() != Pi::infoView)
		Pi::SetView(Pi::infoView);
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
		case MAP_SECTOR: Pi::SetView(Pi::sectorView); break;
		case MAP_SYSTEM: Pi::SetView(Pi::systemView); break;
		case MAP_INFO:
			if (Pi::GetView() == Pi::systemInfoView) {
				Pi::systemInfoView->NextPage();
			} else {
				Pi::SetView(Pi::systemInfoView);
			}
			break;
		case MAP_GALACTIC: Pi::SetView(Pi::galacticView); break;
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
	if ((Pi::game->GetTimeAccel() == val) && (val == Game::TIMEACCEL_PAUSED)) {
		if (Pi::GetView() != Pi::settingsView)
			Pi::SetView(Pi::settingsView);
		else
			Pi::SetView(Pi::worldView);
	}
	else {
		if (Pi::GetView() == Pi::settingsView)
			Pi::SetView(Pi::worldView);
		Pi::game->RequestTimeAccel(val, Pi::KeyState(SDLK_LCTRL) || Pi::KeyState(SDLK_RCTRL));
	}
}

void ShipCpanel::OnClickComms(Gui::MultiStateImageButton *b)
{
	Pi::BoinkNoise();
	if (Pi::player->GetFlightState() == Ship::DOCKED) Pi::SetView(Pi::spaceStationView);
	else {
		Pi::SetView(Pi::worldView);
		Pi::worldView->ToggleTargetActions();
	}
}

void ShipCpanel::OnClickRotationDamping(Gui::MultiStateImageButton *b)
{
	Pi::player->GetPlayerController()->ToggleRotationDamping();
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

void ShipCpanel::Save(Serializer::Writer &wr)
{
	m_scanner->Save(wr);
	wr.Int32(m_camButton->GetState());
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
	for (int i = 0; i < 4; i++) {
		m_overlay[i]->SetText("");
		m_overlay[i]->SetToolTip("");
	}
}
