#include "SpaceStation.h"
#include "SpaceStationView.h"
#include "Pi.h"
#include "Player.h"
#include "WorldView.h"

#define TEXSIZE	128
#define ADD_VIDEO_WIDGET	Add(new DeadVideoLink(295,285), 5, 40)

class DeadVideoLink: public Gui::Widget {
public:
	void PutRandomCrapIntoTexture() {
		int *randcrap = (int*)alloca(TEXSIZE*TEXSIZE);
		for (unsigned int i=0; i<TEXSIZE*TEXSIZE/sizeof(int); i++) randcrap[i] = (Pi::rng.Int32() & 0xfcfcfcfc) >> 2;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, TEXSIZE, TEXSIZE, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, randcrap);
	}
	DeadVideoLink(float w, float h) {
		m_w = w; m_h = h;
		m_created = SDL_GetTicks();
		m_message = new Gui::ToolTip("Video link down");
		glEnable (GL_TEXTURE_2D);
		glGenTextures (1, &m_tex);
		glBindTexture (GL_TEXTURE_2D, m_tex);
		PutRandomCrapIntoTexture();
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glDisable (GL_TEXTURE_2D);
	}
	virtual ~DeadVideoLink() {
		glDeleteTextures(1, &m_tex);
		delete m_message;
	}
	virtual void Draw() {
		float size[2]; GetSize(size);
		if (SDL_GetTicks() - m_created < 1500) {
			m_message->SetText("Connecting...");
			glBegin(GL_QUADS);
				glColor3f(0,0,0);
				glVertex2f(0,0);
				glVertex2f(0,size[1]);
				glVertex2f(size[0],size[1]);
				glVertex2f(size[0],0);
			glEnd();
			DrawMessage();
		} else {
			m_message->SetText("Video link down");

			glEnable (GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, m_tex);
			PutRandomCrapIntoTexture();
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			glBegin(GL_QUADS);
				glColor3f(0,0,0);
				glTexCoord2f(0,0);
				glVertex2f(0,0);
				glTexCoord2f(0,1);
				glVertex2f(0,size[1]);
				glTexCoord2f(1,1);
				glVertex2f(size[0],size[1]);
				glTexCoord2f(1,0);
				glVertex2f(size[0],0);
			glEnd();
			glDisable (GL_TEXTURE_2D);
			if (SDL_GetTicks() & 0x400) {
				DrawMessage();
			}
		}
	}
	virtual void GetSizeRequested(float size[2]) {
		size[0] = m_w;
		size[1] = m_h;
	}
private:
	void DrawMessage() {
		float size[2];
		float msgSize[2];
		GetSize(size);
		m_message->GetSize(msgSize);
		glPushMatrix();
		glTranslatef(size[0]*0.5-msgSize[0]*0.5, size[1]*0.5-msgSize[1]*0.5, 0);
		m_message->Draw();
		glPopMatrix();
	}
	Uint32 m_created;
	GLuint m_tex;
	float m_w, m_h;
	Gui::ToolTip *m_message;
};

class StationSubView: public Gui::Fixed {
public:
	StationSubView(SpaceStationView *parent): Gui::Fixed(Gui::Screen::GetWidth(), Gui::Screen::GetHeight()-64) {
		m_parent = parent;
	}
protected:
	SpaceStationView *m_parent;
};

/////////////////////////////////////////////////////////////////////////////

class StationFrontView: public StationSubView {
public:
	StationFrontView(SpaceStationView *parent);
	virtual void ShowAll();
private:
	void OnClickRequestLaunch()
	{
		Pi::player->SetDockedWith(0,0);
		Pi::SetView(Pi::worldView);
	}

	void OnClickGotoShipYard() {
		m_parent->GotoShipyard();
	}

	void OnClickGotoCommodities() {
		m_parent->GotoCommodities();
	}
};

StationFrontView::StationFrontView(SpaceStationView *parent): StationSubView(parent)
{
	SetTransparency(false);
}

