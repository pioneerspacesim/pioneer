#include "libs.h"
#include "Pi.h"
#include "ShipCpanel.h"
#include "SpaceStationView.h"
#include "Player.h"
#include "InfoView.h"
#include "WorldView.h"
#include "SpaceStation.h"
#include "ShipCpanelMultiFuncDisplays.h"
#include "SectorView.h"
#include "SystemView.h"
#include "SystemInfoView.h"
#include "GalacticView.h"
#include "GameMenuView.h"
#include "Lang.h"
#include "Game.h"

ShipCpanel::ShipCpanel(): Gui::Fixed(float(Gui::Screen::GetWidth()), 80)
{
	m_scanner = new ScannerWidget();

	InitObject();
}

ShipCpanel::ShipCpanel(Serializer::Reader &rd): Gui::Fixed(float(Gui::Screen::GetWidth()), 80)
{
	m_scanner = new ScannerWidget(rd);

	InitObject();
}

void ShipCpanel::InitObject()
{
	Gui::Screen::AddBaseWidget(this, 0, Gui::Screen::GetHeight()-80);
	SetTransparency(true);

	Gui::Image *img = new Gui::Image(PIONEER_DATA_DIR "/icons/cpanel.png");
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
	Gui::ImageRadioButton *b = new Gui::ImageRadioButton(0, PIONEER_DATA_DIR "/icons/timeaccel0.png", PIONEER_DATA_DIR "/icons/timeaccel0_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), Game::TIMEACCEL_PAUSED));
	b->SetShortcut(SDLK_ESCAPE, KMOD_LSHIFT);
	Add(b, 0, 36);
	m_timeAccelButtons[0] = b;
	
	b = new Gui::ImageRadioButton(0, PIONEER_DATA_DIR "/icons/timeaccel1.png", PIONEER_DATA_DIR "/icons/timeaccel1_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), Game::TIMEACCEL_1X));
	b->SetShortcut(SDLK_F1, KMOD_LSHIFT);
	b->SetSelected(true);
	Add(b, 22, 36);
	m_timeAccelButtons[1] = b;
	
	b = new Gui::ImageRadioButton(0, PIONEER_DATA_DIR "/icons/timeaccel2.png", PIONEER_DATA_DIR "/icons/timeaccel2_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), Game::TIMEACCEL_10X));
	b->SetShortcut(SDLK_F2, KMOD_LSHIFT);
	Add(b, 44, 36);
	m_timeAccelButtons[2] = b;
	
	b = new Gui::ImageRadioButton(0, PIONEER_DATA_DIR "/icons/timeaccel3.png", PIONEER_DATA_DIR "/icons/timeaccel3_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), Game::TIMEACCEL_100X));
	b->SetShortcut(SDLK_F3, KMOD_LSHIFT);
	Add(b, 66, 36);
	m_timeAccelButtons[3] = b;
	
	b = new Gui::ImageRadioButton(0, PIONEER_DATA_DIR "/icons/timeaccel4.png", PIONEER_DATA_DIR "/icons/timeaccel4_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), Game::TIMEACCEL_1000X));
	b->SetShortcut(SDLK_F4, KMOD_LSHIFT);
	Add(b, 88, 36);
	m_timeAccelButtons[4] = b;
	
	b = new Gui::ImageRadioButton(0, PIONEER_DATA_DIR "/icons/timeaccel5.png", PIONEER_DATA_DIR "/icons/timeaccel5_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), Game::TIMEACCEL_10000X));
	b->SetShortcut(SDLK_F5, KMOD_LSHIFT);
	Add(b, 110, 36);
	m_timeAccelButtons[5] = b;
		
	m_leftButtonGroup = new Gui::RadioGroup();
	Gui::MultiStateImageButton *cam_button = new Gui::MultiStateImageButton();
	m_leftButtonGroup->Add(cam_button);
	cam_button->SetSelected(true);
	cam_button->AddState(WorldView::CAM_FRONT, PIONEER_DATA_DIR "/icons/cam_front.png", PIONEER_DATA_DIR "/icons/cam_front_on.png", Lang::FRONT_VIEW);
	cam_button->AddState(WorldView::CAM_REAR, PIONEER_DATA_DIR "/icons/cam_rear.png", PIONEER_DATA_DIR "/icons/cam_rear_on.png", Lang::REAR_VIEW);
	cam_button->AddState(WorldView::CAM_EXTERNAL, PIONEER_DATA_DIR "/icons/cam_external.png", PIONEER_DATA_DIR "/icons/cam_external_on.png", Lang::EXTERNAL_VIEW);
	cam_button->SetShortcut(SDLK_F1, KMOD_NONE);
	cam_button->onClick.connect(sigc::mem_fun(this, &ShipCpanel::OnChangeCamView));
	Add(cam_button, 2, 56);

	Gui::MultiStateImageButton *map_button = new Gui::MultiStateImageButton();
	m_leftButtonGroup->Add(map_button);
	map_button->SetSelected(false);
	map_button->SetShortcut(SDLK_F2, KMOD_NONE);
	map_button->AddState(0, PIONEER_DATA_DIR "/icons/cpan_f2_map.png", PIONEER_DATA_DIR "/icons/cpan_f2_map_on.png", Lang::NAVIGATION_STAR_MAPS);
	map_button->onClick.connect(sigc::mem_fun(this, &ShipCpanel::OnChangeToMapView));
	Add(map_button, 34, 56);

	Gui::MultiStateImageButton *info_button = new Gui::MultiStateImageButton();
	m_leftButtonGroup->Add(info_button);
	info_button->SetSelected(false);
	info_button->SetShortcut(SDLK_F3, KMOD_NONE);
	info_button->AddState(0, PIONEER_DATA_DIR "/icons/cpan_f3_shipinfo.png", PIONEER_DATA_DIR "/icons/cpan_f3_shipinfo_on.png", Lang::SHIP_INFORMATION);
	info_button->onClick.connect(sigc::mem_fun(this, &ShipCpanel::OnChangeInfoView));
	Add(info_button, 66, 56);

	Gui::MultiStateImageButton *comms_button = new Gui::MultiStateImageButton();
	m_leftButtonGroup->Add(comms_button);
	comms_button->SetSelected(false);
	comms_button->SetShortcut(SDLK_F4, KMOD_NONE);
	comms_button->AddState(0, PIONEER_DATA_DIR "/icons/comms_f4.png", PIONEER_DATA_DIR "/icons/comms_f4_on.png", Lang::COMMS);
	comms_button->onClick.connect(sigc::mem_fun(this, &ShipCpanel::OnClickComms));
	Add(comms_button, 98, 56);

	m_clock = (new Gui::Label(""))->Color(1.0f,0.7f,0.0f);
	Add(m_clock, 4, 18);
	
	m_rightButtonGroup = new Gui::RadioGroup();
	b = new Gui::ImageRadioButton(m_rightButtonGroup, PIONEER_DATA_DIR "/icons/map_sector_view.png", PIONEER_DATA_DIR "/icons/map_sector_view_on.png");
	m_rightButtonGroup->SetSelected(0);
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnChangeMapView), MAP_SECTOR));
	b->SetShortcut(SDLK_F5, KMOD_NONE);
	b->SetToolTip(Lang::GALAXY_SECTOR_VIEW);
	Add(b, 674, 56);
	m_mapViewButtons[0] = b;
	b = new Gui::ImageRadioButton(m_rightButtonGroup, PIONEER_DATA_DIR "/icons/map_system_view.png", PIONEER_DATA_DIR "/icons/map_system_view_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnChangeMapView), MAP_SYSTEM));
	b->SetShortcut(SDLK_F6, KMOD_NONE);
	b->SetToolTip(Lang::SYSTEM_ORBIT_VIEW);
	Add(b, 706, 56);
	m_mapViewButtons[1] = b;
	b = new Gui::ImageRadioButton(m_rightButtonGroup, PIONEER_DATA_DIR "/icons/map_sysinfo_view.png", PIONEER_DATA_DIR "/icons/map_sysinfo_view_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnChangeMapView), MAP_INFO));
	b->SetShortcut(SDLK_F7, KMOD_NONE);
	b->SetToolTip(Lang::STAR_SYSTEM_INFORMATION);
	Add(b, 738, 56);
	m_mapViewButtons[2] = b;
	b = new Gui::ImageRadioButton(m_rightButtonGroup, PIONEER_DATA_DIR "/icons/map_galactic_view.png", PIONEER_DATA_DIR "/icons/map_galactic_view_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnChangeMapView), MAP_GALACTIC));
	b->SetShortcut(SDLK_F8, KMOD_NONE);
	b->SetToolTip(Lang::GALACTIC_VIEW);
	Add(b, 770, 56);
	m_mapViewButtons[3] = b;

	img = new Gui::Image(PIONEER_DATA_DIR "/icons/alert_green.png");
	img->SetToolTip(Lang::NO_ALERT);
	Add(img, 780, 37);
	m_alertLights[0] = img;
	img = new Gui::Image(PIONEER_DATA_DIR "/icons/alert_yellow.png");
	img->SetToolTip(Lang::SHIP_NEARBY);
	Add(img, 780, 37);
	m_alertLights[1] = img;
	img = new Gui::Image(PIONEER_DATA_DIR "/icons/alert_red.png");
	img->SetToolTip(Lang::LASER_FIRE_DETECTED);
	Add(img, 780, 37);
	m_alertLights[2] = img;

	m_connOnDockingClearanceExpired =
		Pi::onDockingClearanceExpired.connect(sigc::mem_fun(this, &ShipCpanel::OnDockingClearanceExpired));
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
	m_connOnDockingClearanceExpired.disconnect();
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

void ShipCpanel::OnDockingClearanceExpired(const SpaceStation *s)
{
	MsgLog()->ImportantMessage(s->GetLabel(), Lang::DOCKING_CLEARANCE_EXPIRED);
}

void ShipCpanel::Update()
{
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
	Pi::worldView->SetCamType(WorldView::CamType(b->GetState()));
	Pi::SetView(Pi::worldView);
}

void ShipCpanel::OnChangeInfoView(Gui::MultiStateImageButton *b)
{
	Pi::BoinkNoise();
	if (Pi::GetView() == Pi::infoView) {
		Pi::infoView->NextPage();
	} else {
		Pi::infoView->UpdateInfo();
		Pi::SetView(Pi::infoView);
	}
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
		if (Pi::GetView() != Pi::gameMenuView)
			Pi::SetView(Pi::gameMenuView);
		else
			Pi::SetView(Pi::worldView);
	}
	else {
		if (Pi::GetView() == Pi::gameMenuView)
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
	m_scanner->TimeStepUpdate(step);
}

void ShipCpanel::Save(Serializer::Writer &wr)
{
	m_scanner->Save(wr);
}
