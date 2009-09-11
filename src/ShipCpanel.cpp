#include "libs.h"
#include "Pi.h"
#include "ShipCpanel.h"
#include "SpaceStationView.h"
#include "Player.h"
#include "InfoView.h"
#include "WorldView.h"
#include "SpaceStation.h"
#include "ShipCpanelMultiFuncDisplays.h"

ShipCpanel::ShipCpanel(): Gui::Fixed((float)Gui::Screen::GetWidth(), 64)
{
	Gui::Screen::AddBaseWidget(this, 0, Gui::Screen::GetHeight()-64);
	SetTransparency(true);

	Gui::Image *img = new Gui::Image("icons/cpanel.png");
	Add(img, 0, 0);

	m_scanner = new ScannerWidget();
	m_msglog = new MsgLogWidget();
	// where the scanner is
	MultiFuncSelectorWidget *mfsel = new MultiFuncSelectorWidget();
	mfsel->onSelect.connect(sigc::mem_fun(this, &ShipCpanel::OnChangeMultiFunctionDisplay));
	Add(mfsel, 656, 2);
	OnChangeMultiFunctionDisplay(MFUNC_SCANNER);

	Gui::RadioGroup *g = new Gui::RadioGroup();
	Gui::ImageRadioButton *b = new Gui::ImageRadioButton(g, "icons/timeaccel0.png", "icons/timeaccel0_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), 0.0));
	b->SetShortcut(SDLK_ESCAPE, KMOD_LSHIFT);
	Add(b, 0, 20);
	
	b = new Gui::ImageRadioButton(g, "icons/timeaccel1.png", "icons/timeaccel1_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), 1.0));
	b->SetShortcut(SDLK_F1, KMOD_LSHIFT);
	b->SetSelected(true);
	Add(b, 22, 20);
	
	b = new Gui::ImageRadioButton(g, "icons/timeaccel2.png", "icons/timeaccel2_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), 10.0));
	b->SetShortcut(SDLK_F2, KMOD_LSHIFT);
	Add(b, 44, 20);
	
	b = new Gui::ImageRadioButton(g, "icons/timeaccel3.png", "icons/timeaccel3_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), 100.0));
	b->SetShortcut(SDLK_F3, KMOD_LSHIFT);
	Add(b, 66, 20);
	
	b = new Gui::ImageRadioButton(g, "icons/timeaccel4.png", "icons/timeaccel4_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), 1000.0));
	b->SetShortcut(SDLK_F4, KMOD_LSHIFT);
	Add(b, 88, 20);
	
	b = new Gui::ImageRadioButton(g, "icons/timeaccel5.png", "icons/timeaccel5_on.png");
	b->onSelect.connect(sigc::bind(sigc::mem_fun(this, &ShipCpanel::OnClickTimeaccel), 10000.0));
	b->SetShortcut(SDLK_F5, KMOD_LSHIFT);
	Add(b, 110, 20);
		
	g = new Gui::RadioGroup();
	Gui::MultiStateImageButton *cam_button = new Gui::MultiStateImageButton();
	g->Add(cam_button);
	cam_button->SetSelected(true);
	cam_button->AddState(WorldView::CAM_FRONT, "icons/cam_front.png", "Front view");
	cam_button->AddState(WorldView::CAM_REAR, "icons/cam_rear.png", "Rear view");
	cam_button->AddState(WorldView::CAM_EXTERNAL, "icons/cam_external.png", "External view");
	cam_button->SetShortcut(SDLK_F1, KMOD_NONE);
	cam_button->onClick.connect(sigc::mem_fun(this, &ShipCpanel::OnChangeCamView));
	Add(cam_button, 2, 40);

	Gui::MultiStateImageButton *map_button = new Gui::MultiStateImageButton();
	g->Add(map_button);
	map_button->SetSelected(false);
	map_button->SetShortcut(SDLK_F2, KMOD_NONE);
	map_button->AddState(Pi::MAP_SECTOR, "icons/cpan_f2_map.png", "Galaxy sector map");
	map_button->AddState(Pi::MAP_SYSTEM, "icons/cpan_f2_normal.png", "Star system view");
	map_button->onClick.connect(sigc::mem_fun(this, &ShipCpanel::OnChangeMapView));
	Add(map_button, 34, 40);

	Gui::MultiStateImageButton *info_button = new Gui::MultiStateImageButton();
	g->Add(info_button);
	info_button->SetSelected(false);
	info_button->SetShortcut(SDLK_F3, KMOD_NONE);
	info_button->AddState(0, "icons/cpan_f3_shipinfo.png", "Ship information");
	info_button->onClick.connect(sigc::mem_fun(this, &ShipCpanel::OnChangeInfoView));
	Add(info_button, 66, 40);

	Gui::MultiStateImageButton *comms_button = new Gui::MultiStateImageButton();
	g->Add(comms_button);
	comms_button->SetSelected(false);
	comms_button->SetShortcut(SDLK_F4, KMOD_NONE);
	comms_button->AddState(0, "icons/comms_f4.png", "Comms");
	comms_button->onClick.connect(sigc::mem_fun(this, &ShipCpanel::OnClickComms));
	Add(comms_button, 98, 40);

	m_clock = new Gui::Label("");
	m_clock->SetColor(1,0.7,0);
	Add(m_clock, 4, 2);

	tempMsgAge = 0;
	tempMsg = new Gui::Label("");
	Add(tempMsg, 170, 4);

	m_connOnDockingClearanceExpired =
		Pi::onDockingClearanceExpired.connect(sigc::mem_fun(this, &ShipCpanel::OnDockingClearanceExpired));
}

ShipCpanel::~ShipCpanel()
{
	Remove(m_scanner);
	Remove(m_msglog);
	delete m_scanner;
	delete m_msglog;
	m_connOnDockingClearanceExpired.disconnect();
}

void ShipCpanel::OnChangeMultiFunctionDisplay(multifuncfunc_t f)
{
	Gui::Widget *selected = 0;

	Remove(m_scanner);
	Remove(m_msglog);

	if (f == MFUNC_SCANNER) selected = m_scanner;
	if (f == MFUNC_MSGLOG) selected = m_msglog;
	if (selected) {
		selected->ShowAll();
		Add(selected, 200, 2);
	}
}

void ShipCpanel::OnDockingClearanceExpired(const SpaceStation *s)
{
	SetTemporaryMessage(static_cast<const Body*>(s), "Docking clearance expired. If you wish to dock you must repeat your request.");
}

void ShipCpanel::SetTemporaryMessage(const Body *sender, const std::string &msg)
{
	m_msgQueue.push_back(QueuedMsg(sender ? sender->GetLabel() : "", msg));
}

void ShipCpanel::SetTemporaryMessage(const std::string &sender, const std::string &msg)
{
	m_msgQueue.push_back(QueuedMsg(sender, msg));
}

void ShipCpanel::Draw()
{
	std::string time = format_date(Pi::GetGameTime());
	m_clock->SetText(time);

	if ((!tempMsgAge) || ((tempMsgAge && (Pi::GetGameTime() - tempMsgAge > 5.0)))) {
		if (m_msgQueue.empty()) {
			if (tempMsgAge) {
				// current message expired and queue empty
				tempMsg->SetText("");
				tempMsgAge = 0;
			}
		} else {
			// current message expired and more in queue
			QueuedMsg m = m_msgQueue.front();
			m_msgQueue.pop_front();
			if (m.sender == "") {
				tempMsg->SetText("#0f0"+m.message);
			} else {
				tempMsg->SetText(stringf(1024, "#ca0Message from %s:\n%s", m.sender.c_str(), m.message.c_str()));
			}
			tempMsgAge = (float)Pi::GetGameTime();
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
	if (Pi::GetView() == Pi::infoView) {
		Pi::infoView->NextPage();
	} else {
		Pi::infoView->UpdateInfo();
		Pi::SetView(Pi::infoView);
	}
}

void ShipCpanel::OnChangeMapView(Gui::MultiStateImageButton *b)
{
	Pi::SetMapView((enum Pi::MapView)b->GetState());
}

void ShipCpanel::OnClickTimeaccel(Gui::ISelectable *i, double step)
{
	Pi::SetTimeAccel((float)step);
}

void ShipCpanel::OnClickComms(Gui::MultiStateImageButton *b)
{
	if (Pi::player->GetDockedWith()) Pi::SetView(Pi::spaceStationView);
	else {
		Pi::SetView(Pi::worldView);
		Pi::worldView->ToggleTargetActions();
	}
}