void StationFrontView::ShowAll()
{
	DeleteAllChildren();
	SpaceStation *station = Pi::player->GetDockedWith();
	if (!station) return;
	{
		char buf[256];
		snprintf(buf, sizeof(buf), "Welcome to %s", station->GetLabel().c_str());
		Add(new Gui::Label(buf), 10, 10);
	}
	Gui::Label *l = new Gui::Label("Hello friend! Thankyou for docking with this space station!\n"
	"You may have noticed that the docking procedure was not entirely "
	"physically correct. This is a result of unimplemented physics in this "
	"region of the galaxy. We hope to have things back to normal within a "
	"few weeks, and in the mean time would like to offer our apologies for "
	"any loss of earnings, immersion or lunch.  "
	"Currently the usual space station services are not available, but we "
	"can offer you this promotional message from one of the station's sponsors:\n"
	"                       DIET STEAKETTE: IT'S BAD");

	Gui::Fixed *fbox = new Gui::Fixed(450, 400);
	fbox->Add(l, 0, 0);
	Add(fbox, 320, 40);
	fbox->ShowAll();

	Gui::SolidButton *b = new Gui::SolidButton();
	b->SetShortcut(SDLK_1, KMOD_NONE);
	b->onClick.connect(sigc::mem_fun(this, &StationFrontView::OnClickRequestLaunch));
	Add(b, 340, 240);
	l = new Gui::Label("Request Launch");
	Add(l, 365, 240);

	b = new Gui::SolidButton();
	b->SetShortcut(SDLK_2, KMOD_NONE);
	b->onClick.connect(sigc::mem_fun(this, &StationFrontView::OnClickGotoShipYard));
	Add(b, 340, 300);
	l = new Gui::Label("Shipyard");
	Add(l, 365, 300);

	b = new Gui::SolidButton();
	b->SetShortcut(SDLK_3, KMOD_NONE);
	b->onClick.connect(sigc::mem_fun(this, &StationFrontView::OnClickGotoCommodities));
	Add(b, 340, 360);
	l = new Gui::Label("Commodity market");
	Add(l, 365, 360);

	ADD_VIDEO_WIDGET;

	Gui::Fixed::ShowAll();
}

#include <map>
////////////////////////////////////////////////////////////////////

class StationCommoditiesView: public StationSubView {
public:
	StationCommoditiesView(SpaceStationView *parent);
private:
	virtual void ShowAll();
	void OnClickBuy(int commodity_type) {
		m_station->SellItemTo(Pi::player, (Equip::Type)commodity_type);
		UpdateStock(commodity_type);
		UpdateStats();
	}
	void OnClickSell(int commodity_type) {
		Pi::player->SellItemTo(m_station, (Equip::Type)commodity_type);
		UpdateStock(commodity_type);
		UpdateStats();
	}
	void UpdateStock(int commodity_type) {
		char buf[128];
		snprintf(buf, sizeof(buf), "%dt", Pi::player->m_equipment.Count(Equip::SLOT_CARGO, static_cast<Equip::Type>(commodity_type))*EquipType::types[commodity_type].mass);
		m_cargoLabels[commodity_type]->SetText(buf);
		
		snprintf(buf, sizeof(buf), "%dt", m_station->GetEquipmentStock(static_cast<Equip::Type>(commodity_type))*EquipType::types[commodity_type].mass);
		m_stockLabels[commodity_type]->SetText(buf);
	}
	void UpdateStats() {
		char buf[128];
		snprintf(buf, sizeof(buf), "Credits: $%lld", Pi::player->GetMoney());
		m_money->SetText(buf);
	}
	std::map<int, Gui::Label*> m_stockLabels;
	std::map<int, Gui::Label*> m_cargoLabels;
	Gui::Label *m_money;
	SpaceStation *m_station;
};

StationCommoditiesView::StationCommoditiesView(SpaceStationView *parent): StationSubView(parent)
{
	SetTransparency(false);
}

