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
#include "GenericSystemView.h"

ShipCpanel::ShipCpanel(): Gui::Fixed((float)Gui::Screen::GetWidth(), 80)
{
	Gui::Screen::AddBaseWidget(this, 0, Gui::Screen::GetHeight()-80);
	SetTransparency(true);

	Gui::Image *img = new Gui::Image(PIONEER_DATA_DIR "/icons/cpanel.png");
	Add(img, 0, 0);

	m_scanner = new ScannerWidget();
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
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), 0));
	b->SetShortcut(SDLK_ESCAPE, KMOD_LSHIFT);
	Add(b, 0, 36);
	m_timeAccelButtons[0] = b;
	
	b = new Gui::ImageRadioButton(0, PIONEER_DATA_DIR "/icons/timeaccel1.png", PIONEER_DATA_DIR "/icons/timeaccel1_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), 1));
	b->SetShortcut(SDLK_F1, KMOD_LSHIFT);
	b->SetSelected(true);
	Add(b, 22, 36);
	m_timeAccelButtons[1] = b;
	
	b = new Gui::ImageRadioButton(0, PIONEER_DATA_DIR "/icons/timeaccel2.png", PIONEER_DATA_DIR "/icons/timeaccel2_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), 2));
	b->SetShortcut(SDLK_F2, KMOD_LSHIFT);
	Add(b, 44, 36);
	m_timeAccelButtons[2] = b;
	
	b = new Gui::ImageRadioButton(0, PIONEER_DATA_DIR "/icons/timeaccel3.png", PIONEER_DATA_DIR "/icons/timeaccel3_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), 3));
	b->SetShortcut(SDLK_F3, KMOD_LSHIFT);
	Add(b, 66, 36);
	m_timeAccelButtons[3] = b;
	
	b = new Gui::ImageRadioButton(0, PIONEER_DATA_DIR "/icons/timeaccel4.png", PIONEER_DATA_DIR "/icons/timeaccel4_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), 4));
	b->SetShortcut(SDLK_F4, KMOD_LSHIFT);
	Add(b, 88, 36);
	m_timeAccelButtons[4] = b;
	
	b = new Gui::ImageRadioButton(0, PIONEER_DATA_DIR "/icons/timeaccel5.png", PIONEER_DATA_DIR "/icons/timeaccel5_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), 5));
	b->SetShortcut(SDLK_F5, KMOD_LSHIFT);
	Add(b, 110, 36);
	m_timeAccelButtons[5] = b;
		
	Gui::RadioGroup *g = new Gui::RadioGroup();
	Gui::MultiStateImageButton *cam_button = new Gui::MultiStateImageButton();
	g->Add(cam_button);
	cam_button->SetSelected(true);
	cam_button->AddState(WorldView::CAM_FRONT, PIONEER_DATA_DIR "/icons/cam_front.png", PIONEER_DATA_DIR "/icons/cam_front_on.png", "Front view");
	cam_button->AddState(WorldView::CAM_REAR, PIONEER_DATA_DIR "/icons/cam_rear.png", PIONEER_DATA_DIR "/icons/cam_rear_on.png", "Rear view");
	cam_button->AddState(WorldView::CAM_EXTERNAL, PIONEER_DATA_DIR "/icons/cam_external.png", PIONEER_DATA_DIR "/icons/cam_external_on.png", "External view");
	cam_button->SetShortcut(SDLK_F1, KMOD_NONE);
	cam_button->onClick.connect(sigc::mem_fun(this, &ShipCpanel::OnChangeCamView));
	Add(cam_button, 2, 56);

	Gui::MultiStateImageButton *map_button = new Gui::MultiStateImageButton();
	g->Add(map_button);
	map_button->SetSelected(false);
	map_button->SetShortcut(SDLK_F2, KMOD_NONE);
	map_button->AddState(0, PIONEER_DATA_DIR "/icons/cpan_f2_map.png", PIONEER_DATA_DIR "/icons/cpan_f2_map_on.png", "Navigation and star maps");
	map_button->onClick.connect(sigc::mem_fun(this, &ShipCpanel::OnChangeMapView));
	Add(map_button, 34, 56);

	Gui::MultiStateImageButton *info_button = new Gui::MultiStateImageButton();
	g->Add(info_button);
	info_button->SetSelected(false);
	info_button->SetShortcut(SDLK_F3, KMOD_NONE);
	info_button->AddState(0, PIONEER_DATA_DIR "/icons/cpan_f3_shipinfo.png", PIONEER_DATA_DIR "/icons/cpan_f3_shipinfo_on.png", "Ship information");
	info_button->onClick.connect(sigc::mem_fun(this, &ShipCpanel::OnChangeInfoView));
	Add(info_button, 66, 56);

	Gui::MultiStateImageButton *comms_button = new Gui::MultiStateImageButton();
	g->Add(comms_button);
	comms_button->SetSelected(false);
	comms_button->SetShortcut(SDLK_F4, KMOD_NONE);
	comms_button->AddState(0, PIONEER_DATA_DIR "/icons/comms_f4.png", PIONEER_DATA_DIR "/icons/comms_f4_on.png", "Comms");
	comms_button->onClick.connect(sigc::mem_fun(this, &ShipCpanel::OnClickComms));
	Add(comms_button, 98, 56);

	m_clock = (new Gui::Label(""))->Color(1.0f,0.7f,0.0f);
	Add(m_clock, 4, 18);

	m_connOnDockingClearanceExpired =
		Pi::onDockingClearanceExpired.connect(sigc::mem_fun(this, &ShipCpanel::OnDockingClearanceExpired));
}

ShipCpanel::~ShipCpanel()
{
	Remove(m_scanner);
	Remove(m_useEquipWidget);
	Remove(m_msglog);
	delete m_scanner;
	delete m_useEquipWidget;
	delete m_msglog;
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
	MsgLog()->ImportantMessage(s->GetLabel(), "Docking clearance expired. If you wish to dock you must repeat your request.");
}

void ShipCpanel::Update()
{
	int timeAccel = Pi::GetTimeAccelIdx();
	int requested = Pi::GetRequestedTimeAccelIdx();

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
	std::string time = format_date(Pi::GetGameTime());
	m_clock->SetText(time);

	Gui::Fixed::Draw();
}

void ShipCpanel::OnChangeCamView(Gui::MultiStateImageButton *b)
{
	Pi::BoinkNoise();
	Pi::worldView->SetCamType((enum WorldView::CamType)b->GetState());
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

void ShipCpanel::OnChangeMapView(Gui::MultiStateImageButton *b)
{
	Pi::BoinkNoise();
	GenericSystemView::SwitchToCurrentView();
}

void ShipCpanel::OnClickTimeaccel(int val)
{
	Pi::BoinkNoise();
	/* May not happen, as time accel is limited by proximity to stuff */
	Pi::RequestTimeAccel(val);
}

void ShipCpanel::OnClickComms(Gui::MultiStateImageButton *b)
{
	Pi::BoinkNoise();
	if (Pi::player->GetDockedWith()) Pi::SetView(Pi::spaceStationView);
	else {
		Pi::SetView(Pi::worldView);
		Pi::worldView->ToggleTargetActions();
	}
}

