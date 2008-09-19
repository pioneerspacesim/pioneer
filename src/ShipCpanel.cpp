#include "libs.h"
#include "Pi.h"
#include "ShipCpanel.h"
#include "SpaceStationView.h"
#include "Player.h"
#include "InfoView.h"
#include "WorldView.h"

ShipCpanel::ShipCpanel(): Gui::Fixed(640, 64)
{
	Gui::Screen::AddBaseWidget(this, 0, 0);
	SetTransparency(true);

	Gui::Image *img = new Gui::Image("icons/cpanel.png");
	Add(img, 0, 0);

	Gui::RadioGroup *g = new Gui::RadioGroup();
	Gui::ImageRadioButton *b = new Gui::ImageRadioButton(g, "icons/timeaccel0.png", "icons/timeaccel0_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), 0.0));
	b->SetShortcut(SDLK_ESCAPE, KMOD_LSHIFT);
	Add(b, 0, 26);
	
	b = new Gui::ImageRadioButton(g, "icons/timeaccel1.png", "icons/timeaccel1_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), 1.0));
	b->SetShortcut(SDLK_F1, KMOD_LSHIFT);
	b->SetSelected(true);
	Add(b, 22, 26);
	
	b = new Gui::ImageRadioButton(g, "icons/timeaccel2.png", "icons/timeaccel2_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), 10.0));
	b->SetShortcut(SDLK_F2, KMOD_LSHIFT);
	Add(b, 44, 26);
	
	b = new Gui::ImageRadioButton(g, "icons/timeaccel3.png", "icons/timeaccel3_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), 100.0));
	b->SetShortcut(SDLK_F3, KMOD_LSHIFT);
	Add(b, 66, 26);
	
	b = new Gui::ImageRadioButton(g, "icons/timeaccel4.png", "icons/timeaccel4_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), 1000.0));
	b->SetShortcut(SDLK_F4, KMOD_LSHIFT);
	Add(b, 88, 26);
	
	b = new Gui::ImageRadioButton(g, "icons/timeaccel5.png", "icons/timeaccel5_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), 10000.0));
	b->SetShortcut(SDLK_F5, KMOD_LSHIFT);
	Add(b, 110, 26);
		
	g = new Gui::RadioGroup();
	Gui::MultiStateImageButton *cam_button = new Gui::MultiStateImageButton();
	g->Add(cam_button);
	cam_button->SetSelected(true);
	cam_button->AddState(WorldView::CAM_FRONT, "icons/cam_front.png");
	cam_button->AddState(WorldView::CAM_REAR, "icons/cam_rear.png");
	cam_button->AddState(WorldView::CAM_EXTERNAL, "icons/cam_external.png");
	cam_button->SetShortcut(SDLK_F1, KMOD_NONE);
	cam_button->onClick.connect(sigc::mem_fun(this, &ShipCpanel::OnChangeCamView));
	Add(cam_button, 2, 2);

	Gui::MultiStateImageButton *map_button = new Gui::MultiStateImageButton();
	g->Add(map_button);
	map_button->SetSelected(false);
	map_button->SetShortcut(SDLK_F2, KMOD_NONE);
	map_button->AddState(Pi::MAP_SECTOR, "icons/cpan_f2_map.png");
	map_button->AddState(Pi::MAP_SYSTEM, "icons/cpan_f2_normal.png");
	map_button->onClick.connect(sigc::mem_fun(this, &ShipCpanel::OnChangeMapView));
	Add(map_button, 34, 2);

	Gui::MultiStateImageButton *info_button = new Gui::MultiStateImageButton();
	g->Add(info_button);
	info_button->SetSelected(false);
	info_button->SetShortcut(SDLK_F3, KMOD_NONE);
	info_button->AddState(0, "icons/cpan_f3_shipinfo.png");
	info_button->onClick.connect(sigc::mem_fun(this, &ShipCpanel::OnChangeInfoView));
	Add(info_button, 66, 2);

	Gui::MultiStateImageButton *comms_button = new Gui::MultiStateImageButton();
	g->Add(comms_button);
	comms_button->SetSelected(false);
	comms_button->SetShortcut(SDLK_F4, KMOD_NONE);
	comms_button->AddState(0, "icons/comms_f4.png");
	comms_button->onClick.connect(sigc::mem_fun(this, &ShipCpanel::OnClickComms));
	Add(comms_button, 98, 2);

	m_clock = new Gui::Label("");
	m_clock->SetColor(1,0.7,0);
	Add(m_clock, 2, 48);

	tempMsg = new Gui::Label("");
	Add(tempMsg, 170, 44);
}

void ShipCpanel::SetTemporaryMessage(Body * const sender, std::string msg)
{
	std::string poo = "#0f0"+msg;
	if (sender) {
		poo = std::string("#ca0")+"Message from "+sender->GetLabel()+":\n"+poo;
	}
	tempMsg->SetText(poo);
	tempMsgAge = Pi::GetGameTime();
}

void ShipCpanel::Draw()
{
	std::string time = date_format(Pi::GetGameTime());
	m_clock->SetText(time);

	if (tempMsgAge) {
		if (Pi::GetGameTime() - tempMsgAge > 5.0) {
			tempMsg->SetText("");
			tempMsgAge = 0;
		}
	}

	Gui::Fixed::Draw();
	Remove(m_scannerWidget);
}

void ShipCpanel::SetScannerWidget(Widget *w)
{
	m_scannerWidget = w;
	Add(w, 150, 64);
	w->Show();
}

void ShipCpanel::OnChangeCamView(Gui::MultiStateImageButton *b)
{
	Pi::worldView->SetCamType((enum WorldView::CamType)b->GetState());
	Pi::SetView(Pi::worldView);
}

void ShipCpanel::OnChangeInfoView(Gui::MultiStateImageButton *b)
{
	Pi::infoView->UpdateInfo();
	Pi::SetView(Pi::infoView);
}

void ShipCpanel::OnChangeMapView(Gui::MultiStateImageButton *b)
{
	Pi::SetMapView((enum Pi::MapView)b->GetState());
}

void ShipCpanel::OnClickTimeaccel(Gui::ISelectable *i, double step)
{
	Pi::SetTimeAccel(step);
}

void ShipCpanel::OnClickComms(Gui::MultiStateImageButton *b)
{
	if (Pi::player->GetDockedWith()) Pi::SetView(Pi::spaceStationView);
}