void StationCommoditiesView::ShowAll()
{
	DeleteAllChildren();
	m_stockLabels.clear();
	m_cargoLabels.clear();

	m_station = Pi::player->GetDockedWith();
	assert(m_station);
	SetTransparency(false);
	{
		char buf[256];
		snprintf(buf, sizeof(buf), "Welcome to %s commodities market", m_station->GetLabel().c_str());
		Add(new Gui::Label(buf), 10, 10);
	}

	m_money = new Gui::Label("");
	Add(m_money, 10, 450);
	
	Gui::Fixed *fbox = new Gui::Fixed(470, 400);
	Add(fbox, 320, 40);

	Gui::VScrollBar *scroll = new Gui::VScrollBar();
	Gui::VScrollPortal *portal = new Gui::VScrollPortal(450,400);
	scroll->SetAdjustment(&portal->vscrollAdjust);
	//int GetEquipmentStock(Equip::Type t) const { return m_equipmentStock[t]; }

	int NUM_ITEMS = 0;
	const float YSEP = Gui::Screen::GetFontHeight() * 1.5;
	for (int i=1; i<Equip::TYPE_MAX; i++) {
		if (EquipType::types[i].slot == Equip::SLOT_CARGO) NUM_ITEMS++;
	}

	Gui::Fixed *innerbox = new Gui::Fixed(450, NUM_ITEMS*YSEP);
	for (int i=1, num=0; i<Equip::TYPE_MAX; i++) {
		if (EquipType::types[i].slot != Equip::SLOT_CARGO) continue;
		int stock = m_station->GetEquipmentStock(static_cast<Equip::Type>(i));
		Gui::Label *l = new Gui::Label(EquipType::types[i].name);
		innerbox->Add(l,0,num*YSEP);
		Gui::Button *b = new Gui::SolidButton();
		b->onClick.connect(sigc::bind(sigc::mem_fun(this, &StationCommoditiesView::OnClickBuy), i));
		innerbox->Add(b, 380, num*YSEP);
		b = new Gui::SolidButton();
		b->onClick.connect(sigc::bind(sigc::mem_fun(this, &StationCommoditiesView::OnClickSell), i));
		innerbox->Add(b, 415, num*YSEP);
		char buf[128];
		snprintf(buf, sizeof(buf), "$%d", m_station->GetPrice(static_cast<Equip::Type>(i)));
		innerbox->Add(new Gui::Label(buf), 200, num*YSEP);
		
		snprintf(buf, sizeof(buf), "%dt", stock*EquipType::types[i].mass);
		Gui::Label *stocklabel = new Gui::Label(buf);
		m_stockLabels[i] = stocklabel;
		innerbox->Add(stocklabel, 275, num*YSEP);
		
		snprintf(buf, sizeof(buf), "%dt", Pi::player->m_equipment.Count(Equip::SLOT_CARGO, static_cast<Equip::Type>(i))*EquipType::types[i].mass);
		Gui::Label *cargolabel = new Gui::Label(buf);
		m_cargoLabels[i] = cargolabel;
		innerbox->Add(cargolabel, 325, num*YSEP);
		num++;
	}
	innerbox->ShowAll();

	fbox->Add(new Gui::Label("Item"), 0, 0);
	fbox->Add(new Gui::Label("Price"), 200, 0);
	fbox->Add(new Gui::Label("Buy"), 380, 0);
	fbox->Add(new Gui::Label("Sell"), 415, 0);
	fbox->Add(new Gui::Label("Stock"), 275, 0);
	fbox->Add(new Gui::Label("Cargo"), 325, 0);
	fbox->Add(portal, 0, YSEP);
	fbox->Add(scroll, 455, YSEP);
	portal->Add(innerbox);
	portal->ShowAll();
	fbox->ShowAll();
	UpdateStats();
	ADD_VIDEO_WIDGET;

	Gui::Fixed::ShowAll();
}

////////////////////////////////////////////////////////////////////

class StationShipyardView: public StationSubView {
public:
	StationShipyardView(SpaceStationView *parent);
private:
	virtual void ShowAll();
};

