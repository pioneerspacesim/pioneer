#include "libs.h"
#include "Pi.h"
#include "ShipCpanel.h"
#include "SpaceStationView.h"
#include "Player.h"
#include "InfoView.h"
#include "WorldView.h"
#include "Space.h"
#include "SpaceStation.h"

#define SCALE	0.01f
#define YSHRINK 0.75f
class ScannerWidget: public Gui::Widget {
public:
	void GetSizeRequested(float size[2]) {
		size[0] = 400;
		size[1] = 62;
	}

	void Draw() {
		float size[2];
		GetSize(size);
		const float mx = size[0]*0.5f;
		const float my = size[1]*0.5f;
		float c2p[2];
		Widget::SetClipping(size[0], size[1]);
		Gui::Screen::GetCoords2Pixels(c2p);
		glPushAttrib(GL_COLOR_BUFFER_BIT | GL_POINT_BIT | GL_LINE_BIT);
		
		// draw objects below player (and below scanner)
		DrawBlobs(true);
		/* disc */
		glEnable(GL_BLEND);
		glColor4f(0,1,0,0.1);
		glBegin(GL_TRIANGLE_FAN);
		glVertex2f(mx, my);
		for (float a=0; a<2*M_PI; a+=M_PI*0.02) {
			glVertex2f(mx + mx*sin(a), my + YSHRINK*my*cos(a));
		}
		glVertex2f(mx, my + YSHRINK*my);
		glEnd();
		glDisable(GL_BLEND);
		
		glLineWidth(1);
		glColor3f(0,.4,0);
		DrawDistanceRings();
		glPushMatrix();
		glEnable(GL_BLEND);
		glColor4f(0,.4,0,0.25);
		glTranslatef(0.5f*c2p[0],0.5f*c2p[1],0);
		DrawDistanceRings();
		glTranslatef(0,-c2p[1],0);
		DrawDistanceRings();
		glTranslatef(-c2p[0],0,0);
		DrawDistanceRings();
		glTranslatef(0,c2p[1],0);
		DrawDistanceRings();
		glPopMatrix();
		glDisable(GL_BLEND);
		DrawBlobs(false);
		glPopAttrib();
		Widget::EndClipping();
	}
private:
	void DrawBlobs(bool below) {
		float size[2];
		GetSize(size);
		float mx = size[0]*0.5f;
		float my = size[1]*0.5f;
		glLineWidth(2);
		glPointSize(4);
		for (Space::bodiesIter_t i = Space::bodies.begin(); i != Space::bodies.end(); ++i) {
			if ((*i) == Pi::player) continue;
			if ((!(*i)->IsType(Object::SHIP)) &&
			    (!(*i)->IsType(Object::CARGOBODY))) continue;
			switch ((*i)->GetType()) {
				case Object::SHIP: glColor3f(1,0,0); break;
				case Object::CARGOBODY:
					glColor3f(.5,.5,1); break;
				default: continue;
			}
			if ((*i)->GetFrame() == Pi::player->GetFrame()) {
				vector3d pos = (*i)->GetPosition() - Pi::player->GetPosition();
				matrix4x4d rot;
				Pi::player->GetRotMatrix(rot);
				pos = rot.InverseOf() * pos;

				if ((pos.y>0)&&(below)) continue;
				if ((pos.y<0)&&(!below)) continue;

				glBegin(GL_LINES);
				glVertex2f(mx + (float)pos.x*SCALE, my + YSHRINK*(float)pos.z*SCALE);
				glVertex2f(mx + (float)pos.x*SCALE, my + YSHRINK*(float)pos.z*SCALE - YSHRINK*(float)pos.y*SCALE);
				glEnd();
				
				glBegin(GL_POINTS);
				glVertex2f(mx + (float)pos.x*SCALE, my + YSHRINK*(float)pos.z*SCALE - YSHRINK*(float)pos.y*SCALE);
				glEnd();
			}
		}
	}
	void DrawDistanceRings() {
		float size[2];
		GetSize(size);
		float mx = size[0]*0.5f;
		float my = size[1]*0.5f;

		/* soicles */
		for (float sz=1.0f; sz>0.1f; sz-=0.33f) {
			glBegin(GL_LINE_LOOP);
			for (float a=0; a<2*M_PI; a+=(float)(M_PI*0.02)) {
				glVertex2f(mx + sz*mx*sin(a), my + YSHRINK*sz*my*cos(a));
			}
			glEnd();
		}
		/* schpokes */
		glBegin(GL_LINES);
		for (float a=0; a<2*M_PI; a+=(float)(M_PI*0.25)) {
			glVertex2f(mx, my);
			glVertex2f(mx + mx*sin(a), my + YSHRINK*my*cos(a));
		}
		glEnd();

	}
};

ShipCpanel::ShipCpanel(): Gui::Fixed((float)Gui::Screen::GetWidth(), 64)
{
	Gui::Screen::AddBaseWidget(this, 0, Gui::Screen::GetHeight()-64);
	SetTransparency(true);

	Gui::Image *img = new Gui::Image("icons/cpanel.png");
	Add(img, 0, 0);

	Add(new ScannerWidget(), 200, 2);

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
	Add(m_clock, 4, 1);

	tempMsgAge = 0;
	tempMsg = new Gui::Label("");
	Add(tempMsg, 170, 4);

	m_connOnDockingClearanceExpired =
		Pi::onDockingClearanceExpired.connect(sigc::mem_fun(this, &ShipCpanel::OnDockingClearanceExpired));
}

ShipCpanel::~ShipCpanel()
{
	m_connOnDockingClearanceExpired.disconnect();
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
}