StationShipyardView::StationShipyardView(SpaceStationView *parent): StationSubView(parent)
{
	SetTransparency(false);
}

void StationShipyardView::ShowAll()
{
	DeleteAllChildren();

	SpaceStation *station = Pi::player->GetDockedWith();
	assert(station);
	SetTransparency(false);
	{
		char buf[256];
		snprintf(buf, sizeof(buf), "Welcome to %s shipyard", station->GetLabel().c_str());
		Add(new Gui::Label(buf), 10, 10);
	}
	
	Gui::Fixed *fbox = new Gui::Fixed(470, 200);
	Add(fbox, 320, 40);

	Gui::VScrollBar *scroll = new Gui::VScrollBar();
	Gui::VScrollPortal *portal = new Gui::VScrollPortal(450,200);
	scroll->SetAdjustment(&portal->vscrollAdjust);
	//int GetEquipmentStock(Equip::Type t) const { return m_equipmentStock[t]; }

	int NUM_ITEMS = 0;
	const float YSEP = Gui::Screen::GetFontHeight() * 1.5;
	for (int i=1; i<Equip::TYPE_MAX; i++) {
		if (station->GetEquipmentStock(static_cast<Equip::Type>(i))) NUM_ITEMS++;
	}

	Gui::Fixed *innerbox = new Gui::Fixed(400, NUM_ITEMS*YSEP);
	for (int i=1, num=0; i<Equip::TYPE_MAX; i++) {
		int stock = station->GetEquipmentStock(static_cast<Equip::Type>(i));
		if (!stock) continue;
		Gui::Label *l = new Gui::Label(EquipType::types[i].name);
		innerbox->Add(l,0,num*YSEP);
		innerbox->Add(new Gui::SolidButton(), 275, num*YSEP);
		innerbox->Add(new Gui::SolidButton(), 300, num*YSEP);
		char buf[128];
		snprintf(buf, sizeof(buf), "$%d", station->GetPrice(static_cast<Equip::Type>(i)));
		innerbox->Add(new Gui::Label(buf), 200, num*YSEP);
		snprintf(buf, sizeof(buf), "%dt", EquipType::types[i].mass);
		innerbox->Add(new Gui::Label(buf), 370, num*YSEP);
		num++;
	}
	innerbox->ShowAll();

	fbox->Add(new Gui::Label("Item"), 0, 0);
	fbox->Add(new Gui::Label("Price"), 200, 0);
	fbox->Add(new Gui::Label("Fit"), 275, 0);
	fbox->Add(new Gui::Label("Remove"), 300, 0);
	fbox->Add(new Gui::Label("Wt"), 370, 0);
	fbox->Add(portal, 0, YSEP);
	fbox->Add(scroll, 455, YSEP);
	portal->Add(innerbox);
	portal->ShowAll();
	fbox->ShowAll();
	ADD_VIDEO_WIDGET;

	Gui::Fixed::ShowAll();
}

/////////////////////////////////////////////////////////////////////

SpaceStationView::SpaceStationView(): View()
{
	m_frontview = new StationFrontView(this);
	m_shipyard = new StationShipyardView(this);
	m_commodities = new StationCommoditiesView(this);
	m_subview = 0;
	SwitchView(m_frontview);

	Gui::Label *l = new Gui::Label("Comms Link");
	l->SetColor(1,.7,0);
	m_rightRegion2->Add(l, 10, 0);
}

void SpaceStationView::SwitchView(StationSubView *v)
{
	if (m_subview) {
		m_subview->HideAll();
		Remove(m_subview);
	}
	m_subview = v;
	Add(m_subview, 0, 0);
	m_subview->ShowAll();
}

void SpaceStationView::GotoCommodities()
{
	SwitchView(m_commodities);
}

void SpaceStationView::GotoShipyard()
{
	SwitchView(m_shipyard);
}

void SpaceStationView::OnSwitchTo()
{
	SwitchView(m_frontview);
}

void SpaceStationView::Draw3D()
{
}

void SpaceStationView::Update()
{
}
